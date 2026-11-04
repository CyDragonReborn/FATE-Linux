/*
	continuemainmenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _CONTINUEGAMEMENU_H
#define _CONTINUEGAMEMENU_H

#include "../menu.h"
#include "../../CHARACTER/gamesave.h"

class CRefManager;
class CGameStateManager;
class CKeyHandler;
class CSettings;
class CSprite;
class CFontMetric;
class CCharacterSaveInstance;
class CConfirmMenu;

enum EContinueMenuCode
{
	KContinueMenuSelect,
	KContinueMenuDelete,
	KContinueMenuDeleteConfirm,
	KContinueMenuDeleteCancel,
	KContinueMenuBack,
	KContinueMenuScrollUp,
	KContinueMenuScrollDown,
	KContinueMenuCodes
};
const uint32 KProfileSelectionSlots( 4 );



class CContinueGameMenu:
		public CMenu
{
public:
	CContinueGameMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
			   CSettings& pSettings,					//we need this for tracking user preferences
			   CGameStateManager& pGameStateManager,	// game manager
			   CMenuManager& pMenuManager );			// menu manager

	~CContinueGameMenu( void );

	// these methods are overriding the methods in CMenu that this
	// class is derived from

	void	Create( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	void	Update( HWND hWnd,								// hwnd of the active window
					float32 TimeElapsed );					// time elapsed, in seconds

	void	Activate( void );

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	void	RenderUI( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					  WPARAM KeyCode );	// the wparam which will be the keycode

	void	MouseEvent( UINT WMMessage );	// windows message - WM_LBUTTONUP, etc.

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void	UpdatePlayer( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	void	ReloadProfiles( void );

private:

	void	ClearScrollWindow( void );

	void	DeleteProfiles( void );

	void	GetProfilesFromFolder( std::string Path,					// path to the music folder to look in
								   std::vector< std::string >& Paths,	// list of paths to add to
								   std::vector< int32 >& Indices );	// list of indices to add to

	void	UpdateScrollWindow( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device
 
	// these methods are overriding the methods in CMenu that this
	// class is derived from

	void	ProcessInput( HWND hWnd );								// hwnd of the active window


	FSOUND_SAMPLE*	m_pClickSound;

	CConfirmMenu*	m_pConfirmMenu;

	// our logo texture
	CMaterial*	m_pLogoMaterial;

	// basic button material
	CMaterial*	m_pButtonMaterial;

	// font texture and metric
	CMaterial*		m_pFontMaterial;
	CFontMetric*	m_pFontMetric;

	CButton*		m_pContinueButton;

	std::vector< CButton* >		m_pScrollWindowElements;
	std::vector< CText* >		m_pScrollWindowText;

	std::vector< std::string >	m_ProfileFiles;
	std::vector< int32 >		m_ProfileIndices;
	std::vector< CCharacterSaveInstance* >	m_pProfileDescriptions;
	std::vector< CCharacterSaveInstance* >	m_pPetProfileDescriptions;

	int32		m_SelectedProfile;
	int32		m_ScrollWindowIndex;

	bool		m_UpdatePlayer;
	bool		m_OpenDeletion;
	CGameSave*		m_pGameSave;
};

#endif