/***************************************************************************************
	GameTrack.cpp

	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved 

	Description: Game tracking stuff.

	History:   
		02/01/2006	Rui Jiang	Created
***************************************************************************************/
#include "WT/StdAfx.h"
#include <windows.h>
#include <time.h>
#include <accctrl.h>
#include <aclapi.h>
#include "shlobj.h"

#include "WT/System/Logging/Log.h"
#include "WT/System/Net/Net.h"
#include "WT/System/Util/WinUtil.h"
#include "WT/System/Util/wtCDABindingSingleHeader.h"
#include "WT/GameTrack/GameTrack.h"

#pragma comment( lib, "Ws2_32" )
//warning C4127: conditional expression is constant
#pragma warning(disable: 4127)

using namespace WT;
using namespace System;
using namespace Net;
using namespace Util;

using namespace std;

namespace WT
{
namespace Tracking
{

///////////////////////////////////////////////////////////////////////////////
// CRC verification. Copied from 7zip project.
class CCRC
{
	DWORD _value;
public:
	static DWORD Table[256];
	static bool g_TableInited;
	static const DWORD kCRCPoly = 0xEDB88320;
	//Init CRC table if it isn't inited.
	static void InitTable();

	CCRC():  _value(0xFFFFFFFF){};
	void Init() { _value = 0xFFFFFFFF; }
	void UpdateByte(BYTE b)
	{  
		_value = Table[((BYTE)(_value)) ^ b] ^ (_value >> 8);
	}
	void UpdateUInt16(WORD v)
	{
		UpdateByte(BYTE(v));
		UpdateByte(BYTE(v >> 8));
	}
	void UpdateUInt32(DWORD v)
	{
		for (int i = 0; i < 4; i++)
			UpdateByte((BYTE)(v >> (8 * i)));
	}

	void UpdateUInt64(const __int64 &v)
	{
		for (int i = 0; i < 8; i++)
			UpdateByte((BYTE)(v >> (8 * i)));
	}
	void Update(const void *data, DWORD size)
	{
		DWORD v = _value;
		const BYTE *p = (const BYTE *)data;
		for (; size > 0 ; size--, p++)
			v = Table[((BYTE)(v)) ^ *p] ^ (v >> 8);
		_value = v;
	}

