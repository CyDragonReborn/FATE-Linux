/***************************************************************************************
 *
 * MarkerFile.h
 *
 * Copyright (C) 2003-2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 2/13/2003: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/System/Object.h"
#include "WT/Audio/AudioException.h"

namespace WT
{
namespace Audio
{

//
static const int32 KMarkerFileVersion = 1;

//
typedef struct MarkerFileHeader
{
	int8	Identifier[6];
	int32	Version;
}	MarkerFileHeader;

//
#define KMarkerNameLen	32
typedef struct MarkerData
{
	int8	MarkerName[KMarkerNameLen];
	uint32	MarkerOffset;
}	MarkerData;


typedef System::SmartPointer<class CMarkerFileReader> HMarkerFileReader;
class CMarkerFileReader : public System::CObject
{
public:

	WT_CLASS( CMarkerFileReader, System::CObject );

	CMarkerFileReader();
	virtual	~CMarkerFileReader( );

	void				LoadMarkerData( const std::string& FileName );
	uint32				GetNumMarkers();
	void				GetMarkerInfo( uint32 Index, MarkerData& MarkerData );

private:
	void					CloseMarkerFile();

	HANDLE					m_File;
	MarkerFileHeader		m_header;
	std::vector<MarkerData>	m_MarkerData;
};

//
typedef System::SmartPointer<class CMarkerFileWriter> HMarkerFileWriter;
class CMarkerFileWriter : public System::CObject
{
public:
   
	WT_CLASS( CMarkerFileWriter, System::CObject );

	CMarkerFileWriter();
	virtual ~CMarkerFileWriter( );

	void				OpenMarkerFile( const std::string& FileName );
	void				CloseMarkerFile();
	void				WriteMarkerData( const MarkerData& MarkerData );

private:

	HANDLE		m_File;
	std::string	m_FileName;
};

}	// namespace Audio
}	// namespace WT


