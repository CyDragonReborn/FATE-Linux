/*
	settings.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004


	CHANGE HISTORY

	Travis Baldree
	8/24/2004 - Added floating-point settings support

*/

#include <windows.h>
#include <string>
#include <D3D8.h>
#include "D3DX8.h"

using namespace std;

#include "constants.h"
#include "utilities.h"
#include "macros.h"

#include "textfile.h"
#include "capabilities.h"
#include "settings.h"
#include "vistafileutilities.h"

CSettings::CSettings( CCapabilities* pCapabilities,
					  std::string SettingsPath, 
					  std::string DefaultSettingsPath ) :
					  m_pCapabilities( pCapabilities ),
					  m_ChangeResolutionRequested( kFalse ),
					  m_ShouldSuggestSettings( kFalse )

{
	m_SettingsPath			= CVistaFileUtilities::GetSharedDataPath();	
	m_DefaultSettingsPath	= CVistaFileUtilities::GetSharedDataPath();

	// prepare the arrays for all of our settings data
	m_SettingsValues		= new int32[KSetCount];
	m_SettingsFloatValues	= new float32[KSetCount];
	m_SettingsStringValues	= new std::string[KSetStringCount];

	// make sure they match the order of the enumerations,
	// otherwise it won't work properly!
	if( !VerifySettingAssignments() )
	{
		MessageBox( NULL,
					"Settings enumeration is out of order!",
					"SETUP ERROR",
					MB_ICONERROR | MB_OK );
	}
	else
	{
		AssignDefaultSettingValues();
	}

	// clear all our IO mapping and set to default
	ClearAllInputMapping();
	SetDefaultKeyboardMapping();
	SetDefaultJoystickMapping();

} // CSettings::CSettings()

CSettings::~CSettings()
{
	DELETE_ARRAY_SAFELY( m_SettingsValues );
	DELETE_ARRAY_SAFELY( m_SettingsFloatValues );
	DELETE_ARRAY_SAFELY( m_SettingsStringValues );
} // CSettings::~CSettings()

// make sure that the settings table is set up properly
bool CSettings::VerifySettingAssignments( void )
{
	for( uint32 i = 0; i < KSetCount; i++ )
	{
		if( KSettings[i].m_Setting != i )
		{
			return kFalse;
		}
	}

	return kTrue;

} // CSettings::VerifySettingAssignments()

// assign default values to the settings table
void CSettings::AssignDefaultSettingValues( void )
{

	for( uint32 i = 0; i < KSetCount; i++ )
	{
		m_SettingsValues[i] = KSettings[i].m_DefaultValue;
	}


	for( i = 0; i < KSetFloatCount; i++ )
	{
		m_SettingsFloatValues[i] = KSettingsFloat[i].m_DefaultValue;
	}

} // CSettings::AssignDefaultSettingValues()


int32 CSettings::GetSettings( ESettings Setting )	// Setting value to retrieve
{
	return m_SettingsValues[Setting];

} // CSettings::GetSettings()

void CSettings::SetSettings( ESettings Setting,		// Setting to change
							 int32 Value )			// new value for this setting
{
	uint32 ScreenWidth = GetSettings( KSetScreenWidth );
	uint32 ScreenHeight = GetSettings( KSetScreenHeight );

	m_SettingsValues[Setting] = Value;

	// if this setting changed screen resolution in any way, we need to request
	// a d3d resolution change
	if ( ScreenWidth != GetSettings( KSetScreenWidth ) || 
		 ScreenHeight != GetSettings( KSetScreenHeight ) )
	{
		m_ChangeResolutionRequested = kTrue;
	}

} // CSettings::SetSettings()

float32 CSettings::GetSettings( ESettingsFloat Setting )// Setting value to retrieve
{
	return m_SettingsFloatValues[Setting];

} // CSettings::GetSettings()

void CSettings::SetSettings( ESettingsFloat Setting,	// Setting to change
							 float32 Value )			// new value for this setting
{
	m_SettingsFloatValues[Setting] = Value;

} // CSettings::SetSettings()

const std::string& CSettings::GetSettings( ESettingsString Setting )	// Setting value to retrieve
{
	return m_SettingsStringValues[Setting];

} // CSettings::GetSettings()

void CSettings::SetSettings( ESettingsString Setting,		// Setting to change
							 const std::string& Value )		// new value for this setting
{
	m_SettingsStringValues[Setting] = Value;

} // CSettings::SetSettings()

