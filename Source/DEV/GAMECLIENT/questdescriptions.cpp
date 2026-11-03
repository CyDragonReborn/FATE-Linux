/*
	questdescriptions.cpp

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
#include "quest.h"
#include "questdescriptions.h"

CQuestDescriptions::CQuestDescriptions(  const std::string& Path )// path to folder with data files inside
{
	Load( Path );
} // CQuestDescriptions::CQuestDescriptions()

CQuestDescriptions::~CQuestDescriptions( void )
{
} // CQuestDescriptions::~CQuestDescriptions()

void CQuestDescriptions::Load( const std::string& Path )	// path to folder
{

	// get the root path to the vehicle
	std::string RootPath = GetLocalPath() + "\\" + Path;
	queue<std::string>	Tokens;
	CDataFile* pDataFile = new CDataFile( RootPath );
	
	for( uint32 i = 0; i < KQuestTypes; i++ )
	{
		uint32 Items = pDataFile->DataGroups( KQuestName[i] );

		for( uint32 w = 0; w < Items; w++ )
		{
			// parse tile widths
			CDataGroup* pMasterGroup = pDataFile->GetDataGroup( KQuestName[i], w );
			std::string Text = pMasterGroup->Item( 0 );
			while( Text.find( "\\n" ) != -1 )
			{
				int32 Index = Text.find( "\\n" );
				Text.replace( Index, 2, "\n" );
			}
			while( Text.find( "\\b" ) != -1 )
			{
				int32 Index = Text.find( "\\b" );
				Text.replace( Index, 2, "\b" );
			}

			m_QuestDescription[i].push_back( Text );
		}
	}
	DELETE_SAFELY( pDataFile );

} // CQuestDescriptions::Load()

const std::string& CQuestDescriptions::RandomQuestDescription( EQuestType Type )	// type of quest
{
	assert( m_QuestDescription[Type].size() > 0 );

	return m_QuestDescription[Type][ RandomIntegerBetween( 0, m_QuestDescription[Type].size() - 1 ) ];
} // CQuestDescriptions::RandomQuestDescription()
