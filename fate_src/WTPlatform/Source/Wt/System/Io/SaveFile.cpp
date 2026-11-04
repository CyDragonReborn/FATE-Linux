/*
	SaveFile.cpp

	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	3/28/2006
*/

#include "WT/stdafx.h"
#include <shlobj.h>
#include <sddl.h>
#include "WT/System/IO/SaveFile.h"
#include "WT/System/Logging/Log.h"
#include "WT/System/IO/FileStream.h"
#include "WT/System/IO/MemoryStream.h"
#include "WT/System/IO/FilePath.h"
#include "WT/System/Util/WinUtil.h"
#include "WT/Application/RuntimeAttribute.h"
#include "WT/System/Xml/XmlReader.h"

//TODO use Windows SDK GameUX.h includes instead of defining RM_ constants
//TODO find an SDK include for RICH_GAME_MEDIA_HEADER structure instead of cutting & pasting from documentation
//TODO worry about language-specific thumbnails in GDF.DLL?
//TODO worry about UPX'd GDF.DLL? (Extracting resources would be harder.)

#define RM_MAXLENGTH    1024
#define RM_MAGICNUMBER  'HMGR'

using WT::Application::CRuntimeAttribute;
using WT::System::Xml::CXmlReader;

namespace WT
{
namespace System
{
namespace IO
{

bool IsWindows2KOrBetter()
{
	OSVERSIONINFO OSVersionInfo;
	OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
	::GetVersionEx( &OSVersionInfo );
	return OSVersionInfo.dwMajorVersion >= 5;
}

//Remove initial, final, and repeated slashes
void CleanSlashes( std::string& s )
{
	while (!s.empty() && CFilePath::IsDirectorySeparator( *s.begin() )) s.erase( s.begin() );
	while (!s.empty() && CFilePath::IsDirectorySeparator( *s.rbegin() )) s.erase( s.begin() + (s.size() - 1) );

	for (std::string::iterator it = s.begin(); it != s.end();)
	{
		std::string::iterator next = it;
		++next;

		if (CFilePath::IsDirectorySeparator( *it ))
		{
			if (next != s.end() && CFilePath::IsDirectorySeparator( *next ))
			{
				it = s.erase( it );
				continue;
			}
		}

		it = next;
	}
}

class CSecurityAttributes
{
public:
	CSecurityAttributes( bool Inheritable = false )
	{
		m_Attributes.nLength = sizeof m_Attributes;
		m_Attributes.lpSecurityDescriptor = NULL;
		m_Attributes.bInheritHandle = Inheritable;
	}

	~CSecurityAttributes()
	{
		::LocalFree( m_Attributes.lpSecurityDescriptor );
	}

	void SetInheritable( bool Inheritable )
	{
		m_Attributes.bInheritHandle = Inheritable;
	}

	bool GetInheritable() const
	{
		return m_Attributes.bInheritHandle != FALSE;
	}

	//Set the security descriptor. Requires Windows 2000 or later.
	void SetSecurityDescriptor( std::string const& SecurityDescriptor );

	SECURITY_ATTRIBUTES const& GetAttributes() const {return m_Attributes;}

private:
	SECURITY_ATTRIBUTES m_Attributes;
};

class CSaveFileManager
{
public:
	static CSaveFileManager const& GetInstance();

