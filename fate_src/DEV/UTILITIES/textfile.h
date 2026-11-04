/*
	textfile.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	2/26/2004
	
*/

#pragma warning(disable:4786)

#ifndef _TEXTFILE_H
#define _TEXTFILE_H

#include <windows.h>
#include <queue>
#include <string>

class CTextFile
{
public:

				CTextFile();
				~CTextFile();

	bool		IsEOF();

	bool		Open( const std::string& FileName );		// file to open

	void		Close();

	void		ReadBuffer( void* pBuffer,		// buffer to read to
						uint32 BufferSize );	// amount to read

	std::string	ReadLine();
	void		TokenizeNextLine( std::queue<std::string>* pList,	// queue of tokens to fill Result
								  int8 CommentCharacter = '#' );	// comment character - default '#'

	void		TokenizeNextLine( std::queue<std::string>* pList,	// queue of tokens to fill Result 
								  int8* Delimiters,					// delimiters to use i.e. ".:/"
								  int8 CommentCharacter = '#' );	// comment character - default '#'
private:

	bool		m_EndOfFile;
	FILE*		m_pFile;
	std::string	m_FileName;


};


#endif 