HRESULT CSettings::Load( std::string FileName )	// name of config file to open
{
	CTextFile	File;

	// if we couldn't open the one explicitly requested
	if ( !File.Open( FileName.c_str() ) )
	{

		// then try opening one given the path we already have
		if ( !File.Open( ( m_SettingsPath + "/config.dat" ).c_str() ) )
		{
			// if that didn't work, we need to suggest new settings
			m_ShouldSuggestSettings = kTrue;
			// and try to open the default one
			if ( !File.Open( ( m_DefaultSettingsPath + "/default_config.dat" ).c_str() ) )
			{
				GameSpecificSetup();
				return S_OK;
			}
		}
	}

	queue<std::string>	Tokens;

	bool Done	= kFalse;
	while ( !Done )
	{
		File.TokenizeNextLine( &Tokens, ":,\n\t\r", '#' );	
		if ( Tokens.size() <= 0 )
		{
			break;
		}
		if ( File.IsEOF() )
		{
			break;
		}

		bool ParsedLine = kFalse;

		// see if this entry matches any of the int32 based settings
		for( uint32 i = 0; i < KSetCount; i++ )
		{
			if( Tokens.front() == KSettings[i].m_Name )
			{
				ParsedLine = kTrue;
				Tokens.pop();
				if( Tokens.size() > 0 )
				{
					m_SettingsValues[i] = atoi( Tokens.front().c_str() );
				}
				break;
			}
		}


		if( ParsedLine )
		{
			ClearQueue( Tokens );
			continue;
		}

		// see if this entry matches any of the float32 based settings
		for( i = 0; i < KSetFloatCount; i++ )
		{
			if( Tokens.front() == KSettingsFloat[i].m_Name )
			{
				ParsedLine = kTrue;
				Tokens.pop();
				if( Tokens.size() > 0 )
				{
					m_SettingsFloatValues[i] = (float32)atof( Tokens.front().c_str() );
				}
				break;
			}
		}


		if( ParsedLine )
		{
			ClearQueue( Tokens );
			continue;
		}

		// see if this entry matches any of the string based settings
		for( i = 0; i < KSetStringCount; i++ )
		{
			if( Tokens.front() == KSettingsStrings[i].m_Name )
			{
				ParsedLine = kTrue;
				Tokens.pop();
				if( Tokens.size() > 0 )
				{
					m_SettingsStringValues[i] = Tokens.front();
				}
				break;
			}
		}

		if( ParsedLine )
		{
			ClearQueue( Tokens );
			continue;
		}

		if( Tokens.front() == "KEYMAP" )
		{
			ParsedLine = kTrue;
			Tokens.pop();
			if( Tokens.size() > 0 )
			{
				uint32 Control = atoi( Tokens.front().c_str() );

				Tokens.pop();
				if( Tokens.size() > 0 )
				{
					uint32 Code = atoi( Tokens.front().c_str() );

					SetKeyButtonMap( Control, Code );
				}
			}

			ClearQueue( Tokens );
			continue;
		}

		if( Tokens.front() == "JOYBUTTONMAP" )
		{
			ParsedLine = kTrue;
			Tokens.pop();
			if( Tokens.size() > 0 )
			{
				uint32 Control = atoi( Tokens.front().c_str() );

				Tokens.pop();
				if( Tokens.size() > 0 )
				{
					uint32 Code = atoi( Tokens.front().c_str() );

					SetJoyButtonMap( Control, Code );
				}
			}

			ClearQueue( Tokens );
			continue;
		}

		if( Tokens.front() == "JOYAXISMAP" )
		{
			ParsedLine = kTrue;
			Tokens.pop();
			if( Tokens.size() > 0 )
			{
				uint32 Control = atoi( Tokens.front().c_str() );

				Tokens.pop();
				if( Tokens.size() > 0 )
				{
					uint32 Axis = atoi( Tokens.front().c_str() );

					Tokens.pop();
					if( Tokens.size() > 0 )
					{
						uint32 Direction = atoi( Tokens.front().c_str() );
						
						SetJoyAxisMap( Control, Axis, Direction );
					}
				}
			}

			ClearQueue( Tokens );
			continue;
		}


		ClearQueue( Tokens );
	}


	File.Close();
	GameSpecificSetup();


	return S_OK;
} // CSettings::Create()


