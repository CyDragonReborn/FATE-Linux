// SaveFileSample.cpp : Defines the entry point for the application.
//

#include <memory>
#include <windows.h>
#include <tchar.h>
#include "WT\System\IO\SaveFile.h"
#include "WT\System\IO\FileStream.h" //Needed for CSaveFile::Open constants

using namespace WT::System::IO;

int APIENTRY _tWinMain(HINSTANCE /*hInstance*/,
                     HINSTANCE /*hPrevInstance*/,
                     LPTSTR    /*lpCmdLine*/,
                     int       /*nCmdShow*/)
{
	std::string Message;

	std::auto_ptr<CSaveFile> SaveFile;

	try
	{
		SaveFile.reset( new CSaveFile( "player3\\save.dat" ) );
	} catch (...) {
		Message = "An exception occured in CSaveFile::CSaveFile";
		MessageBox( NULL, Message.c_str(), "SaveFileSample", MB_OK | MB_ICONERROR );
		return EXIT_FAILURE;
	}

	HANDLE hFile = SaveFile->Open( FileMode::Create );
	if (hFile == INVALID_HANDLE_VALUE)
	{
		Message = "Failed to create '";
		Message += SaveFile->GetPath();
		Message += '\'';
		MessageBox( NULL, Message.c_str(), "SaveFileSample", MB_OK | MB_ICONEXCLAMATION );
		return EXIT_FAILURE;
	} else {

		::CloseHandle( hFile );

		Message = "Successfully created '";
		Message += SaveFile->GetPath();
		Message += '\'';
		MessageBox( NULL, Message.c_str(), "SaveFileSample", MB_OK | MB_ICONINFORMATION );
	}

	try
	{
		SaveFile->Delete();
	} catch (...) {
		Message = "Failed to delete '";
		Message += SaveFile->GetPath();
		Message += '\'';
		MessageBox( NULL, Message.c_str(), "SaveFileSample", MB_OK | MB_ICONEXCLAMATION );
		return EXIT_FAILURE;
	}
	
	Message = "Successfully deleted '";
	Message += SaveFile->GetPath();
	Message += '\'';
	MessageBox( NULL, Message.c_str(), "SaveFileSample", MB_OK | MB_ICONINFORMATION );
	return EXIT_SUCCESS;
}
