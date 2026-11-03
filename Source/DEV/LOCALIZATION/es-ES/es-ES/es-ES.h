// es-ES.h : main header file for the es-ES DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CesESApp
// See es-ES.cpp for the implementation of this class
//

class CesESApp : public CWinApp
{
public:
	CesESApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
