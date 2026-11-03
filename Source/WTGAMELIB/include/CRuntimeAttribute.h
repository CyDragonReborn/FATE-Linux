//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	CRuntimeAttribute.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Brent Orford
	3/8/2005
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>

#ifndef ___IWTGAMELIB_H___
#define ___IWTGAMELIB_H___

	class CRuntimeAttribute
	{
		public:

			// Constructor and Destructor
			CRuntimeAttribute();
			~CRuntimeAttribute();

			void	GetSKU		( TCHAR* ReturnValue, int ReturnValueBufferSize );

			void	GetString	( const TCHAR* Name, TCHAR* ReturnValue, int ReturnValueBufferSize );
			int		GetInt		( const TCHAR* Name );
			float	GetFloat	( const TCHAR* Name );
			bool	GetBool		( const TCHAR* Name );

		private:
			TCHAR m_SKUNumber[_MAX_PATH];

			void FindSKU ( TCHAR* ReturnValue, int ReturnValueBufferSize );

			void GetApplicationDirectory ( TCHAR* ReturnValue, int ReturnValueBufferSize );
			void GetApplicationExecutableName ( TCHAR* ReturnValue, int ReturnValueBufferSize );
			void GetRegString ( const LPTSTR HKLM_KeyName, const LPTSTR StringName, LPBYTE ReturnValue, DWORD ReturnValueBufferSize );
			void GetRegStringIfPresent ( const LPTSTR HKLM_KeyName, const LPTSTR StringName, LPBYTE ReturnValue, DWORD ReturnValueBufferSize );
			bool IniFileExists( TCHAR* IniFileName, DWORD IniFileNameBufferSize );
			void GetIniFileString ( const LPTSTR KeyName, TCHAR* ReturnValue, DWORD ReturnValueBufferSize );

			bool m_DevSku;
			bool m_UseLocalIniFile;
	};

#endif