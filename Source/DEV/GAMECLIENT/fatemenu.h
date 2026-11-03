/*
	fatemenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _FATEMENU_H
#define _FATEMENU_H

#include "../IO/keyhandler.h"
#include "../IO/mousehandler.h"
#include "../GAME/gamestatemanager.h"

class CKeyHandler;
class CSprite;
class CMaterial;
class CGameStateManager;
class CCharacter;
class CButton;
class CText;
class CItem;
class CCamera;
class CGenericModel;
class CGameClient;
class CGlowManager;
class CGlow;
class CParticleManager;
class CScreenFade;

// manage the 'fate' menu

class CFateMenu
{
public:
				CFateMenu( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
							CRefManager& pRefManager,		// the reference manager, for tracking media we create
							CSettings& pSettings,			//we need this for tracking user preferences
						  	CParticleManager& pParticleManager,		// particle manager
							CGlowManager& pGlowManager,				// glow manager
							CGameStateManager& GameStateManager );	// to determine whether needed media exists


				~CFateMenu( void );

	void	Update( HWND hWnd,								// hwnd of the active window
				    CLevel& pLevel,						// active level
					float32 TimeElapsed );					// time elapsed, in seconds

	void	Render3D( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
					  CGameClient& pClient );				// game client

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
				    const D3DXMATRIX& ViewportMatri,		// viewport matrix ( for bringing 3d objects to screen space
					const D3DXMATRIX& ProjectionMatrix );	// projection matrix  ( for bringing 3d objects to screen space


	void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					  WPARAM KeyCode );	// the wparam which will be the keycode

	void	MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONUP, etc.
					    WPARAM WMParam );	// the wparam which contains mousewheel info

	void	RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////
	
	bool	Open( void )			{	return m_Open;		};


	bool	MouseOver( void );

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	void	FlushAll( void );

	void	SetPlayerCharacter( CCharacter* pCharacter )		{	m_pPlayerCharacter = pCharacter;		};

	void	SetOpen( bool Open );

	void	Open( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
				  CLevel& pLevel );							// active level

	int32	UpdateButtonPress( bool ButtonPressed );	// is the left mouse button pressed?

	int32	UpdateButtonOver( void );

	int32	UpdateButtonOver( std::string& TooltipText );	// tooltip text to fill out

private:
	void	CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	void	ProcessInput( HWND hWnd );						// hwnd of the active window

	void			BringToFront( CSprite* pSprite );			// pointer to sprite in the internal list

	CSprite*		AddSprite( CSprite* pSprite );				// pointer to sprite to add to internal list

	CSprite*		AddSpriteFront( CSprite* pSprite );			// pointer to sprite to add to internal list

	void			RemoveSprite( CSprite* pSprite );			// pointer to sprite to remove from internal list

	void			DestroySprites( void );

	void			RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	CButton*		AddButton( CButton* pButton );			// pointer to button to add to internal list

	void			RemoveButton( CButton* pButton );		// pointer to button to remove from internal list

	void			UpdateButtons( float32 TimeElapsed );		// time elapsed in seconds

	void			DestroyButtons();

	void			RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	CText*			AddText( CText* pText );			// pointer to text to add to internal list

	void			RemoveText( CText* pText );			// pointer to text to remove from internal list

	void			DestroyText();

	void			RenderText( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	void			EmitParticles( void );

	void			FindParticles( void );

	void			ClearParticles( void );

	CKeyHandler				m_KeyHandler;
	CMouseHandler			m_MouseHandler;

	CSettings&				m_pSettings;		
	CRefManager&			m_pRefManager;
	CGameStateManager&		m_pGameStateManager;

	CLList<CButton>*		m_pButtonList;
	CLList<CSprite>*		m_pSpriteList;
	CLList<CText>*			m_pTextList;

	// font texture and metric
	CMaterial*		m_pFontMaterial;
	CFontMetric*	m_pFontMetric;


	CCharacter*		m_pPlayerCharacter;

	CMaterial*		m_pUIButtons;

	CText*			m_pChoice1Text;
	CText*			m_pChoice2Text;
	CText*			m_pChoice3Text;
	CButton*		m_pChoice1Button;
	CButton*		m_pChoice2Button;
	CButton*		m_pChoice3Button;

	bool			m_Open;

	FSOUND_SAMPLE*	m_pClickSound;
	FSOUND_SAMPLE*				m_pFateSound;

	std::string		m_Message;

	int32			m_FameLoss1;
	int32			m_ExperienceLoss1;
	int32			m_GoldLoss1;

	int32			m_FameLoss2;
	int32			m_ExperienceLoss2;
	int32			m_GoldLoss2;

	int32			m_FameLoss3;
	int32			m_ExperienceLoss3;
	int32			m_GoldLoss3;

	CCamera*		m_pCamera;

	CGenericModel*	m_pFateModel;

	std::vector< CGlow* >		m_pGlowList;
	std::vector< CGlow* >		m_pModelGlowList;
	std::vector< uint32 >		m_GlowIndex;
	std::vector< D3DXVECTOR3 >	m_GlowPosition;


	std::vector< D3DXVECTOR3 >	m_ParticlePosition;
	std::vector< D3DXMATRIX >	m_ParticleOrientation;
	std::vector< int32 >		m_ParticleType;

	std::vector< uint32 >		m_TagParticleIndex;
	std::vector< int32 >		m_TagParticleType;

	CParticleManager&			m_pParticleManager;
	CGlowManager&				m_pGlowManager;

	D3DXMATRIX					m_DisplayMatrix;

	int32						m_FateSoundChannel;

	CScreenFade*	m_pScreenFade;

	float32						m_FadeValue;
	float32						m_FadeDirection;

	bool						m_RequestingQuit;
};

#endif