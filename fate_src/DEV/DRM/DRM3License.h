// -------------------------------------------------------------------------------------
//
// File Name:
//
//    CDRM3License.h
//
// Legal Trademarks:
//
//    Copyright (C) 2003 WildTangent, Inc. 
//    All Rights Reserved
//
// Description:
//
//    CDRM3License implements a wrapper around the DRM3 DLL. It is intended to be
//    compiled into a C++ game.
//    Typical usage is to create a DRM3License object, call SetLicenseInfo() to
//    set the store and license GUID, and then make calls to the various Get/Set
//    member functions to retrieve and set values.
//
// Authors:
//
//    David Elliot, Ronen Borshack
//
// Creattion Date:
//
//    2/26/2003
//
// -------------------------------------------------------------------------------------

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------

#include <windows.h>

// -------------------------------------------------------------------------------------
// Types
// -------------------------------------------------------------------------------------

typedef long (*PFNGETINTPROPERTY)(long, const char*, const char*, const char*, int*, int);
typedef long (*PFNGETFLOATPROPERTY)(long, const char*, const char*, const char*, float*, float);
typedef long (*PFNGETBOOLPROPERTY)(long, const char*, const char*, const char*, BOOL*, BOOL);
typedef long (*PFNGETDATEPROPERTY)(long, const char*, const char*, const char*, DWORD*, DWORD);
typedef long (*PFNGETSTRINGPROPERTY)(long, const char*, const char*, const char*, DWORD, const char*, const char*);

typedef long (*PFNUNLOCK)(long, const char*, const char*, const char*);

typedef long (*PFNSETINTPROPERTY)(long, const char*, const char*, const char*, const int);
typedef long (*PFNSETFLOATPROPERTY)(long, const char*, const char*, const char*, const float);
typedef long (*PFNSETBOOLPROPERTY)(long, const char*, const char*, const char*, const BOOL);
typedef long (*PFNSETDATEPROPERTY)(long, const char*, const char*, const char*, const DWORD);
typedef long (*PFNSETSTRINGPROPERTY)(long, const char*, const char*, const char*, const char*);

/*
#ifdef _DEBUG_DRM
#define dlld "d"
#else
#define dlld ""
#endif
#define DRMDLLNAME		"DRM0302" dlld ".dll"
*/
#define DRMDLLNAME		"DRM0302.dll"

// -------------------------------------------------------------------------------------
// Failure codes for Unlock
// -------------------------------------------------------------------------------------
#define DRM3_FAIL                  -1
#define DRM3_FAIL_OPEN_INTERNET    -2
#define DRM3_FAIL_INTERNET_CONNECT -3
#define DRM3_FAIL_INTERNET_REQUEST -4

#define DRM3LICENSE_FAIL_INIT		-102
#define DRM3LICENSE_FAIL_ARGS		-101

// -------------------------------------------------------------------------------------
// Classes
// -------------------------------------------------------------------------------------

class CDRM3License
{
public:

	// -------------------------------------------------------------------------------------
	// Constructors
	// -------------------------------------------------------------------------------------

	// -------------------------------------------------------------------------------------
	// This constructor is generally used when creating an instance of CDRM3License as a
	// member of a class. In this case, you need to call SetLicenseInfo before calling any
	// of the Get/Set functions. 
	// -------------------------------------------------------------------------------------

	CDRM3License ( void );

	// -------------------------------------------------------------------------------------
	// This constructor allows you to set the store name and license GUID when creating the
	// object.
	// -------------------------------------------------------------------------------------

	CDRM3License ( const char* Store , const char* LicenseGUID );

	// -------------------------------------------------------------------------------------
	// destructor
	// -------------------------------------------------------------------------------------

	virtual ~CDRM3License ( void );

	// -------------------------------------------------------------------------------------
	// This function takes a key string, a product GUID, and a content GUID, and it returns
	// 0 if the license was successfully obtained. If the function returns 0, you will
	// need to get the new values of the license properties.
	//
	// If the function returns a non-zero value, the value corresponds to a failure code
	// as defined above.
	//
	// The key string is supplied by the user (this is sent to them in an email message from
	// the E-Commerce system). 
	//
	// The product GUID is supplied to you by the packaging system. The method for getting
	// this GUID has not yet been defined. 
	//
	// The content GUID is supplied to you by Marketing. It may also be supplied by the
	// packaging system for convenience. 
	// -------------------------------------------------------------------------------------

	long Unlock ( const char* KeyString , const char* ContentGUID , const char* ProductGUID );

	// -------------------------------------------------------------------------------------
	// This function takes the store name and the license GUID. You only need to call this
	// routine if you used the constructor with no arguments.
	// -------------------------------------------------------------------------------------

	bool SetLicenseInfo ( const char* Store , const char* LicenseGUID );

	// -------------------------------------------------------------------------------------
	// The following functions obtain properties from the license. The function will return
	// false if there is a problem obtaining a value.
	// You need to call the appropriate routine for the property's defined type, and a
	// pointer to space to hold the value.
	// -------------------------------------------------------------------------------------

	bool GetIntProperty      ( const char* Name , int*   Result , int Default );
	bool GetFloatProperty    ( const char* Name , float* Result , float Default );
	bool GetBooleanProperty  ( const char* Name , bool*  Result , bool Default );
	bool GetDateTimeProperty ( const char* Name , DWORD* Result , DWORD Default );
	bool GetStringProperty   ( const char* Name , char*  Result , DWORD MaxLength, const char* Default );

	// -------------------------------------------------------------------------------------
	// The following functions sets properties in the license. The function will return
	// false if there is a problem setting the Value, or the property is not of the required
	// type, or the call is to apply to a read-only property. 
	// -------------------------------------------------------------------------------------

	bool SetIntProperty      ( const char* Name , int Value );
	bool SetFloatProperty    ( const char* Name , float Value );
	bool SetBooleanProperty  ( const char* Name , bool Value );
	bool SetDateTimeProperty ( const char* Name , DWORD Value );
	bool SetStringProperty   ( const char* Name , const char* Value );

private:	// Methods

	void Init ( void );
	static bool GetSessionIDSeed  ( long SessionID , long &Seed );
	static long GenerateSessionID ( long Seed );
	static long GenerateSeed ( void );

private:	// Data

	bool m_Initialized;

	bool m_InfoSet;
	char m_Store[256];
	char m_LicenseGUID[256];

	HMODULE m_hLicenseDLL;
	PFNGETINTPROPERTY m_getIntProperty;
	PFNGETFLOATPROPERTY m_getFloatProperty;
	PFNGETBOOLPROPERTY m_getBoolProperty;
	PFNGETDATEPROPERTY m_getDateProperty;
	PFNGETSTRINGPROPERTY m_getStringProperty;

	PFNUNLOCK m_unlock;

	PFNSETINTPROPERTY m_setIntProperty;
	PFNSETFLOATPROPERTY m_setFloatProperty;
	PFNSETBOOLPROPERTY m_setBoolProperty;
	PFNSETDATEPROPERTY m_setDateProperty;
	PFNSETSTRINGPROPERTY m_setStringProperty;
};