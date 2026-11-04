/*
	namegenerator.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	8/12/2004

*/

#ifndef _NAMEGENERATOR_H
#define _NAMEGENERATOR_H

#include "../UTILITIES/constants.h"

// loads a list of all cars and tracks required by the game
class CNameGenerator
{
public:
			CNameGenerator( const std::string& Path );	// path to the directory containing data files
			~CNameGenerator( void );

	void	Load(  const std::string& Path );	// path to the directory containing data files

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	std::string GenerateName( void );

	std::string GenerateSimpleName( void );

	std::string GeneratePhonicName( void );
private:

	std::vector< std::string >		m_Prefix;
	std::vector< std::string >		m_Suffix;


	std::vector< std::string >		m_Prefix2;
	std::vector< std::string >		m_Suffix2;

	std::vector< std::string >		m_Title;
};


#endif