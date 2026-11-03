/***************************************************************************************
	GameTrack.h

	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved 

	Description: Game tracking stuff.

	History:   
		02/01/2006	Rui Jiang	Created

	CGameTrack will check network condition before upload. If an upload fails for any reason 
	(network request failed etc), Upload thread will abort.

	For testing, you should use the test cda server. Change your hosts file, add following
	line:
		209.67.107.104 gamedata.cda.wildtangent.com
	You can access the server log files at the following UNC
		\\10.14.1.11\e\logfiles\cda.wildtangent.com
	For user name and password, contact Mark Scattergood.

	Example use:

void Test()
{
	CGameTrack gt;
	gt.Init();
	for(int i = 0; i < 10; i++)
	{
		//Webdriver
		gt.Log(GAMETRACK_LOGGERID_WEBDDRIVER, "ED9EB6E2-04A8-432B-B8F9-056AACA7BEAA", "WDTestGame", "LC=1,TP=276,GP=1,UL=1");
		//GameChannel
		gt.Log(GAMETRACK_LOGGERID_GAMECHANNEL, "ED9EB6E2-04A8-432B-B8F9-056AACA7BEAA", "GCTestGame", "BC=1,DL=3");	
		//Other.
		gt.Log("{C360A165-B00D-481B-B562-82A301D9B87A}", "ED9EB6E2-04A8-432B-B8F9-056AACA7BEAA", "OTTestGame", "DD=0,CC=1");
	}
	printf("Begin uploading...\n");
	if (gt.StartUpload(50, false, true, 0, 0, 0))
	{
		while(gt.IsUploading())
		{
			DWORD dw = gt.GetLogBytes(); 
			printf("%d bytes left\n", dw);
			//if (dw == 0) break;
			Sleep(1000);
		}
	}
}
***************************************************************************************/

#pragma once
#include <windows.h>
#include <string>
#include <vector>

namespace WT
{
namespace Tracking
{

struct CLogFileHeader;

//The maximum file size. We stop logging after we exceed the limit.
#define MAX_GAMETRACK_FILESIZE (5 * 1024 * 1024)
//If deleted log size exceed this amount we will start cleaning up deleted records.
#define MAX_GAMETRACK_CLEANUPSIZE (1024 * 1024)
//Plain text records are saved as XOR char by char with this one. Currently by default We don't encrypt.
#define RECORD_XOR_CHAR 0xB7

//The file name of the log. Don't use "gtd.dat", because we shipped the one with fixed XOR encryption.
//Now we will have each record decide encryption or not.
#define GAMETRACK_GLOBALFILENAME "gtd2.dat"
//This is file that has the report id.
#define GAMETRACK_SUPPORTIDFILENAME "support.dat"
#define GAMETRACK_CHECKINURL "http://gamedata.cda.wildtangent.com/GameData/Checkin.aspx"
//#define GAMETRACK_CHECKINURL "http://rjiang-pc/CDAServer/GameData/Checkin.aspx"

#define GAMETRACK_LOGGERID_WEBDDRIVER	"{FA13A9FA-CA9B-11D2-9780-00104B242EA3}"
#define GAMETRACK_LOGGERID_GAMECHANNEL	"{33D05E32-C48E-449e-8FD6-B9C64B9F8585}"

class CGameTrack
{
public:
	CGameTrack(void);
	~CGameTrack(void);

	//Create the mutex etc. Do not include any path. Valid name like: gtd.dat, abc.def etc.
	bool Init(const std::string& FileName = GAMETRACK_GLOBALFILENAME);

	// Queue game tracking data
	// Params:
	//		LoggerID:	It can be one of following values:
	//					{FA13A9FA-CA9B-11D2-9780-00104B242EA3}: This log is from webdriver
	//					{33D05E32-C48E-449e-8FD6-B9C64B9F8585}: This log is from gamechannel
	//					It can be also any other values. In this case, the log is written to "other". 
	//					On CDA server, logs are saved to file e.g.:
	//						cdainfo-gamedata_webdriver-20060207.log
	//						cdainfo-gamechannel-other-20060207.log
	//					Use the define in this header: GAMETRACK_LOGGERID_WEBDDRIVER etc.
	//					This is a legacy issue and in general, you should use 
	//					GAMETRACK_LOGGERID_WEBDDRIVER
	//		Sku:		Important- The product's Sku. Each game packed with Keres will have a different Sku.
	//		Key, Value:	Important- A Key-Value pair that matters.
	bool Log(const std::string& LoggerID, const std::string& Sku, const std::string& Key, const std::string& Value);

	bool IsUploading() { return m_IsUploading; }

	//Mandatory break uploading. May terminate the thread. DO NOT use it unless the program 
	//is shuting down. The destructor will automatically call this if the upload thread is 
	//still running. dwTimeOut will specify how long you want to wait before 
	// the thread automatically stop. 
	void StopUpload(DWORD dwTimeOut);