// output all settings by keyword, followed by value
void CSettings::Save()
{
	FILE*	pFile;
	pFile = fopen( ( m_SettingsPath + "/config.dat" ).c_str(), "wt" );
	if ( pFile != NULL )
	{

		for( uint32 i = 0; i < KSetCount; i++ )
		{

			fputs( ( KSettings[i].m_Name + ":" + 
				     IntegerToString( m_SettingsValues[i] ) + "\n" ).c_str(), 
				     pFile );
		}

		for( i = 0; i < KSetFloatCount; i++ )
		{

			fputs( ( KSettingsFloat[i].m_Name + ":" + 
				     FloatToString( m_SettingsFloatValues[i] ) + "\n" ).c_str(), 
				     pFile );
		}

		for( i = 0; i < KSetStringCount; i++ )
		{
			fputs( ( KSettingsStrings[i].m_Name + ":" + 
				     m_SettingsStringValues[i] + "\n" ).c_str(), 
				     pFile );
		}

		for( i = 0; i < 255; i ++ )
		{
			if( IsKeyMapped( i ) )
			{
				fputs( ( "KEYMAP:" + IntegerToString( i ) + ":" + 
						 IntegerToString( GetMappedKey( i ) ) + "\n" ).c_str(), 
					     pFile );
			}
			if( IsJoyButtonMapped( i ) )
			{
				fputs( ( "JOYBUTTONMAP:" + IntegerToString( i ) + ":" + 
						 IntegerToString( GetMappedJoyButton( i ) ) + "\n" ).c_str(), 
					     pFile );
			}
			if( IsJoyAxisMapped( i ) )
			{
				fputs( ( "JOYAXISMAP:" + IntegerToString( i ) + ":" + 
						 IntegerToString( GetMappedJoyAxis( i ) ) + ":" + 
						 IntegerToString( GetMappedJoyAxisDirection( i ) ) + "\n" ).c_str(), 
					     pFile );
			}
		}

		fclose( pFile );
	}
} // CSettings::Save()


