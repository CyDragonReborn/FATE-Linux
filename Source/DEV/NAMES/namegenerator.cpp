/*
	namegenerator.cpp

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
#include "../UTILITIES/textfile.h"
#include "../UTILITIES/loadlocalizedstring.h"
#include "../GAME/game.h"

#include "namegenerator.h"

CNameGenerator::CNameGenerator(  const std::string& Path )// path to folder with data files inside
{
	Load( Path );
} // CNameGenerator::CNameGenerator()

CNameGenerator::~CNameGenerator( void )
{
} // CNameGenerator::~CNameGenerator()

void CNameGenerator::Load( const std::string& Path )	// path to folder
{

	// get the root path to the vehicle
	std::string RootPath = GetLocalPath() + "\\" + Path + "\\" + CGame::GetLocaleString() + "\\" + "prefix.dat";
	queue<std::string>	Tokens;
	CTextFile*	pDataFile = new CTextFile();
	
	// load first prefix set	
	pDataFile->Open( RootPath );


	bool Done	= kFalse;
	while ( !Done )
	{
		pDataFile->TokenizeNextLine( &Tokens, "\n\t\r", '#' );	
		if ( Tokens.size() <= 0 )
		{
			break;
		}
		if ( pDataFile->IsEOF() )
		{
			break;
		}
		m_Prefix.push_back( Tokens.front() );
		ClearQueue( Tokens );
	}

	pDataFile->Close();

	// load second prefix set	


	RootPath = GetLocalPath() + "\\" + Path + "\\" + CGame::GetLocaleString() + "\\" + "prefix2.dat";
	pDataFile->Open( RootPath );

	Done = kFalse;
	while ( !Done )
	{
		pDataFile->TokenizeNextLine( &Tokens, "\n\t\r", '#' );	
		if ( Tokens.size() <= 0 )
		{
			break;
		}
		if ( pDataFile->IsEOF() )
		{
			break;
		}
		m_Prefix2.push_back( Tokens.front() );
		ClearQueue( Tokens );
	}

	pDataFile->Close();

	// first suffix set

	RootPath = GetLocalPath() + "\\" + Path + "\\" + CGame::GetLocaleString() + "\\" + "suffix.dat";
	pDataFile->Open( RootPath );
	Done = kFalse;
	while ( !Done )
	{
		pDataFile->TokenizeNextLine( &Tokens, "\n\t\r", '#' );	
		if ( Tokens.size() <= 0 )
		{
			break;
		}
		if ( pDataFile->IsEOF() )
		{
			break;
		}
		m_Suffix.push_back( Tokens.front() );
		ClearQueue( Tokens );
	}

	pDataFile->Close();

	// second suffix set

	RootPath = GetLocalPath() + "\\" + Path + "\\" + CGame::GetLocaleString() + "\\" + "suffix2.dat";
	pDataFile->Open( RootPath );
	Done = kFalse;
	while ( !Done )
	{
		pDataFile->TokenizeNextLine( &Tokens, "\n\t\r", '#' );	
		if ( Tokens.size() <= 0 )
		{
			break;
		}
		if ( pDataFile->IsEOF() )
		{
			break;
		}
		m_Suffix2.push_back( Tokens.front() );
		ClearQueue( Tokens );
	}

	pDataFile->Close();

	// title set

	RootPath = GetLocalPath() + "\\" + Path + "\\" + CGame::GetLocaleString() + "\\" + "title.dat";
	pDataFile->Open( RootPath );
	Done = kFalse;
	while ( !Done )
	{
		pDataFile->TokenizeNextLine( &Tokens, "\n\t\r", '#' );	
		if ( Tokens.size() <= 0 )
		{
			break;
		}
		if ( pDataFile->IsEOF() )
		{
			break;
		}
		m_Title.push_back( Tokens.front() );
		ClearQueue( Tokens );
	}

	pDataFile->Close();

	DELETE_SAFELY( pDataFile );

} // CNameGenerator::Load()

std::string CNameGenerator::GenerateName( void )
{
	std::string Name( "" );
	if( rand() %1000 < 500 )
	{
		uint32 Prefix = RandomIntegerBetween( 0, m_Prefix.size() - 1 );
		uint32 Suffix = RandomIntegerBetween( 0, m_Suffix.size() - 1 );
		
		if( RandomIntegerBetween( 0, 100 ) < 30 )
		{
			Name = m_Prefix[Prefix];
		}
		else
		{
			if( RandomIntegerBetween( 0, 100 ) < 20 )
			{
				Name = m_Prefix[Prefix] + "'" + m_Suffix[Suffix];
			}
			else
			{
				Name = m_Prefix[Prefix] + m_Suffix[Suffix];
			}
		}
	}
	else
	{
		uint32 Prefix = RandomIntegerBetween( 0, m_Prefix2.size() - 1 );
		uint32 Suffix = RandomIntegerBetween( 0, m_Suffix2.size() - 1 );
		Name = m_Prefix2[Prefix] + m_Suffix2[Suffix];
	}
	if( m_Title.size() > 0 )
	{
		uint32 Title = RandomIntegerBetween( 0, m_Title.size() - 1 );
		Name = LoadLocalizedString2Variables(990, Name, m_Title[Title] );
	}

	return Name;
} // CNameGenerator::GenerateName()

std::string CNameGenerator::GenerateSimpleName( void )
{
	std::string Name( "" );
	if( rand() %1000 < 500 )
	{
		uint32 Prefix = RandomIntegerBetween( 0, m_Prefix.size() - 1 );
		uint32 Suffix = RandomIntegerBetween( 0, m_Suffix.size() - 1 );
		if( rand() %1000 < 200 )
		{
			Name = m_Prefix[Prefix] + "'" + m_Suffix[Suffix];
		}
		else
		{
			Name = m_Prefix[Prefix] + m_Suffix[Suffix];
		}
	}
	else
	{
		uint32 Prefix = RandomIntegerBetween( 0, m_Prefix2.size() - 1 );
		uint32 Suffix = RandomIntegerBetween( 0, m_Suffix2.size() - 1 );
		Name = m_Prefix2[Prefix] + m_Suffix2[Suffix];
	}
	return Name;
} // CNameGenerator::GenerateSimpleName()

std::string CNameGenerator::GeneratePhonicName( void )
{
	std::string Name( "" );

	uint32 Prefix = RandomIntegerBetween( 0, m_Prefix.size() - 1 );
	uint32 Suffix = RandomIntegerBetween( 0, m_Suffix.size() - 1 );
	if( RandomIntegerBetween( 0, 100 ) < 30 )
	{
		Name = m_Prefix[Prefix];
	}
	else
	{
		if( RandomIntegerBetween( 0, 100 ) < 20 )
		{
			Name = m_Prefix[Prefix] + "'" + m_Suffix[Suffix];
		}
		else
		{
			Name = m_Prefix[Prefix] + m_Suffix[Suffix];
		}
	}

	return Name;
} // CNameGenerator::GeneratePhonicName()