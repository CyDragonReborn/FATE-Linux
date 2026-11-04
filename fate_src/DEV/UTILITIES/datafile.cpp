/*
  datafile.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	6/2/2004
	
	CHANGE HISTORY
	7/24/2004	-	added support for nested groups of data.

	10/8/2004	-	nested objects were not properly being nested

*/

#include <stdio.h>
#include <assert.h>

#include "constants.h"
#include "macros.h"
#include "utilities.h"

#include "textfile.h"
#include "datafile.h"

using namespace std;

/////////////////////////////////////////////
// Data File
/////////////////////////////////////////////

CDataFile::CDataFile( const std::string& FilePath ) :	// path to data file we want to parse
														m_pTextFile( new CTextFile() ),
														m_Path ( FilePath )
{
	if( m_pTextFile->Open( FilePath ) )
	{

		ParseFile();

		m_pTextFile->Close();
	}

	DELETE_SAFELY( m_pTextFile );


} // CDataFile::CDataFile()

CDataFile::~CDataFile( void )
{
	DELETE_SAFELY( m_pTextFile );
} // CDataFile::~CDataFile()

// extract all data groups from the text file and place them in a
// vector of data objects
void CDataFile::ParseFile( void )
{
	queue<std::string>	Tokens;

	bool Done( kFalse );

	while ( !Done )
	{
		m_pTextFile->TokenizeNextLine( &Tokens, ":\n\t\r", '#' );	

		if ( m_pTextFile->IsEOF() )
		{
			break;
		}

		// we have to have at LEAST 2 tokens to get a data group
		if ( Tokens.size() < 1 )
		{
			ClearQueue( Tokens );
			continue;
		}

		// the first token is our key
		std::string KeyName = Tokens.front();
		Tokens.pop();

		// now we need to find out if this is a valid key - should be bracketed with <>
		if( KeyName.length() > 2 )
		{
			// is it a standard data group?
			if( KeyName.find( '<', 0 ) == 0 &&
				KeyName.rfind( '>', KeyName.length() ) == KeyName.length() - 1 )
			{
				uint32 Index = m_DataGroups.size();
				// ok, now we make a new datagroup, and start plugging in the data
				m_DataGroups.push_back( CDataGroup() );
				m_DataGroups[Index].SetKeyName( KeyName.substr( 1, KeyName.length() - 2 ) );
				
				while( Tokens.size() > 0 )
				{
					m_DataGroups[Index].AddItem( Tokens.front() );
					Tokens.pop();
				}
			}
			// or is it nested?
			else if( KeyName.find( '[', 0 ) == 0 &&
				KeyName.rfind( ']', KeyName.length() ) == KeyName.length() - 1 )
			{
				uint32 Index = m_DataGroups.size();
				// ok, now we make a new datagroup, and start plugging in the data
				m_DataGroups.push_back( CDataGroup() );
				m_DataGroups[Index].SetKeyName( KeyName.substr( 1, KeyName.length() - 2 ) );
				
				m_DataGroups[Index].ParseSubGroups( *m_pTextFile );
			}
		}

		ClearQueue( Tokens );
	}
} // CDataFile::ParseFile()


