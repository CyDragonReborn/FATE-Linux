/*
	questdescriptions.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	8/12/2004

*/

#ifndef _QUESTDESCRIPTIONS_H
#define _QUESTDESCRIPTIONS_H

#include "../UTILITIES/constants.h"

// loads a list of all cars and tracks required by the game
class CQuestDescriptions
{
public:
			CQuestDescriptions( const std::string& Path );	// path to the directory containing data files
			~CQuestDescriptions( void );

	void	Load(  const std::string& Path );	// path to the directory containing data files

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	const std::string&	RandomQuestDescription( EQuestType Type );

	uint32				Descriptions( EQuestType Type )			{	return m_QuestDescription[Type].size();	};
private:

	std::vector< std::string >		m_QuestDescription[KQuestTypes];
};


#endif