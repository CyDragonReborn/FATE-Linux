// it-IT.h : main header file for the it-IT DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CitITApp
// See it-IT.cpp for the implementation of this class
//

class CitITApp : public CWinApp
{
public:
	CitITApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
