// CUpdateManager_Demo.h : main header file for the CUpdateManager_Demo application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "WT/System/Net/Net.h"
#include "WT/System/IO/Base64Decoder.h"

// CCUpdateManager_DemoApp:
// See CUpdateManager_Demo.cpp for the implementation of this class
//

class CCUpdateManager_DemoApp : public CWinApp
{
public:
	CCUpdateManager_DemoApp();

	//DEMO: added the destructor
	~CCUpdateManager_DemoApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileCheckforupdates();

	//DEMO: added member variables for AUM
public:
	WT::System::Net::CUpdateManager* m_pUpdateManager;
	//DEMO: added methods for handling 
	void OnAUMTimer();
	bool IsPatchAvailable() const 
	{ 
		const bool Retval = m_pUpdateManager ? m_pUpdateManager->IsPatchAvailable() : false;
		return Retval;
	}
	bool IsPatchReady() const 
	{ 
		const bool Retval = m_pUpdateManager ? m_pUpdateManager->IsPatchReady() : false;
		return Retval;
	}
	int GetPatchState() const
	{
		const int Retval = m_pUpdateManager ? (int)m_pUpdateManager->GetCurrentState () : -1;
		return Retval;
	}


	afx_msg void OnFileDownloadNewUpdate();
	afx_msg void OnFileInstallNewUpdate();
	afx_msg void OnFileBadmanifest();
	afx_msg void OnFileBadpatch();
	afx_msg void OnFileCancelupdateinformationdownload();
	afx_msg void OnFileCancelupdatedownload();
	afx_msg void OnFileAbouttheupdate();
	afx_msg void OnLocaleEnglishAmerica();
	afx_msg void OnLocaleEnglishCanada();
	afx_msg void OnLocaleFrench();
	afx_msg void OnLocaleFrenchFrance();
	afx_msg void OnLocaleFrenchCanada();
	afx_msg void OnLocaleShowlocale();
};

extern CCUpdateManager_DemoApp theApp;