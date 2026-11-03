#pragma once

#include "WT/System/IO/Stream.h"
#include <string>
#include "windows.h"

namespace WT
{
namespace System
{
namespace IO
{

namespace FileMode
{
	enum Enum
	{
		Append,
		Create,
		CreateNew,
		Open,
		OpenOrCreate,
		Truncate
	};
}

namespace FileAccess
{
	enum Enum
	{
		Read      = 1,
		ReadWrite = 3,
		Write     = 2
	};
}

namespace FileShare
{
	enum Enum
	{
		Inheritable = 16,
		None        = 0,
		Read        = 1,
		ReadWrite   = 3,
		Write       = 2
	};
}

namespace FileFlags
{
	int32 const KSequentialScan = FILE_FLAG_SEQUENTIAL_SCAN;
	int32 const KRandomAccess   = FILE_FLAG_RANDOM_ACCESS;
}

typedef SmartPointer<class CFileStream> HFileStream;
class CFileStream : public CStream
{
public:
	WT_CLASS_NO_CREATOR( CFileStream , CStream )

	// throws CIOException( "bad file handle" ) if the file cannot be opened
	CFileStream ( const string& path , FileMode::Enum mode );

	// throws CIOException( "bad file handle" ) if the file cannot be opened
	CFileStream ( const string& path , FileMode::Enum mode , FileAccess::Enum access, 
		FileShare::Enum share = FileShare::Read,
		uint32 Flags = 0,
		int32 bufferSize = DefaultBufferSize, 
		bool useAsync = false );

	virtual ~CFileStream();

	virtual bool CanRead() const;
	virtual bool CanSeek() const;
	virtual bool CanWrite() const;
	virtual void* GetHandle();
	virtual bool IsAsync() const;
	virtual int32 GetLength() const;
	virtual void SetPosition( int32 Position );
	virtual int32 GetPosition() const;
	string const& GetName() const;

	//BeginRead
	//BeginWrite
	virtual void Close();
	//EndRead
	//EndWrite
	virtual void Flush();
	virtual int32 Read ( uint8 buffer[], int32 count );
	virtual int32 ReadByte ();
	virtual int32 Seek ( int32 offset , SeekOrigin::Enum origin );
	virtual void SetLength ( int32 length );
	virtual void Write ( uint8 const buffer[], int32 count );
	virtual void WriteByte ( uint8 value );

private:
	void Init ( const string& path , FileMode::Enum mode , FileAccess::Enum access , FileShare::Enum share , 
		uint32 Flags, int32 bufferSize , bool useAsync , bool fromProxy );

	static const int32 DefaultBufferSize = 4096;
	uint8* m_Buffer;
	string m_FileName;
	bool m_IsOpen;
	bool m_IsAsync;
	static const bool gm_CanUseAsync = false;
	bool m_CanRead;
	bool m_CanWrite;
	bool m_CanSeek;
	bool m_OwnsHandle;
	bool m_IsPipe;
	void* m_Handle;
};

}
}
}