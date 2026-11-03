// en-UK.h : main header file for the en-UK DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CenUKApp
// See en-UK.cpp for the implementation of this class
//

class CenUKApp : public CWinApp
{
public:
	CenUKApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