	DWORD GetDigest() const { return _value ^ 0xFFFFFFFF; } 
	static DWORD CalculateDigest(const void *data, DWORD size)
	{
		CCRC crc;
		crc.Update(data, size);
		return crc.GetDigest();
	}
	static bool VerifyDigest(DWORD digest, const void *data, DWORD size)
	{
		return (CalculateDigest(data, size) == digest);
	}
};

DWORD CCRC::Table[256];
bool CCRC::g_TableInited = false;

void CCRC::InitTable()
{
	if (g_TableInited) return;
	for (DWORD i = 0; i < 256; i++)
	{
		DWORD r = i;
		for (int j = 0; j < 8; j++)
			if (r & 1) 
				r = (r >> 1) ^ kCRCPoly;
			else     
				r >>= 1;
		CCRC::Table[i] = r;
	}
	g_TableInited = true;
}

// Get the difference between local and UTC
// This actually returns the negative of the value we would expect because the
// original ISA system did this.
DWORD GetLocalTimeOffset()
{
	SYSTEMTIME UTCSystemTime;
	FILETIME UTCFileTime, LocalFileTime;
	GetSystemTime ( &UTCSystemTime );
	SystemTimeToFileTime ( &UTCSystemTime , &UTCFileTime );
	FileTimeToLocalFileTime ( &UTCFileTime , &LocalFileTime );

	__int64 utc = *(__int64*)&UTCFileTime;
	__int64 local = *(__int64*)&LocalFileTime;
	return DWORD((utc - local) / 10000000 / 60);
}

// Returns the current date and time formatted as mm-dd-yy HH:MM:SS
std::string GetLocalTimeString()
{
	SYSTEMTIME UTCSystemTime, lst;
	FILETIME UTCFileTime, lt;
	GetSystemTime ( &UTCSystemTime );
	SystemTimeToFileTime ( &UTCSystemTime , &UTCFileTime );
	FileTimeToLocalFileTime ( &UTCFileTime , &lt);
	FileTimeToSystemTime(&lt, &lst);

	char buf[200];
	sprintf(buf, "%02d-%02d-%02d %02d:%02d:%02d", lst.wMonth, lst.wDay, lst.wYear % 100, lst.wHour, lst.wMinute, lst.wSecond);
	return buf;
}

struct CLogFileHeader
{
	DWORD Version;
	//How many item do we have?
	DWORD ItemCount;
	//Offset of the first non-deleted item.
	DWORD StartOffset;
	DWORD NextItemId;
	//CRC must be the last member!!!
	DWORD CRC;
};

struct CItemHeader
{
	//Total size of the item, including this header and record.
	DWORD	ItemSize;
	//Size of this header. Future version might add more stuff. Data starts after header.
	DWORD	HeaderSize;
	//This CRC is only for the actual data following the ItemHeader.
	DWORD	CRC;
	DWORD	Id;
	//Which char did we use to xor the actual data.
	BYTE	XORChar;
};


///////////////////////////////////////////////////////////////////////////////
// CGameTrack

CGameTrack::CGameTrack(void)
{
	m_FileMutex = NULL;
	m_UploadMutex = NULL;
	m_IsUploading = false;
	m_hThread = NULL;
	InitializeCriticalSection(&m_ThreadHandleCs);
	m_hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_XorChar = 0;
}

CGameTrack::~CGameTrack(void)
{
	StopUpload(10);
	if (m_FileMutex) CloseHandle(m_FileMutex);
	if (m_UploadMutex) CloseHandle(m_UploadMutex);
	CloseHandle(m_hStopEvent);
	//Must be after stop upload.
	DeleteCriticalSection(&m_ThreadHandleCs);
}

//Mandatory break uploading. May terminate the thread.
void CGameTrack::StopUpload(DWORD dwTimeOut)
{
	if (m_hThread == NULL) return;
	//Trigger stop event.
	SetEvent(m_hStopEvent);

	DWORD Start = ::GetTickCount();
	while(GetTickCount() - Start < dwTimeOut)
	{
		//It has quit beautifully.
		if (m_hThread == NULL) return;
		Sleep(1);
	}

	//Have to manually kill it.
	EnterCriticalSection(&m_ThreadHandleCs);
	if (m_hThread)
	{
		LOG("Thread didn't finish in %d ms. Have to manually kill the thread.", dwTimeOut);
		TerminateThread(m_hThread, 1);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	m_IsUploading = false;
	LeaveCriticalSection(&m_ThreadHandleCs);
}


std::string CGameTrack::GetSupportID()
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	GUID guid;

	if (m_SupportId.size()) return m_SupportId;
	//First try to open the supportid file. 
	bool readinGuid = false;
	string FullPath = (GetStorePath() + "\\" + GAMETRACK_SUPPORTIDFILENAME);
	hFile = CreateFile(FullPath.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwRead = 0;
		if (ReadFile(hFile, &guid, sizeof(guid), &dwRead, NULL) && dwRead == sizeof(guid))
		{
			readinGuid = true;
		}
		CloseHandle(hFile);
	}
	
	//If we failed to read in the guid.
	if (!readinGuid)
	{
		LOG("Failed to read in support id from file");
		//Get it from old CDA
		char cdaGuid[MAX_PATH];
		memset(cdaGuid, 0, sizeof(cdaGuid));
		bool loadedGuidFromCDA = false;
		try
		{
			if (cdaEngineInit())
			{
				if (cdaGetConfigAsString ("UserSupport", cdaGuid, MAX_PATH, ""))
				{
					LOG("Get supportid from CDA: %s", cdaGuid);
					string newGuid = string("{") + cdaGuid + "}";
					if (SUCCEEDED(CLSIDFromString((LPOLESTR)AnsiToWide(newGuid).c_str(), &guid)))
					{
						loadedGuidFromCDA = true;
					}
				}
				cdaEngineFree();
			}
		}
		catch(...)
		{
		}
		if (!loadedGuidFromCDA)
		{
			CoCreateGuid(&guid);
		}
		//We now need to write it into the file.
		LOG("Writing new id to file: %s", FullPath.c_str());
		HANDLE hFile = CreateFileForEveryOne(FullPath.c_str(), 
			GENERIC_WRITE | GENERIC_READ, 
			FILE_SHARE_READ|FILE_SHARE_WRITE, 
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			//If it fails we can do nothing. So let it be.
			DWORD dw;
			WriteFile(hFile, &guid, sizeof(guid), &dw, NULL);
			CloseHandle(hFile);
		}
	}

	//OK, now we got the guid.
	LPOLESTR str = NULL;
	StringFromCLSID(guid, &str);
	if (str == NULL)
	{
		//it couldn't happen, really.
		m_SupportId = "a90c129d-cded-4eae-adc9-c52c22cea9cd";
	}
	else
	{
		m_SupportId = WideToAnsi(str);
		//When uploading, there is no {}. Legacy issue.
		if (m_SupportId.size() && m_SupportId[0] == '{') 
			m_SupportId.erase(m_SupportId.begin());
		if (m_SupportId.size() && m_SupportId[m_SupportId.size() - 1] == '}') 
			m_SupportId.erase(m_SupportId.begin() + m_SupportId.size() - 1);
		CoTaskMemFree(str);
	}
	return m_SupportId;
}

bool CGameTrack::Init(const std::string& FileName)
{
	CCRC::InitTable();
	string MutexName = FileName + "_wtGmTk8BC3";
	m_FileMutex = CreateMutexA(NULL, FALSE, MutexName.c_str());
	if (m_FileMutex == NULL)
	{
		LOG("Failed to create mutex. Name: %s", MutexName.c_str());
		return false;
	}
	string UploadMutexName = MutexName + "_uld";
	m_UploadMutex = CreateMutexA(NULL, FALSE, UploadMutexName.c_str());
	if (m_UploadMutex == NULL)
	{
		LOG("Failed to create upload mutex: %s", UploadMutexName.c_str());
		return false;
	}
	string Path = GetStorePath();
	m_TrackFilePathName = Path + "\\" + FileName;
	LOG("Track file name: %s", m_TrackFilePathName.c_str());
	if (!CreateDirRecursive(Path))
	{
		LOG("Failed to create store path.");
		return false;
	}
	return true;
}


void CGameTrack::Lock()
{
	WaitForSingleObject(m_FileMutex, INFINITE);
}

void CGameTrack::Unlock()
{
	ReleaseMutex(m_FileMutex);
}

std::string CGameTrack::GetStorePath()
{
	return WT::System::Util::GetCommonAppDataPath() + "\\WildTangent\\GameData";
}


//Return the number of bytes of the current Un-uploaded logs.
DWORD CGameTrack::GetLogBytes()
{
	DWORD dwRes = 0;
	Lock();
	HANDLE hFile = INVALID_HANDLE_VALUE;
	CLogFileHeader Header;
	do
	{
		hFile = OpenLogFile(&Header);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			LOG("ERROR: Failed to open/create file: %s", m_TrackFilePathName.c_str());
			break;
		}
		dwRes = GetFileSize(hFile, NULL) - Header.StartOffset;
	} while(false);
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