// verify that all general-purpose settings are possible
void CSettings::VerifySettings( void )
{

	if ( m_ShouldSuggestSettings )
	{
		m_ShouldSuggestSettings = kFalse;
		SuggestSettings();
	}

	int32 Version = m_pCapabilities->GetWindowsVersion();
	if( Version == KWindows95 ||
		Version == KWindows98 ||
		Version == KWindowsME )
	{
		SetSettings( KSetAllowCDAudio, kFalse );
	}

	if( !m_pCapabilities->IsCapable( KCanDoModAlphaAdd ) )
	{
		SetSettings( KSetSceneReflection, kFalse );
		SetSettings( KSetCubeMapping, kFalse );
		SetSettings( KSetSpecularMapping, kFalse );
		SetSettings( KSetReflectionSpecularMapping, kFalse );
	}

	if( GetSettings( KSetShadowResolution ) > 
		m_pCapabilities->GetCapabilityValue( KValueMaxRenderToTextureSize ) )
	{
		SetSettings( KSetShadowResolution, m_pCapabilities->GetCapabilityValue( KValueMaxRenderToTextureSize ) );
	}


	if( !m_pCapabilities->IsCapable( KCanDoTextureProjection ) )
	{
		SetSettings( KSetSceneReflection, kFalse );
	}

	if( !m_pCapabilities->IsCapable( KCanRenderToTexture ) )
	{
		SetSettings( KSetDynamicReflections, kFalse );
		SetSettings( KSetSceneReflection, kFalse );
		SetSettings( KSetMotionBlur, kFalse );
	}

	if( m_pCapabilities->GetCapabilityValue ( KValueMaxSimultaneousTextures ) < 2 )
	{
		SetSettings( KSetSceneReflection, kFalse );
		SetSettings( KSetNormalMapping, kFalse );
		SetSettings( KSetCubeMapping, kFalse );
	}

	if( !m_pCapabilities->IsCapable( KCanDoReflectionSpecularMapping ) )
	{
		SetSettings( KSetReflectionSpecularMapping, kFalse );
	}

	if( !m_pCapabilities->IsCapable( KCanDoSpecularMapping ) )
	{
		SetSettings( KSetSpecularMapping, kFalse );
	}

	if( m_pCapabilities->GetCapabilityValue ( KValueMaxSimultaneousBlendStages ) < 3 )
	{
		SetSettings( KSetNormalMapping, kFalse );
		SetSettings( KSetReflectionSpecularMapping, kFalse );
	}

	if( m_pCapabilities->GetCapabilityValue ( KValueMaxSimultaneousBlendStages ) < 2 )
	{
		SetSettings( KSetSceneReflection, kFalse );
		SetSettings( KSetCubeMapping, kFalse );
		SetSettings( KSetSpecularMapping, kFalse );
		SetSettings( KSetReflectionSpecularMapping, kFalse );
	}

	if( !m_pCapabilities->IsCapable ( KCanDoModulate4X ) )
	{
		if( !m_pCapabilities->IsCapable ( KCanDoModulate2X ) )
		{
			SetSettings( KSetNormalMapModulate, D3DTOP_MODULATE );
		}
		else
		{
			SetSettings( KSetNormalMapModulate, D3DTOP_MODULATE2X );
		}
	}

	if( !m_pCapabilities->IsCapable ( KCanDoAdditiveSmooth ) )
	{
		SetSettings( KSetNormalMapAdditive, D3DTOP_ADD );
	}

	if( !m_pCapabilities->IsCapable ( KCanDoAdditive ) )
	{
		SetSettings( KSetNormalMapping, kFalse );
	}

	if( !m_pCapabilities->IsCapable ( KCanDoDot3 ) )
	{
		SetSettings( KSetNormalMapping, kFalse );
	}

	if( !m_pCapabilities->IsCapable( KCanDoCubeMap ) )
	{
		SetSettings( KSetCubeMapping, kFalse );
	}

	if( !m_pCapabilities->IsCapable( KCanDoStencilShadows ) )
	{
		SetSettings( KSetStencilShadows, kFalse );
	}

	if( !m_pCapabilities->IsCapable( KCanFadeAlpha ) )
	{
		SetSettings( KSetAlphaFade, kFalse );
	}

	if( !m_pCapabilities->IsCapable( KCanAlphaTest ) )
	{
		SetSettings( KSetAlphaTest, kFalse );
	}

	if( !m_pCapabilities->IsCapable( KCanDoFogTable ) )
	{
		SetSettings( KSetFogTable, kFalse );
	}

	if( !m_pCapabilities->IsCapable( KCanDoPointSprites ) )
	{
		SetSettings( KSetPointSprites, kFalse );
	}

	SetSettings( KSetMaxLights, 
				 m_pCapabilities->GetCapabilityValue ( KValueMaxLights ) );

	SetSettings( KSetClampUVs, 
				 m_pCapabilities->GetCapabilityValue ( KValueClampUVs ) );

	SetSettings( KSetMipFilter, 
				 m_pCapabilities->GetCapabilityValue ( KValueMipFilter ) );
	
	SetSettings( KSetProjectedShadows, kTrue );

	if( !m_pCapabilities->IsCapable( KCanRenderToTexture ) )
	{
		SetSettings( KSetProjectedShadows, kFalse );
	}
	if( !m_pCapabilities->IsCapable( KCanDoShadowTextureProjection ) &&
		!m_pCapabilities->IsCapable( KCanDoOrthoShadowTextureProjection ) )
	{
		SetSettings( KSetProjectedShadows, kFalse );
	}


	// make sure that anything that needs to be checked for this game is done
	GameSpecificSetup();
} // CSettings::VerifySettings()

// verify that hardware is capable of producing some of our game-specific settings
void CSettings::VerifyGameSpecificSettings( void )
{
	// make sure that anything that needs to be checked for this game is done
	GameSpecificSetup();
} // CSettings::VerifyGameSpecificSettings()

// does any tasks that involve the relationship between settings.
// i.e., I'm going to adjust my foozle to half resolution, 
//	because this machine stinks...
// the difference between this and suggestsettings is that
// these are absolute - they CANNOT be overriden by the user
void CSettings::GameSpecificSetup( void )
{
	if( !GetSettings( KSetProjectedShadows ) &&
		GetSettings( KSetShadowDetail ) > 0 )
	{
		SetSettings( KSetShadowDetail, 0 );
	}

	SetSettings( KSetStencilShadows, kFalse );

} // CSettings::GameSpecificSetup()


