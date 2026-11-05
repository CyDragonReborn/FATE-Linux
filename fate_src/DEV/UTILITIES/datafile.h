/*
	datafile.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	6/2/2004
	
*/

#pragma warning(disable:4786)

#ifndef _DATAFILE_H
#define _DATAFILE_H

#include <string>
#include <vector>
#include "constants.h"

class CTextFile;

// a data group is really just a vector of strings with a master keyword
// to identify them.
// a data group can contain SUB-data groups however - nested sets of data.
class CDataGroup
{
public:
				CDataGroup( void )	{	};
				~CDataGroup( void )	{	};

	void				SetKeyName( const std::string& KeyName )	{	m_KeyName = KeyName;			};

	void				AddItem( const std::string& Data )			{	m_DataItems.push_back( Data );	};



	void		ParseSubGroups( CTextFile& TextFile );	// file to parse sub-groups from

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////
	const std::string&	KeyName( void )		{	return m_KeyName;			};

	uint32				Items( void )		{	return m_DataItems.size();	};

	const std::string&	Item( uint32 Index ){	return m_DataItems[Index];	};

	bool		SubKeyExists( const std::string& KeyName );	// key we want to check the existence of

	CDataGroup*	GetSubDataGroup( const std::string& KeyName );	// key we want to get the data group of

	CDataGroup*	GetSubDataGroup( const std::string& KeyName,	// key we want to get the data group of
							  uint32 Index );				// index of the data group of title KeyName

	uint32		SubDataGroups( void )		{	return m_SubDataGroups.size();		};
	
	uint32		SubDataGroups( const std::string& KeyName );	// key we want to check the existence of

private:


	std::string m_KeyName;

	std::vector< std::string > m_DataItems;

	std::vector< CDataGroup >	m_SubDataGroups;
};

// the data file is inteded for parsing general purpose files with data in the format -
// <DATAKEY>:data:data:data.......:data
// where the key for each data grouping is surrounded by <> and the data is delimited by : characters
// comments are expected to be prefixed with #

class CDataFile
{
public:

				CDataFile( const std::string& FilePath );	// path to data file we want to parse
				~CDataFile();

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	bool		KeyExists( const std::string& KeyName );	// key we want to check the existence of

	CDataGroup*	GetDataGroup( const std::string& KeyName );	// key we want to get the data group of

	CDataGroup*	GetDataGroup( const std::string& KeyName,	// key we want to get the data group of
							  uint32 Index );				// index of the data group of title KeyName

	uint32		DataGroups( void )		{	return m_DataGroups.size();		};
	
	uint32		DataGroups( const std::string& KeyName );	// key we want to check the existence of

	const std::string& Path( void )		{	return m_Path;					};

private:

	void		ParseFile( void );

	CTextFile*	m_pTextFile;

	std::string	m_Path;	// path to this file

	std::vector< CDataGroup >	m_DataGroups;
};


#endif 
