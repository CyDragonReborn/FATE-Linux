#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include "drm3license.h"
#include <time.h>

// -------------------------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------------------------

#define FIRST_SEED_NUMBER 97003
#define SECOND_SEED_NUMBER 541

#define CString		"Do not use MFC classes in this code!!!"


CDRM3License::CDRM3License(void)
{
	m_InfoSet = false;
	m_Initialized = false;
	m_hLicenseDLL = NULL;

	Init();
}

CDRM3License::CDRM3License(const char* Store, const char* licenseGUID)
{
	m_InfoSet = false;
	Init();
	SetLicenseInfo(Store, licenseGUID);
}

void CDRM3License::Init(void)
{
	// Attempt to load the DLL by the full path

	HKEY hAppsKey = NULL;
	if ( RegOpenKeyEx ( HKEY_LOCAL_MACHINE , "SOFTWARE\\WildTangent" , NULL , KEY_READ , &hAppsKey ) == ERROR_SUCCESS )
	{

		char appsPath[MAX_PATH + 1];
		DWORD bufSize = sizeof(appsPath);
		LONG rv = RegQueryValueEx( hAppsKey, "Apps", NULL, NULL, (LPBYTE)appsPath, &bufSize);
		RegCloseKey(hAppsKey);
		if ( rv == ERROR_SUCCESS)
		{
			char dllPath[MAX_PATH + 1];
			strcpy(dllPath, appsPath);
			strcat(dllPath, DRMDLLNAME);
			m_hLicenseDLL = ::LoadLibrary(dllPath);
		}
	}

	// Second-chance attempt at loading the DLL

	if (m_hLicenseDLL == NULL)
	{
		m_hLicenseDLL = ::LoadLibrary(DRMDLLNAME);
		if (m_hLicenseDLL == NULL)
		{
			return;
		}
	}

	// Get* functions

	m_getIntProperty = (PFNGETINTPROPERTY)::GetProcAddress(m_hLicenseDLL, "DRM3_Function1");
	if (m_getIntProperty == NULL)
		return;
	
	m_getFloatProperty = (PFNGETFLOATPROPERTY)::GetProcAddress(m_hLicenseDLL, "DRM3_Function2");
	if (m_getFloatProperty == NULL)
		return;

	m_getBoolProperty = (PFNGETBOOLPROPERTY)::GetProcAddress(m_hLicenseDLL, "DRM3_Function3");
	if (m_getBoolProperty == NULL)
		return;

	m_getDateProperty = (PFNGETDATEPROPERTY)::GetProcAddress(m_hLicenseDLL, "DRM3_Function4");
	if (m_getDateProperty == NULL)
		return;

	m_getStringProperty = (PFNGETSTRINGPROPERTY)::GetProcAddress(m_hLicenseDLL, "DRM3_Function5");
	if (m_getStringProperty == NULL)
		return;

	// Unlock function

	m_unlock = (PFNUNLOCK)::GetProcAddress(m_hLicenseDLL, "DRM3_Function7");
	if (m_unlock == NULL)
		return;

	// Set* functions

	m_setIntProperty = (PFNSETINTPROPERTY)::GetProcAddress(m_hLicenseDLL, "DRM3_Function8");
	if (m_setIntProperty == NULL)
		return;
	
	m_setFloatProperty = (PFNSETFLOATPROPERTY)::GetProcAddress(m_hLicenseDLL, "DRM3_Function9");
	if (m_setFloatProperty == NULL)
		return;

	m_setBoolProperty = (PFNSETBOOLPROPERTY)::GetProcAddress(m_hLicenseDLL, "DRM3_Function10");
	if (m_setBoolProperty == NULL)
		return;

	m_setDateProperty = (PFNSETDATEPROPERTY)::GetProcAddress(m_hLicenseDLL, "DRM3_Function11");
	if (m_setDateProperty == NULL)
		return;

	m_setStringProperty = (PFNSETSTRINGPROPERTY)::GetProcAddress(m_hLicenseDLL, "DRM3_Function12");
	if (m_setStringProperty == NULL)
		return;

	// Seed the random number generator

	srand((unsigned)time(NULL));

	// Ready to go

	m_Initialized = true;

	return;
}

