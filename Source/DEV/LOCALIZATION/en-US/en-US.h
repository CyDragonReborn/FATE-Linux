// en-US.h : main header file for the en-US DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CenUSApp
// See en-US.cpp for the implementation of this class
//

class CenUSApp : public CWinApp
{
public:
	CenUSApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