	Unlock();

	return dwRes;
}

// Queues or uploads game tracking data
bool CGameTrack::Log(const std::string& LoggerID, const std::string& Sku, const std::string& Key, const std::string& Value)
{
	bool res = false;
	Lock();

	HANDLE hFile = INVALID_HANDLE_VALUE;
	CLogFileHeader Header;
	do
	{
		hFile = OpenLogFile(&Header);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			LOG("ERROR: Failed to create file: %s", m_TrackFilePathName.c_str());
			break;
		}
		DWORD dwCurrentFileSize = GetFileSize(hFile, NULL);
		if (dwCurrentFileSize > MAX_GAMETRACK_FILESIZE)
		{
			LOG("ERROR: Log file is too big: %s", dwCurrentFileSize);
			break;
		}
		SetFilePointer(hFile, 0, NULL, FILE_END);

		vector<BYTE> data;
		string EntryContent = FormatLogEntry(LoggerID, Sku, Key, Value);
		ComposeLogItem(EntryContent, Header.NextItemId, data);
		DWORD cbBytesWritten = 0;
		if (!WriteFile(hFile, &data[0], (DWORD)data.size(), &cbBytesWritten, NULL) || cbBytesWritten != data.size())
		{
			LOG("ERROR: Failed to append record.");
			break;
		}

		//Update header
		Header.ItemCount ++;
		Header.NextItemId ++;
		if (!UpdateFileHeader(hFile, &Header))
		{
			LOG("ERROR: Failed to rewrite header.");
			break;
		}
		res = true;
	} while(false);

	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

