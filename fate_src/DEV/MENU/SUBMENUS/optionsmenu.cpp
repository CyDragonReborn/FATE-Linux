/*
	optionsmenu.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/8/2004

*/


#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>
#include <math.h>

#include "../../UTILITIES/constants.h"
#include "../../UTILITIES/utilities.h"
#include "../../UTILITIES/capabilities.h"
#include "../../UTILITIES/math3d.h"
#include "../menu.h"
#include "../../UI/fontmetric.h"
#include "../../UTILITIES/spawn.h"
#include "../../NAMES/namegenerator.h"
#include "../../UTILITIES/loadlocalizedstring.h"

#include "optionsmenu.h"


COptionsMenu::COptionsMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
					  CSettings& pSettings,		// the settings manager, for keeping track of card/machine capabilities
					  CGameStateManager& pGameStateManager,	// game manager
					  CMenuManager& pMenuManager ) :		// menu manager
														CMenu( pRefManager,
																pSettings,
																pGameStateManager,
																pMenuManager ),
														m_pLogoMaterial( NULL ),
														m_pFontMetric( NULL ),
														m_pButtonMaterial( NULL ),
														m_pDropdownMaterial( NULL ),
														m_pFontMaterial( NULL ),
														m_pSoundSprite( NULL ),
														m_pGammaSprite( NULL ),
														m_pMusicSprite( NULL ),
														m_SoundDragging( kFalse ),
														m_MusicDragging( kFalse ),
														m_GammaDragging( kFalse ),
														m_pClickSound( NULL ),
														m_pResolutionButton( NULL ),
														m_pResolutionDropdown( NULL ),
														m_pShadowButton( NULL ),
														m_pShadowDropdown( NULL ),
														m_pShadowSizeButton( NULL ),
														m_pShadowSizeDropdown( NULL ),
														m_pParticleButton( NULL ),
														m_pParticleDropdown( NULL ),
														m_pFSAAButton( NULL ),
														m_pFSAADropdown( NULL ),
														m_pVSYNCButton( NULL ),
														m_pVSYNCDropdown( NULL ),
														m_pFogButton( NULL ),
														m_pFogDropdown( NULL ),
														m_pGlowButton( NULL ),
														m_pGlowDropdown( NULL ),
														m_pCheckTipButton( NULL )
{

} // COptionsMenu::COptionsMenu()

COptionsMenu::~COptionsMenu( void )
{
	DELETE_SAFELY( m_pSoundSprite );
	DELETE_SAFELY( m_pMusicSprite );
	DELETE_SAFELY( m_pGammaSprite );

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );
	m_pRefManager.RemoveMaterial( m_pLogoMaterial );
	m_pRefManager.RemoveMaterial( m_pButtonMaterial );
	m_pRefManager.RemoveMaterial( m_pDropdownMaterial );

} // COptionsMenu::~COptionsMenu()

void COptionsMenu::Activate( void )
{
	CMenu::Activate();

	UpdateTipCheck();
} // COptionsMenu::Activate()

void COptionsMenu::UpdateTipCheck( void )
{
	if( m_pSettings.GetSettings( KSetShowTips ) )
	{
		m_pCheckTipButton->SetUVs( 182 * m_pButtonMaterial->PixelToU(),
									196 * m_pButtonMaterial->PixelToV(),
									251 * m_pButtonMaterial->PixelToU(),
									242 * m_pButtonMaterial->PixelToV() );
		m_pCheckTipButton->SetOverUVs( 182 * m_pButtonMaterial->PixelToU(),
										196 * m_pButtonMaterial->PixelToV(),
										251 * m_pButtonMaterial->PixelToU(),
										242 * m_pButtonMaterial->PixelToV() );
	}
	else
	{
		m_pCheckTipButton->SetUVs( 111 * m_pButtonMaterial->PixelToU(),
									196 * m_pButtonMaterial->PixelToV(),
									180 * m_pButtonMaterial->PixelToU(),
									242 * m_pButtonMaterial->PixelToV() );
		m_pCheckTipButton->SetOverUVs( 111 * m_pButtonMaterial->PixelToU(),
										196 * m_pButtonMaterial->PixelToV(),
										180 * m_pButtonMaterial->PixelToU(),
										242 * m_pButtonMaterial->PixelToV() );
	}

} // COptionsMenu::UpdateTipCheck()

