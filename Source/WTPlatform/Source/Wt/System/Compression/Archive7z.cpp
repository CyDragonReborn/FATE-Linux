/***************************************************************************************
	Archive7z.cpp

	Copyright (C) 2002-2005 WildTangent, Inc. 
	All Rights Reserved 

	Description:  7z decoding stuff for WT Platform. 

	History:   
		09/28/2005	Rui Jiang	Created
***************************************************************************************/

#include "WT/stdafx.h"

#include "Archive7z.h"
#include "Archive7zInternal.h"

namespace WT
{
namespace System
{
namespace Compression
{
using namespace IO;

class CStreamWrap : public Archive7zInternal::IStream
{
public: 
	CStreamWrap(CStream* pStream)
	{
		m_pStream = pStream;
	}
	//////////////  IStream stuff ///////////////////////
	virtual void Read(void *buffer, int size)
	{		
		CheckFile();
		int res = (int)m_pStream->Read((WT::uint8*) buffer, size);
		if (res != size)
		{
			throw "Read in size is different than specified";
		}
	}
	virtual void Write(const void* buffer, int size)
	{		
		CheckFile();
		//The CStream should throw exception. But looks like now it's not,
		//and not returning how many bytes were written either.
		m_pStream->Write((const WT::uint8*)buffer, size);
		/*
		int res = (int)fwrite(buffer, 1, size, m_File);
		if (res != size)
		{
			throw "Write size is different than specified";
		}
		*/
	}
	virtual void Seek(int offset, int origin)
	{
		CheckFile();
		SeekOrigin::Enum so = SeekOrigin::Begin;
		if (origin == SEEK_CUR)
			so = SeekOrigin::Current;
		else if (origin == SEEK_END)
			so = SeekOrigin::End;

		m_pStream->Seek(offset, so);
		//TODO: handle filestream error.
	}
	virtual int Tell()
	{
		CheckFile();
		return m_pStream->GetPosition();
	}
	virtual int GetLength()
	{
		CheckFile();
		return m_pStream->GetLength();
	}
protected:
	CStream* m_pStream;

	void CheckFile()
	{
		assert(m_pStream);
		if (!m_pStream)
		{
			throw "Underlying stream is not opened";
		}
	}
};

//Wrap Archive to provide call back etc.
class CArchiveWrap : public Archive7zInternal::Archive
{
public:
	CArchiveWrap()
	{
		m_pCallback = NULL;
	}
	void SetCallback(IArchive7zCallback* p)
	{
		m_pCallback = p;
	}
	//Just expose it.
	virtual void SetError(ErrorCodes ec)
	{
		__super::SetError(ec);
	}