	std::string const& GetRoot() const {return m_Root;}
	std::string const& GetProductRoot() const {return m_ProductRoot;}
	std::string const& GetProductCodeName() const {return m_ProductCodeName;}
	std::string const& GetProductDisplayName() const {return m_ProductDisplayName;}
	void GetSecurityAttributes( SECURITY_ATTRIBUTES& Attributes, bool Inheritable = false ) const;
	bool GetIsWindows2KOrBetter() const {return m_IsWindows2KOrBetter;}

private:
	CSaveFileManager();

private:
	CSecurityAttributes m_Attributes;
	std::string			m_Root;
	std::string			m_ProductRoot;
	std::string			m_ProductCodeName;
	std::string			m_ProductDisplayName;
	bool				m_IsWindows2KOrBetter;
};

CSaveFileManager const& CSaveFileManager::GetInstance()
{
	static std::auto_ptr<CSaveFileManager> Manager;

	if (Manager.get() == NULL)
	{
		Manager.reset( new CSaveFileManager() );
	}
	
	assert( Manager.get() != NULL );
	return *Manager.get();
}

CSaveFileManager::CSaveFileManager()
	: m_Attributes( false )
{
	m_IsWindows2KOrBetter = IsWindows2KOrBetter();

	m_Root = WT::System::Util::GetCommonAppDataPath();

	if (m_IsWindows2KOrBetter)
	{
		m_Attributes.SetSecurityDescriptor( 
			"D:P" //DACL
			"(A;OICI;GA;;;SY)" //Allow local system full control
			"(A;OICI;GA;;;BA)" //Allow builtin admins full control
			"(A;OICI;RCSDFRFWFXRPWPCCDCLCSWLODT;;;WD)" //Allow everyone various permissions, but not full control
		);
	}

	{
		CRuntimeAttribute RuntimeAttribute;
		m_ProductCodeName = RuntimeAttribute.GetProductCodeName();
		m_ProductDisplayName = RuntimeAttribute.GetProductDisplayName();
	}

	{
		//Remove initial, final, and repeated slashes from product code name
		std::string ProductCodeName = m_ProductCodeName;
		CleanSlashes( ProductCodeName );

		assert( !ProductCodeName.empty() );

		m_ProductRoot = m_Root;
		m_ProductRoot += "\\WildTangent\\";
		m_ProductRoot += ProductCodeName;
	}
}

void CSaveFileManager::GetSecurityAttributes( SECURITY_ATTRIBUTES& Attributes, bool Inheritable ) const
{
	Attributes = m_Attributes.GetAttributes();

	assert( Attributes.bInheritHandle == FALSE );
	if (Inheritable)
	{
		Attributes.bInheritHandle = TRUE;
	}
}

void GetGUID( std::wstring const& GUIDString, GUID& GUID_ )
{
	char Nibbles[32];
	char* pOutNibble = Nibbles;

	int32 const GUIDStringSize = static_cast<int32>( GUIDString.size() );
	for (int32 i = 0; i < GUIDStringSize; ++i)
	{
		wchar_t c = GUIDString[i];
		char Nibble;

		if (iswdigit( c ))
		{
			Nibble = static_cast<char>( c - L'0' );
		} else {
			c = towupper( c );
			if (c >= L'A' && c <= L'F')
			{
				Nibble = static_cast<char>( c + (10 - L'A') );
			} else {
				continue;
			}
		}

		if (pOutNibble >= Nibbles + sizeof Nibbles)
		{
			throw std::runtime_error( "ProductGUID has excess hex digits" );
		}

		*pOutNibble++ = Nibble;
	}

	if (pOutNibble < Nibbles + sizeof Nibbles)
	{
		throw std::runtime_error( "ProductGUID has insufficient hex digits" );
	}
	
	for (int32 i = 0; i < sizeof GUID_.Data1; ++i)
	{
		reinterpret_cast<char*>( &GUID_.Data1 )[i] = (Nibbles[6 - (i << 1)] << 4) + Nibbles[7 - (i << 1)];
	}

	reinterpret_cast<char*>( &GUID_.Data2 )[0] = (Nibbles[10] << 4) + Nibbles[11];
	reinterpret_cast<char*>( &GUID_.Data2 )[1] = (Nibbles[ 8] << 4) + Nibbles[ 9];
	reinterpret_cast<char*>( &GUID_.Data3 )[0] = (Nibbles[14] << 4) + Nibbles[15];
	reinterpret_cast<char*>( &GUID_.Data3 )[1] = (Nibbles[12] << 4) + Nibbles[13];

	for (int32 i = 0; i < sizeof GUID_.Data4; ++i)
	{
		GUID_.Data4[i] = (Nibbles[16 + (i << 1)] << 4) + Nibbles[17 + (i << 1)];
	}
}

void CSecurityAttributes::SetSecurityDescriptor( std::string const& SecurityDescriptor )
{
	assert( IsWindows2KOrBetter() );

	::LocalFree( m_Attributes.lpSecurityDescriptor );
	m_Attributes.lpSecurityDescriptor = NULL;

	std::string DLLPath = WT::System::Util::GetSystemPath() + "\\ADVAPI32.DLL";

	HMODULE hModule = ::LoadLibraryEx( DLLPath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
	if (hModule == NULL)
	{
		return;
	}

	BOOL (WINAPI* pConvertStringSecurityDescriptorToSecurityDescriptor)(
		LPCTSTR StringSecurityDescriptor,
		DWORD StringSDRevision,
		PSECURITY_DESCRIPTOR* SecurityDescriptor,
		PULONG SecurityDescriptorSize
		) = reinterpret_cast<BOOL (WINAPI*)(LPCTSTR,DWORD,PSECURITY_DESCRIPTOR*,PULONG)>(
			::GetProcAddress( hModule, "ConvertStringSecurityDescriptorToSecurityDescriptorA" ) );

	if (pConvertStringSecurityDescriptorToSecurityDescriptor != NULL)
	{
		if (!(*pConvertStringSecurityDescriptorToSecurityDescriptor)(
			SecurityDescriptor.c_str(),
			SDDL_REVISION_1,
			&m_Attributes.lpSecurityDescriptor,
			NULL ))
		{
			::FreeLibrary( hModule );
			throw std::runtime_error( "ConvertStringSecurityDescriptorToSecurityDescriptor failure" );
		}
	}

	::FreeLibrary( hModule );
}

namespace
{

struct RICH_GAME_MEDIA_HEADER
{
    DWORD				dwMagicNumber;
    DWORD				dwHeaderVersion;
    DWORD				dwHeaderSize;
    /*QWORD*/ULONG64    qwThumbnailOffset;
    DWORD				dwThumbnailSize;
    GUID				guidGameId;
    WCHAR				szGameName[RM_MAXLENGTH];
    WCHAR				szSaveName[RM_MAXLENGTH];
    WCHAR				szLevelName[RM_MAXLENGTH];
    WCHAR				szComments[RM_MAXLENGTH];
};

// Get the files and directories directly under the specified directory
void GetPathElements( std::string const& Path, std::vector<std::string>& Elements )
{
	assert( !CFilePath::IsDirectorySeparator( *Path.rbegin() ));

	std::string const Search = Path + "\\*";
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = ::FindFirstFile( Search.c_str(), &FindFileData );
	while (hFind != INVALID_HANDLE_VALUE)
	{
		//Ignore special entries
		if (FindFileData.cFileName[0] != '.')
		{
			Elements.push_back( FindFileData.cFileName );
		}

		if (!::FindNextFile( hFind, &FindFileData ))
		{
			break;
		}
	}
	::FindClose( hFind );
}

void DeleteRecursive( std::string const& Path )
{
	assert( !CFilePath::IsDirectorySeparator( *Path.rbegin() ));

	DWORD dwFileAttributes = ::GetFileAttributes( Path.c_str() );
	
	if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
	{
		std::string Message( "CSaveFile::Delete: file '" );
		Message += Path;
		Message += "' does not exist";
		throw std::runtime_error( Message );
	}

	if (!(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		BOOL Result = ::DeleteFile( Path.c_str() );
		if (Result == FALSE)
		{
			std::string Message( "DeleteFile failed for '" );
			Message += Path;
			Message += '\'';
			throw std::runtime_error( Message );
		}

		return;
	}

	std::vector<std::string> Elements;
	GetPathElements( Path, Elements );

	uint32 const ElementCount = static_cast<uint32>( Elements.size() );
	for (uint32 i = 0; i < ElementCount; ++i)
	{
		DeleteRecursive( Path + CFilePath::DirectorySeparatorChar + Elements[i] );
	}

	if (::RemoveDirectory( Path.c_str() ) == FALSE)
	{
		throw std::runtime_error( "RemoveDirectory failure" );
	}
}

std::string GetParentPath( std::string const& Path )
{
	assert( !Path.empty() );
	assert( !CFilePath::IsDirectorySeparator( *Path.begin() ));
	assert( !CFilePath::IsDirectorySeparator( *Path.rbegin() ));
	for (int32 i = static_cast<int32>( Path.size() - 2 ); i >= 0; --i)
	{
		if (CFilePath::IsDirectorySeparator( Path[i] ))
		{
			return Path.substr( 0, i );
		}
	}

	throw std::runtime_error( "CSaveFile::GetParentPath failure" );
}

void CreateSupportingDirectories( std::string const& Path, int32 Depth )
{
	assert( Depth >= 1 );

	if (Depth == 1)
	{
		return;
	}

	std::string const ParentPath = GetParentPath( Path );
	CreateSupportingDirectories( ParentPath, Depth - 1 );

	SECURITY_ATTRIBUTES SecurityAttributes;
	CSaveFileManager::GetInstance().GetSecurityAttributes( SecurityAttributes );
	
	if (::CreateDirectory( ParentPath.c_str(), 
		const_cast<LPSECURITY_ATTRIBUTES>( &SecurityAttributes ) ) == FALSE)
	{
		DWORD const dwError = ::GetLastError();
		if (dwError != ERROR_ALREADY_EXISTS)
		{
			throw std::runtime_error( "CreateDirectory failure" );
		}
	}
}

void DeleteSupportingDirectories( std::string const& Path_, int32 Depth )
{
	assert( Depth >= 1 );

	std::string Path = Path_;
	for (; Depth > 1; --Depth)
	{
		Path = GetParentPath( Path );

		DWORD const dwFileAttributes = ::GetFileAttributes( Path.c_str() );
		if (dwFileAttributes != INVALID_FILE_ATTRIBUTES)
		{
			//Directory/file exists
			if (::RemoveDirectory( Path.c_str() ) == FALSE)
			{
				DWORD const dwError = ::GetLastError();
				if (dwError == ERROR_DIR_NOT_EMPTY)
				{
					return; //Directory not empty; stop recursion
				}

				throw std::runtime_error( "RemoveDirectory failure" );
			}
		}
	}
}

} //anonymous namespace

std::string __fastcall CSaveFile::GetRootPath( EType Type )
{
	if (Type < 0 || Type >= KTypeCount)
	{
		throw std::invalid_argument( "CSaveFile::GetRootPath called with invalid Type" );
	}

	std::string Result = CSaveFileManager::GetInstance().GetProductRoot();

	if (Type == KTypeCache)
	{
		Result += "\\Cache";
	} else {
		Result += "\\Persistent";
	}

	return Result;
}

//Eliminate "." and ".." path components, check for invalid characters and empty file name, 
//and use standard directory separator ('\\').
void RectifyFileName( std::string& FileName )
{
	//Split the filename into components
	std::vector<string> Components;
	{
		int32 Prev = 0;

		int32 Size = static_cast<int32>( FileName.size() );
		for (int32 i = 0; i <= Size; ++i)
		{
			if (i < Size && !CFilePath::IsValidPathCharacter( FileName[i] ))
			{
				throw CArgumentException( "CSaveFile::CSaveFile FileName contains invalid character" );
			}

			if (i == Size || CFilePath::IsDirectorySeparator( FileName[i] ))
			{
				std::string Component = FileName.substr( Prev, i - Prev );
				Prev = i + 1;

				if (Component.empty())
				{
					throw std::invalid_argument( "CSaveFile::CSaveFile FileName contains spurious slashes" );
				}
				
				if (isspace( Component[0] ))
				{
					throw std::invalid_argument( "CSaveFile::CSaveFile FileName contains invalid whitespace" );
				}

				if (Component[0] == '.')
				{
					if (Component == ".")
					{
						continue;
					}

					if (Component == "..")
					{
						if (!Components.empty())
						{
							Components.pop_back();
							continue;
						}
					}

					throw std::invalid_argument( "CSaveFile::CSaveFile FileName contains invalid period" );
				}

				Components.push_back( Component );
			}
		}
	}

	int32 const ComponentCount = static_cast<int32>( Components.size() );
	if (!ComponentCount)
	{
		throw std::invalid_argument( "CSaveFile::CSaveFile FileName is empty" );
	}

	//Reconstitute file name
	FileName = Components[0];
	for (int32 i = 1; i < ComponentCount; ++i)
	{
		FileName += CFilePath::DirectorySeparatorChar;
		FileName += Components[i];
	}
}

CSaveFile::CSaveFile( std::string const& FileName, EType Type )
	: m_FileName( FileName )
	, m_Type( Type )
{
	if (Type < 0 || Type >= KTypeCount)
	{
		throw std::invalid_argument( "CSaveFile::CSaveFile called with invalid Type" );
	}

	RectifyFileName( m_FileName );

#if 0
	if (Type == KTypeSavedGame)
	{
		m_FileName += '.';
		m_FileName += CSaveFileManager::GetInstance().GetProductCodeName();
		m_FileName += "SavedGame";
	}
#endif

	std::string const Root = CSaveFileManager::GetInstance().GetRoot();

	//Set m_Path
	m_Path = GetRootPath( m_Type );

	m_Path += CFilePath::DirectorySeparatorChar;
	assert( !m_FileName.empty() );
	m_Path += m_FileName;
	
	if (m_Path.size() > MAX_PATH && CSaveFileManager::GetInstance().GetIsWindows2KOrBetter())
	{
		//Requirement of Windows 95/98/ME violated
		throw std::runtime_error( "CSaveFile::CSaveFile path too long" );
	}

	//Compute number of directories, excluding the root, that would be supporting this save file
	m_Depth = 
		static_cast<int32>( 
			+ std::count( m_Path.begin(), m_Path.end(), CFilePath::DirectorySeparatorChar )
			+ std::count( m_Path.begin(), m_Path.end(), CFilePath::AltDirectorySeparatorChar )
			- std::count( Root.begin(), Root.end(), CFilePath::DirectorySeparatorChar )
			- std::count( Root.begin(), Root.end(), CFilePath::AltDirectorySeparatorChar )
		);
	assert( m_Depth >= 1 );
}

CSaveFile::CSaveFile( CSaveFile const& SaveFile )
	: m_FileName( SaveFile.m_FileName )
	, m_Type( SaveFile.m_Type )
	, m_Path( SaveFile.m_Path )
{
}

CSaveFile& CSaveFile::operator=( CSaveFile const& SaveFile )
{
	m_FileName			= SaveFile.m_FileName;
	m_Type				= SaveFile.m_Type;
	m_Path				= SaveFile.m_Path;
	return *this;
}

CSaveFile::~CSaveFile()
{
}

bool CSaveFile::operator==( CSaveFile const& SaveFile ) const
{
	return m_Path == SaveFile.m_Path;
}

bool CSaveFile::operator!=( CSaveFile const& SaveFile ) const
{
	return m_Path != SaveFile.m_Path;
}

std::string CSaveFile::GetFileName() const
{
	return m_FileName;
}

CSaveFile::EType CSaveFile::GetType() const
{
	return m_Type;
}

std::string CSaveFile::GetPath() const
{
	return m_Path;
}

bool CSaveFile::FileExists() const
{
	DWORD dwFileAttributes = ::GetFileAttributes( m_Path.c_str() );
	return dwFileAttributes != INVALID_FILE_ATTRIBUTES;
}

HANDLE CSaveFile::Open(
	DWORD dwDesiredAccess, 
	DWORD dwShareMode, 
	DWORD dwCreationDisposition, 
	DWORD dwFlagsAndAttributes,
	bool Inheritable ) const
{
	SECURITY_ATTRIBUTES SecurityAttributes;
	CSaveFileManager::GetInstance().GetSecurityAttributes( SecurityAttributes, Inheritable );

	if (dwCreationDisposition == CREATE_ALWAYS || dwCreationDisposition == CREATE_NEW ||
		dwCreationDisposition == OPEN_ALWAYS)
	{
		CreateSupportingDirectories( m_Path, m_Depth );
	}

	HANDLE const hFile = ::CreateFile( m_Path.c_str(),
		dwDesiredAccess,
		dwShareMode,
		const_cast<LPSECURITY_ATTRIBUTES>( &SecurityAttributes ),
		dwCreationDisposition,
		dwFlagsAndAttributes,
		NULL );
	
	return hFile;
}

HANDLE CSaveFile::Open( FileMode::Enum Mode ) const
{
	return Open( Mode, (Mode == FileMode::Append ? FileAccess::Write : FileAccess::ReadWrite) );
}

//Create a handle to the specified save file. INVALID_HANDLE_VALUE is returned on failure.
HANDLE CSaveFile::Open( FileMode::Enum Mode,
	FileAccess::Enum Access, FileShare::Enum Share, unsigned int Flags, bool Async ) const
{
	DWORD dwDesiredAccess;
	switch (Access)
	{
	case FileAccess::Read:		dwDesiredAccess = GENERIC_READ; break;
	case FileAccess::Write:		dwDesiredAccess = GENERIC_WRITE; break;
	case FileAccess::ReadWrite: dwDesiredAccess = GENERIC_READ | GENERIC_WRITE; break;
	default:
		throw std::invalid_argument( "CSaveFile::Open Access" );
	}

	bool SeekToEnd = false;
	
	if (Mode == FileMode::Append)
	{
		Mode = FileMode::OpenOrCreate;
		SeekToEnd = true;
	}

	DWORD dwCreationDisposition;
	switch (Mode)
	{
		case FileMode::Append:			dwCreationDisposition = OPEN_ALWAYS; break;
		case FileMode::Create:			dwCreationDisposition = CREATE_ALWAYS; break;
		case FileMode::CreateNew:		dwCreationDisposition = CREATE_NEW; break;
		case FileMode::Open:			dwCreationDisposition = OPEN_EXISTING; break;
		case FileMode::OpenOrCreate:	dwCreationDisposition = OPEN_ALWAYS; break;
		case FileMode::Truncate:		dwCreationDisposition = TRUNCATE_EXISTING; break;
		default:
			throw std::invalid_argument( "CSaveFile::Open Mode" );
	}

	if (dwCreationDisposition == CREATE_ALWAYS || dwCreationDisposition == CREATE_NEW || 
		dwCreationDisposition == OPEN_ALWAYS)
	{
		CreateSupportingDirectories( m_Path, m_Depth );
	}

	if (Async)
	{
		Flags |= FILE_FLAG_OVERLAPPED;
	}

	SECURITY_ATTRIBUTES SecurityAttributes;
	CSaveFileManager::GetInstance().GetSecurityAttributes( SecurityAttributes, 
		(Share & FileShare::Inheritable) != 0 );

	HANDLE const hFile = ::CreateFile( m_Path.c_str(),
		dwDesiredAccess,
		Share,
		const_cast<LPSECURITY_ATTRIBUTES>( &SecurityAttributes ),
		dwCreationDisposition,
		Flags | FILE_ATTRIBUTE_NORMAL,
		NULL );

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return hFile;
	}

	if (SeekToEnd)
	{
		DWORD const Result = ::SetFilePointer( hFile, 0, NULL, FILE_END );
		if (Result == INVALID_SET_FILE_POINTER) //Possible error
		{
			DWORD const dwError = ::GetLastError();
			if (dwError != NO_ERROR) 
			{
				throw std::runtime_error( "SetFilePointer failure" );
			}
		}
	}

	return hFile;
}

void CSaveFile::CreateDirectory() const
{
	CreateSupportingDirectories( m_Path, m_Depth );

	SECURITY_ATTRIBUTES SecurityAttributes;
	CSaveFileManager::GetInstance().GetSecurityAttributes( SecurityAttributes );

	if (::CreateDirectory( m_Path.c_str(), &SecurityAttributes ) == FALSE)
	{
		std::string Message( "CreateDirectory failed for path '" );
		Message += m_Path;
		Message += '\'';
		throw std::runtime_error( Message );
	}
}

void CSaveFile::Delete() const
{
	DeleteRecursive( m_Path );
	DeleteSupportingDirectories( m_Path, m_Depth );
}


void SetAttributes( std::string const& Path )
{
	SECURITY_ATTRIBUTES SecurityAttributes;
	CSaveFileManager::GetInstance().GetSecurityAttributes( SecurityAttributes );

	BOOL Result = ::SetFileSecurity( Path.c_str(), PROTECTED_DACL_SECURITY_INFORMATION,
		SecurityAttributes.lpSecurityDescriptor );
	if (Result == FALSE)
	{
		std::string Message( "SetFileSecurity failed for file '" );
		Message += Path;
		Message += '\'';
		throw std::runtime_error( Message );
	}
}

void SetAttributesRecursive( std::string const& Path )
{
	SetAttributes( Path );

	std::vector<std::string> Elements;
	GetPathElements( Path, Elements );

	int32 const ElementCount = static_cast<int32>( Elements.size() );
	for (int32 i = 0; i < ElementCount; ++i)
	{
		SetAttributesRecursive( Path + CFilePath::DirectorySeparatorChar + Elements[i] );
	}
}

void CSaveFile::SetAttributes( bool Recurse ) const
{
	if (Recurse)
	{
		SetAttributesRecursive( m_Path );
	} else {
		IO::SetAttributes( m_Path );
	}
}

void GetResourceData( HMODULE hModule, char const Data[], std::vector<uint8>& Result )
{
	HRSRC hResInfo = ::FindResource( hModule, Data, "DATA" );
	if (hResInfo == NULL)
	{
		throw std::runtime_error( "FindResource failed on GDF.DLL" );
	}
	
	HGLOBAL hResData = ::LoadResource( hModule, hResInfo );
	if (hResData == NULL)
	{
		throw std::runtime_error( "LoadResource failed on GDF.DLL" );
	}

	uint8 const* pData = static_cast<uint8*>( ::LockResource( hResData ) );
	if (pData == NULL)
	{
		throw std::runtime_error( "LockResource failed on GDF.DLL" );
	}

	DWORD SizeOfResource = ::SizeofResource( hModule, hResInfo );

	Result.resize( SizeOfResource );
	std::copy( pData, pData + SizeOfResource, &Result[0] );
}

#if 0
//Write a Rich Save Game header to the specified file. The header supports
//the Rich Save Game system used by Windows Vista Explorer and Windows Vista 
//Desktop Search. If used, this should be written to the beginning of your
//save file. This function depends on the existence of a GDF.DLL in your
//executable's directory containing Vista Game Explorer-compliant XML and 
//thumbnail data.
/*static*/ void __fastcall CSaveFile::WriteRichHeader(
	HANDLE hFile, 
	std::string const& SaveName, //Name of save file (not including the game name)
	std::string const& LevelName = "" )
{
	//TODO make sure the correct runtime attribute is used for GameName
	std::string const GameName = CSaveFileManager::GetInstance().GetProductDisplayName();

	std::string const Comments = "This is a saved game for " + GameName;

	std::vector<uint8> XMLData;
	std::vector<uint8> ThumbnailData;
	std::wstring GameIDString;

	{
		HMODULE hModule = ::LoadLibraryEx( "GDF.DLL", NULL, LOAD_LIBRARY_AS_DATAFILE );
		if (hModule == NULL)
		{
			throw std::runtime_error( "LoadLibraryEx failed to load GDF.DLL" );
		}

		try
		{
			GetResourceData( hModule, /*ID_GDF_XML_STR*/ "__GDF_XML", XMLData );
			GetResourceData( hModule, /*ID_GDF_THUMBNAIL_STR*/ "__GDF_THUMBNAIL", ThumbnailData );
		} catch (...) {
			::FreeLibrary( hModule );
			throw;
		}
		::FreeLibrary( hModule );
	}

	{
		CMemoryStream XMLStream( XMLData );
		CXmlReader XMLReader( &XMLStream );

		try
		{
			while (!XMLReader.IsStartElement())	if (!XMLReader.Read()) throw 0;
			
			if (XMLReader.NodeType() != Xml::NodeType::StartElementNode ||
				XMLReader.Name() != L"GameDefinitionFile")
			{
				throw 0;
			}

			do {if (!XMLReader.Read()) throw 0;} while (!XMLReader.IsStartElement());
		
			if (XMLReader.NodeType() != Xml::NodeType::StartElementNode ||
				XMLReader.Name() != L"GameDefinition")
			{
				throw 0;
			}

			XMLReader.CXmlReader::GetAttribute( L"gameID", GameIDString );
		} catch (int) {
			throw std::runtime_error( "Corrupt XML data in GDF.DLL" );
		}
	}

	//Compute game ID
	GUID GameID;
	GetGUID( GameIDString, GameID );

	RICH_GAME_MEDIA_HEADER Header;
	::ZeroMemory( &Header, sizeof Header );
	Header.dwMagicNumber		= RM_MAGICNUMBER;
	Header.dwHeaderVersion		= 1;
	Header.dwHeaderSize			= sizeof Header;
	//Place the thumbnail immediately after the header so user's can skip everything when reading in.
	Header.qwThumbnailOffset	= 0; 
	Header.dwThumbnailSize		= static_cast<DWORD>( ThumbnailData.size() );
	Header.guidGameId			= GameID;
	{
		std::wstring const& WGameName = AnsiToWide( GameName );
		std::copy( WGameName .begin(), WGameName .size() > RM_MAXLENGTH - 1 ? WGameName.begin() + (RM_MAXLENGTH - 1) : WGameName .end(), Header.szGameName );
	}
	{
		std::wstring WSaveName  = AnsiToWide( SaveName );
		std::copy( WSaveName .begin(), WSaveName .size() > RM_MAXLENGTH - 1 ? WSaveName.begin() + (RM_MAXLENGTH - 1) : WSaveName .end(), Header.szSaveName );
	}
	{
		std::wstring WLevelName = AnsiToWide( LevelName );
		std::copy( WLevelName.begin(), WLevelName.size() > RM_MAXLENGTH - 1 ? WLevelName.begin() + (RM_MAXLENGTH - 1) : WLevelName.end(), Header.szLevelName );
	}
	{
		std::wstring WComments  = AnsiToWide( Comments );
		std::copy( WComments .begin(), WComments .size() > RM_MAXLENGTH - 1 ? WComments.begin() + (RM_MAXLENGTH - 1) : WComments .end(), Header.szComments );
	}

	DWORD NumberOfBytesWritten = 0;
	BOOL Result = ::WriteFile( hFile, &Header, sizeof Header, &NumberOfBytesWritten, NULL );
	if (Result == FALSE || NumberOfBytesWritten != sizeof Header)
	{
		throw std::runtime_error( "WriteFile failure" );
	}

	NumberOfBytesWritten = 0;
	Result = ::WriteFile( hFile, &ThumbnailData[0], static_cast<DWORD>( ThumbnailData.size() ), 
		&NumberOfBytesWritten, NULL );
	if (Result == FALSE || NumberOfBytesWritten != ThumbnailData.size())
	{
		throw std::runtime_error( "WriteFile failure" );
	}
}

//Skip over the Rich Save Game header at the beginning of the specified file.
//This only works if the rich header was written with CSaveFile::WriteRichHeader.
/*static*/ void __fastcall CSaveFile::SkipRichHeader( HANDLE hFile )
{
	RICH_GAME_MEDIA_HEADER Header;

	DWORD NumberOfBytesRead = 0;
	BOOL Result = ::ReadFile( hFile, &Header, sizeof Header, &NumberOfBytesRead, NULL );
	if (Result == FALSE || NumberOfBytesRead != sizeof Header)
	{
		throw std::runtime_error( "ReadFile failure" );
	}

	if (Header.dwMagicNumber		!= RM_MAGICNUMBER ||
		Header.dwHeaderVersion		!= 1 ||
		Header.dwHeaderSize			!= sizeof Header ||
		Header.qwThumbnailOffset	!= 0)
	{
		throw std::runtime_error( "CSaveFile::SkipRichHeader found corrupt or unsupported header" );
	}

	std::vector<uint8> Thumbnail( Header.dwThumbnailSize );
	NumberOfBytesRead = 0;
	Result = ::ReadFile( hFile, &Thumbnail[0], Header.dwThumbnailSize, &NumberOfBytesRead, NULL );
	if (Result == FALSE || NumberOfBytesRead != Header.dwThumbnailSize)
	{
		throw std::runtime_error( "ReadFile failure" );
	}
}
#endif

} // namespace IO
} // namespace System
} // namespace WT
