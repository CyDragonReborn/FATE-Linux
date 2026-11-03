// Archve7zSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "WT/System/Platform.h"
#include "WT/System/DateTime.h"
#include "WT/system/IO/FileStream.h"
#include "WT/system/Compression/Archive7z.h"

#ifdef _DEBUG
#pragma comment(lib, "..\\..\\..\\..\\..\\Build\\Debug\\WTPlatform1D.lib")
#else
#pragma comment(lib, "..\\..\\..\\..\\..\\Build\\Release\\WTPlatform1.lib")
#endif

using namespace WT;
using namespace System;
using namespace IO;
using namespace Compression;

class CArchive7zTester : public IArchive7zCallback
{
public:
	CArchive7z m_Arch;

	bool Test()
	{
		const string root = "\\\\data1\\Work\\WTPlatform\\Archive7zTestFiles\\";
		TestOneStepByStep(root + "bgamefiles.7z", "c:\\temp\\cablztest\\output");

		TestOne(root + "GlacialListOrg.7z", "c:\\temp\\cablztest\\output");
		TestOne(root + "bgamefiles.7z", "c:\\temp\\cablztest\\output");
		TestOne(root + "notexist.7z", "c:\\temp\\cablztest\\output");
		TestOne(root + "doco.cab", "c:\\temp\\cablztest\\output");

		return true;
	}
	bool TestOne(const string& filename, const string& outdir)
	{
		if (m_Arch.Extract(filename, outdir, true, this))
		{
			cout << "SUCCESS: Extracted file " << filename << " to " << outdir << endl;
			ShowStats(&m_Arch);
			return true;
		}
		else
		{
			cout << "FAILED: Extracted file " << filename << " to " << outdir 
				<< " ERROR CODE: " << m_Arch.GetErrorCode() << endl;
			ShowStats(&m_Arch);
			return false;
		}
	}
	static void ShowStats(CArchive7z* arch)
	{
		printf("CArchive7z Status Dump =================\n");
		printf("    IsFinished()                   %d\n", arch->IsFinished());
		printf("    IsSucceeded()                  %d\n", arch->IsSucceeded());
		printf("    GetErrorCode()                 %d\n", arch->GetErrorCode());
		printf("    GetTotalExtractSize()          %d\n", arch->GetTotalExtractSize());
		printf("    GetTotalBytesExtracted()       %d\n", arch->GetTotalBytesExtracted());
		printf("    GetArchiveSize()               %d\n", arch->GetArchiveSize());
		printf("    GetTotalFiles()                %d\n", arch->GetTotalFiles());
		printf("    GetTotalFilesExtracted()       %d\n", arch->GetTotalFilesExtracted());
		printf("    GetCurrentFileSize()           %d\n", arch->GetCurrentFileSize());
		printf("    GetCurrentFileBytesExtracted() %d\n", arch->GetCurrentFileBytesExtracted());
		printf("    GetCurrentFileName()           %s\n", arch->GetCurrentFileName().c_str());
		printf("    GetPercentDone()               %f\n", arch->GetPercentDone());
		printf("\n");
	}
	bool TestOneStepByStep(const string& filename, const string& outdir)
	{
		if (!m_Arch.StartExtract(filename, outdir, true, NULL)) 
		{
			cout << "FAILED: Start Extracting file " << filename << " to " << outdir 
				<< " ERROR CODE: " << m_Arch.GetErrorCode() << endl;		
			ShowStats(&m_Arch);
			return false;
		}
		while(m_Arch.ProcessBlock())
		{
			printf("Percent done: %.1f%%\n", m_Arch.GetPercentDone() * 100);
		}
		if (m_Arch.IsSucceeded())
		{
			cout << "SUCCESS: Extracted file " << filename << " to " << outdir << endl;
			ShowStats(&m_Arch);
			return true;
		}
		else
		{
			cout << "FAILED: Extracted file " << filename << " to " << outdir 
				<< " ERROR CODE: " << m_Arch.GetErrorCode() << endl;
			ShowStats(&m_Arch);
			return false;
		}
	}

	///IArchive7zCallback interface
	//First started..
	virtual void OnStartExtraction()
	{
	}
	//These are events called by processing. If return 0, continue; 1: skip.
	virtual int OnStartExtractFile(const char* FileName, int FileSize)
	{
		//cout << "Start extracting file: " << FileName << " Size: " << FileSize << endl;
		//printf("OnStartExtractFile: Percent: %.1f%%\n", m_Arch.GetPercentDone() * 100); 
		if (string(FileName).find(".gif") != string::npos) 
		{
			ShowStats(&m_Arch);
			return 0;
		}
		//return 1;
		return 0;
	}

	virtual void OnWriteFile(const char* FileName, int FileSize, int BytesWritten)
	{
		//printf("OnWriteFile: Percent: %.1f%%\n", m_Arch.GetPercentDone() * 100); 
	}
	virtual void OnEndExtractFile(const char* FileName, int FileSize, int BytesWritten)
	{
		//printf("OnEndExtractFile: %s, %d, %d \n", FileName, FileSize, BytesWritten);
	}
	//Everything stopped.
	virtual void OnEndExtraction()
	{
		printf("OnEndExtraction: Percent: %.1f%%\n", m_Arch.GetPercentDone() * 100); 
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	//CFileStream fs("c:\\temp\\1.txt", FileMode::Open);
	CArchive7zTester tester;
	tester.Test();
	return 0;
}

