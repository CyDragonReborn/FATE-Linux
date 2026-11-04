/*
	optionsmenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/

#include <windows.h>
#include "d3d8.h"
#include "d3d8types.h"

#ifndef _OPTIONSMENU_H
#define _OPTIONSMENU_H

#include "../menu.h"
#include "../../UTILITIES/vistafileutilities.h"

class CRefManager;
class CGameStateManager;
class CKeyHandler;
class CSettings;
class CSprite;
class CFontMetric;

enum EOptionsMenuCode
{
	KOptionsMenuDone,
	KOptionsMenuRandomName,
	KOptionsMenuResolution,
	KOptionsMenuShadows,
	KOptionsMenuShadowSize,
	KOptionsMenuParticles,
	KOptionsMenuFSAA,
	KOptionsMenuGamma,
	KOptionsMenuGlows,
	KOptionsMenuVSYNC,
	KOptionsMenuFog,
	KOptionsMenuMusicVolume,
	KOptionsMenuSoundVolume,
	KOptionsMenuTips,
	KOptionsMenuCodes
};

enum EShadowSize
{
	KShadow128,
	KShadow256,
	KShadow512,
	KShadow1024,
	KShadowSizes
};
const uint32 KShadowSize[KShadowSizes] = 
{
	128, 256, 512, 1024
};

const std::string KShadowSizeNames[KShadowSizes] = 
{
	LoadLocalizedString(1133), LoadLocalizedString(1134), LoadLocalizedString(1135), LoadLocalizedString(1136)
};

enum EResolution
{
	K640x480,
	K800x600,
	K1024x768,
	K1152x864,
	K1280x1024,
	K1600x1200,
	K1280x768,
	K1366x768,
	K1920x1080,
	K1920x1200,
	KResolutions
};

const ESettings KResolutionSettings[KResolutions] = 
{
	KSet640x480Available,
	KSet800x600Available,
	KSet1024x768Available,
	KSet1152x864Available,
	KSet1280x1024Available,
	KSet1600x1200Available,
	KSet1280x768Available,
	KSet1366x768Available,
	KSet1920x1080Available,
	KSet1920x1200Available
};

const uint32 KResolutionWidth[KResolutions] = 
{
	640, 800, 1024, 1152, 1280, 1600, 1280, 1366, 1920, 1920
};

const uint32 KResolutionHeight[KResolutions] = 
{
	480, 600, 768, 864, 1024, 1200, 768, 768, 1080, 1200
};

const std::string KResolutionNames[KResolutions] = 
{
	LoadLocalizedString(1066), LoadLocalizedString(1050), LoadLocalizedString(1067), LoadLocalizedString(1068), LoadLocalizedString(1069), LoadLocalizedString(1070), LoadLocalizedString(1071), "1366x768", "1920x1080", LoadLocalizedString(1072)
};

enum EShadows
{
	KShadowsOff,
	KPlayerOnly,
	KCharactersOnly,
	KCharactersAndObjects,
	KShadows
};

const std::string KShadowNames[KShadows] =
{
	LoadLocalizedString(1059),
	LoadLocalizedString(1073),
	LoadLocalizedString(1074),
	LoadLocalizedString(1075)
};

enum EParticles
{
	KParticlesLow,
	KParticlesMed,
	KParticlesHigh,
	KParticles
};

const std::string KParticleNames[KParticles] =
{
	LoadLocalizedString(1133),
	LoadLocalizedString(1134),
	LoadLocalizedString(1135)
};

enum EFSAA
{
	KFSAAOff,
	KFSAAOn,
	KFSAA
};

const std::string KFSAANames[KFSAA] =
{
	LoadLocalizedString(1059),
	LoadLocalizedString(1052),
};

enum EVSYNC
{
	KVSYNCOff,
	KVSYNCOn,
	KVSYNCs
};

const std::string KVSYNCNames[KVSYNCs] =
{
	LoadLocalizedString(1059),
	LoadLocalizedString(1052),
};

enum EFog
{
	KFogPlain,
	KFogAnimated,
	KFogs
};

const std::string KFogNames[KFogs] =
{
	LoadLocalizedString(1076),
	LoadLocalizedString(1077)
};

enum EGlow
{
	KGlowOff,
	KGlowOn,
	KGlows
};

const std::string KGlowNames[KGlows] =
{
	LoadLocalizedString(1059),
	LoadLocalizedString(1052),
};
const int KResolutionCode = 1000;
const int KShadowCode = 2000;
const int KShadowSizeCode = 3000;
const int KParticleCode = 4000;
const int KFSAACode = 5000;
const int KGlowCode = 6000;
const int KVSYNCCode = 7000;
const int KFogCode = 8000;



class COptionsMenu:
		public CMenu
{
public:
	COptionsMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
			   CSettings& pSettings,					//we need this for tracking user preferences
			   CGameStateManager& pGameStateManager,	// game manager
			   CMenuManager& pMenuManager );			// menu manager

	~COptionsMenu( void );

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

private:

	void	UpdateTipCheck( void );
	

	void	ResetButtons( LPDIRECT3DDEVICE8 pD3DDevice );

	// these methods are overriding the methods in CMenu that this
	// class is derived from

	void	ProcessInput( HWND hWnd );								// hwnd of the active window

	FSOUND_SAMPLE*	m_pClickSound;

	// our logo texture
	CMaterial*	m_pLogoMaterial;

	// basic button material
	CMaterial*	m_pButtonMaterial;
	// dropdown butotn material
	CMaterial*	m_pDropdownMaterial;

	// font texture and metric
	CMaterial*		m_pFontMaterial;
	CFontMetric*	m_pFontMetric;

	CButton*		m_pResolutionButton;
	CDropdown*		m_pResolutionDropdown;

	CButton*		m_pShadowButton;
	CDropdown*		m_pShadowDropdown;

	CButton*		m_pShadowSizeButton;
	CDropdown*		m_pShadowSizeDropdown;

	CButton*		m_pParticleButton;
	CDropdown*		m_pParticleDropdown;

	CButton*		m_pFSAAButton;
	CDropdown*		m_pFSAADropdown;

	CButton*		m_pVSYNCButton;
	CDropdown*		m_pVSYNCDropdown;

	CButton*		m_pFogButton;
	CDropdown*		m_pFogDropdown;

	CButton*		m_pGlowButton;
	CDropdown*		m_pGlowDropdown;

	CSprite*		m_pSoundSprite;
	CSprite*		m_pMusicSprite;
	CSprite*		m_pGammaSprite;

	CButton*		m_pCheckTipButton;

	bool			m_SoundDragging;
	bool			m_MusicDragging;
	bool			m_GammaDragging;
};

#endif
