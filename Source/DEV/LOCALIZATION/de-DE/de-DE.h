// de-DE.h : main header file for the de-DE DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CdeDEApp
// See de-DE.cpp for the implementation of this class
//

class CdeDEApp : public CWinApp
{
public:
	CdeDEApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