CDRM3License::~CDRM3License(void)
{
	if (m_hLicenseDLL)
	{
		::FreeLibrary(m_hLicenseDLL);
		m_hLicenseDLL = NULL;
	}
}

// Set up the Store and GUID

bool CDRM3License::SetLicenseInfo(const char* Store, const char* licenseGUID)
{
	// Save the Store and licenseGUID

	if (Store == NULL || strlen(Store) > (sizeof(m_Store) - 1))
		return false;

	if (licenseGUID == NULL || strlen(licenseGUID) > (sizeof(m_LicenseGUID) - 1))
		return false;

	strcpy(m_Store, Store);
	strcpy(m_LicenseGUID, licenseGUID);

	m_InfoSet = true;

	return true;
}

// The following routines create, encode and decode the session Seed

bool CDRM3License::GetSessionIDSeed(long SessionID, long& Seed)
{
	if ( ((SessionID - FIRST_SEED_NUMBER) % SECOND_SEED_NUMBER ) != 0 )
		return false;

	Seed = ( SessionID - FIRST_SEED_NUMBER ) / SECOND_SEED_NUMBER ;
	return true;
}

long CDRM3License::GenerateSessionID(long Seed)
{
	return Seed * SECOND_SEED_NUMBER + FIRST_SEED_NUMBER;
}

long CDRM3License::GenerateSeed(void)
{
	long Seed = (long)(rand() % 9876);
	return Seed;
}

// These routines all get properties from the license

bool CDRM3License::GetIntProperty(const char* Name, int* Result, int Default)
{
	if (Result == NULL || !m_Initialized || !m_InfoSet || Name == NULL)
	{
		if (Result != NULL)
		{
			*Result = Default;
		}

		return false;
	}

	long Seed = GenerateSeed();
	long SessionID = GenerateSessionID(Seed);

	// Call the function

	long ResultSeed = (*m_getIntProperty)(SessionID, m_Store, m_LicenseGUID, Name, Result, Default);
	if (ResultSeed != Seed)
	{
		*Result = Default;
		return false;
	}

	return true;
}

bool CDRM3License::GetFloatProperty(const char* Name, float* Result, float Default)
{
	if (Result == NULL || !m_Initialized || !m_InfoSet || Name == NULL)
	{
		if (Result != NULL)
		{
			*Result = Default;
		}

		return false;
	}

	long Seed = GenerateSeed();
	long SessionID = GenerateSessionID(Seed);

	// Call the function

	long ResultSeed = (*m_getFloatProperty)(SessionID, m_Store, m_LicenseGUID, Name, Result, Default);
	if (ResultSeed != Seed)
	{
		*Result = Default;
		return false;
	}

	return true;
}

bool CDRM3License::GetBooleanProperty(const char* Name, bool* Result, bool Default)
{
	if (Result == NULL || !m_Initialized || !m_InfoSet || Name == NULL)
	{
		if (Result != NULL)
		{
			*Result = Default;
		}

		return false;
	}

	long Seed = GenerateSeed();
	long SessionID = GenerateSessionID(Seed);

	// Call the function

	long ResultSeed = (*m_getBoolProperty)(SessionID, m_Store, m_LicenseGUID, Name, (BOOL*)Result, (BOOL)Default);
	if (ResultSeed != Seed)
	{
		*Result = (bool)Default;
		return false;
	}

	return true;
}


bool CDRM3License::GetDateTimeProperty(const char* Name, DWORD* Result, DWORD Default)
{
	if (Result == NULL || !m_Initialized || !m_InfoSet || Name == NULL)
	{
		if (Result != NULL)
		{
			*Result = Default;
		}

		return false;
	}

	long Seed = GenerateSeed();
	long SessionID = GenerateSessionID(Seed);

	// Call the function

	long ResultSeed = (*m_getDateProperty)(SessionID, m_Store, m_LicenseGUID, Name, Result, Default);
	if (ResultSeed != Seed)
	{
		*Result = Default;
		return false;
	}

	return true;
}

