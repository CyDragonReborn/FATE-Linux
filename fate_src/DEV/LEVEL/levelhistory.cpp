/*
	levelhistory.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/
#include <assert.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/quadtree.h"

#include "../ITEM/item.h"
#include "../CHARACTER/character.h"
#include "levelhistory.h"

CLevelHistory::CLevelHistory( int32 Level,		// level the map is for
								uint32 Width,		// width of map
								uint32 Height ) :	// height of map
													m_Level( Level ),
													m_Width( Width ),
													m_Height( Height ),
													m_LastPopulationTime( 0 ),
													m_VisibilityData ( NULL )
{

	m_VisibilityData = new bool*[m_Width];
	for( uint32 i = 0; i < m_Width; i++ )
	{
		m_VisibilityData[i] = new bool[m_Height];
	}

	ClearVisibilityData();

} // CLevelHistory::CLevelHistory()

CLevelHistory::CLevelHistory( CLevelHistory& pMap ) :	// map to copy
				m_Level( pMap.Level() ),
				m_Width( pMap.Width() ),
				m_Height( pMap.Height() )

{
	m_VisibilityData = new bool*[m_Width];
	for( uint32 i = 0; i < m_Width; i++ )
	{
		m_VisibilityData[i] = new bool[m_Height];
	}
	ClearVisibilityData();

	Copy( pMap );

} // CLevelHistory::CLevelHistory()

void CLevelHistory::AddItem( CItem& pItem )			// world item to add to history
{
	CItemSaveInstance* pInstance = new CItemSaveInstance();
	pItem.FillInstance( *pInstance, 0, kFalse );
	m_pItemHistory.push_back( pInstance );
} // CLevelHistory::AddItem( )

void CLevelHistory::AddCharacter( CCharacter& pCharacter,	// character to add to history
								  CLevel& pLevel )
{
	CCharacterSaveInstance* pInstance = new CCharacterSaveInstance();
	pCharacter.FillInstance( *pInstance, pLevel );
	m_pCharacterHistory.push_back( pInstance );
} // CLevelHistory::AddCharacter()

void CLevelHistory::Copy( CLevelHistory& pMap ) 	// map to copy
{
	if( pMap.Width() == m_Width &&
		pMap.Height() == m_Height )
	{
		for( uint32 x = 0; x < m_Width; x++ )
		{
			for( uint32 y = 0; y < m_Height; y++ )
			{
				SetVisibility( x, y, pMap.Visible( x, y ) );
			}
		}
	}
} // CLevelHistory::Copy()

CLevelHistory::~CLevelHistory( void )
{
	if( m_VisibilityData != NULL )
	{
		for( uint32 i = 0; i < m_Width; i++ )
		{
			DELETE_ARRAY_SAFELY( m_VisibilityData[i] );
		}
	}
	DELETE_ARRAY_SAFELY( m_VisibilityData );

} // CLevelHistory::~CLevelHistory()

void CLevelHistory::ClearVisibilityData( void )
{
	for( uint32 i = 0; i < m_Width; i++ )
	{
		for( uint32 j = 0; j < m_Height; j++ )
		{
			m_VisibilityData[i][j] = kFalse;
		}
	}

	for( i = 0; i < m_pCharacterHistory.size(); i++ )
	{
		DELETE_SAFELY( m_pCharacterHistory[i] );
	}
	m_pCharacterHistory.clear();

	for( i = 0; i < m_pItemHistory.size(); i++ )
	{
		DELETE_SAFELY( m_pItemHistory[i] );
	}
	m_pItemHistory.clear();
} // CLevelHistory::ClearVisibilityData()

void CLevelHistory::DiscoverAll( void )
{
	for( uint32 i = 0; i < m_Width; i++ )
	{
		for( uint32 j = 0; j < m_Height; j++ )
		{
			m_VisibilityData[i][j] = kTrue;
		}
	}
} // CLevelHistory::DiscoverAll()

bool CLevelHistory::Visible( int32 X,	// visibility cell to check x
							  int32 Y )	// visibility cell to check y	
{
	if( X > 0 && Y > 0 && X < (int32)m_Width - 1 && Y < (int32)m_Height - 1 )
	{
		return m_VisibilityData[X][Y];
	}
	return kFalse;
} // CLevelHistory::Visible()

void CLevelHistory::SetVisibility( int32 X,		// visibility cell to check x
									int32 Y,		// visibility cell to check y	
									bool Value )	// visibility value
{
	if( X > 0 && Y > 0 && X < (int32)m_Width - 1 && Y < (int32)m_Height - 1 )
	{
		m_VisibilityData[X][Y] = Value;
	}
} // CLevelHistory::SetVisibility()

void CLevelHistory::Save( FILE* pFile )	// open file to save to
{

	fwrite( &m_Level, sizeof( uint32 ), 1, pFile );
	fwrite( &m_Width, sizeof( uint32 ), 1, pFile );
	fwrite( &m_Height, sizeof( uint32 ), 1, pFile );
	fwrite( &m_LastPopulationTime, sizeof( float32 ), 1, pFile );


	for( uint32 i = 0; i < m_Width; i++ )
	{
		for( uint32 j = 0; j < m_Height; j++ )
		{
			fwrite( &m_VisibilityData[i][j], sizeof( bool ), 1, pFile );
		}
	}

	uint32 Size = m_pCharacterHistory.size();
	fwrite( &Size, sizeof( uint32 ), 1, pFile );
	for( i = 0; i < Size; i++ )
	{
		m_pCharacterHistory[i]->Save( pFile );
	}
	Size = m_pItemHistory.size();
	fwrite( &Size, sizeof( uint32 ), 1, pFile );
	for( i = 0; i < Size; i++ )
	{
		m_pItemHistory[i]->Save( pFile );
	}
} // CLevelHistory::Save()

void CLevelHistory::Load( FILE* pFile )	// open file to load from
{
	for( uint32 i = 0; i < m_Width; i++ )
	{
		DELETE_ARRAY_SAFELY( m_VisibilityData[i] );
	}
	DELETE_ARRAY_SAFELY( m_VisibilityData );


	fread( &m_Level, sizeof( uint32 ), 1, pFile );
	fread( &m_Width, sizeof( uint32 ), 1, pFile );
	fread( &m_Height, sizeof( uint32 ), 1, pFile );
	fread( &m_LastPopulationTime, sizeof( float32 ), 1, pFile );

	m_VisibilityData = new bool*[m_Width];
	for( i = 0; i < m_Width; i++ )
	{
		m_VisibilityData[i] = new bool[m_Height];
	}

	for( i = 0; i < m_Width; i++ )
	{
		for( uint32 j = 0; j < m_Height; j++ )
		{
			fread( &m_VisibilityData[i][j], sizeof( bool ), 1, pFile );
		}
	}

	uint32 Size = m_pCharacterHistory.size();
	fread( &Size, sizeof( uint32 ), 1, pFile );
	for( i = 0; i < Size; i++ )
	{
		CCharacterSaveInstance* pInstance = new CCharacterSaveInstance();
		pInstance->Load( pFile );
		m_pCharacterHistory.push_back( pInstance );
	}
	Size = m_pItemHistory.size();
	fread( &Size, sizeof( uint32 ), 1, pFile );
	for( i = 0; i < Size; i++ )
	{
		CItemSaveInstance* pInstance = new CItemSaveInstance();
		pInstance->Load( pFile );
		m_pItemHistory.push_back( pInstance );
	}

} // CLevelHistory::Load()
