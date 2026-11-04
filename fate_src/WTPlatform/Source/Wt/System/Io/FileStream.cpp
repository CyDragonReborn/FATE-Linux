#include "WT/stdafx.h"

#include "WT/System/IO/FileStream.h"
#include "WT/System/IO/FilePath.h"
#include "WT/System/IO/IOException.h"
#include "WT/System/Logging/Log.h"
#include <windows.h>

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(CFileStream)

CFileStream::CFileStream ( const string& path , FileMode::Enum mode )
{
	Init ( path , mode , ( mode == FileMode::Append ? FileAccess::Write : FileAccess::ReadWrite), 
		FileShare::Read, 0, DefaultBufferSize, false, false );
}

CFileStream::CFileStream ( const string& path , FileMode::Enum mode , FileAccess::Enum access , 
						   FileShare::Enum share, uint32 Flags, int32 bufferSize , bool useAsync )
{
	Init ( path , mode , access , share , Flags, bufferSize , useAsync , false );
}

CFileStream::~CFileStream()
{
	Close();
}

void CFileStream::Init ( const string& path , FileMode::Enum mode , FileAccess::Enum access , FileShare::Enum share , 
						 uint32 Flags, int32 bufferSize , bool useAsync , bool FromProxy )
{
	m_IsOpen = false;
	m_Buffer = NULL;
	m_Handle = INVALID_HANDLE_VALUE;
/*	if (path == null)
		throw new ArgumentNullException("path", Environment.GetResourceString("ArgumentNull_Path"));
	if (path.Length == 0)
		throw new ArgumentException(Environment.GetResourceString("Argument_EmptyPath"));
	if (mode < FileMode.CreateNew || mode > FileMode.Append ||
		access < FileAccess.Read || access > FileAccess.ReadWrite ||
		share < FileShare.None || share > FileShare.ReadWrite) {
		String badArg = "mode";
		if (access < FileAccess.Read || access > FileAccess.ReadWrite)
			badArg = "access";
		if (share < FileShare.None || share > FileShare.ReadWrite)
			badArg = "share";
		throw new ArgumentOutOfRangeException(badArg, Environment.GetResourceString("ArgumentOutOfRange_Enum"));
	}
	if (bufferSize <= 0)
		throw new ArgumentOutOfRangeException("bufferSize", Environment.GetResourceString("ArgumentOutOfRange_NeedPosNum"));
*/
	DWORD fAccess = access == FileAccess::Read ? GENERIC_READ:
		access == FileAccess::Write? GENERIC_WRITE:
		GENERIC_READ | GENERIC_WRITE;

	string filePath = CFilePath::GetFullPath(path);

	m_FileName = filePath;
/*
	FileIOPermissionAccess secAccess = FileIOPermissionAccess.NoAccess;
	if ((access & FileAccess.Read) != 0) {
		if (mode==FileMode.Append)
			throw new ArgumentException(Environment.GetResourceString("Argument_InvalidAppendMode"));
		else
			secAccess = secAccess | FileIOPermissionAccess.Read;
	}

	if ((access & FileAccess.Write) != 0) {
		if (mode==FileMode.Append)
			secAccess = secAccess | FileIOPermissionAccess.Append;
		else
			secAccess = secAccess | FileIOPermissionAccess.Write;
	}
	else {
		// No write access
		if (mode==FileMode.Truncate || mode==FileMode.CreateNew || mode==FileMode.Create || mode==FileMode.Append)
			throw new ArgumentException(String.Format(Environment.GetResourceString("Argument_InvalidFileMode&AccessCombo"), mode, access));
	}

	new FileIOPermission(secAccess, new String[] { filePath }, false, false).Demand();
*/
	bool seekToEnd = (mode==FileMode::Append);

	if (mode == FileMode::Append)
		mode = FileMode::OpenOrCreate;

/*	SECURITY_ATTRIBUTES secAttrs = null;
	if ((share & FileShare.Inheritable) != 0) {
		secAttrs = new Win32Native.SECURITY_ATTRIBUTES();
		secAttrs.nLength = (int)Marshal.SizeOf(secAttrs);
		secAttrs.bInheritHandle = 1;
		share &= ~FileShare.Inheritable;
	}
*/

	DWORD WinMode;
	switch (mode)
	{
		case FileMode::Append: WinMode = OPEN_ALWAYS; break;
		case FileMode::Create: WinMode = CREATE_ALWAYS; break;
		case FileMode::CreateNew: WinMode = CREATE_NEW; break;
		case FileMode::Open: WinMode = OPEN_EXISTING; break;
		case FileMode::OpenOrCreate: WinMode = OPEN_ALWAYS; break;
		case FileMode::Truncate: WinMode = TRUNCATE_EXISTING; break;
		default:
			throw std::invalid_argument( "FileStream mode" );
	}

	if (gm_CanUseAsync && useAsync)
	{
		m_Handle = CreateFile ( filePath.c_str() , fAccess , share , NULL , WinMode , FILE_FLAG_OVERLAPPED | Flags, NULL );
		m_IsAsync = true;
	}
	else
	{
		m_Handle = CreateFile ( filePath.c_str() , fAccess , share , NULL , WinMode , FILE_ATTRIBUTE_NORMAL | Flags, NULL );
		m_IsAsync = false;
	}

	if (m_Handle == INVALID_HANDLE_VALUE)
	{
/*		int hr GetLastWin32Error();
		if (hr==ERROR_PATH_NOT_FOUND && filePath.Equals(Directory.InternalGetDirectoryRoot(filePath)))
			hr = __Error.ERROR_ACCESS_DENIED;

		// We need to give an exception, and preferably it would include
		// the fully qualified path name.  Do security check here.  If
		// we fail, give back the msgPath, which should not reveal much.
		bool canGiveFullPath = false;

		if (!bFromProxy)
		{
			try {
				new FileIOPermission(FileIOPermissionAccess.PathDiscovery, new String[] { _fileName }, false, false ).Demand();
				canGiveFullPath = true;
			}
			catch(CSecurityException) {}
		}

		if (canGiveFullPath)
			__Error.WinIOError(hr, _fileName);
		else
			__Error.WinIOError(hr, msgPath);
			*/

		LOG_RED( "CreateFile() failed for '%s'", path.c_str() );
		throw CIOException( "failed to open file" );
	}

	m_CanRead = (access & FileAccess::Read) != 0;
	m_CanWrite = (access & FileAccess::Write) != 0;
	m_CanSeek = true;
	m_IsPipe = false;
	m_IsOpen = true;

	// For Append mode...
	if (seekToEnd)
	{
		DWORD Result = SetFilePointer( m_Handle, 0, 0, FILE_END );
		if (Result == INVALID_SET_FILE_POINTER) //Possible error
		{
			DWORD LastError = GetLastError();
			if (LastError != NO_ERROR)
			{
				throw CIOException( "failed to seek to end of file" );
			}
		}
	}
}

