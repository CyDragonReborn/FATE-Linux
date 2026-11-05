/*
  textfile.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	2/26/2004
	
	11/15/2004	-	some int returns for finding characters were being cast to uints - fixed
*/

#include <stdio.h>
#include "constants.h"

#include "textfile.h"

using namespace std;

CTextFile::CTextFile( void ) :
								m_pFile( NULL ),
								m_EndOfFile( kFalse ),
								m_FileName( "" )
{
} // CTextFile::CTextFile()

CTextFile::~CTextFile( void )
{
	// double check to make sure the user closed the file.
	if ( m_pFile != NULL )
	{
		Close();
	}
} // CTextFile::~CTextFile()

// open a file for reading
bool CTextFile::Open( const std::string& FileName )		// file to open
{
	// Create the file handle
	m_pFile = fopen( FileName.c_str(), "rb" );

	// make sure everything went well
	if ( m_pFile == NULL )
	{
		return kFalse;
	}

	m_FileName = std::string( FileName );
	m_EndOfFile = kFalse;
	return kTrue;
} // CTextFile::Open()

// close a currently open file
void CTextFile::Close( void )
{
	if ( m_pFile == NULL )
	{
		return;
	} // the file is already closed

	fclose( m_pFile );
	m_pFile = NULL;
} // CTextFile::Close()

//read a specified length into the buffer
void CTextFile::ReadBuffer( void* pBuffer,		// buffer to read to
							uint32 BufferSize )	// amount to read
{
	if ( m_pFile == NULL )
	{
		m_EndOfFile = kTrue;
		return; // error
	}

	uint32	numRead	= fread( pBuffer, 1, BufferSize, m_pFile );

	if ( numRead != BufferSize )
	{
		if ( feof( m_pFile ) )
		{
			m_EndOfFile = kTrue;
			return;
		}
	}
} // CTextFile::ReadBuffer()

bool CTextFile::IsEOF( void )
{
	if ( feof( m_pFile ) )
	{
		m_EndOfFile = kTrue;
		return kTrue;
	}
	return m_EndOfFile;
} // CTextFile::IsEOF()

// read one line from the file
std::string CTextFile::ReadLine( void )
{
	int8	CurrentCharacter;
	bool	Done	= kFalse;

	int32		CharactersRead	= 0;

	std::string	Result;

	while ( !Done )
	{
		if ( feof( m_pFile ) )
		{
			m_EndOfFile = kTrue;
			break;
		}

		ReadBuffer( &CurrentCharacter, 1 );
		CharactersRead++;

		// end at the EOL
		if ( CurrentCharacter == '\0' || CurrentCharacter == '\n' )
		{
			break;
		}

		Result += CurrentCharacter;
	}
	return Result;
} // CTextFile::ReadLine()


// tokenizes a line using space, tab, and carriage return
void CTextFile::TokenizeNextLine( queue<std::string>* pList,		// queue of tokens to fill Result 
								  int8 CommentCharacter )			// comment character - default '#'
{
	std::string	Line;

	if ( feof( m_pFile ) )
	{
		m_EndOfFile = kTrue;
		return;
	}
	while ( 1 )
	{
		Line = ReadLine();
		if ( Line[0] != CommentCharacter )
		{
			break;
		}
	}
	if ( m_EndOfFile )
	{
		return;
	}

	std::string::size_type CommentLocation = Line.find( CommentCharacter, 0 );
	if ( CommentLocation != Line.npos )
	{
		Line = Line.erase( CommentLocation );
	}

	int8	Delimiters[]	= " \t\n\r:";

	std::string::size_type TokenStart = Line.find_first_not_of( ( int8 * ) Delimiters, 0 );
	std::string::size_type TokenEnd;
	while ( TokenStart != Line.npos )
	{
		TokenEnd = Line.find_first_of( ( int8 * ) Delimiters, TokenStart );
		string	Token	= Line.substr( TokenStart, TokenEnd - TokenStart );
		pList->push( Token );

		TokenStart = Line.find_first_not_of( ( int8 * ) Delimiters, TokenEnd );
	}

} // CTextFile::TokenizeNextLine()

void CTextFile::TokenizeNextLine( queue<std::string>* pList,	// queue of tokens to fill Result
								  const int8* Delimiters,				// delimiters to use i.e. ".:/"
								  int8 CommentCharacter )		// comment character - default '#'
{
	std::string	Line;
	if ( m_pFile == NULL || feof( m_pFile ) )
	{
		m_EndOfFile = kTrue;
		return;
	}
	while ( 1 )
	{
		Line = ReadLine();
		if ( Line[0] != CommentCharacter )
		{
			break;
		}
	}
	if ( m_EndOfFile )
	{
		return;
	}

	std::string::size_type CommentLocation = Line.find( CommentCharacter, 0 );
	if ( CommentLocation != Line.npos )
	{
		Line = Line.erase( CommentLocation );
	}


	std::string::size_type TokenStart = Line.find_first_not_of( ( int8 * ) Delimiters, 0 );
	std::string::size_type TokenEnd;
	while ( TokenStart != Line.npos )
	{
		TokenEnd = Line.find_first_of( ( int8 * ) Delimiters, TokenStart );
		string	Token	= Line.substr( TokenStart, TokenEnd - TokenStart );
		pList->push( Token );

		TokenStart = Line.find_first_not_of( ( int8 * ) Delimiters, TokenEnd );
	}
	return;
} // CTextFile::TokenizeNextLine()


