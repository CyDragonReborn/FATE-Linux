//
// ResourceStream.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/System/IO/ResourceStream.h"
#include "WT/System/IO/IOException.h"


namespace WT
{
namespace System
{
namespace IO
{


CResourceStream::CResourceStream()
{
}

CResourceStream::CResourceStream( HMODULE hModule, LPCTSTR pResName, LPCTSTR pResType )
{
	LoadResource( hModule, pResName, pResType );
}

/* virtual */
CResourceStream::~CResourceStream()
{
}

/* virtual */
void CResourceStream::LoadResource( HMODULE hModule, LPCTSTR pResName, LPCTSTR pResType )
{
	// Load the resource and get a pointer to the resource data
	// Note that there is no Unlock or Release functions for system resource data pointers and handles.
	// The system takes care of all resource clean up.
	HRSRC hResInfo = ::FindResource( hModule, pResName, pResType );
	if ( hResInfo == NULL )
	{
		throw CIOException( "CResourceStream::LoadResource - Unable to find resource" );
	}

	uint32 RsrcSize = ::SizeofResource( hModule, hResInfo );	// Resource data size in bytes
	if ( RsrcSize == 0 )
	{
		throw CIOException( "CResourceStream::LoadResource - Zero resource data size" );
	}

	HGLOBAL hRsrc = ::LoadResource( hModule, hResInfo );
	if ( hRsrc == NULL )
	{
		throw CIOException( "CResourceStream::LoadResource - Unable to load resource" );
	}

	uint8* pBuffer = static_cast<uint8*>( ::LockResource( hRsrc ) );
	if ( pBuffer == NULL )
	{
		throw CIOException( "CResourceStream::LoadResource - Unable to lock resource" );
	}

	// Copy the resource data to a memory stream
	m_MemoryStream.SetLength( RsrcSize );
	m_MemoryStream.Seek( 0, SeekOrigin::Begin );
	m_MemoryStream.Write( pBuffer, RsrcSize );
	m_MemoryStream.Seek( 0, SeekOrigin::Begin );
}

//
// Base class method overrides
//

/* virtual */
bool CResourceStream::CanRead() const
{
	return true;
}

/* virtual */
bool CResourceStream::CanSeek() const
{
	return true;
}

/* virtual */
bool CResourceStream::CanWrite() const
{
	return false;
}

/* virtual */
int32 CResourceStream::GetLength() const
{
	return m_MemoryStream.GetLength();
}

/* virtual */
void CResourceStream::SetPosition( int32 Position )
{
	m_MemoryStream.SetPosition( Position );
}

/* virtual */
int32 CResourceStream::GetPosition() const
{
	return m_MemoryStream.GetPosition();
}

/* virtual */
void CResourceStream::Close()
{
	return m_MemoryStream.Close();
}

/* virtual */
void CResourceStream::Flush()
{
	m_MemoryStream.Flush();
}

/* virtual */
int32 CResourceStream::Read( uint8 buffer[], int32 count )
{
	return m_MemoryStream.Read( buffer, count );
}

/* virtual */
int32 CResourceStream::ReadByte()
{
	return m_MemoryStream.ReadByte();
}

/* virtual */
int32 CResourceStream::Seek( int32 offset , SeekOrigin::Enum origin )
{
	return m_MemoryStream.Seek( offset, origin );
}

/* virtual */
void CResourceStream::SetLength( int32 length )
{
	throw CIOException( "CResourceStream::SetLength - Invalid resource stream operation" );
}

/* virtual */
void CResourceStream::Write ( uint8 const buffer[], int32 count )
{
	throw CIOException( "CResourceStream::Write - Invalid resource stream operation" );
}

/* virtual */
void CResourceStream::WriteByte ( uint8 value )
{
	throw CIOException( "CResourceStream::WriteByte - Invalid resource stream operation" );
}


} // namespace WT
} // namespace System
} // namespace WT