// perform creation of any elements that this menu needs
void COptionsMenu::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	// we need to update the buttons first thing
	UpdateElements();

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );
	m_pFontMetric->SetCharacterHeight( 24 );

	m_pLogoMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													  "MENUS/logo.png",
													  "MENUS/logoa.png" );
	m_pButtonMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													    "MENUS/buttons.png",
													    "MENUS/buttonsa.png" );
	m_pDropdownMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													      "MENUS/dropdown.png",
													      "MENUS/dropdowna.png" );

	// game logo
	AddSprite( new CSprite( pD3DDevice,
							m_pLogoMaterial,
							16,
							20,
							512,
							256,
							0,
							0,
							1,
							1,
							KSpriteTopLeft ) );

	
// new game button
	CButton* pButton;
	CDropdown* pDropdown;

// options button
	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  700,
									  640,
									  256,
									  48,
									  0 * m_pButtonMaterial->PixelToU(),
									  49 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  97 * m_pButtonMaterial->PixelToV(),
									  0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV(),
									  KButtonTopLeft,
									  KOptionsMenuDone ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(915),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(915),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );


	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1044),
						100,
						280,
						1.0f,
						kFalse,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1049),
						150,
						320,
						1.0f,
						kTrue,
						KJustifyLeft ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pDropdownMaterial,
									  NULL,
									  m_pClickSound,
									  150,
									  350,
									  256,
									  40,
									  0 * m_pDropdownMaterial->PixelToU(),
									  1 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  0 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  81 * m_pDropdownMaterial->PixelToV(),
									  KButtonTopLeft,
									  KOptionsMenuResolution ) );
	m_pResolutionButton = pButton;

	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1050),
					  m_pFontMaterial,
					  m_pFontMetric,
					  25,
					  8,
					  .75f,
					  kFalse,
					  KJustifyLeft );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1050),
						m_pFontMaterial,
						m_pFontMetric,
						25,
						8,
						.75f,
						kFalse,
						KJustifyLeft );

	pDropdown = AddDropdown( new CDropdown( pD3DDevice,
											m_pDropdownMaterial,
											m_pFontMaterial,
											m_pFontMetric,
											150,
											390,
											256,
											17,
											17,
											20,
											.75f,
											25,
											0,
											0 * m_pDropdownMaterial->PixelToU(),
											84 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											100 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											103 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											104 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											110 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											127 * m_pDropdownMaterial->PixelToV() ) );

	for( uint32 i = 0; i < KResolutions; i++ )
	{
		if( m_pSettings.GetSettings( KResolutionSettings[i] ) )
		{
			pDropdown->AddElement( KResolutionNames[i],
								KResolutionCode + i );
		}
	}

	m_pResolutionDropdown = pDropdown;

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1051),
						450,
						320,
						1.0f,
						kTrue,
						KJustifyLeft ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pDropdownMaterial,
									  NULL,
									  m_pClickSound,
									  450,
									  350,
									  256,
									  40,
									  0 * m_pDropdownMaterial->PixelToU(),
									  1 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  0 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  81 * m_pDropdownMaterial->PixelToV(),
									  KButtonTopLeft,
									  KOptionsMenuGlows ) );
	m_pGlowButton = pButton;

	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1052),
					  m_pFontMaterial,
					  m_pFontMetric,
					  25,
					  8,
					  .75f,
					  kFalse,
					  KJustifyLeft );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1052),
						m_pFontMaterial,
						m_pFontMetric,
						25,
						8,
						.75f,
						kFalse,
						KJustifyLeft );

	pDropdown = AddDropdown( new CDropdown( pD3DDevice,
											m_pDropdownMaterial,
											m_pFontMaterial,
											m_pFontMetric,
											450,
											390,
											256,
											17,
											17,
											20,
											.75f,
											25,
											0,
											0 * m_pDropdownMaterial->PixelToU(),
											84 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											100 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											103 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											104 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											110 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											127 * m_pDropdownMaterial->PixelToV() ) );

	if( m_pSettings.Capabilities().IsCapable( KCanRenderToTexture ) )
	{
		for( uint32 i = 0; i < KGlows; i++ )
		{
			pDropdown->AddElement( KGlowNames[i],
								KGlowCode + i );
		}
	}
	else
	{
		pDropdown->AddElement( KGlowNames[0],
							   KGlowCode );
	}

	m_pGlowDropdown = pDropdown;


	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1053),
						150,
						400,
						1.0f,
						kTrue,
						KJustifyLeft ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pDropdownMaterial,
									  NULL,
									  m_pClickSound,
									  150,
									  430,
									  256,
									  40,
									  0 * m_pDropdownMaterial->PixelToU(),
									  1 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  0 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  81 * m_pDropdownMaterial->PixelToV(),
									  KButtonTopLeft,
									  KOptionsMenuShadows ) );
	m_pShadowButton = pButton;

	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1054),
					  m_pFontMaterial,
					  m_pFontMetric,
					  25,
					  8,
					  .75f,
					  kFalse,
					  KJustifyLeft );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1054),
						m_pFontMaterial,
						m_pFontMetric,
						25,
						8,
						.75f,
						kFalse,
						KJustifyLeft );

	pDropdown = AddDropdown( new CDropdown( pD3DDevice,
											m_pDropdownMaterial,
											m_pFontMaterial,
											m_pFontMetric,
											150,
											470,
											256,
											17,
											17,
											20,
											.75f,
											25,
											0,
											0 * m_pDropdownMaterial->PixelToU(),
											84 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											100 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											103 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											104 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											110 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											127 * m_pDropdownMaterial->PixelToV() ) );

	if( m_pSettings.GetSettings( KSetProjectedShadows ) )
	{
		for( uint32 i = 0; i < KShadows; i++ )
		{
			pDropdown->AddElement( KShadowNames[i],
								KShadowCode + i );
		}
	}
	else
	{
		pDropdown->AddElement( KShadowNames[0],
							   KShadowCode );
	}

	m_pShadowDropdown = pDropdown;


	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1055),
						450,
						400,
						1.0f,
						kTrue,
						KJustifyLeft ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pDropdownMaterial,
									  NULL,
									  m_pClickSound,
										450,
										430,
										256,
										40,
										0 * m_pDropdownMaterial->PixelToU(),
										1 * m_pDropdownMaterial->PixelToV(),
										256 * m_pDropdownMaterial->PixelToU(),
										41 * m_pDropdownMaterial->PixelToV(),
										0 * m_pDropdownMaterial->PixelToU(),
										41 * m_pDropdownMaterial->PixelToV(),
										256 * m_pDropdownMaterial->PixelToU(),
										81 * m_pDropdownMaterial->PixelToV(),
										KButtonTopLeft,
										KOptionsMenuShadowSize ) );
		m_pShadowSizeButton = pButton;


		pButton->AddText( pD3DDevice,
						LoadLocalizedString(1056),
						m_pFontMaterial,
						m_pFontMetric,
						25,
						8,
						.75f,
						kFalse,
						KJustifyLeft );
		pButton->AddOverText( pD3DDevice,
							LoadLocalizedString(1056),
							m_pFontMaterial,
							m_pFontMetric,
							25,
							8,
							.75f,
							kFalse,
							KJustifyLeft );
		

		pDropdown = AddDropdown( new CDropdown( pD3DDevice,
												m_pDropdownMaterial,
												m_pFontMaterial,
												m_pFontMetric,
												450,
												470,
												256,
												17,
												17,
												20,
												.75f,
												25,
												0,
												0 * m_pDropdownMaterial->PixelToU(),
												84 * m_pDropdownMaterial->PixelToV(),
												256 * m_pDropdownMaterial->PixelToU(),
												100 * m_pDropdownMaterial->PixelToV(),
												0 * m_pDropdownMaterial->PixelToU(),
												103 * m_pDropdownMaterial->PixelToV(),
												256 * m_pDropdownMaterial->PixelToU(),
												104 * m_pDropdownMaterial->PixelToV(),
												0 * m_pDropdownMaterial->PixelToU(),
												110 * m_pDropdownMaterial->PixelToV(),
												256 * m_pDropdownMaterial->PixelToU(),
												127 * m_pDropdownMaterial->PixelToV() ) );

		for( int32 i = 0; i < KShadowSizes; i++ )
		{
			pDropdown->AddElement( KShadowSizeNames[i],
								   KShadowSizeCode + i );
		}

		m_pShadowSizeDropdown = pDropdown;


	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1057),
						150,
						480,
						1.0f,
						kTrue,
						KJustifyLeft ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pDropdownMaterial,
									  NULL,
									  m_pClickSound,
									  150,
									  510,
									  256,
									  40,
									  0 * m_pDropdownMaterial->PixelToU(),
									  1 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  0 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  81 * m_pDropdownMaterial->PixelToV(),
									  KButtonTopLeft,
									  KOptionsMenuParticles ) );

	m_pParticleButton = pButton;

	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1056),
					  m_pFontMaterial,
					  m_pFontMetric,
					  25,
					  8,
					  .75f,
					  kFalse,
					  KJustifyLeft );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1056),
						m_pFontMaterial,
						m_pFontMetric,
						25,
						8,
						.75f,
						kFalse,
						KJustifyLeft );

	pDropdown = AddDropdown( new CDropdown( pD3DDevice,
											m_pDropdownMaterial,
											m_pFontMaterial,
											m_pFontMetric,
											150,
											550,
											256,
											17,
											17,
											20,
											.75f,
											25,
											0,
											0 * m_pDropdownMaterial->PixelToU(),
											84 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											100 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											103 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											104 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											110 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											127 * m_pDropdownMaterial->PixelToV() ) );

	for( int32 i = 0; i < KParticles; i++ )
	{
		pDropdown->AddElement( KParticleNames[i],
							   KParticleCode + i );
	}

	m_pParticleDropdown = pDropdown;


	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1058),
						450,
						480,
						1.0f,
						kTrue,
						KJustifyLeft ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pDropdownMaterial,
									  NULL,
									  m_pClickSound,
									  450,
									  510,
									  256,
									  40,
									  0 * m_pDropdownMaterial->PixelToU(),
									  1 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  0 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  81 * m_pDropdownMaterial->PixelToV(),
									  KButtonTopLeft,
									  KOptionsMenuVSYNC ) );

	m_pVSYNCButton = pButton;

	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1059),
					  m_pFontMaterial,
					  m_pFontMetric,
					  25,
					  8,
					  .75f,
					  kFalse,
					  KJustifyLeft );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1059),
						m_pFontMaterial,
						m_pFontMetric,
						25,
						8,
						.75f,
						kFalse,
						KJustifyLeft );

	pDropdown = AddDropdown( new CDropdown( pD3DDevice,
											m_pDropdownMaterial,
											m_pFontMaterial,
											m_pFontMetric,
											450,
											550,
											256,
											17,
											17,
											20,
											.75f,
											25,
											0,
											0 * m_pDropdownMaterial->PixelToU(),
											84 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											100 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											103 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											104 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											110 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											127 * m_pDropdownMaterial->PixelToV() ) );

	for( int32 i = 0; i < KVSYNCs; i++ )
	{
		pDropdown->AddElement( KVSYNCNames[i],
							   KVSYNCCode + i );
	}

	m_pVSYNCDropdown = pDropdown;




	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1060),
						450,
						560,
						1.0f,
						kTrue,
						KJustifyLeft ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pDropdownMaterial,
									  NULL,
									  m_pClickSound,
									  450,
									  590,
									  256,
									  40,
									  0 * m_pDropdownMaterial->PixelToU(),
									  1 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  0 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  81 * m_pDropdownMaterial->PixelToV(),
									  KButtonTopLeft,
									  KOptionsMenuFog ) );

	m_pFogButton = pButton;

	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1059),
					  m_pFontMaterial,
					  m_pFontMetric,
					  25,
					  8,
					  .75f,
					  kFalse,
					  KJustifyLeft );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1059),
						m_pFontMaterial,
						m_pFontMetric,
						25,
						8,
						.75f,
						kFalse,
						KJustifyLeft );

	pDropdown = AddDropdown( new CDropdown( pD3DDevice,
											m_pDropdownMaterial,
											m_pFontMaterial,
											m_pFontMetric,
											450,
											630,
											256,
											17,
											17,
											20,
											.75f,
											25,
											0,
											0 * m_pDropdownMaterial->PixelToU(),
											84 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											100 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											103 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											104 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											110 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											127 * m_pDropdownMaterial->PixelToV() ) );

	for( int32 i = 0; i < KFogs; i++ )
	{
		pDropdown->AddElement( KFogNames[i],
							   KFogCode + i );
	}

	m_pFogDropdown = pDropdown;

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1061),
						150,
						560,
						1.0f,
						kTrue,
						KJustifyLeft ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pDropdownMaterial,
									  NULL,
									  m_pClickSound,
									  150,
									  590,
									  256,
									  40,
									  0 * m_pDropdownMaterial->PixelToU(),
									  1 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  0 * m_pDropdownMaterial->PixelToU(),
									  41 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  81 * m_pDropdownMaterial->PixelToV(),
									  KButtonTopLeft,
									  KOptionsMenuFSAA ) );
	m_pFSAAButton = pButton;

	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1052),
					  m_pFontMaterial,
					  m_pFontMetric,
					  25,
					  8,
					  .75f,
					  kFalse,
					  KJustifyLeft );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1052),
						m_pFontMaterial,
						m_pFontMetric,
						25,
						8,
						.75f,
						kFalse,
						KJustifyLeft );

	pDropdown = AddDropdown( new CDropdown( pD3DDevice,
											m_pDropdownMaterial,
											m_pFontMaterial,
											m_pFontMetric,
											150,
											630,
											256,
											17,
											17,
											20,
											.75f,
											25,
											0,
											0 * m_pDropdownMaterial->PixelToU(),
											84 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											100 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											103 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											104 * m_pDropdownMaterial->PixelToV(),
											0 * m_pDropdownMaterial->PixelToU(),
											110 * m_pDropdownMaterial->PixelToV(),
											256 * m_pDropdownMaterial->PixelToU(),
											127 * m_pDropdownMaterial->PixelToV() ) );

	for( int32 i = 0; i < KFSAA; i++ )
	{
		pDropdown->AddElement( KFSAANames[i],
							   KFSAACode + i );
	}

	m_pFSAADropdown = pDropdown;

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1062),
						150,
						640,
						1.0f,
						kTrue,
						KJustifyLeft ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pDropdownMaterial,
									  NULL,
									  m_pClickSound,
									  150,
									  670,
									  256,
									  34,
									  0 * m_pDropdownMaterial->PixelToU(),
									  128 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  162 * m_pDropdownMaterial->PixelToV(),
									  0 * m_pDropdownMaterial->PixelToU(),
									  128 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  162 * m_pDropdownMaterial->PixelToV(),
									  KButtonTopLeft,
									  KOptionsMenuGamma ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1063),
						550,
						120,
						1.0f,
						kTrue,
						KJustifyLeft ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pDropdownMaterial,
									  NULL,
									  m_pClickSound,
									  550,
									  150,
									  256,
									  34,
									  0 * m_pDropdownMaterial->PixelToU(),
									  128 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  162 * m_pDropdownMaterial->PixelToV(),
									  0 * m_pDropdownMaterial->PixelToU(),
									  128 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  162 * m_pDropdownMaterial->PixelToV(),
									  KButtonTopLeft,
									  KOptionsMenuSoundVolume ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1064),
						550,
						200,
						1.0f,
						kTrue,
						KJustifyLeft ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pDropdownMaterial,
									  NULL,
									  m_pClickSound,
									  550,
									  230,
									  256,
									  34,
									  0 * m_pDropdownMaterial->PixelToU(),
									  128 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  162 * m_pDropdownMaterial->PixelToV(),
									  0 * m_pDropdownMaterial->PixelToU(),
									  128 * m_pDropdownMaterial->PixelToV(),
									  256 * m_pDropdownMaterial->PixelToU(),
									  162 * m_pDropdownMaterial->PixelToV(),
									  KButtonTopLeft,
									  KOptionsMenuMusicVolume ) );

	m_pSoundSprite = new CSprite( pD3DDevice, 
								  m_pDropdownMaterial,
								  0,
								  0,
								  35,
								  45,
								  0 * m_pDropdownMaterial->PixelToU(),
								  211 * m_pDropdownMaterial->PixelToV(),
								  35 * m_pDropdownMaterial->PixelToU(),
								  256 * m_pDropdownMaterial->PixelToV(),
								  KSpriteCenter );

	m_pMusicSprite = new CSprite( pD3DDevice, 
								  m_pDropdownMaterial,
								  0,
								  0,
								  35,
								  45,
								  0 * m_pDropdownMaterial->PixelToU(),
								  211 * m_pDropdownMaterial->PixelToV(),
								  35 * m_pDropdownMaterial->PixelToU(),
								  256 * m_pDropdownMaterial->PixelToV(),
								  KSpriteCenter );

	m_pGammaSprite = new CSprite( pD3DDevice, 
								  m_pDropdownMaterial,
								  0,
								  0,
								  35,
								  45,
								  0 * m_pDropdownMaterial->PixelToU(),
								  211 * m_pDropdownMaterial->PixelToV(),
								  35 * m_pDropdownMaterial->PixelToU(),
								  256 * m_pDropdownMaterial->PixelToV(),
								  KSpriteCenter );

	m_pCheckTipButton = AddButton( new CButton( pD3DDevice,
												m_pButtonMaterial,
												NULL,
												m_pClickSound,
												760,
												35,
												70,
												47,
												111 * m_pButtonMaterial->PixelToU(),
												196 * m_pButtonMaterial->PixelToV(),
												180 * m_pButtonMaterial->PixelToU(),
												242 * m_pButtonMaterial->PixelToV(),
												111 * m_pButtonMaterial->PixelToU(),
												196 * m_pButtonMaterial->PixelToV(),
												180 * m_pButtonMaterial->PixelToU(),
												242 * m_pButtonMaterial->PixelToV(),
												KButtonTopLeft,
												KOptionsMenuTips ) );


	AddText( new CText( pD3DDevice,
						 m_pFontMaterial,
						 m_pFontMetric,
						 LoadLocalizedString(1065),
						 550,
						 40,
						 1.0f,
						 kTrue,
						 KJustifyLeft ) );

} // COptionsMenu::Create()

void COptionsMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	CMenu::KeyEvent( WMMessage, KeyCode );


} // COptionsMenu::KeyEvent()


void COptionsMenu::ProcessInput( HWND hWnd )								// hwnd of the active window
{
	m_MouseHandler.Update( hWnd );
	if( !m_MouseHandler.ButtonHeld( KButtonLeft ) )
	{
		m_SoundDragging = kFalse;
		m_MusicDragging = kFalse;
		m_GammaDragging = kFalse;
	}
	int32 ReturnCode = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) );

	switch( ReturnCode )
	{

	// if pressed, we go to main menu
	case KOptionsMenuDone :
		m_pGameStateManager.SetTargetMenu( KMainMenu );
		break;
	// if pressed, open resolution dropdown
	case KOptionsMenuResolution :
		m_pResolutionDropdown->ToggleOpen();
		break;
	// if pressed, open glow dropdown
	case KOptionsMenuGlows :
		m_pGlowDropdown->ToggleOpen();
		break;
	// if pressed, open shadow dropdown
	case KOptionsMenuShadows :
		m_pShadowDropdown->ToggleOpen();
		break;
	// if pressed, open shadow size dropdown
	case KOptionsMenuShadowSize :
		m_pShadowSizeDropdown->ToggleOpen();
		break;
	// if pressed, open particle dropdown
	case KOptionsMenuParticles :
		m_pParticleDropdown->ToggleOpen();
		break;
	// if pressed, open FSAA dropdown
	case KOptionsMenuFSAA :
		m_pFSAADropdown->ToggleOpen();
		break;
	// if pressed, open VSYNC dropdown
	case KOptionsMenuVSYNC :
		m_pVSYNCDropdown->ToggleOpen();
		break;
	// if pressed, open FOG dropdown
	case KOptionsMenuFog :
		m_pFogDropdown->ToggleOpen();
		break;
	case KOptionsMenuSoundVolume :
		m_SoundDragging = kTrue;
		break;
	// if pressed, we set music volume
	case KOptionsMenuMusicVolume :
		m_MusicDragging = kTrue;
		break;
	// if pressed, we set gamma
	case KOptionsMenuGamma :
		m_GammaDragging = kTrue;
		break;
	case KOptionsMenuTips :
		m_pSettings.SetSettings( KSetShowTips, !m_pSettings.GetSettings( KSetShowTips ) );
		UpdateTipCheck();
		break;
	}

	// we selected a resolution from the dropdown
	if( ReturnCode >= KResolutionCode &&
		ReturnCode < KResolutionCode + KResolutions )
	{
		ReturnCode -= KResolutionCode;
		if( KResolutionWidth[ReturnCode] !=
			m_pSettings.GetSettings( KSetScreenWidth ) )
		{
			m_pSettings.SetSettings( KSetScreenWidth, KResolutionWidth[ReturnCode] );
			m_pSettings.SetSettings( KSetScreenHeight, KResolutionHeight[ReturnCode] );
			UpdateElements();
		}
		CloseDropdowns();
	}

	// we selected a resolution from the dropdown
	if( ReturnCode >= KShadowSizeCode &&
		ReturnCode < KShadowSizeCode + KShadowSizes )
	{
		ReturnCode -= KShadowSizeCode;
		if( KShadowSize[ReturnCode] !=
			m_pSettings.GetSettings( KSetShadowResolution ) )
		{
			m_pSettings.SetSettings( KSetShadowResolution, KShadowSize[ReturnCode] );
			UpdateElements();
		}
		CloseDropdowns();
	}

	// we selected shadows from the dropdown
	if( ReturnCode >= KShadowCode &&
		ReturnCode < KShadowCode + KShadows )
	{
		ReturnCode -= KShadowCode;
		if( ReturnCode !=
			m_pSettings.GetSettings( KSetShadowDetail ) )
		{
			m_pSettings.SetSettings( KSetShadowDetail, ReturnCode );
			m_pSettings.VerifySettings();
			UpdateElements();
		}
		CloseDropdowns();
	}

	// we selected partilces from the dropdown
	if( ReturnCode >= KParticleCode &&
		ReturnCode < KParticleCode + KParticles )
	{
		ReturnCode -= KParticleCode;
		if( ReturnCode !=
			m_pSettings.GetSettings( KSetParticleDetail ) )
		{
			m_pSettings.SetSettings( KSetParticleDetail, ReturnCode );
			UpdateElements();
		}
		CloseDropdowns();
	}

	// we selected FSAA from the dropdown
	if( ReturnCode >= KFSAACode &&
		ReturnCode < KFSAACode + KFSAA )
	{
		ReturnCode -= KFSAACode;
		if( ReturnCode !=
			m_pSettings.GetSettings( KSetFSAA ) )
		{
			m_pSettings.SetSettings( KSetFSAA, ReturnCode );
			UpdateElements();
		}
		CloseDropdowns();
	}

	// we selected VSYNC from the dropdown
	if( ReturnCode >= KVSYNCCode &&
		ReturnCode < KVSYNCCode + KVSYNCs )
	{
		ReturnCode -= KVSYNCCode;
		if( ReturnCode !=
			m_pSettings.GetSettings( KSetVSYNC ) )
		{
			m_pSettings.SetSettings( KSetVSYNC, ReturnCode );
			UpdateElements();
		}
		CloseDropdowns();
	}

	// we selected VSYNC from the dropdown
	if( ReturnCode >= KFogCode &&
		ReturnCode < KFogCode + KFogs )
	{
		ReturnCode -= KFogCode;
		if( ReturnCode !=
			m_pSettings.GetSettings( KSetAnimatedFog ) )
		{
			m_pSettings.SetSettings( KSetAnimatedFog, ReturnCode );
			UpdateElements();
		}
		CloseDropdowns();
	}

	// we selected glows from the dropdown
	if( ReturnCode >= KGlowCode &&
		ReturnCode < KGlowCode + KGlows )
	{
		ReturnCode -= KGlowCode;
		if( ReturnCode !=
			m_pSettings.GetSettings( KSetMotionBlur ) )
		{
			m_pSettings.SetSettings( KSetMotionBlur, ReturnCode );
			UpdateElements();
		}
		CloseDropdowns();
	}

	bool DropdownOpen( kFalse );

	CLLNode<CDropdown>* pDNode = m_pDropdownList->GetFirst();
	while( pDNode != NULL )
	{
		if( pDNode->Data()->IsOpen() )
		{
			DropdownOpen = kTrue;
		}
		pDNode = pDNode->Next();
	}

	if( !DropdownOpen &&
		m_SoundDragging ||
		m_MusicDragging ||
		m_GammaDragging )
	{
		POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
																(float32)m_pSettings.GetSettings( KSetScreenHeight ),
																1024,
																768 );

		// update sliders
		ReturnCode = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
										m_MouseHandler.ButtonHeld( KButtonLeft ),
										kFalse );
		
		int32 NewValue( 0 );
		switch( ReturnCode )
		{

		// if pressed, we set sound volume
		case KOptionsMenuSoundVolume :
			if( m_SoundDragging )
			{
				NewValue = MousePosition.x - 550;
				if( NewValue > 256 )
				{
					NewValue = 256;
				}
				if( NewValue < 0 )
				{
					NewValue = 0;
				}
				m_pSettings.SetSettings( KSetSoundFXVolume, NewValue );
			}
			break;
		// if pressed, we set music volume
		case KOptionsMenuMusicVolume :
			if( m_MusicDragging )
			{
				NewValue = MousePosition.x - 550;
				if( NewValue > 256 )
				{
					NewValue = 256;
				}
				if( NewValue < 0 )
				{
					NewValue = 0;
				}
				m_pSettings.SetSettings( KSetMusicVolume, NewValue );
			}
			break;
		// if pressed, we set gamma
		case KOptionsMenuGamma :
			if( m_GammaDragging )
			{
				NewValue = MousePosition.x - 150;
				if( NewValue > 256 )
				{
					NewValue = 256;
				}
				if( NewValue < 0 )
				{
					NewValue = 0;
				}
				m_pSettings.SetSettings( KSetGammaRamp, NewValue );
				UpdateElements();
			}
			break;
		}
	}
} // COptionsMenu::ProcessInput()

