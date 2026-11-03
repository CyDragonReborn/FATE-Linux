/*
	tips.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	8/12/2004

*/
#define STRICT

#include <windows.h>
#include <string>
#include <assert.h>

using namespace std;

#include "../UTILITIES/utilities.h" 
#include "../UTILITIES/macros.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/datafile.h"
#include "tips.h"

CTips::CTips(  const std::string& Path )// path to folder with data files inside
{
	ClearState();
	Load( Path );
} // CTips::CTips()

CTips::~CTips( void )
{
} // CTips::~CTips()

void CTips::ClearState( void )
{
	for( uint32 j = 0; j < KContextTips; j++ )
	{
		m_TipSeen[j] = kFalse;
	}
} // CTips::ClearState()


void CTips::Load( const std::string& Path )	// path to folder
{

	// get the root path to the vehicle
	std::string RootPath = GetLocalPath() + "\\" + Path;
	queue<std::string>	Tokens;
	CDataFile* pDataFile = new CDataFile( RootPath );
	
	uint32 Tips = pDataFile->DataGroups( "TIP" );
	for( uint32 i = 0; i < Tips; i++ )
	{
		CDataGroup* pGroup = pDataFile->GetDataGroup( "TIP", i );
		std::string Tip = pGroup->Item( 0 );
		while( Tip.find( "\\n" ) != -1 )
		{
			int32 Index = Tip.find( "\\n" );
			Tip.replace( Index, 2, "\n" );
		}
		while( Tip.find( "\\b" ) != -1 )
		{
			int32 Index = Tip.find( "\\b" );
			Tip.replace( Index, 2, "\b" );
		}
		m_GeneralTips.push_back( Tip );
	}

	Tips = pDataFile->DataGroups( "CONTEXT_TIP" );
	for( i = 0; i < Tips; i++ )
	{
		CDataGroup* pGroup = pDataFile->GetDataGroup( "CONTEXT_TIP", i );
		std::string Type = StringUpper( pGroup->Item( 0 ) );
		std::string Tip = pGroup->Item( 1 );
		while( Tip.find( "\\n" ) != -1 )
		{
			int32 Index = Tip.find( "\\n" );
			Tip.replace( Index, 2, "\n" );
		}
		while( Tip.find( "\\b" ) != -1 )
		{
			int32 Index = Tip.find( "\\b" );
			Tip.replace( Index, 2, "\b" );
		}
		for( uint32 j = 0; j < KContextTips; j++ )
		{
			if( KContextTipName[j] == Type )
			{
				m_ContextTips[j] = Tip;
				break;
			}
		}
	}
	DELETE_SAFELY( pDataFile );

} // CTips::Load()

const std::string& CTips::RandomTip( void )
{
	assert( m_GeneralTips.size() > 0 );

	return m_GeneralTips[ RandomIntegerBetween( 0, m_GeneralTips.size() - 1 ) ];
} // CTips::RandomTip()

const std::string& CTips::ContextTip( EContextTip Tip )	// type of tip
{
	return m_ContextTips[Tip];
} // CTips::RandomTip()
