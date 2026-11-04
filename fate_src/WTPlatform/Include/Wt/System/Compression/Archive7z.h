/***************************************************************************************
	Archive7z.h

	Copyright (C) 2002-2005 WildTangent, Inc. 
	All Rights Reserved 

	Description:  7z decoding stuff for WT Platform. 

	History:   
		09/28/2005	Rui Jiang	Created
***************************************************************************************/

#pragma once

#include "WT/System/Platform.h"
#include "WT/system/IO/FileStream.h"
#include <string>

namespace WT
{
namespace System
{
namespace Compression
{

//Forward declaration. Not to be used by outside world.
namespace Archive7zInternal
{
class Archive;
class IStream;
};

//Forward declaration. Not to be used by outside world.
class CArchiveWrap;

namespace Archive7zErrorCode
{
	//This enum matches error codes in Archive7zInternal::ArchiveBase
	enum Enum
	{
		KOK,					//No error. Extraction is successful.
		KUnknown,		
		KInvalidHeader,			//Invalid 7z header. Could be corrupt or invalid file type.
		KFailedToOpenArchive,	//Failed to open archive file.
		KFailedToReadArchive,	//Failed to read from the archive or stream.
		KFailedToCreateDirectory,	//Failed to create directory for extracted files.
		KFailedToCreateFile,		//Failed to create extracted file.
		KFailedToWriteFile,			//Failed to write to the file.
		KDecoding,					//Failed to decode. Could be the original data is corrupted.
		KPackedDataCRCError,		//CRC verfication failed for the original Archive
		KUnpackedDataCRCError,		//CRC verification failed for extracted file.

		//Not in Archive7zInternal::ArchiveBase
		//The stream doesn't support seek and/or read.
		KBadStream
	};
};

//The callback interface for calling class.
class IArchive7zCallback
{
public:
	//First started..
	virtual void OnStartExtraction() = 0;
	//These are events called by processing. If return 0, continue; 1: skip.
	virtual int OnStartExtractFile(const char* FileName, int FileSize) = 0;
	virtual void OnWriteFile(const char* FileName, int FileSize, int BytesWritten) = 0;
	virtual void OnEndExtractFile(const char* FileName, int FileSize, int BytesWritten) = 0;
	//Everything stopped.
	virtual void OnEndExtraction() = 0;
};

/*//////////////////////////////////////////////////////////////////////////////
CArchive7z is the class that can extract a 7z archive file.
In general, there are two ways of using this class. The simplest is to extract 
a file until it returns, using the two "Extract" functions. Example:

	CArchive7z m_Arch;
	bool TestOne(const string& filename, const string& outdir)
	{
		if (m_Arch.Extract(filename, outdir, true, NULL))
		{
			cout << "SUCCESS: Extracted file " << filename << " to " << outdir << endl;
			return true;
		}
		else
		{
			cout << "FAILED: Extracted file " << filename << " to " << outdir 
				<< " ERROR CODE: " << m_Arch.GetErrorCode() << endl;
			return false;
		}
	}

Or you could process the archive one block a time, like:

	CArchive7z m_Arch;
	bool TestOneStepByStep(const string& filename, const string& outdir)
	{
		if (!m_Arch.StartExtract(filename, outdir, true, NULL)) 
		{
			cout << "FAILED: Start Extracting file " << filename << " to " << outdir 
				<< " ERROR CODE: " << m_Arch.GetErrorCode() << endl;		
			return false;
		}
		while(m_Arch.ProcessBlock())
		{
			printf("Percent done: %.1f%%\n", m_Arch.GetPercentDone() * 100);
		}
		if (m_Arch.IsSucceeded())
		{
			cout << "SUCCESS: Extracted file " << filename << " to " << outdir << endl;
			return true;
		}
		else
		{
			cout << "FAILED: Extracting file " << filename << " to " << outdir 
				<< " ERROR CODE: " << m_Arch.GetErrorCode() << endl;
			return false;
		}
	}

Note the the Extract and StartExtract that take a CStream version, the input 
stream must support Seek and Read. Otherwise it will fail immediately. CArchive7z
isn't really a streaming extracter because the 7z format isn't really streamable.

You could implement the IArchive7zCallback to update the UI when CArchive7z 
extracting files. You could also skip some of the files by returning 1 in 
OnStartExtractFile of the callback interface. e.g., if you only want to extract 
gif files: (case is not handled here)

	virtual int OnStartExtractFile(const char* FileName, int FileSize)
	{
		if (string(FileName).find(".gif") != string::npos) 
		{
			return 0;
		}
		else 
		{
			return 1;
		}
	}


Keep in mind that Lzma decoding may consume 2 MB bytes memory (depends on how 
the archive is compressed). CArchive7z will also use 128KB for buffering. So
it is not recommended to create multiple CArchive7z object to do extraction
at the same time. Although CArchive7z will release all buffers when extraction
is complete, it may still use some extra memory for keeping the status.
it is recommended you either delete the CArchive7z, or call CArchive7z::Clear to 
release all resources.

This class is not thread safe.
//////////////////////////////////////////////////////////////////////////////*/

typedef SmartPointer<class CArchive7z> HArchive7z;
class CArchive7z : public CObject
{
public:
	WT_CLASS( CArchive7z, CObject)

