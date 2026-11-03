/*
	gamestatemanager.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/8/2004

*/


#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>
#include <math.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"

#include "../MODEL/model.h"

#ifdef ONLINEVERSION
#include "../CDA/cdamanager.h"
#endif

#include "gamestatemanager.h"

CGameStateManager::CGameStateManager( const std::string& Params,		// command line for our DRM object
									  const std::string& BaseURL ) :	// base url for CDAManager to laod from
									  m_GameState( KGameStateNone ),
									  m_Menu( KNoMenu ),
									  m_IntroComplete( kFalse )
{
#ifdef ONLINEVERSION
	m_pCDAManager = NULL;
	m_pCDAManager = new CCDAManager( "downloadmanifest.dat",
									 BaseURL,
									 GetLocalPath(),
									 "FATE" );
									 
#endif

} // CGameStateManager::CGameStateManager()

CGameStateManager::~CGameStateManager( void )
{
#ifdef ONLINEVERSION
	DELETE_SAFELY( m_pCDAManager );
#endif
} // CGameStateManager::~CGameStateManager()

void CGameStateManager::Update( float32 TimeElapsed )	// time in seconds
{
	// update any download threads, if applicable
#ifdef ONLINEVERSION
	if( m_pCDAManager != NULL )
	{
		m_pCDAManager->Update( TimeElapsed );
	}
#endif

	if( m_QueuedGameState.size() > 0 &&
		m_GameState != KGameStateDownloadMenu )
	{
		m_QueuedTimeToSet.front() -= TimeElapsed;

		if( m_QueuedTimeToSet.front() <= 0 )
		{
			if( m_Menu == KLogoMenu )
			{
				m_IntroComplete = kTrue;
			}
			m_GameState = m_QueuedGameState.front();
			m_Menu = m_QueuedMenu.front();
			m_QueuedGameState.pop_front();
			m_QueuedMenu.pop_front();
			m_QueuedTimeToSet.pop_front();
		}
	}
} // CGameStateManager::Update()

bool CGameStateManager::InGame( void )
{
	return m_GameState == KGameStateInGame;
} // CGameStateManager::InGame()

bool CGameStateManager::InMenu( void )
{
	return m_GameState == KGameStateMenu;
} // CGameStateManager::InMenu()

bool CGameStateManager::Loading( void )
{
	return m_GameState == KGameStateLoadMenu;
} // CGameStateManager::InMenu()

void CGameStateManager::SetTargetMenu( EMenu Menu,			// menu to go to
									   float32 Time )		// time until the change occurs
{	
	if( m_GameState == KGameStateInGame ||
		m_GameState == KGameStatePaused )
	{
		SetGameState( KGameStateQuitToMenu, Menu, Time );
	}
	else
	{
		SetGameState( KGameStateLoadMenu, Menu, Time );
	}		
} // CGameStateManager::SetTargetMenu()

void CGameStateManager::SetPausedMenu( EMenu Menu,			// menu to go to
									   float32 Time )		// time until the change occurs
{	
	SetGameState( KGameStateLoadPauseMenu, Menu, Time );
} // CGameStateManager::SetPauseMenu()

void CGameStateManager::SetGameState( EGameState GameState,	// gamestate to change to
									  float32 Time )		// time until the change occurs
{
// online builds have to go through a media verification step
#ifdef ONLINEVERSION
	if( m_GameState != KGameStateDownloadMenu )
	{
		m_QueuedGameState.push_back( KGameStateDownloadMenu );
		m_QueuedMenu.push_back( KLoadingMenu );
		m_QueuedTimeToSet.push_back( 0 );
	}
#endif

	m_QueuedGameState.push_back( GameState );
	m_QueuedMenu.push_back( KNoMenu );
	m_QueuedTimeToSet.push_back( Time );
} // CGameStateManager::SetGameState()

void CGameStateManager::SetGameState( EGameState GameState,		// gamestate to change to
									  EMenu Menu,				// path of menu to switch to
									  float32 Time )			// time until the change occurs
{
// online builds have to go through a media verification step
#ifdef ONLINEVERSION
	if( m_GameState != KGameStateDownloadMenu )
	{
		m_QueuedGameState.push_back( KGameStateDownloadMenu );
		m_QueuedMenu.push_back( KLoadingMenu );
		m_QueuedTimeToSet.push_back( 0 );
	}
#endif

	m_QueuedGameState.push_back( GameState );
	m_QueuedMenu.push_back( Menu );
	m_QueuedTimeToSet.push_back( Time );
} // CGameStateManager::SetGameState()

void CGameStateManager::SetGameStateImmediate( EGameState GameState,	// gamestate to change to
											   EMenu Menu,				// path of menu to switch to
											   float32 Time )			// time until the change occurs
{
	m_GameState = GameState;
	m_Menu = Menu;
	m_QueuedGameState.clear();
	m_QueuedMenu.clear();
	m_QueuedTimeToSet.clear();
} // CGameStateManager::SetGameState()

bool CGameStateManager::ItemsLoaded( const std::string& Keyword )	// keyword to check
{
#ifdef ONLINEVERSION
	if( m_pCDAManager == NULL )
	{
		return kTrue;
	}
	return m_pCDAManager->ItemsLoaded( Keyword );
#else
	return kTrue;
#endif
} // CGameStateManager::ItemsLoaded()