	//Return the number of bytes of the current Un-uploaded logs. If the file can not be
	//open, return 0. 
	DWORD GetLogBytes();

	// Uploads queued game tracking data. Each record is about 150 bytes (depends on the value length). 
	// So a MaxCount less than 100 seems appropriate. Do not use a too high MaxCount. 
	// Params: 
	//		MaxCount:	How many records we want to upload at one time.
	//		Repeat:		If this is true, then the upload thread will not stop. It will continuously 
	//					query the log file and upload.
	//		StopWhenNoRecords:	If this is true, upload thread will stop when there are no records left.
	//					Otherwise it will continue query for new records.
	//		SleepInterval:	The thread is put to sleep for this amount of time after each upload.
	//					You could set it to 0 so it will upload non-stop. Or some bigger value
	//					so that it wouldn't cause too much traffic.
	//		NoRecordsSleepInterval: The thread is put to sleep for this amount of time when there is nothing to upload.
	//					Set it to a big number to avoid frequent query of log file. Say 60000 (1 min).
	//		TimeOut:	If this value != INFINITE, then the upload thread will stop after TimeOut milliseconds.
	//					Note that if it is blocked by network call, then it may not return in time.
	bool StartUpload(DWORD MaxCount, bool Repeat, bool StopWhenNoRecords, 
		DWORD SleepInterval, DWORD NoRecordsSleepInterval, DWORD TimeOut);

	//By default, we don't encrypt record. But you can use SetXorChar to change it to 
	//some arbitary char. Then following logs will be encrypted. encrypted/unencrypted
	//log could be mixed.
	void SetXorChar(BYTE c) { m_XorChar = c; }
protected:
	//Mutex for accessing the log file.
	HANDLE		m_FileMutex;
	//Only one process can upload at a time. 
	HANDLE		m_UploadMutex;
	//Critical section used to access the thread handle and status.
	CRITICAL_SECTION m_ThreadHandleCs;
	//path Name of the log file.
	std::string	m_TrackFilePathName;
	bool		m_IsUploading;
	HANDLE		m_hThread;
	//The unique machine id.
	std::string m_SupportId;
	//Which char shall we use to XOR the record? If 0, then unecrypted. This is default.
	BYTE		m_XorChar;

	//Upload parameters. No need to initialize in constructor.
	std::string m_DpId;
	DWORD		m_MaxCount;
	bool		m_Repeat;
	bool		m_StopWhenNoRecords;
	DWORD		m_SleepInterval;
	DWORD		m_NoRecordsSleepInterval;
	DWORD		m_TimeOut;

	//Event for shutdown.
	HANDLE		m_hStopEvent;

	void Lock();
	void Unlock();

	//Open/create the file and validate the header.
	HANDLE OpenLogFile(CLogFileHeader* pHeader);
	//Load MaxCount records and lock them.
	bool GetRecordsForUpload(DWORD MaxCount, LPDWORD lpRecordsRead, LPDWORD lpNewOffset, std::string& Records);
	//After a successful uploading, we need to remove the records.
	bool DeleteRecords(DWORD dwNewOffset, DWORD DeletedItemCount);

	static std::string GetStorePath();
	static std::string CGameTrack::FormatLogEntry(const std::string& LoggerID, const std::string& Sku, const std::string& Key, const std::string& Value);
	void ComposeLogItem(const std::string& Entry, DWORD Id, std::vector<BYTE>& data);
	
	//Read in header from the file handle and verify if header is valid.
	static bool ReadAndVerifyHeader(HANDLE hFile, CLogFileHeader* pHeader);
	//Reset the content of header as if it is an empty log, and write to file.
	static bool ResetFileHeader(HANDLE hFile, CLogFileHeader* pHeader);
	//Recalculate CRC of header and write the header back to file
	static bool UpdateFileHeader(HANDLE hFile, CLogFileHeader* pHeader);

	static DWORD WINAPI StaticThreadProc(LPVOID lpParameter);
	DWORD ThreadUpload();
	bool UploadOnce(LPDWORD lpcbBytes);

	std::string GetSupportID();

	// Uploads queued game tracking data (maximum 50 records a time). The uploading thread will
	// stop after uploading one time. This function just call: StartUpload(50, false, true, 0, 0, 0);
	bool StartUploadOnce();

	// Uploads queued game tracking data repeatedly, 50 records a time. Upload will stop 
	// when there is no records left. The upload thread will also sleep dwInterval miliseconds 
	// time between each upload. There are 3 conditions upload thread will stop:
	//    1. No records left in the log file.
	//    2. You explicitly call StopUpload.
	//    3. The object is destructed.
	// This function just call: StartUpload(50, true, true, dwInterval, dwInterval, INFINITE);
	bool StartUploadRepeat(DWORD dwInterval);
};

}} //namespace