	Unlock();
	return res;
}

//Open/create the file and validate the header.
HANDLE CGameTrack::OpenLogFile(CLogFileHeader* pHeader)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	hFile = CreateFile(m_TrackFilePathName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		if (::GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			SECURITY_ATTRIBUTES sa;
			PACL acl = NULL;
			memset(&sa, 0, sizeof(sa));
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.bInheritHandle = false;
			sa.lpSecurityDescriptor = CreateSecurityDescriptorForUser(&acl, "EVERYONE", NO_INHERITANCE, 
				SPECIFIC_RIGHTS_ALL | STANDARD_RIGHTS_ALL);
			hFile = CreateFile(m_TrackFilePathName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 
				sa.lpSecurityDescriptor? &sa : NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (sa.lpSecurityDescriptor)
			{
				LocalFree(sa.lpSecurityDescriptor);
			}
			if (acl) LocalFree(acl);
		}
	}
	if (hFile == INVALID_HANDLE_VALUE)
	{
		LOG("Failed to create file. Last Error 0x%08x: %s", GetLastError(), m_TrackFilePathName.c_str());
		return hFile;
	}
	if (!ReadAndVerifyHeader(hFile, pHeader))
	{
		LOG("Failed to read header");
		ResetFileHeader(hFile, pHeader);
	}
	return hFile;
}


