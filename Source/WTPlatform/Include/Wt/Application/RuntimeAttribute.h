//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#pragma once

#include "WT/System/Object.h"
#include "WT/System/UserPropertyCollection.h"

namespace WT
{
namespace Application
{

// This class provides access to the registry based information given to the application
// by the packaging system. Most of this information is related to the package's SKU

typedef System::SmartPointer<class CRuntimeAttribute> HRuntimeAttribute;
class CRuntimeAttribute : public System::CObject
{
public:

	WT_BEGIN_CLASS( CRuntimeAttribute, System::CObject );
		WT_BEGIN_PROPERTIES();
			WT_PROP_RO_BYVAL( SKU,					std::string );
			WT_PROP_RO_BYVAL( ProductDisplayName,	std::string );
			WT_PROP_RO_BYVAL( DPGUID,				std::string );
			WT_PROP_RO_BYVAL( DPCodeName,			std::string );
			WT_PROP_RO_BYVAL( ProductGUID,			std::string );
			WT_PROP_RO_BYVAL( ProductCodeName,		std::string );
			WT_PROP_RO_BYVAL( ExeName,				std::string );  // The name of the exe that has the game (starts the game, but still may wrap the game for DRM purposes)
			WT_PROP_RO_BYVAL( LaunchExeName,		std::string );	// The name of the exe to call to start the game (may invoke a setup step before starting the game)
			WT_PROP_RO_BYVAL( RuntimeExeName,		std::string );	// The name of the exe that is running when the game is running (unwrapped, final exe)
			WT_PROP_RO_BYVAL( UseLeaderboard,		bool );
			WT_PROP_RO_BYVAL( UseTracking,			bool );
		WT_END_PROPERTIES();
	WT_END_CLASS();

	CRuntimeAttribute();
	virtual ~CRuntimeAttribute();

	std::string const	GetString	( const std::string &Name ) const;
	int					GetInt		( const std::string &Name ) const;
	float				GetFloat	( const std::string &Name ) const;
	bool				GetBool		( const std::string &Name ) const;

	// properties
	std::string 	GetSKU					() const;
	std::string 	GetProductDisplayName	() const {return GetString("ProductDisplayName");}
	std::string 	GetDPGUID				() const {return GetString("DPGUID");}
	std::string 	GetDPCodeName			() const {return GetString("DPCodeName");}
	std::string 	GetProductGUID			() const {return GetString("ProductGUID");}
	std::string 	GetProductCodeName		() const {return GetString("ProductCodeName");}
	std::string 	GetExeName				() const {return GetString("ExeName");}  
	std::string 	GetLaunchExeName		() const {return GetString("LaunchExeName");}
	std::string		GetRuntimeExeName		() const {return GetString("RuntimeExeName");}
	bool			GetUseLeaderboard		() const {return GetBool  ("UseLeaderboard");}
	bool 			GetUseTracking			() const {return GetBool  ("UseTracking");}

private:

	std::string m_SKU;
	std::string m_IniFileName;

	std::string const FindSKU ( void );
	std::string const GetPath( std::string FileName ) const;
	std::string const GetApplicationDirectory ( void ) const;
	std::string const GetApplicationExecutableName ( void ) const;
	std::string const GetRegString ( const std::string &HKLM_KeyName, const std::string &StringName ) const;
	std::string const GetRegStringIfPresent ( const std::string &HKLM_KeyName, const std::string &StringName ) const;
	bool IniFileExists( void ) const;
	std::string const GetIniFileString ( const std::string &KeyName ) const;

	bool m_DevSku			: 1;
	bool m_UseLocalIniFile	: 1;
	bool m_NoSku			: 1; //Used internally in debug mode only
};

}
}
