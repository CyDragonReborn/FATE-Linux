/*
	gamestatemanager.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/4/2004

*/


#ifndef _GAMESTATEMANAGER_H
#define _GAMESTATEMANAGER_H
#include "gamestates.h"

#ifdef ONLINEVERSION
#include "../CDA/cdamanager.h"
#endif

class CRace;

// manages gamestate, and transition between gamestates

class CGameStateManager
{
public:
				CGameStateManager( const std::string& Params,		// command line for our DRM object
								   const std::string& BaseURL );	// base url for CDAManager to laod from


				~CGameStateManager( void );

	void	Update( float32 TimeElapsed );	// time in seconds

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	bool	IntroComplete( void )			{	return m_IntroComplete;		};

	bool	InGame( void );
	bool	InMenu( void );
	bool	Loading( void );

	EGameState	GameState( void )			{	return m_GameState;			};

	EMenu		GetTargetMenu( void )		{	return m_Menu;					};

	uint32		QueuedGameStates( void )	{	return m_QueuedGameState.size();	};

	EGameState	NextGameState( void )		{	return m_QueuedGameState[0];	};

	EMenu		NextTargetMenu( void )		{	return m_QueuedMenu[0];			};

	// check for loading download items
	bool			ItemsLoaded( const std::string& Keyword );	// keyword to check

#ifdef ONLINEVERSION
	bool			UsingCDA( void )			{	return m_pCDAManager != NULL;		};

	bool			Downloading( void )			{	return m_pCDAManager->ActiveDownloads() > 0;	};

	CCDAManager*	CDAManager( void )			{	return m_pCDAManager;				};
#endif

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void	SetGameState( EGameState GameState,		// gamestate to change to
						  float32 Time = 0 );		// time until the change occurs

	void	SetGameState( EGameState GameState,		// gamestate to change to
						  EMenu Menu,				// path of menu to switch to
						  float32 Time = 0 );		// time until the change occurs

	void	SetTargetMenu( EMenu Menu,				// menu to switch to
						   float32 Time = 0 );		// time until the change occurs

	void	SetPausedMenu( EMenu Menu,				// menu to switch to
						   float32 Time = 0 );		// time until the change occurs

	void	SetGameStateImmediate( EGameState GameState,			// gamestate to change to
								   EMenu Menu = KNoMenu,			// path of menu to switch to
								   float32 Time = 0 );				// time until the change occurs

private:

	EGameState	m_GameState;
	EMenu		m_Menu;


	std::deque<float32>		m_QueuedTimeToSet;		// time left until we set the gamestate	
	std::deque<EGameState>	m_QueuedGameState;
	std::deque<EMenu>		m_QueuedMenu;

#ifdef ONLINEVERSION
	CCDAManager*			m_pCDAManager;			// manages downloads
#endif

	bool		m_IntroComplete;
};

#endif