// suggest starting settings based upon the user's machine
// NOTE THAT THESE ARE SUGGESTIONS! They may not be possible on the machine,
// so we will verify once complete to ensure that they're OK.
void CSettings::SuggestSettings( void )
{
	EMachineRanking	Defaultsettings	= QuantifyMachine();
	
	switch ( Defaultsettings )
	{
	case KRankingHighest:
		SetSettings( KSetScreenWidth, 1024 );
		SetSettings( KSetScreenHeight, 768 );
		SetSettings( KSetFSAA, kTrue );
		SetSettings( KSetSpecular, kTrue );
		SetSettings( KSetAlphaFade, kTrue );
		SetSettings( KSetPointLights, kTrue );
		SetSettings( KSetStencilShadows, kTrue );
		break;

	case KRankingHigh:
		SetSettings( KSetScreenWidth, 1024 );
		SetSettings( KSetScreenHeight, 768 );
		SetSettings( KSetFSAA, kFalse );
		SetSettings( KSetSpecular, kTrue );
		SetSettings( KSetAlphaFade, kTrue );
		SetSettings( KSetPointLights, kTrue );
		SetSettings( KSetStencilShadows, kTrue );
		break;

	case KRankingMedium:
		SetSettings( KSetScreenWidth, 800 );
		SetSettings( KSetScreenHeight, 600 );
		SetSettings( KSetFSAA, kFalse );
		SetSettings( KSetSpecular, kTrue );
		SetSettings( KSetAlphaFade, kTrue );
		SetSettings( KSetPointLights, kFalse );
		SetSettings( KSetStencilShadows, kFalse );
		break;

	case KRankingLow:
		SetSettings( KSetScreenWidth, 640 );
		SetSettings( KSetScreenHeight, 480 );
		SetSettings( KSetFSAA, kFalse );
		SetSettings( KSetSpecular, kTrue );
		SetSettings( KSetAlphaFade, kFalse );
		SetSettings( KSetPointLights, kFalse );
		SetSettings( KSetStencilShadows, kFalse );
		break;

	case KRankingLowest:
		SetSettings( KSetScreenWidth, 640 );
		SetSettings( KSetScreenHeight, 480 );
		SetSettings( KSetFSAA, kFalse );
		SetSettings( KSetSpecular, kTrue );
		SetSettings( KSetAlphaFade, kFalse );
		SetSettings( KSetPointLights, kFalse );
		SetSettings( KSetStencilShadows, kFalse );
		break;
	}

	// now perform any suggestions that are specific to this game
	SuggestGameSpecificSettings();

	// you should ALWAYS re-verify your settings, so that you can make sure
	// that any suggestions are OK on this machine!

	VerifySettings();
	VerifyGameSpecificSettings();

} // CSettings::SuggestSettings()


// suggest starting settings based upon the user's machine
// this should be customized to your game to take into account
// any special settings you may have.
// NOTE THAT THESE ARE SUGGESTIONS! They may not be possible on the machine,
// so we will verify once complete to ensure that they're OK.
void CSettings::SuggestGameSpecificSettings( void )
{
	int32	Defaultsettings	= QuantifyMachine();
	
	switch ( Defaultsettings )
	{
	case KRankingHighest:
		break;

	case KRankingHigh:
		break;

	case KRankingMedium:
		break;

	case KRankingLow:
		break;

	case KRankingLowest:
		break;
	}

} // CSettings::SuggestGameSpecificSettings()

// determine a rough range of capability for this machine
EMachineRanking CSettings::QuantifyMachine( void )
{
	bool	IsIntel	= m_pCapabilities->IsIntelCard();

	EMachineRanking	MachineRanking	= KRankingHighest;
	int32	mHZ				= m_pCapabilities->GetProcessorSpeed();
	int32	RAM				= m_pCapabilities->GetAvailableRAM();
	int32	VRAM			= m_pCapabilities->GetAvailableVRAM();


	if( VRAM <= 0 )
	{
		VRAM = 32;
	}
	if ( mHZ <= 300 )
	{
		mHZ = 1000;
	}

	if ( VRAM < 40 )
	{
		MachineRanking = KRankingHigh;
	}

	if ( mHZ < 1300 )
	{
		MachineRanking = KRankingHigh;
	}

	if ( RAM < 192 )
	{
		MachineRanking = KRankingHigh;
	}

	if ( VRAM < 28 )
	{
		MachineRanking = KRankingMedium;
	}

	if ( mHZ < 900 )
	{
		MachineRanking = KRankingMedium;
	}
	if ( RAM < 150 )
	{
		MachineRanking = KRankingMedium;
	}

	if ( VRAM < 17 )
	{
		MachineRanking = KRankingLow;
	}

	if ( mHZ < 800 )
	{
		MachineRanking = KRankingLow;
	}

	if ( mHZ < 600 )
	{
		MachineRanking = KRankingLowest;
	}
	if ( RAM < 110 )
	{
		MachineRanking = KRankingLowest;
	}

	if ( VRAM < 15 )
	{
		MachineRanking = KRankingLowest;
	}


	if ( ( MachineRanking == KRankingHighest || 
		   MachineRanking == KRankingHigh ) &&
		 IsIntel )
	{
		MachineRanking = KRankingMedium;
	}

	return MachineRanking;

} // CSettings::QuantifyMachine()


