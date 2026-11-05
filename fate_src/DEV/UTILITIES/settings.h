/*
	settings.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/


#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <D3D8.h>
#include "constants.h"
#include "utilities.h"

// machine performance rankings for auto-detection
enum	EMachineRanking
{
	KRankingLowest,
	KRankingLow,
	KRankingMedium,
	KRankingHigh,
	KRankingHighest,
	KRankingCount
};

// settings used by the game
enum ESettingsString
{
	KSetGameName,
	KSetGUID,
	KSetStringCount
};

// settings used by the game
enum ESettings
{
	KSetScreenWidth,
	KSetScreenHeight,
	KSetShowFPS,
	KSetFullscreen,
	KSetMasterVolume,
	KSetMusicVolume,
	KSetSoundFXVolume,
	KSetAllowCDAudio,
	KSetCDMusic,
	KSetFSAA,
	KSetStencilShadows,
	KSetCubeMapping,
	KSetSpecularMapping,
	KSetReflectionSpecularMapping,
	KSetNormalMapping,
	KSetSceneReflection,
	KSetUseJoystick,
	KSetUseForceFeedback,
	KSetOnline,
	KSetAlphaFade,
	KSetPointSprites,
	KSetSpecular,
	KSetAlphaTest,
	KSetFogTable,
	KSetMaxLights,
	KSetPointLights,
	KSetMipFilter,
	KSetClampUVs,
	KSetNormalMapModulate,
	KSetNormalMapAdditive,
	KSetMotionBlur,
	KSetGamesPlayed,
	
	// settings specific to a game - add your own,
	// and remove those you don't need

	KSetDynamicReflections,
	KSetProjectedShadows,
	KSetShadowResolution,
	KSetDungeonSeed,
	KSetShadowDetail,
	KSetGammaRamp,
	KSetParticleDetail,
	KSetShowTips,
	KSetConfirmLevel,
	KSetVSYNC,
	KSetAnimatedFog,
	KSet640x480Available,
	KSet800x600Available,
	KSet1024x768Available,
	KSet1152x864Available,
	KSet1280x1024Available,
	KSet1600x1200Available,
	KSet1280x768Available,
	KSet1366x768Available,
	KSet1920x1080Available,
	KSet1920x1200Available,
	KSetRedBlood,
	KSetCustomCursor,
	KSetCount

};

// settings used by the game
enum ESettingsFloat
{
	KSetNearClip,
	KSetFarClip,
	KSetFOV,

	// settings specific to a game - add your own,
	// and remove those you don't need
	KSetVersion,

	KSetFloatCount
};

class CEnum
{
public :
	explicit CEnum( ESettings Setting,			// Setting 
					const std::string& Name,	// string to be assigned to setting
					int32	DefaultValue ) :	// default value of settings
					m_Setting( Setting ),
					m_Name( Name ),
					m_DefaultValue( DefaultValue )
					{ };

	ESettings	m_Setting;
	std::string	m_Name;
	int32		m_DefaultValue;
};

class CEnumFloat
{
public :
	explicit CEnumFloat( ESettingsFloat Setting,			// Setting 
						 const std::string& Name,	// string to be assigned to setting
						 float32 DefaultValue ) :	// default value of settings
						 m_Setting( Setting ),
						 m_Name( Name ),
						 m_DefaultValue( DefaultValue )
						 { };

	ESettingsFloat	m_Setting;
	std::string		m_Name;
	float32			m_DefaultValue;
};

class CEnumString
{
public :
	explicit CEnumString( ESettingsString Setting,			// Setting 
						  const std::string& Name,			// string to be assigned to setting
						  std::string	DefaultValue ) :	// default value of settings
						  m_Setting( Setting ),
						  m_Name( Name ),
						  m_DefaultValue( DefaultValue )
						  { };

	ESettingsString		m_Setting;
	std::string			m_Name;
	std::string			m_DefaultValue;
};

// should match the enums - these strings will be used in the settings file
const static CEnumString	KSettingsStrings[KSetStringCount] = 
{
	// standard game settings
	CEnumString( KSetGameName,				"GAMENAME",			"MYGAME" ),
	CEnumString( KSetGUID,					"GUID",				"GUID" )
};

// should match the enums - these strings will be used in the settings file
const static CEnum	KSettings[KSetCount] = 
{
	// standard game settings
	CEnum( KSetScreenWidth,			"SCREENWIDTH",			640 ),
	CEnum( KSetScreenHeight,		"SCREENHEIGHT",			480 ),
	CEnum( KSetShowFPS,				"SHOWFPS",				kFalse ),
	CEnum( KSetFullscreen,			"FULLSCREEN",			kTrue ),
	CEnum( KSetMasterVolume,		"MASTERVOLUME",			60 ),
	CEnum( KSetMusicVolume,			"MUSICVOLUME",			100 ),
	CEnum( KSetSoundFXVolume,		"SFXVOLUME",			100 ),
	CEnum( KSetAllowCDAudio,		"ALLOWCDAUDIO",			kTrue ),
	CEnum( KSetCDMusic,				"CDMUSIC",				1 ),
	CEnum( KSetFSAA,				"FSAA",					kFalse ),
	CEnum( KSetStencilShadows,		"STENCILSHADOWS",		0 ),
	CEnum( KSetCubeMapping,			"CUBEMAPPING",			kTrue ),
	CEnum( KSetSpecularMapping,		"SPECULARMAPPING",		kTrue ),
	CEnum( KSetReflectionSpecularMapping,		"REFLECTIONSPECULARMAPPING",		kTrue ),
	CEnum( KSetNormalMapping,		"NORMALMAPPING",		kTrue ),
	CEnum( KSetSceneReflection,		"SCENEREFLECTION",		kTrue ),
	CEnum( KSetUseJoystick,			"JOYSTICK",				1 ),
	CEnum( KSetUseForceFeedback,	"FORCEFEEDBACK",		kTrue ),
	CEnum( KSetOnline,				"ONLINE",				kTrue ),
	CEnum( KSetAlphaFade,			"ALPHAFADE",			kTrue ),
	CEnum( KSetPointSprites,		"POINTSPRITES",			kTrue ),
	CEnum( KSetSpecular,			"SPECULAR",				kTrue ),
	CEnum( KSetAlphaTest,			"ALPHAWRITE",			kTrue ),
	CEnum( KSetFogTable,			"FOGTABLE" ,			kTrue ),
	CEnum( KSetMaxLights,			"MAXLIGHTS",			8 ),
	CEnum( KSetPointLights,			"POINTLIGHTS",			kTrue ),
	CEnum( KSetMipFilter,			"MIPFILTER",			D3DTEXF_LINEAR ),
	CEnum( KSetClampUVs,			"CLAMPUVS",				D3DTADDRESS_CLAMP ),
	CEnum( KSetNormalMapModulate,	"NORMALMODULATE",		D3DTOP_MODULATE4X ),
	CEnum( KSetNormalMapAdditive,	"NORMALADDITIVE",		D3DTOP_ADDSMOOTH ),
	CEnum( KSetMotionBlur,			"MOTIONBLUR",			kFalse ),
	CEnum( KSetGamesPlayed,			"GAMESPLAYED",			0 ),

	// settings specific to a game - add your own,
	// and remove those you don't need

	CEnum( KSetDynamicReflections,	"DYNAMICREFLECTIONS",			kTrue ),
	CEnum( KSetProjectedShadows,	"PROJECTEDSHADOWS",				kTrue ),
	CEnum( KSetShadowResolution,	"SHADOWRESOLUTION",				512 ),
	CEnum( KSetDungeonSeed,			"DUNGEONSEED",					0 ),
	CEnum( KSetShadowDetail,		"SHADOWDETAIL",					3 ),
	CEnum( KSetGammaRamp,			"GAMMARAMP",					90 ),
	CEnum( KSetParticleDetail,		"PARTICLEDETAIL",				2 ),
	CEnum( KSetShowTips,			"SHOWTIPS",						1 ),
	CEnum( KSetConfirmLevel,		"CONFIRMLEVEL",					1 ),
	CEnum( KSetVSYNC,				"VSYNC",						0 ),
	CEnum( KSetAnimatedFog,			"ANIMATEDFOG",					kTrue ),
	CEnum( KSet640x480Available,			"640x460",				kTrue ),
	CEnum( KSet800x600Available,			"800x600",				kTrue ),
	CEnum( KSet1024x768Available,			"1024x768",				kTrue ),
	CEnum( KSet1152x864Available,			"1152x864",				kTrue ),
	CEnum( KSet1280x1024Available,			"1280x1024",			kTrue ),
	CEnum( KSet1600x1200Available,			"1600x1200",			kTrue ),
	CEnum( KSet1280x768Available,			"1280x768",				kTrue ),
	CEnum( KSet1366x768Available,			"1366x768",				kTrue ),
	CEnum( KSet1920x1080Available,			"1920x1080",			kTrue ),
	CEnum( KSet1920x1200Available,			"1920x1200",			kTrue ),	
	CEnum( KSetRedBlood,			"REDBLOOD",						kFalse ),
	CEnum( KSetCustomCursor,			"CUSTOMCURSOR",						kTrue )
};

// should match the enums - these strings will be used in the settings file
const static CEnumFloat	KSettingsFloat[KSetFloatCount] = 
{
	// standard game settings
	CEnumFloat( KSetNearClip,			"NEARCLIP",				10.0f ),
	CEnumFloat( KSetFarClip,			"FARCLIP",				256 ),
	CEnumFloat( KSetFOV,				"FOV",					45 ),
	CEnumFloat( KSetVersion,			"VERSION",				0 )

	// settings specific to a game - add your own,
	// and remove those you don't need
};

class CCapabilities;

class CSettings
{
public:


				CSettings::CSettings( CCapabilities* pCapabilities,
									  std::string SettingsPath, 
									  std::string DefaultSettingsPath ),

				~CSettings();


	HRESULT		Load(	std::string FileName );		// name of config file to open

	int32		GetSettings( ESettings Setting );	// Setting value to retrieve

	void		SetSettings( ESettings Setting,		// Setting to change
							 int32 Value );			// new value for this setting

	float32		GetSettings( ESettingsFloat Setting );	// Setting value to retrieve

	void		SetSettings( ESettingsFloat Setting,	// Setting to change
							 float32 Value );			// new value for this setting

	const std::string&	GetSettings( ESettingsString Setting );	// Setting value to retrieve

	void		SetSettings( ESettingsString Setting,		// Setting to change
							 const std::string& Value );	// new value for this setting

	void		VerifySettings( void );

	void		VerifyGameSpecificSettings( void );

	void		GameSpecificSetup( void );

	void		SuggestSettings( void );

	void		SuggestGameSpecificSettings( void );

	EMachineRanking		QuantifyMachine( void );

	void		Save();


	void		ClearAllInputMapping( void );

	void		SetDefaultKeyboardMapping( void );
	
	void		SetDefaultJoystickMapping( void );
	
	void		ClearMappingForKeyCode( uint32 code );
	
	void		ClearMappingForJoyButton( uint32 Code );		// button number to unmap

	void 		ClearMappingForJoyAxis( uint32 Axis,			// Axis number to unmap
									    int32 AxisDirection );	// Axis direction must also correspond

	void		SetKeyButtonMap( uint32 ControlNumber,			// which control to map to
								 uint32 KeyCode );				// which key to map to this control

	void		SetJoyButtonMap( uint32 ControlNumber,			// which control to map to
								 uint32 Button );				// which button to map to this control

	void		SetJoyAxisMap( uint32 ControlNumber,			// which control to map to
							   uint32 Axis,						// which button to map to this control
							   int32  AxisDirection );			// +/- axis

	int32		GetMappedKey( uint32 ControlNumber );			// control to retrieve keymapping for

	int32		GetMappedJoyButton( uint32 ControlNumber );		// control to retrieve buttonmapping for

	int32		GetMappedJoyAxis( uint32 ControlNumber );		// control to retrieve axismapping for

	int32		GetMappedJoyAxisDirection( uint32 ControlNumber );		// control to retrieve axismapping for

	bool		IsKeyMapped( uint32 ControlNumber );			// control to check mapping for

	bool		IsJoyButtonMapped( uint32 ControlNumber );		// control to check mapping for

	bool		IsJoyAxisMapped( uint32 ControlNumber );		// control to check mapping for

	bool		IsResolutionChangeRequested( void );

	CCapabilities&	Capabilities( void )			{	return *m_pCapabilities;	};

private:
	int32			m_KeyMap[255];				// stores which key is mapped to control x (-1 is none)
	int32			m_JoyMap[255];				// stores which button is mapped to control x (-1 is none) 
	int32			m_JoyMapAxis[255];			// stores which joysick axis is mapped to control x (-1 for no axis)
	int32			m_JoyMapAxisDirection[255];	// 1 is standard, -1 is reversed

	CCapabilities*	m_pCapabilities;

	bool			m_ChangeResolutionRequested;

	bool			m_ShouldSuggestSettings;


	std::string		m_SettingsPath;
	std::string		m_DefaultSettingsPath;

	int32*			m_SettingsValues;

	float32*		m_SettingsFloatValues;

	std::string*	m_SettingsStringValues;	

	bool			VerifySettingAssignments( void );

	void			AssignDefaultSettingValues( void );
};


#endif