bool CDataFile::KeyExists( const std::string& KeyName )	// key we want to check the existence of
{
	for( uint32 i = 0; i < m_DataGroups.size(); i++ )
	{
		if( KeyName == m_DataGroups[i].KeyName() )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CDataFile::KeyExists()

// find the number of data groups that have the requested key name
uint32 CDataFile::DataGroups( const std::string& KeyName )	// key we want to check the existence of
{
	uint32 Count( 0 );
	for( uint32 i = 0; i < m_DataGroups.size(); i++ )
	{
		if( KeyName == m_DataGroups[i].KeyName() )
		{
			Count++;
		}
	}
	return Count;
} // CDataFile::DataGroups()

// return a pointer to a data group
CDataGroup*	CDataFile::GetDataGroup( const std::string& KeyName )	// key we want to get the data group of
{
	for( uint32 i = 0; i < m_DataGroups.size(); i++ )
	{
		if( KeyName == m_DataGroups[i].KeyName() )
		{
			return &m_DataGroups[i];
		}
	}
	return NULL;
} // CDataFile::GetDataGroup()

// return a pointer to a data group  by sub-index.
// for instance, there might be many keys named DATA, and in this way, you can
// retrieve DATA key 2, 3, etc.
CDataGroup*	CDataFile::GetDataGroup( const std::string& KeyName,	// key we want to get the data group of
									 uint32 Index )					// index of the data group of title KeyName
{
	uint32 SubIndex( 0 );
	for( uint32 i = 0; i < m_DataGroups.size(); i++ )
	{
		if( KeyName == m_DataGroups[i].KeyName() )
		{
			if( SubIndex == Index )
			{
				return &m_DataGroups[i];
			}
			SubIndex++;
		}
	}
	return NULL;
} // CDataFile::GetDataGroup()


//////////////////////////////////////////////////////////////////////////////////////////////
// Data Group
//////////////////////////////////////////////////////////////////////////////////////////////

bool CDataGroup::SubKeyExists( const std::string& KeyName )	// key we want to check the existence of
{
	for( uint32 i = 0; i < m_SubDataGroups.size(); i++ )
	{
		if( KeyName == m_SubDataGroups[i].KeyName() )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CDataGroup::SubKeyExists()

// find the number of data groups that have the requested key name
uint32 CDataGroup::SubDataGroups( const std::string& KeyName )	// key we want to check the existence of
{
	uint32 Count( 0 );
	for( uint32 i = 0; i < m_SubDataGroups.size(); i++ )
	{
		if( KeyName == m_SubDataGroups[i].KeyName() )
		{
			Count++;
		}
	}
	return Count;
} // CDataGroup::SubDataGroups()

// return a pointer to a data group
CDataGroup*	CDataGroup::GetSubDataGroup( const std::string& KeyName )	// key we want to get the data group of
{
	for( uint32 i = 0; i < m_SubDataGroups.size(); i++ )
	{
		if( KeyName == m_SubDataGroups[i].KeyName() )
		{
			return &m_SubDataGroups[i];
		}
	}
	return NULL;
} // CDataGroup::GetSubDataGroup()

// return a pointer to a data group  by sub-index.
// for instance, there might be many keys named DATA, and in this way, you can
// retrieve DATA key 2, 3, etc.
CDataGroup*	CDataGroup::GetSubDataGroup( const std::string& KeyName,	// key we want to get the data group of
										 uint32 Index )					// index of the data group of title KeyName
{
	uint32 SubIndex( 0 );
	for( uint32 i = 0; i < m_SubDataGroups.size(); i++ )
	{
		if( KeyName == m_SubDataGroups[i].KeyName() )
		{
			if( SubIndex == Index )
			{
				return &m_SubDataGroups[i];
			}
			SubIndex++;
		}
	}
	return NULL;
} // CDataGroup::GetSubDataGroup()

void CDataGroup::ParseSubGroups( CTextFile& TextFile )	// file to parse sub-groups from
{
	queue<std::string>	Tokens;

	bool Done( kFalse );

	while ( !Done )
	{
		TextFile.TokenizeNextLine( &Tokens, ":\n\t\r", '#' );	

		if ( TextFile.IsEOF() )
		{
			break;
		}

		// we have to have at LEAST 2 tokens to get a data group
		if ( Tokens.size() < 1 )
		{
			ClearQueue( Tokens );
			continue;
		}

		// the first token is our key
		std::string KeyName = Tokens.front();
		Tokens.pop();

		// now we need to find out if this is a valid key - should be bracketed with <>
		if( KeyName.length() > 2 )
		{
			// is it a standard data group?
			if( KeyName.find( '<', 0 ) == 0 &&
				KeyName.rfind( '>', KeyName.length() ) == KeyName.length() - 1 )
			{
				uint32 Index = m_SubDataGroups.size();
				// ok, now we make a new datagroup, and start plugging in the data
				m_SubDataGroups.push_back( CDataGroup() );
				m_SubDataGroups[Index].SetKeyName( KeyName.substr( 1, KeyName.length() - 2 ) );
				
				while( Tokens.size() > 0 )
				{
					m_SubDataGroups[Index].AddItem( Tokens.front() );
					Tokens.pop();
				}
			}
			// is it the close of this nested set?
			else if( KeyName.find( '[', 0 ) == 0 &&
					 KeyName.find( '/', 1 ) == 1 &&
					 KeyName.rfind( ']', KeyName.length() ) == KeyName.length() - 1 )
			{
				return;
			}
			// or is it nested?
			else if( KeyName.find( '[', 0 ) == 0 &&
					 KeyName.rfind( ']', KeyName.length() ) == KeyName.length() - 1 )
			{

				uint32 Index = m_SubDataGroups.size();
				// ok, now we make a new datagroup, and start plugging in the data
				m_SubDataGroups.push_back( CDataGroup() );
				m_SubDataGroups[Index].SetKeyName( KeyName.substr( 1, KeyName.length() - 2 ) );
				
				m_SubDataGroups[Index].ParseSubGroups( TextFile );
			}
		}

		ClearQueue( Tokens );
	}

} // CDataGroup::ParseSubGroups()