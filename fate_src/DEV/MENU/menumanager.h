/*
	menumanager.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/19/2004

*/

#ifndef _MENUMANAGER_H
#define _MENUMANAGER_H

#include "d3dx8_shim.h"

#include "../IO/keyhandler.h"
#include "../IO/mousehandler.h"
#include "../UTILITIES/frustum.h"

#include "../GAMECLIENT/gameclient.h"

#include "../GAME/gamestates.h"

class CRefManager;
class CSettings;
class CCamera;
class CFrustum;
class CLightManager;
class CCharacter;
class CGameStateManager;
class CDataLight;
class CMaterial;
class CParticleManager;
class CGlowManager;
class CBlurLayer;
class CPositionalAudioManager;
class CSoundManager;
class CSprite;
class CButton;
class CText;
class CFontMetric;
class CMenu;
class CGameClient;

// owns the in-game experience - 
// manages levels, objects, creatures, lights, and in-game UI
class CMenuManager
{


public:

			CMenuManager( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						 CRefManager& pRefManager,					// pointer to the reference manager, for tracking media
						 CSettings& pSettings,						// pointer to the settings manager, for keeping track of card/machine capabilities
						 CGameClient& m_pGameClient,				// game client
						 CGameStateManager& pGameStateManager,		// game manager
						 CSoundManager& pSoundManager );

			~CMenuManager( void );

	void	FinalizeCreation( LPDIRECT3DDEVICE8 pD3DDevice );			// direct3d device

	void	PreRender( LPDIRECT3DDEVICE8 pD3DDevice );					// direct3d device

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice );						// direct3d device

	void	RenderUI( LPDIRECT3DDEVICE8 pD3DDevice );					// direct3d device

	void	RenderBlurLayer( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	void	RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device

	void	Update( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
					HWND hWnd,								// hwnd of the active window
					float32 TimeElapsed );					// time elapsed, in seconds

	void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					  WPARAM KeyCode );	// the wparam which will be the keycode

	void	MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONUP, etc.
					    WPARAM WMParam );	// the wparam which contains mousewheel info


	void	Reload( LPDIRECT3DDEVICE8 pD3DDevice );				// direct3d device

	void	ResetMouse( LPDIRECT3DDEVICE8 pD3DDevice );				// direct3d device

	void	Unload( void );

	void	Activate( void );				// direct3d device

	void	Deactivate( void );

	void	FindObjects( void );

	void	SetActiveMenu( EMenu ActiveMenu );				// target menu

	void	CreatePlayer( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						  CCharacterSaveInstance* pInstance,	// instance to copy over
						  CCharacterSaveInstance* pPetInstance,	// instance to copy over
						  EGender Gender = KMale,
						  int32 HeadIndex = 0,
						  int32 HairIndex = 0);

	void	CreatePet( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
					   CCharacterSaveInstance* pPetInstance,	// instance to copy over
					   bool IsDog );							// dog?

	void				ReloadPlayer( void )	{	m_ReloadPlayer = kTrue;	};

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	CCharacter*			Player( void )			{	return m_pPlayer;		};

	CCamera*			Camera( void )			{	return m_pCamera;		};

	CGameClient&		GameClient( void )		{	return m_pGameClient;	};


private:

	void	EmitParticles( void );


	void	CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void	RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   const D3DXMATRIX& ViewportMatrix );		// viewport matrix


	void	ProcessInput( HWND hWnd );		// hwnd of the active window

	CGameStateManager&	m_pGameStateManager;

	CSettings&		m_pSettings;
	CRefManager&	m_pRefManager;

	CSoundManager&	m_pSoundManager;

	CGameClient&	m_pGameClient;

	CKeyHandler		m_KeyHandler;
	CMouseHandler	m_MouseHandler;

	CFrustum		m_ShadowFrustum;

	CScene*			m_pBackgroundScene;

	CCamera*		m_pCamera;

	float32			m_BlurTime;

	CCharacter*		m_pPlayer;
	CCharacter*		m_pPet;

	CMaterial*		m_pBlack;

	LPDIRECT3DSURFACE8	m_pMouseSurface;

	CSprite*	m_pMousePointer;
	CMaterial*	m_pPointerMaterial;
	CMaterial*	m_pPointerSelectMaterial;

	// font texture and metric
	CMaterial*		m_pFontMaterial;
	CMaterial*		m_pFontHighlightMaterial;
	CFontMetric*	m_pFontMetric;

	CLList<CButton>*		m_pButtonList;
	CLList<CSprite>*		m_pSpriteList;
	CLList<CText>*			m_pTextList;

	D3DXMATRIX		m_ShadowViewMatrix;

	CMenu*		m_pMenus[KMenus];

	EMenu		m_ActiveMenu;

	bool		m_IsDog;

	bool		m_ReloadPlayer;

	int32		m_CameraPath;
	float32		m_CameraPathPosition;

	bool		m_Finalized;

	std::vector< D3DXVECTOR3 >	m_ParticlePosition;
	std::vector< D3DXMATRIX >	m_ParticleOrientation;
	std::vector< int32 >		m_ParticleType;

	D3DXVECTOR3					m_PreviewPosition;
	D3DXMATRIX					m_PreviewOrientation;
	D3DXVECTOR3					m_PetPreviewPosition;
	D3DXMATRIX					m_PetPreviewOrientation;
};

#endif