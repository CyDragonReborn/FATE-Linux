//
// ResourceStream.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// Overview: TODO
//
#pragma once

#include "WT\System\Platform.h"
#include "WT\System\IO\Stream.h"
#include "WT\System\IO\MemoryStream.h"
#include <windows.h>


namespace WT
{
namespace System
{
namespace IO
{


typedef SmartPointer<class CResourceStream> HResourceStream;
class CResourceStream : public CStream
{
public:
	WT_CLASS( CResourceStream, CStream )

	CResourceStream();
	CResourceStream( HMODULE hModule, LPCTSTR pResName, LPCTSTR pResType );
	virtual ~CResourceStream();

	virtual void LoadResource( HMODULE hModule, LPCTSTR pResName, LPCTSTR pResType );

	//
	// Base class method overrides
	//

	virtual bool CanRead() const;
	virtual bool CanSeek() const;
	virtual bool CanWrite() const;
	virtual int32 GetLength() const;
	virtual void SetPosition( int32 Position );
	virtual int32 GetPosition() const;

	//BeginRead
	//BeginWrite
	virtual void Close();
	//EndRead
	//EndWrite
	virtual void Flush();

	virtual int32 Read( uint8 buffer[], int32 count );
	virtual int32 ReadByte();
	virtual int32 Seek( int32 offset, SeekOrigin::Enum origin );

	virtual void SetLength( int32 length );
	virtual void Write( uint8 const buffer[], int32 count );
	virtual void WriteByte( uint8 value );

private: // data

	CMemoryStream	m_MemoryStream;
};


} // namespace IO
} // namespace System
} // namespace WT