void COptionsMenu::Update( HWND hWnd,								// hwnd of the active window
						float32 TimeElapsed )					// time elapsed, in seconds
{

	ProcessInput( hWnd );

	UpdateButtons( TimeElapsed );
	
	m_pSoundSprite->SetPosition( 550 + (float32)m_pSettings.GetSettings( KSetSoundFXVolume ), 166 );

	m_pMusicSprite->SetPosition( 550 + (float32)m_pSettings.GetSettings( KSetMusicVolume ), 246 );

	m_pGammaSprite->SetPosition( 150 + (float32)m_pSettings.GetSettings( KSetGammaRamp ), 686 );
	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();

} // COptionsMenu::Update()

void COptionsMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	CMenu::Render( pD3DDevice );

	if( m_UpdateElements )
	{
		ResetButtons( pD3DDevice );
		m_UpdateElements = kFalse;
		m_pMenuManager.GameClient().UpdateShadowResolution( pD3DDevice );
	}


} // COptionsMenu::Render()

void COptionsMenu::ResetButtons( LPDIRECT3DDEVICE8 pD3DDevice )
{

	std::string ResolutionText = KResolutionNames[0];
	uint32 ScreenWidth = m_pSettings.GetSettings( KSetScreenWidth );
	uint32 ScreenHeight = m_pSettings.GetSettings( KSetScreenHeight );
	for( uint32 i = 0; i < KResolutions; i++ )
	{
		if( KResolutionWidth[i] == ScreenWidth &&
			KResolutionHeight[i] == ScreenHeight )
		{
			ResolutionText = KResolutionNames[i];
			break;
		}
	}
	m_pResolutionButton->SetText( pD3DDevice,
								  ResolutionText );
	m_pResolutionButton->SetOverText( pD3DDevice,
									  ResolutionText );

	ResolutionText = KShadowSizeNames[0];
	uint32 ShadowWidth = m_pSettings.GetSettings( KSetShadowResolution );
	for( int32 i = 0; i < KShadowSizes; i++ )
	{
		if( KShadowSize[i] == ShadowWidth )
		{
			ResolutionText = KShadowSizeNames[i];
			break;
		}
	}
	m_pShadowSizeButton->SetText( pD3DDevice,
								  ResolutionText );
	m_pShadowSizeButton->SetOverText( pD3DDevice,
									  ResolutionText );

	m_pShadowButton->SetText( pD3DDevice,
							  KShadowNames[m_pSettings.GetSettings( KSetShadowDetail )] );
	m_pShadowButton->SetOverText( pD3DDevice,
								  KShadowNames[m_pSettings.GetSettings( KSetShadowDetail )] );

	m_pParticleButton->SetText( pD3DDevice,
							  KParticleNames[m_pSettings.GetSettings( KSetParticleDetail )] );
	m_pParticleButton->SetOverText( pD3DDevice,
								  KParticleNames[m_pSettings.GetSettings( KSetParticleDetail )] );

	m_pFSAAButton->SetText( pD3DDevice,
							  KFSAANames[m_pSettings.GetSettings( KSetFSAA )] );
	m_pFSAAButton->SetOverText( pD3DDevice,
								  KFSAANames[m_pSettings.GetSettings( KSetFSAA )] );

	m_pVSYNCButton->SetText( pD3DDevice,
							  KVSYNCNames[m_pSettings.GetSettings( KSetVSYNC )] );
	m_pVSYNCButton->SetOverText( pD3DDevice,
								  KVSYNCNames[m_pSettings.GetSettings( KSetVSYNC )] );

	m_pFogButton->SetText( pD3DDevice,
						   KFogNames[m_pSettings.GetSettings( KSetAnimatedFog )] );
	m_pFogButton->SetOverText( pD3DDevice,
							   KFogNames[m_pSettings.GetSettings( KSetAnimatedFog )] );

	m_pGlowButton->SetText( pD3DDevice,
							KGlowNames[m_pSettings.GetSettings( KSetMotionBlur )] );
	m_pGlowButton->SetOverText( pD3DDevice,
								KGlowNames[m_pSettings.GetSettings( KSetMotionBlur )] );

	D3DGAMMARAMP Ramp;
	uint32 Gamma = m_pSettings.GetSettings( KSetGammaRamp );
	Gamma = 256 - Gamma;
	Gamma += 75;

	FillGammaRamp( Ramp, Gamma );

    pD3DDevice->SetGammaRamp( D3DSGR_NO_CALIBRATION, &Ramp );


} // COptionsMenu::ResetButtons()

// render any UI elements
void COptionsMenu::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{


	RenderSprites( pD3DDevice );
	RenderButtons( pD3DDevice );
	RenderText( pD3DDevice );

	m_pSoundSprite->Render( pD3DDevice );
	m_pMusicSprite->RenderNoTexture( pD3DDevice );
	m_pGammaSprite->RenderNoTexture( pD3DDevice );

	RenderDropdowns( pD3DDevice );
} // COptionsMenu::RenderUI()