bool CGameTrack::GetRecordsForUpload(DWORD MaxCount, LPDWORD lpRecordsRead, LPDWORD lpNewOffset, std::string& Records)
{
	bool res = false;
	Lock();

	HANDLE hFile = INVALID_HANDLE_VALUE;
	CLogFileHeader Header;
	Records.clear();
	DWORD Count = 0;
	do
	{
		hFile = OpenLogFile(&Header);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			LOG("Failed to open/create file: %s", m_TrackFilePathName.c_str());
			break;
		}
		if (SetFilePointer(hFile, Header.StartOffset, NULL, FILE_BEGIN) != Header.StartOffset) break;
		DWORD CurrentOffset = Header.StartOffset;
		bool BadRecords = true;
		//Now, let's read in records one by one and validate them.
		while(Count < MaxCount)
		{
			DWORD ItemSize = 0;
			DWORD	cbRead;

			//assume break is because of bad records.
			BadRecords = true;

			if (!ReadFile(hFile, &ItemSize, sizeof(DWORD), &cbRead, NULL)) break;
			//EOF
			if (cbRead == 0) { BadRecords = false; break;}
			if (cbRead != sizeof(DWORD)) break;
			
			//The record size can't be so big. ~1 M
			if (ItemSize > 1000000) break;

			//Now, allocate buffer and read in the real record.
			vector<char> data;
			try
			{
				data.resize(ItemSize);
			}
			catch(...)
			{
				break;
			}
			if (data.size() != ItemSize) break;
			if (ItemSize < sizeof(DWORD)) break;

			CItemHeader * pHeader = (CItemHeader *) &data[0];
			pHeader->ItemSize = ItemSize;
			if (!ReadFile(hFile, &data[sizeof(DWORD)], ItemSize - sizeof(DWORD), &cbRead, NULL) 
				|| cbRead != ItemSize - sizeof(DWORD)) break;
			//Check if header is valid.
			if (pHeader->HeaderSize > ItemSize) break;

			for(DWORD i = pHeader->HeaderSize; i < ItemSize; i++) data[i] ^= pHeader->XORChar;
			//Before we already check that data size must >= 1
			//Check the NULL terminator.
			if (data[data.size() - 1] != 0) break;
			//Verify CRC
			if (CCRC::CalculateDigest(&data[pHeader->HeaderSize], pHeader->ItemSize - pHeader->HeaderSize) != pHeader->CRC) break;

			//OK, this record is good!. now add this record.
			Count++;
			BadRecords = false;
			Records += &data[pHeader->HeaderSize];
			//Update current offset.
			CurrentOffset += ItemSize;
			if (lpNewOffset) *lpNewOffset = CurrentOffset;
		}
		if (BadRecords) { break;}
		res = true;
	} while(false);

	if (hFile != INVALID_HANDLE_VALUE) 
	{
		if (!res) 
		{
			LOG("ERROR: happened during getting the records. Reset the file.");
			ResetFileHeader(hFile, &Header);
		}
		CloseHandle(hFile);
	}

	Unlock();
	
	if (!res)
	{
		Records.clear();
	}
	if (lpRecordsRead) *lpRecordsRead = res? Count : 0;
	return res;
}

/*
//After a successful uploading, we need to remove the records.
bool CGameTrack::DeleteRecords(std::vector<CLockItem>& Records)
{
	assert(Records.size() > 0);
	if (Records.size() == 0) return true;
	bool res = false;
	Lock();

	HANDLE hFile = INVALID_HANDLE_VALUE;
	CLogFileHeader Header;
	do
	{
		hFile = OpenLogFile(&Header);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			LOG("Failed to open/create file: %s", m_TrackFilePathName.c_str());
			break;
		}
		for(DWORD i = 0; i < Records.size(); i++)
		{
			CLockItem& item = Records[i];
			if (SetFilePointer(hFile, item.Offset, NULL, FILE_BEGIN) != item.Offset) continue;
			CItemHeader ItemHeader;
			DWORD cbRead = 0;
			if (!ReadFile(hFile, &ItemHeader, sizeof(CItemHeader), &cbRead, NULL) || cbRead != sizeof(CItemHeader)) continue;
			//Check if id match.
			if (ItemHeader.Id == item.Id)
			{
				ItemHeader.Flag = ItemFlag::Deleted;
			}
			SetFilePointer(hFile, item.Offset, NULL, FILE_BEGIN);
			WriteFile(hFile, &ItemHeader, sizeof(CItemHeader), &cbRead, NULL);
		}
		//Now need to update the head pointer.
		if (Header.StartOffset == Records[0].Offset)
		{

		}
		res = true;
	} while(false);

	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

	Unlock();
	return res;
}
*/