bool CFileStream::CanRead() const
{
	return m_CanRead;
}

bool CFileStream::CanSeek() const
{
	return m_CanSeek;
}

bool CFileStream::CanWrite() const
{
	return m_CanWrite;
}

void* CFileStream::GetHandle()
{
	Flush();
	return m_Handle;
}

bool CFileStream::IsAsync() const
{
	return m_IsAsync;
}

int32 CFileStream::GetLength() const
{
	if (m_Handle == INVALID_HANDLE_VALUE)
	{
		throw CIOException( "bad file handle" );
	}

	DWORD FileSize = GetFileSize( m_Handle , NULL );
	if (  FileSize == INVALID_FILE_SIZE )
	{
		return -1;
	}
	else
		return FileSize;
}

string const& CFileStream::GetName() const
{
	return m_FileName;
}

void CFileStream::SetPosition( int32 position )
{
	if (m_Handle == INVALID_HANDLE_VALUE)
	{
		throw CIOException( "bad file handle" );
	}

	DWORD Result = SetFilePointer( m_Handle, position, 0, FILE_BEGIN );
	if (Result == INVALID_SET_FILE_POINTER) //Possible error
	{
		DWORD LastError = GetLastError();
		if (LastError != NO_ERROR)
		{
			throw CIOException( "failed to seek in file" );
		}
	}
}

int32 CFileStream::GetPosition() const
{
	if (m_Handle == INVALID_HANDLE_VALUE)
	{
		throw CIOException( "bad file handle" );
	}

	DWORD Position = SetFilePointer( m_Handle , 0 , NULL , FILE_CURRENT );
	if (Position == INVALID_SET_FILE_POINTER) //Possible error
	{
		DWORD LastError = GetLastError();
		if (LastError != NO_ERROR)
		{
			throw CIOException( "get file position failed" );
		}
	}

	return Position;
}

//BeginRead
//BeginWrite
void CFileStream::Close()
{
	if ( m_Handle != INVALID_HANDLE_VALUE )
	{
		Flush();
		CloseHandle ( m_Handle );
		m_Handle = INVALID_HANDLE_VALUE;
	}

	m_IsOpen = false;
	m_CanRead = false;
	m_CanWrite = false;
	m_CanSeek = false;
}

