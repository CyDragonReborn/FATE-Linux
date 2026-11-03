/*
	levelhistory.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/19/2004

*/

#ifndef _LEVELHISTORY_H
#define _LEVELHISTORY_H

#include "../UTILITIES/quadtree.h"


class CMaze;
class CFaceList;
class CLevel;
class CCharacterSaveInstance;
class CItemSaveInstance;
class CItem;
class CCharacter;

// a LevelHistory contains the visited states of cells for a level,
// as well as descriptions of objects and characters that were there

class CLevelHistory
{
public:

			CLevelHistory( int32 Level,		// level the map is for
							uint32 Width,		// width of map
							uint32 Height );	// height of map

			CLevelHistory( CLevelHistory& pMap );	// visibility map to copy

			~CLevelHistory( void );

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////
	bool	Visible( int32 X,	// visibility cell to check x
					 int32 Y );	// visibility cell to check y	

	uint32	Width( void )		{	return m_Width;		};

	uint32	Height( void )		{	return m_Height;	};

	int32	Level( void )		{	return m_Level;		};

	uint32	Characters( void )	{	return m_pCharacterHistory.size();	};

	uint32	Items( void )		{	return m_pItemHistory.size();		};

	CCharacterSaveInstance*	GetCharacter( uint32 Index )	{	return m_pCharacterHistory[Index];	};

	CItemSaveInstance*		GetItem( uint32 Index )		{	return m_pItemHistory[Index];		};

	bool	HasFog( void )		{	return m_Height != 0;				};

	float32	LastPopulationTime( void )	{	return m_LastPopulationTime;		};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	void	Copy( CLevelHistory& pMap );	// visibility map to copy

	void	ClearVisibilityData( void );

	void	DiscoverAll( void );

	void	SetVisibility( int32 X,		// visibility cell to check x
						   int32 Y,		// visibility cell to check y	
						   bool Value );// visibility value

	void	AddItem( CItem& pItem );			// world item to add to history

	void	AddCharacter( CCharacter& pCharacter,	// character to add to history
						  CLevel& pLevel );

	void	Load( FILE* pFile );	// open file to load from

	void	Save( FILE* pFile );	// open file to save to

	void	ResetPopulationTime( void )				{	m_LastPopulationTime = 0;		};

	void	SetLastPopulationTime( float32 Value )	{	m_LastPopulationTime = Value;	};

private:

	uint32		m_Width;
	uint32		m_Height;

	int32		m_Level;

	bool**		m_VisibilityData;

	std::vector< CCharacterSaveInstance* >	m_pCharacterHistory;
	std::vector< CItemSaveInstance* >		m_pItemHistory;

	float32		m_LastPopulationTime;
};

#endif