//Old games' code has a bug of appending a "0xd" at the end. Need to fix that.
//Following character checking code was copied from original logger code.
std::string FixLogField(const std::string& LogField)
{
	string s;
	for(DWORD i = 0; i < LogField.size(); i++)
	{
		char c = LogField[i];
		if ( ( c >= 'a' && c <= 'z' ) || ( c >='A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || c == '.' || c == ',' ||
			 c == '=' || c == '_' || c =='-' || c == '{' || c == '}' || c == ':' || c == '/' || c =='\\' )
			 s += c;
		else
		{
			LOG("ERROR: Invalid character in LogField: %s", LogField.c_str());
		}
	}
	return s;
}

std::string CGameTrack::FormatLogEntry(const std::string& LoggerID, const std::string& Sku, const std::string& Key, const std::string& Value)
{
	// Format the log entry
	char TimeOffset[100];
	sprintf(TimeOffset, "%d", GetLocalTimeOffset());

	std::string LogEntryString = "[record]|";
	LogEntryString += GetLocalTimeString();
	LogEntryString += "|";
	LogEntryString += TimeOffset;
	LogEntryString += "|";
	LogEntryString += FixLogField(LoggerID);
	LogEntryString += "|-|0|-|";	// No URL, flags = 0, no misc
	LogEntryString += FixLogField(Sku);
	LogEntryString += "|";
	LogEntryString += FixLogField(Key);
	LogEntryString += "|";
	LogEntryString += FixLogField(Value);
	LogEntryString += "|[/record]\r\n";

	return LogEntryString;
}

void CGameTrack::ComposeLogItem(const string& Entry, DWORD Id, std::vector<BYTE>& data)
{
	DWORD size = sizeof(CItemHeader) + (DWORD)Entry.size() + 1;
	data.resize(size);
	CItemHeader * pItem = (CItemHeader * )&data[0];
	memset(pItem, 0, sizeof(CItemHeader));
	pItem->HeaderSize = sizeof(CItemHeader);
	pItem->ItemSize = size;
	pItem->Id = Id;
	pItem->XORChar = m_XorChar;
	memcpy(&data[sizeof(CItemHeader)], Entry.c_str(), Entry.size()+1);
	//XOR the string..
	for(DWORD i = 0; i <= Entry.size(); i++)
	{
		data[sizeof(CItemHeader) + i] ^= pItem->XORChar;
	}
	pItem->CRC = CCRC::CalculateDigest(Entry.c_str(), (DWORD)Entry.size()+1);
}

bool CGameTrack::ReadAndVerifyHeader(HANDLE hFile, CLogFileHeader* pHeader)
{
	DWORD cbBytesRead = 0;
	if (!ReadFile(hFile, pHeader, sizeof(CLogFileHeader), &cbBytesRead, NULL)) return false;
	if (cbBytesRead != sizeof(CLogFileHeader)) return false;
	if (pHeader->Version != 1) return false;

	DWORD crc = CCRC::CalculateDigest((LPBYTE)pHeader, sizeof(CLogFileHeader) - sizeof(DWORD));
	if (crc != pHeader->CRC) return false;
	return true;
}

bool CGameTrack::ResetFileHeader(HANDLE hFile, CLogFileHeader* pHeader)
{
	memset(pHeader, 0, sizeof(CLogFileHeader));
	pHeader->ItemCount = 0;
	pHeader->Version = 1;
	pHeader->StartOffset = sizeof(CLogFileHeader);
	pHeader->NextItemId = rand();

	if (!UpdateFileHeader(hFile, pHeader)) return false;
	SetEndOfFile(hFile);
	return true;
}

bool CGameTrack::UpdateFileHeader(HANDLE hFile, CLogFileHeader* pHeader)
{
	pHeader->CRC = CCRC::CalculateDigest((LPBYTE)pHeader, sizeof(CLogFileHeader) - sizeof(DWORD));
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	DWORD cbBytesWritten = 0;
	if (!WriteFile(hFile, pHeader, sizeof(CLogFileHeader), &cbBytesWritten, NULL) || cbBytesWritten != sizeof(CLogFileHeader))
	{
		LOG("Failed to write header.");
		return false;
	}
	return true;
}

//After a successful uploading, we need to remove the records.
bool CGameTrack::DeleteRecords(DWORD dwNewOffset, DWORD DeletedItemCount)
{
	bool res = false;
	Lock();

	HANDLE hFile = INVALID_HANDLE_VALUE;
	CLogFileHeader Header;
	do
	{
		hFile = OpenLogFile(&Header);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			LOG("Failed to open/create file: %s", m_TrackFilePathName.c_str());
			break;
		}
		Header.ItemCount -= DeletedItemCount;
		//More than one mega bytes?
		if (dwNewOffset > MAX_GAMETRACK_CLEANUPSIZE)
		{
			DWORD dwSize = GetFileSize(hFile, NULL);
			if (dwSize > dwNewOffset)
			{
				//Read the old records.
				BYTE* buf = NULL;
				bool  readOK = false;
				DWORD dwRestRecordSize = dwSize - dwNewOffset;
				do
				{
					buf = new BYTE[dwRestRecordSize];
					if (buf == NULL) break;
					SetFilePointer(hFile, dwNewOffset, NULL, FILE_BEGIN);
					DWORD cbRead = 0;
					if (!ReadFile(hFile, buf, dwRestRecordSize, &cbRead, NULL))
					{
						LOG("ERROR: Failed to read in rest of the records.");
						break;
					}
					if (cbRead != dwRestRecordSize) 
					{
						LOG("ERROR: Read in size isn't correct.");
						break;
					}
					readOK  = true;
				}while(false);

				if (readOK)
				{
					assert(buf);
					Header.StartOffset = sizeof(CLogFileHeader);
					UpdateFileHeader(hFile, &Header);
					DWORD cbWritten;
					if (!WriteFile(hFile, buf, dwRestRecordSize, &cbWritten, NULL) || cbWritten != dwRestRecordSize)
					{
						LOG("ERROR: Failed to write the rest records.");
					}
					SetEndOfFile(hFile);
				}
				else 
				{
					//Failed to read in. What to do?
					ResetFileHeader(hFile, &Header);
				}
				if (buf) delete [] buf;
			}
			else //Nothing left.
			{
				ResetFileHeader(hFile, &Header);
			}
		}
		else
		{
			//Don't move the records. Just update the offset.
			Header.StartOffset = dwNewOffset;
			UpdateFileHeader(hFile, &Header);
		}
		res = true;
	} while(false);

	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);

	Unlock();
	return res;
}