bool CDRM3License::GetStringProperty(const char* Name, char* Result, DWORD MaxLenth, const char* Default )
{
	if (Result == NULL || !m_Initialized || !m_InfoSet || Name == NULL)
	{
		if (Result != NULL && Default != NULL)
		{
			strcpy(Result, Default);
		}

		return false;
	}

	long Seed = GenerateSeed();
	long SessionID = GenerateSessionID(Seed);

	// Call the function

	long ResultSeed = (*m_getStringProperty)(SessionID, m_Store, m_LicenseGUID, Name, MaxLenth, Result, Default );
	if (ResultSeed != Seed)
	{
		if (Result != NULL && Default != NULL)
		{
			strcpy(Result, Default);
		}
		return false;
	}

	return true;
}

// Unlock using the specified key and the product and content IDs

long CDRM3License::Unlock(const char* keyString, const char* contentID, const char* productID)
{
	if (!m_Initialized)
		return DRM3LICENSE_FAIL_INIT;

	if (keyString == NULL)
		return DRM3LICENSE_FAIL_ARGS;

	if (productID == NULL && contentID == NULL)
		return DRM3LICENSE_FAIL_ARGS;

	long Seed = GenerateSeed();
	long SessionID = GenerateSessionID(Seed);

	// Call the function

	long ResultSeed = (*m_unlock)(SessionID, keyString, contentID, productID);
	if (ResultSeed != Seed)
		return ResultSeed;

	return 0;
}

// These routines all set properties in the license

bool CDRM3License::SetIntProperty(const char* Name, int Value)
{
	if (!m_Initialized || !m_InfoSet || Name == NULL)
		return false;

	long Seed = GenerateSeed();
	long SessionID = GenerateSessionID(Seed);

	// Call the function

	long ResultSeed = (*m_setIntProperty)(SessionID, m_Store, m_LicenseGUID, Name, Value);
	if (ResultSeed != Seed)
		return false;

	return true;
}

bool CDRM3License::SetFloatProperty(const char* Name, float Value)
{
	if (!m_Initialized || !m_InfoSet || Name == NULL)
		return false;

	long Seed = GenerateSeed();
	long SessionID = GenerateSessionID(Seed);

	// Call the function

	long ResultSeed = (*m_setFloatProperty)(SessionID, m_Store, m_LicenseGUID, Name, Value);
	if (ResultSeed != Seed)
		return false;

	return true;
}

bool CDRM3License::SetBooleanProperty(const char* Name, bool Value)
{
	if (!m_Initialized || !m_InfoSet || Name == NULL)
		return false;

	long Seed = GenerateSeed();
	long SessionID = GenerateSessionID(Seed);

	// Call the function

	long ResultSeed = (*m_setBoolProperty)(SessionID, m_Store, m_LicenseGUID, Name, (BOOL)Value);
	if (ResultSeed != Seed)
		return false;

	return true;
}


bool CDRM3License::SetDateTimeProperty(const char* Name, DWORD Value)
{
	if (!m_Initialized || !m_InfoSet || Name == NULL)
		return false;

	long Seed = GenerateSeed();
	long SessionID = GenerateSessionID(Seed);

	// Call the function

	long ResultSeed = (*m_setDateProperty)(SessionID, m_Store, m_LicenseGUID, Name, Value);
	if (ResultSeed != Seed)
		return false;

	return true;
}

bool CDRM3License::SetStringProperty(const char* Name, const char* Value)
{
	if (!m_Initialized || !m_InfoSet || Name == NULL || Value == NULL)
		return false;

	long Seed = GenerateSeed();
	long SessionID = GenerateSessionID(Seed);

	// Call the function

	long ResultSeed = (*m_setStringProperty)(SessionID, m_Store, m_LicenseGUID, Name, Value);
	if (ResultSeed != Seed)
		return false;

	return true;
}