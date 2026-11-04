/***************************************************************************************
 *
 * MarkerFile.cpp
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 2/13/2003: Jason Glover:	Created
 *
 ***************************************************************************************/

// -----------------------------------------------------------------
// Include Files
// -----------------------------------------------------------------
#include "WT/stdafx.h"
#include "WT/System/SystemException.h"
#include "WT/Audio/MarkerFile.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CMarkerFileReader)
WT_DEFINE_CLASS(CMarkerFileWriter)

///////////////////////////////////////////////////////////////////////
// CMarkerFileReader
///////////////////////////////////////////////////////////////////////
CMarkerFileReader::CMarkerFileReader() :
	m_File(INVALID_HANDLE_VALUE)
{
}

CMarkerFileReader::~CMarkerFileReader()
{
	CloseMarkerFile();
}

void CMarkerFileReader::LoadMarkerData( const std::string& FileName )
{
	MarkerFileHeader header;
	strncpy( header.Identifier, "wtmrkr", 6 );
	header.Version = KMarkerFileVersion;

	CloseMarkerFile();
	m_MarkerData.clear();

	m_File = CreateFile(	FileName.c_str(), 
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL );

	if( m_File == INVALID_HANDLE_VALUE )
	{
		WT_THROW( System::CRuntimeException, string( "CreateFile failure for " ) + FileName );
	}

	MarkerFileHeader header2;
	MarkerData markerdata;
	uint32 bytesProcessed;

	if (!ReadFile( m_File, &header2, sizeof(MarkerFileHeader), &bytesProcessed, NULL ))
	{
		CloseMarkerFile();
		WT_THROW( System::CRuntimeException, "ReadFile failure" );
	}

	if(	header.Identifier[0] != header2.Identifier[0] ||
		header.Identifier[1] != header2.Identifier[1] ||
		header.Identifier[2] != header2.Identifier[2] ||
		header.Identifier[3] != header2.Identifier[3] ||
		header.Identifier[4] != header2.Identifier[4] ||
		header.Identifier[5] != header2.Identifier[5]	)
	{
		CloseMarkerFile();
		WT_THROW( System::CRuntimeException, "Corrupt file" );
	}

	if( header.Version != header2.Version )
	{
		CloseMarkerFile();
		WT_THROW( System::CRuntimeException, "Unsupported file version" );
	}

	while( ReadFile( m_File, &markerdata, sizeof(MarkerData), &bytesProcessed, NULL ) &&
		( bytesProcessed == sizeof(MarkerData)) )
	{
		markerdata.MarkerName[KMarkerNameLen-1] = 0;
		m_MarkerData.push_back( markerdata );
	}

	CloseMarkerFile();
}

void CMarkerFileReader::CloseMarkerFile()
{
	if( m_File != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_File );
		m_File = INVALID_HANDLE_VALUE;
	}
}

uint32 CMarkerFileReader::GetNumMarkers()
{
	return static_cast<uint32>(m_MarkerData.size());
}

void CMarkerFileReader::GetMarkerInfo( uint32 Index, MarkerData& MarkerData )
{
	if( Index < 0 || Index >= m_MarkerData.size() )
	{
		WT_THROW( System::CArgumentOutOfRangeException, "Index" );
	}

	memcpy( &MarkerData, &m_MarkerData[Index], sizeof MarkerData );
}


///////////////////////////////////////////////////////////////////////
// CMarkerFileWriter
///////////////////////////////////////////////////////////////////////
CMarkerFileWriter::CMarkerFileWriter() :
	m_File(INVALID_HANDLE_VALUE)
{
}

CMarkerFileWriter::~CMarkerFileWriter()
{
	CloseMarkerFile();
}

void CMarkerFileWriter::OpenMarkerFile( const std::string& FileName )
{
	MarkerFileHeader header;
	strncpy( header.Identifier, "wtmrkr", 6 );
	header.Version = KMarkerFileVersion;

	CloseMarkerFile();
	m_FileName.clear();

	m_File = CreateFile(	FileName.c_str(), 
							GENERIC_WRITE|GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL );

	if( m_File == INVALID_HANDLE_VALUE )
	{
		WT_THROW( System::CRuntimeException, string( "CreateFile failure for " ) + FileName );
	}
	m_FileName = FileName;

	uint32 bytesProcessed;
	if( !WriteFile( m_File, &header, sizeof(MarkerFileHeader), &bytesProcessed, NULL ) )
	{
		CloseMarkerFile();
		::DeleteFile( FileName.c_str() );
		WT_THROW( System::CRuntimeException, string( "WriteFile failure for " ) + FileName );
	}
}

void CMarkerFileWriter::CloseMarkerFile()
{
	if( m_File != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_File );
		m_File = INVALID_HANDLE_VALUE;
	}
}

void CMarkerFileWriter::WriteMarkerData( const MarkerData& MarkerData )
{
	//fwrite( MarkerData, sizeof(MarkerData), 1, m_pFile );
	if( m_File == INVALID_HANDLE_VALUE )
	{
		WT_THROW( System::CRuntimeException, "No file open" );
	}

	uint32 bytesProcessed;
	if( !WriteFile( m_File, &MarkerData, sizeof(MarkerData), &bytesProcessed, NULL ) )
	{
		CloseMarkerFile();
		::DeleteFile( m_FileName.c_str() );
		m_FileName.clear();
		WT_THROW( System::CRuntimeException, "WriteFile failure" );
	}
}

}	// namespace Audio
}	// namespace WT