// io mapping

// set all input mappint to nothing
void CSettings::ClearAllInputMapping( void )
{
	for ( uint32 i = 0; i < 255; i++ )
	{
		m_KeyMap[i] = -1;
		m_JoyMap[i] = -1;
		m_JoyMapAxis[i] = -1;
		m_JoyMapAxisDirection[i] = 0;
	}
} // CSettings::ClearAllInputMapping()

// set your own default keyboard mapping here
// the order of the controls is up to you - this is simply
// a way for you to store mapping, and prevent mapping collisions
void CSettings::SetDefaultKeyboardMapping( void )
{
	// clear current mapping
	for ( uint32 i = 0; i < 255; i++ )
	{
		m_KeyMap[i] = -1;
	}

	m_KeyMap[0] = 37;
	m_KeyMap[1] = 39;
	m_KeyMap[2] = 38;
	m_KeyMap[3] = 40;
	m_KeyMap[4] = 67;
	m_KeyMap[5] = 83;
	m_KeyMap[6] = 82;
	m_KeyMap[7] = 77;
	m_KeyMap[8] = 80;
	m_KeyMap[9] = 65;
	m_KeyMap[10] = 90;
	m_KeyMap[11] = 32;
	m_KeyMap[12] = 72;

} // CSettings::SetDefaultKeyboardMapping()

// set your own default joystick mapping here
// the order of the controls is up to you - this is simply
// a way for you to store mapping, and prevent mapping collisions
void CSettings::SetDefaultJoystickMapping( void )
{
	// clear current mapping
	for ( uint32 i = 0; i < 255; i++ )
	{
		m_JoyMap[i] = -1;
		m_JoyMapAxis[i] = -1;
		m_JoyMapAxisDirection[i] = 0;
	}

	m_JoyMapAxis[0] = 1;
	m_JoyMapAxis[1] = 1;

	m_JoyMapAxisDirection[0] = -1;
	m_JoyMapAxisDirection[1] = 1;

	m_JoyMap[2] = 0;
	m_JoyMap[3] = 3;
	m_JoyMap[4] = 4;
	m_JoyMap[5] = 5;
	m_JoyMap[6] = 2;

	m_JoyMap[11] = 1;
	m_JoyMap[9] = 7;
	m_JoyMap[10] = 6;


} // CSettings::SetDefaultJoystickMapping()

// if any mappings exist with the given keycode, clear them out
void CSettings::ClearMappingForKeyCode( uint32 code )
{
	for ( uint32 i = 0; i < 13; i++ )
	{
		if ( m_KeyMap[i] == code )
		{
			m_KeyMap[i] = -1;
		}
	}
} // CSettings::ClearMappingForKeyCode(

// if any mappings exist with the given joystick button, or axis combination, clear them out
void CSettings::ClearMappingForJoyButton( uint32 Code )		// button number to unmap
{
	for ( uint32 i = 0; i < 255; i++ )
	{
		if ( m_JoyMap[i] == Code )
		{
			m_JoyMap[i] = -1;
		}
	}
}// CSettings::ClearMappingForJoyButton()

// if any mappings exist with the given Axis combination, clear them out
void CSettings::ClearMappingForJoyAxis( uint32 Axis,			// Axis number to unmap
									    int32 AxisDirection )	// Axis direction must also correspond
{
	
	for ( uint32 i = 0; i < 255; i++ )
	{
		if ( m_JoyMapAxis[i] == Axis && 
			 m_JoyMapAxisDirection[i] == AxisDirection )
		{
			m_JoyMapAxis[i] = -1;
			m_JoyMapAxisDirection[i] = 0;
		}
	}
	
}// CSettings::ClearMappingForJoyAxis()