// Uploads queued game tracking data (maximum 50 records a time). The uploading thread will
// stop after uploading. 
bool CGameTrack::StartUploadOnce()
{
	return StartUpload(50, false, true, 0, 0, 0);
}

// Uploads queued game tracking data repeatedly. Sleep dwInterval miliseconds time between
// each upload. You need to explicitly call StopUpload to stop the upload thread, or 
// it will be stopped when the class is destructed.
bool CGameTrack::StartUploadRepeat(DWORD dwInterval)
{
	return StartUpload(50, true, true, dwInterval, dwInterval, INFINITE);
}

// Uploads queued game tracking data
bool CGameTrack::StartUpload(DWORD MaxCount, bool Repeat, bool StopWhenNoRecords, DWORD SleepInterval, DWORD NoRecordsSleepInterval, DWORD TimeOut)
{
	if (m_IsUploading) return false;
	m_IsUploading = true;

	m_MaxCount = MaxCount;
	m_Repeat = Repeat;
	m_StopWhenNoRecords = StopWhenNoRecords;
	m_SleepInterval = SleepInterval;
	m_NoRecordsSleepInterval = NoRecordsSleepInterval;
	m_TimeOut = TimeOut;

	m_DpId = "-";
	DWORD dwThreadId;
	m_hThread = CreateThread( NULL, 0, CGameTrack::StaticThreadProc, this, 0, &dwThreadId );
	LOG("Created thread: %d", m_hThread);
	return true;
}

DWORD WINAPI CGameTrack::StaticThreadProc(LPVOID lpParameter)
{
	return ((CGameTrack*)lpParameter)->ThreadUpload();
}

/*
	    "%URL_GAMEDATA_CHECKIN%?version=1.0&support="..CL:GetUserSupport().."&dpid="..CL:GetDP(), 
		"%WT_SCRIPT%\\GameData.log", -- TODO: get a better tmp file?
		"",
		;
	    postdata="support="..userID.."&version=1.0&ldata="..DataToSend,
	    postdataencode="application/x-www-form-urlencoded",
	    userurl="GameDataImmediateCheckin",
	    dlfile=""
*/

