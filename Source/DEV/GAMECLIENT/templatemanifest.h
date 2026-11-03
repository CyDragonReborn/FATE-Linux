/*
	templatemanifest.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	8/12/2004

*/

#ifndef _MANIFEST_H
#define _MANIFEST_H

#include "../UTILITIES/constants.h"

// loads a list of all cars and tracks required by the game
class CTemplateManifest
{
public:
			CTemplateManifest( const std::string& ManifestPath );	// path to manifest.dat file
			~CTemplateManifest( void );

	void	LoadManifest(  const std::string& ManifestPath );	// path to manifest.dat file

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	uint32	Templates( void )		{	return m_TemplatePaths.size();	};

	const std::string&	TemplatePath( uint32 Index )	{	return m_TemplatePaths[Index];		};

private:

	std::vector< std::string >		m_TemplatePaths;
};


#endif