// set mapping for a control to a specific Keyboard key
void CSettings::SetKeyButtonMap( uint32 ControlNumber,			// which control to map to
								 uint32 KeyCode )				// which key to map to this control
{
	// make sure no other control is using this button #
	ClearMappingForKeyCode( KeyCode );

	if( ControlNumber < 255 )
	{
		m_KeyMap[ControlNumber] = KeyCode;
	}
} // CSettings::SetKeyButtonMap()

// set mapping for a control to a specific joystick button
void CSettings::SetJoyButtonMap( uint32 ControlNumber,			// which control to map to
								 uint32 Button )				// which button to map to this control
{
	// make sure no other control is using this button #
	ClearMappingForJoyButton( Button );

	if( ControlNumber < 255 )
	{
		m_JoyMap[ControlNumber] = Button;
		m_JoyMapAxis[ControlNumber] = -1;
		m_JoyMapAxisDirection[ControlNumber] = 0;
	}
} // CSettings::SetJoyButtonMap()

// set mapping for a control to a specific joystick axxis
void CSettings::SetJoyAxisMap( uint32 ControlNumber,			// which control to map to
							   uint32 Axis,						// which button to map to this control
							   int32  AxisDirection )			// +/- axis
{
	// make sure no other control is using this button #
	ClearMappingForJoyAxis( Axis, AxisDirection );

	if( ControlNumber < 255 )
	{
		m_JoyMap[ControlNumber] = -1;
		m_JoyMapAxis[ControlNumber] = Axis;
		m_JoyMapAxisDirection[ControlNumber] = AxisDirection;
	}
} // CSettings::SetJoyAxisMap()

// get the keycode mapped to a particular control
int32 CSettings::GetMappedKey( uint32 ControlNumber )		// control to retrieve keymapping for
{
	if( ControlNumber < 255 )
	{
		return m_KeyMap[ControlNumber];
	}

	return -1;
}// CSettings::GetMappedKey()

// get the joystick button mapped to a particular control
int32 CSettings::GetMappedJoyButton( uint32 ControlNumber )		// control to retrieve buttonmapping for
{
	if( ControlNumber < 255 )
	{
		return m_JoyMap[ControlNumber];
	}

	return -1;
}// CSettings::GetMappedJoyButton()

// get the joystick axis mapped to a particular control
int32 CSettings::GetMappedJoyAxis( uint32 ControlNumber )		// control to retrieve axismapping for
{
	if( ControlNumber < 255 )
	{
		return m_JoyMapAxis[ControlNumber];
	}

	return -1;
}// CSettings::GetMappedJoyAxis()

// get the keycode mapped to a particular control
int32 CSettings::GetMappedJoyAxisDirection( uint32 ControlNumber )		// control to retrieve axismapping for
{
	if( ControlNumber < 255 )
	{
		return m_JoyMapAxisDirection[ControlNumber];
	}

	return -1;
}// CSettings::GetMappedJoyAxisDirection()

// see if a requested control is mapped to the keyboard
bool CSettings::IsKeyMapped( uint32 ControlNumber )			// control to check mapping for
{
	if( m_KeyMap[ControlNumber] >= 0 )
	{
		return kTrue;
	}

	return kFalse;
} // CSettings::IsKeyMapped()

// see if a requested control is mapped to a joystick button
bool CSettings::IsJoyButtonMapped( uint32 ControlNumber )			// control to check mapping for
{
	if( m_JoyMap[ControlNumber] >= 0 )
	{
		return kTrue;
	}

	return kFalse;
} // CSettings::IsJoyButtonMapped()

// see if a requested control is mapped to a joystick axis
bool CSettings::IsJoyAxisMapped( uint32 ControlNumber )			// control to check mapping for
{
	if( m_JoyMapAxis[ControlNumber] >= 0 )
	{
		return kTrue;
	}

	return kFalse;
} // CSettings::IsJoyAxisMapped()

// see if we should be changing screen resolution
// see if a requested control is mapped to a joystick axis
bool CSettings::IsResolutionChangeRequested( void )
{
	if( m_ChangeResolutionRequested )
	{
		m_ChangeResolutionRequested = kFalse;
		return kTrue;
	}

	return kFalse;
} // CSettings::IsResolutionChangeRequested()