	enum Enum
	{
		KDefaultBlockSize = 0x10000
	};

	CArchive7z();
	~CArchive7z();

	//Do synchronous extraction, extract all the files. 
	//Return true if all files have been extracted successfully.
	bool Extract(const string& FilePath, const string& DestDir, bool OverwriteExistingFiles,
		IArchive7zCallback* pCallBack);
	//Do synchronous extraction on the stream, extract all the files. 
	//Return true if all files have been extracted successfully.
	bool Extract(IO::CStream* pStream, const string& DestDir, bool OverwriteExistingFiles,
		IArchive7zCallback* pCallBack);

	//Start extraction, parse the headers etc. Then you need to call ProcessBlock 
	//continuously until it returns false.
	bool StartExtract(const string& FilePath, const string& DestDir, bool OverwriteExistingFiles,
		IArchive7zCallback* pCallBack);
	//Start extraction, parse the headers etc. Then you need to call ProcessBlock 
	//continuously until it returns false.
	bool StartExtract(IO::CStream* pStream, const string& DestDir, bool OverwriteExistingFiles,
		IArchive7zCallback* pCallBack);
	//Process maximum of OutSize then return. Return true if there is still something need to be extracted.
	bool ProcessBlock(int OutSize = KDefaultBlockSize);
	//If we are still processing, return false; Otherwise true.
	bool IsFinished() const;
	//Whether we have successfully extracted all files.
	bool IsSucceeded() const;

	//Return the detailed error code.
	Archive7zErrorCode::Enum GetErrorCode()  const;
	
	//Return a double from 0.0 - 1.0 indicate how much haven't been extracted. If we haven't started, returns 0.
	double GetPercentDone() const;
	//This is the total size of all files, uncompressed. If we haven't started, return 0.
	int GetTotalExtractSize() const;
	//This is acurately how many bytes have been extracted. If we haven't started, return 0.
	int GetTotalBytesExtracted() const;
	//The size of the archive or input stream. If we haven't started, return 0.
	int GetArchiveSize() const;
	//The number of files in the archive. If we haven't started, return 0.
	int GetTotalFiles() const;
	//The number of files that have been successfully extracted. If we haven't started, return 0.
	int GetTotalFilesExtracted() const;
	// Retrieves the file length, in bytes, of the current file being extracted. If we haven't started, return 0.
	int GetCurrentFileSize() const;
	//The bytes extracted for the current file only. If we haven't started, return 0.
	int GetCurrentFileBytesExtracted() const;
	//Return the current file's name that is being extracted.
	std::string GetCurrentFileName() const;

	//Release all resouces taken.
	void Clear();
protected:
	//This is the internal archive class that really does the dirty work.
	CArchiveWrap*	m_pArchive;
	//This is either the stream param in Extract, or the file stream that we opened.
	IO::CStream*	m_pStream;
	//Whether we own the above stream or not. If true, we need to delete it.
	bool			m_OwnStream;
	//This is the stream class that wraps IO::CStream.
	Archive7zInternal::IStream*	m_pWrapStream;
	//This is a temp store when we don't have m_pArchive yet.
	Archive7zErrorCode::Enum	m_BackupErrorCode;

	bool InternalExtract(IO::CStream* pStream, const string& DestDir, bool OverwriteExistingFiles,
		IArchive7zCallback* pCallBack);
	bool InternalStartExtract(IO::CStream* pStream, const string& DestDir, bool OverwriteExistingFiles,
		IArchive7zCallback* pCallBack);
};

}
}
}