//EndRead
//EndWrite
void CFileStream::Flush()
{
	if (m_Handle == INVALID_HANDLE_VALUE)
	{
		throw CIOException( "bad file handle" );
	}

	FlushFileBuffers( m_Handle );
}

int32 CFileStream::Read ( uint8 buffer[], int32 count )
{
	if (m_Handle == INVALID_HANDLE_VALUE)
	{
		throw CIOException( "bad file handle" );
	}

	DWORD NumberOfBytesRead = 0;
	ReadFile( m_Handle, buffer, count, &NumberOfBytesRead, NULL );
	return NumberOfBytesRead;
}

int32 CFileStream::ReadByte ()
{
	if (m_Handle == INVALID_HANDLE_VALUE)
	{
		throw CIOException( "bad file handle" );
	}

	uint8 c;
	DWORD NumberOfBytesRead = 0;
	ReadFile( m_Handle, &c, 1, &NumberOfBytesRead, NULL );
	return NumberOfBytesRead == 1 ? c : -1;
}


int32 CFileStream::Seek ( int32 offset, SeekOrigin::Enum origin )
{
	if (m_Handle == INVALID_HANDLE_VALUE)
	{
		throw CIOException( "bad file handle" );
	}

	DWORD MoveMethod = FILE_CURRENT;

	switch (origin)
	{
		case SeekOrigin::Begin:
			MoveMethod = FILE_BEGIN;
			break;
		case SeekOrigin::Current:
			MoveMethod = FILE_CURRENT;
			break;
		case SeekOrigin::End:
			MoveMethod = FILE_END;
			break;
		default:
			throw std::invalid_argument( "bad seek origin" );
	}

	DWORD NewPosition = SetFilePointer( m_Handle , offset , NULL , MoveMethod );
	if (NewPosition == INVALID_SET_FILE_POINTER) //Possible error
	{
		DWORD LastError = GetLastError();
		if (LastError != NO_ERROR)
		{
			throw CIOException( "get file position failed" );
		}
	}

	return NewPosition;
}

void CFileStream::SetLength ( int32 length )
{
	if (m_Handle == INVALID_HANDLE_VALUE)
	{
		throw CIOException( "bad file handle" );
	}

	DWORD CurrentPosition = SetFilePointer( m_Handle , 0 , NULL , FILE_CURRENT );
	if (CurrentPosition == INVALID_SET_FILE_POINTER)
	{
		DWORD LastError = GetLastError();
		if (LastError != NO_ERROR)
		{
			throw CIOException( "failed to get file pointer" );
		}
	}

	{
		DWORD Result = SetFilePointer( m_Handle, length, 0, FILE_BEGIN );
		if (Result == INVALID_SET_FILE_POINTER)
		{
			DWORD LastError = GetLastError();
			if (LastError != NO_ERROR)
			{
				throw CIOException( "failed to seek to beginning of file" );
			}
		}
	}

	{
		BOOL Result = SetEndOfFile( m_Handle );
		if (Result == FALSE)
		{
			throw CIOException( "SetEndOfFile() failure" );
		}
	}

	//Return to original position
	CurrentPosition = SetFilePointer( m_Handle, CurrentPosition , NULL, FILE_BEGIN );
	if (CurrentPosition == INVALID_SET_FILE_POINTER)
	{
		DWORD LastError = GetLastError();
		if (LastError != NO_ERROR)
		{
			throw CIOException( "failed to seek in file" );
		}
	}
}

void CFileStream::Write ( uint8 const buffer[], int32 count )
{
	if (m_Handle == INVALID_HANDLE_VALUE)
	{
		throw CIOException( "bad file handle" );
	}

	DWORD BytesWritten = 0;
	BOOL Result = WriteFile( m_Handle, buffer, count, &BytesWritten, NULL );

	if (Result == FALSE)
	{
		throw CIOException( "WriteFile() failure" );
	}

	if (BytesWritten != (DWORD)count)
	{
		throw CIOException( "WriteFile() wrote incorrect number of bytes" );
	}
}

void CFileStream::WriteByte ( uint8 value )
{
	if (m_Handle == INVALID_HANDLE_VALUE)
	{
		throw CIOException( "bad file handle" );
	}

	DWORD BytesWritten = 0;
	BOOL Result = WriteFile( m_Handle, &value, 1, &BytesWritten, NULL );

	if (Result == FALSE)
	{
		throw CIOException( "WriteFile() failure" );
	}

	if (BytesWritten != 1)
	{
		throw CIOException( "WriteFile() wrote incorrect number of bytes" );
	}
}

}
}
}