bool CGameTrack::UploadOnce(LPDWORD lpcbBytes)
{
	bool res = false;
	*lpcbBytes = 0;
	std::string Records;
	DWORD dwNewOffset;
	DWORD dwRecordsRead;


	WaitForSingleObject(m_UploadMutex, INFINITE);

	do
	{
		if (!GetRecordsForUpload(m_MaxCount, &dwRecordsRead, &dwNewOffset, Records)) 
		{
			LOG("Failed to get records");
			break;
		}
		*lpcbBytes = (DWORD)Records.size();
		if (Records.size() == 0) 
		{
			res = true;
			break;
		}
		
		//string CheckinUrl = 
		string CheckinUrl = GAMETRACK_CHECKINURL;
		CheckinUrl += string("?version=1.0&support=") + m_SupportId + "&dpid=" + m_DpId;
		CHttpWebRequest* pRequest = CWebRequest::CreateHttp (CheckinUrl);
		pRequest->SetContentType("application/x-www-form-urlencoded");
		pRequest->SetMethod ("POST");

		WT::System::IO::CStream& Stream = pRequest->GetRequestStream();		
		string PostData = string("support=") + m_SupportId + "&version=1.0&ldata=" + Records;
		Stream.WriteString(PostData);

		LOG("Before getting repsonse");
		CWebResponse& Resp = pRequest->GetResponse();
		LOG("After get response");
		Resp;
		int StatusCode = pRequest->GetStatusCode();
		LOG("Web reponse status code: %d", StatusCode);
		if (StatusCode >= 200 && StatusCode < 300)
		{
			//Success
			DeleteRecords(dwNewOffset, dwRecordsRead);
			res = true;
#ifdef _DEBUG
			CStream* pStream = Resp.GetResponseStream();
			if (pStream)
			{
				int32 len = pStream->GetLength();
				if (len > 0)
				{
					char* buf = new char[len+1];
					pStream->Read((BYTE*)buf, len);
					buf[len] = 0;
					LOG("Returned content: %s", buf);
					delete [] buf;
				}
			}
#endif
		}
		delete pRequest;
	} while(false);

	ReleaseMutex(m_UploadMutex);
	return res;
}

//Running in a different thread to upload.
DWORD CGameTrack::ThreadUpload()
{
	DWORD StartTime = GetTickCount();
	//It essentially get the supportid.
	LOG("Starting thread upload.");
	GetSupportID();

	do
	{
		DWORD dwBytes = 0;
		//Only when it's connected we start uploading.
		LOG("Before getting connect status");
		if (CNetInfo::IsOnline())
		{
			LOG("Connected!");
			//If we failed to upload, there must be some network problem etc.
			if (!UploadOnce(&dwBytes)) 
			{
				LOG("Failed to upload.");
				break;
			}
		}
		else
		{
			LOG("ERROR: Net is not connected. Won't upload.");
		}
		
		//If network is not connected or log file is empty.
		if (dwBytes == 0 && m_StopWhenNoRecords) break;

		if (m_TimeOut != INFINITE)
		{
			if (GetTickCount() - StartTime >= m_TimeOut) break;
		}
		if (m_Repeat) 
		{
			DWORD dwRestInterval = dwBytes? m_SleepInterval : m_NoRecordsSleepInterval;
			if (WaitForSingleObject(m_hStopEvent, dwRestInterval) == WAIT_OBJECT_0)
			{
				break;
			}
		}
	} while(m_Repeat);	

	EnterCriticalSection(&m_ThreadHandleCs);
	if (m_hThread) 
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	m_IsUploading = false;
	LeaveCriticalSection(&m_ThreadHandleCs);

	if (!m_Repeat)
	{
		LOG("No repeat. Quit.");
	}
	return 0;
}

}} //namespace