	//First started..
	virtual void OnStartExtraction()
	{
		if (m_pCallback) m_pCallback->OnStartExtraction();
	}
	//These are events called by processing. If return 0, continue; 1: skip.
	virtual int OnStartExtractFile(const char* FileName, int FileSize)
	{
		if (m_pCallback) return m_pCallback->OnStartExtractFile(FileName, FileSize);
		else return 0;
	}
	virtual void OnWriteFile(const char* FileName, int FileSize, int BytesWritten)
	{
		if (m_pCallback) m_pCallback->OnWriteFile(FileName, FileSize, BytesWritten);
	}
	virtual void OnEndExtractFile(const char* FileName, int FileSize, int BytesWritten)
	{
		if (m_pCallback) m_pCallback->OnEndExtractFile(FileName, FileSize, BytesWritten);
	}
	//Everything stopped.
	virtual void OnEndExtraction()
	{
		if (m_pCallback) m_pCallback->OnEndExtraction();
	}
protected:
	IArchive7zCallback* m_pCallback;
};

CArchive7z::CArchive7z()
{
	//Init table for CRC if necessary.
	Archive7zInternal::CCRC::InitTable();

	m_pArchive = NULL;
	m_pWrapStream = NULL;
	m_pStream = NULL;
	m_OwnStream = false;
	m_BackupErrorCode = Archive7zErrorCode::KOK;
}

CArchive7z::~CArchive7z()
{
	Clear();
}

//Release all resouces taken.
void CArchive7z::Clear()
{
	delete m_pArchive;
	m_pArchive = NULL;
	delete m_pWrapStream;
	m_pWrapStream = NULL;
	if (m_OwnStream) delete m_pStream;
	m_pStream = NULL;
	m_OwnStream = false;
	m_BackupErrorCode = Archive7zErrorCode::KOK;
}

//Do synchronous extraction, extract all the files.
bool CArchive7z::Extract(const string& FilePath, const string& DestDir, bool OverwriteExistingFiles,
	IArchive7zCallback* pCallBack)
{
	Clear();
	try
	{
		m_pStream = new CFileStream(FilePath, FileMode::Open, FileAccess::Read);
	}
	catch(...)
	{
		m_BackupErrorCode = Archive7zErrorCode::KFailedToOpenArchive;
		return false;
	}
	m_OwnStream = true;	
	bool res = InternalExtract(m_pStream, DestDir, OverwriteExistingFiles, pCallBack);
	return res;
}

//Do synchronous extraction on the stream, extract all the files.
bool CArchive7z::Extract(IO::CStream* pStream, const string& DestDir, bool OverwriteExistingFiles,
	IArchive7zCallback* pCallBack)
{
	Clear();
	if (!pStream->CanRead() || !pStream->CanSeek())
	{
		m_BackupErrorCode = Archive7zErrorCode::KBadStream;
		return false;
	}
	bool res = InternalExtract(pStream, DestDir, OverwriteExistingFiles, pCallBack);
	return res;
}

//Start extraction, parse the headers etc. Then you need to call ProcessBlock 
//continuously until it returns false.
bool CArchive7z::StartExtract(const string& FilePath, const string& DestDir, bool OverwriteExistingFiles,
	IArchive7zCallback* pCallBack)
{
	Clear();
	try
	{
		m_pStream = new CFileStream(FilePath, FileMode::Open, FileAccess::Read);
	}
	catch(...)
	{
		m_BackupErrorCode = Archive7zErrorCode::KFailedToOpenArchive;
		return false;
	}
	m_OwnStream = true;	
	return InternalStartExtract(m_pStream, DestDir, OverwriteExistingFiles, pCallBack);
}

//Start extraction, parse the headers etc. Then you need to call ProcessBlock 
//continuously until it returns false.
bool CArchive7z::StartExtract(IO::CStream* pStream, const string& DestDir, bool OverwriteExistingFiles,
	IArchive7zCallback* pCallBack)
{
	Clear();
	if (!pStream->CanRead() || !pStream->CanSeek())
	{
		m_BackupErrorCode = Archive7zErrorCode::KBadStream;
		return false;
	}
	m_pStream = pStream;
	return InternalStartExtract(m_pStream, DestDir, OverwriteExistingFiles, pCallBack);
}

bool CArchive7z::InternalExtract(IO::CStream* pStream, const string& DestDir, bool OverwriteExistingFiles,
	IArchive7zCallback* pCallBack)
{
	if (!InternalStartExtract(pStream, DestDir, OverwriteExistingFiles, pCallBack)) return false;

	while(ProcessBlock(KDefaultBlockSize))
	{
	}
	return m_pArchive->GetStatus() == Archive7zInternal::ArchiveBase::stDone;
}


bool CArchive7z::InternalStartExtract(IO::CStream* pStream, const string& DestDir, bool OverwriteExistingFiles,
	IArchive7zCallback* pCallBack)
{
	m_pArchive = new CArchiveWrap();
	m_pArchive->SetCallback(pCallBack);

	m_pWrapStream = new CStreamWrap(pStream);
	bool res = m_pArchive->ExtractInit(m_pWrapStream);
	if (!res) return false;

	m_pArchive->SetExtractOptions(DestDir.c_str(), OverwriteExistingFiles);
	return true;
}

bool CArchive7z::ProcessBlock(int OutSize)
{
	assert(m_pArchive);
	return m_pArchive->ProcessBlock(OutSize);
}

//If we are still processing, return false; Otherwise true.
bool CArchive7z::IsFinished() const
{
	if (!m_pArchive) return true;
	return m_pArchive->IsFinished();
}

//Whether we have successfully extracted all files.
bool CArchive7z::IsSucceeded()  const
{
	if (!m_pArchive) return false;
	if (!IsFinished()) return false;
	return GetErrorCode() == Archive7zErrorCode::KOK;
}

//Return the detailed error code.
Archive7zErrorCode::Enum CArchive7z::GetErrorCode() const
{
	if (!m_pArchive) return m_BackupErrorCode;
	else return (Archive7zErrorCode::Enum)m_pArchive->GetLastError();
}

//The size of the archive or input stream. If we haven't started, return 0.
int CArchive7z::GetArchiveSize() const
{
	return m_pArchive? m_pArchive->GetArchiveSize() : 0;
}

//This is the total size of all files, uncompressed. If we haven't started (StartExtract not called), return 0.
int CArchive7z::GetTotalExtractSize() const
{
	return m_pArchive? m_pArchive->GetUnpackSize() : 0;
}

//This is acurately how many bytes have been extracted. If we haven't started (StartExtract not called), return 0.
int CArchive7z::GetTotalBytesExtracted() const
{
	return m_pArchive? m_pArchive->GetUnpackBytesProcessed() : 0;
}

//The number of files in the archive.
int CArchive7z::GetTotalFiles() const
{
	return m_pArchive? m_pArchive->GetTotalFiles() : 0;
}

//The number of files that have been successfully extracted.
int CArchive7z::GetTotalFilesExtracted() const
{
	return m_pArchive? m_pArchive->GetTotalFilesExtracted() : 0;
}

// Retrieves the file length, in bytes, of the current file being extracted
int CArchive7z::GetCurrentFileSize() const
{
	return m_pArchive? m_pArchive->GetCurrentFileSize() : 0;
}

//The bytes extracted for the current file only.
int CArchive7z::GetCurrentFileBytesExtracted() const
{
	return m_pArchive? m_pArchive->GetCurrentFileBytesExtracted() : 0;
}

//Return a double from 0.0 - 1.0 indicate how much haven't been extracted. If we haven't started, returns 0.
double CArchive7z::GetPercentDone() const
{
	if (m_pArchive && !m_pArchive->IsUninited())
	{
		return m_pArchive->GetUnpackSize() == 0? 1 : m_pArchive->GetUnpackBytesProcessed() / (double)m_pArchive->GetUnpackSize();
	}
	else 
	{
		return 0;
	}
}

//Return the current file's name that is being extracted.
std::string CArchive7z::GetCurrentFileName() const
{
	return m_pArchive? m_pArchive->GetCurrentFileName() : "";
}

}
}
}