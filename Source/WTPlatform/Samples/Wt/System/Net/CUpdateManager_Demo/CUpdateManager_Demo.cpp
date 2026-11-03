// CUpdateManager_Demo.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CUpdateManager_Demo.h"
#include "MainFrm.h"
#include ".\cupdatemanager_demo.h"
#include <mmsystem.h>


WT_IMPLEMENT_CREFERENCECOUNTED_MFC_OPERATORS();

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCUpdateManager_DemoApp

BEGIN_MESSAGE_MAP(CCUpdateManager_DemoApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_CHECKFORUPDATES, OnFileCheckforupdates)
	ON_COMMAND(ID_FILE_DOWNLOADNEWUPDATE, OnFileDownloadNewUpdate)
	ON_COMMAND(ID_FILE_INSTALLNEWUPDATE, OnFileInstallNewUpdate)
	ON_COMMAND(ID_FILE_BADMANIFEST, OnFileBadmanifest)
	ON_COMMAND(ID_FILE_BADPATCH, OnFileBadpatch)
	ON_COMMAND(ID_FILE_CANCELUPDATEINFORMATIONDOWNLOAD, OnFileCancelupdateinformationdownload)
	ON_COMMAND(ID_FILE_CANCELUPDATEDOWNLOAD, OnFileCancelupdatedownload)
	ON_COMMAND(ID_FILE_ABOUTTHEUPDATE, OnFileAbouttheupdate)
	ON_COMMAND(ID_LOCALE_ENGLISH, OnLocaleEnglishAmerica)
	ON_COMMAND(ID_LOCALE_ENGLISH32782, OnLocaleEnglishCanada)
	ON_COMMAND(ID_LOCALE_FRENCH, OnLocaleFrench)
	ON_COMMAND(ID_LOCALE_FRENCH32784, OnLocaleFrenchFrance)
	ON_COMMAND(ID_LOCALE_FRENCH32785, OnLocaleFrenchCanada)
	ON_COMMAND(ID_LOCALE_SHOWLOCALE, OnLocaleShowlocale)
END_MESSAGE_MAP()


// CCUpdateManager_DemoApp construction

CCUpdateManager_DemoApp::CCUpdateManager_DemoApp()
{
}

//DEMO: new routine
CCUpdateManager_DemoApp::~CCUpdateManager_DemoApp()
{
	//DEMO: delete the update manager that I made
	delete m_pUpdateManager;
	m_pUpdateManager = 0;
}


// The one and only CCUpdateManager_DemoApp object

CCUpdateManager_DemoApp theApp;

// CCUpdateManager_DemoApp initialization



BOOL CCUpdateManager_DemoApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("WT_System_Net_CUpdateManager_Demo"));
	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);
	// The one and only window has been initialized, so show and update it
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand

	//DEMO: initialize my update manager
	WT::Using<WT::System::Net::CMasterUpdate>();
	WT::Using<WT::System::Net::CNameValueI18N>();
	WT::Using<WT::System::Net::CPatchManifest>();
	WT::Using<WT::System::Net::CStringI18NSet>();
	WT::Using<WT::System::Net::CStringI18N>();
	WT::Using<WT::System::Net::CUpdateHistory>();
	WT::Using<WT::System::Net::CUpdateManager>();
	WT::Using<WT::System::Net::CUpdateStatus>();
	//WT::Using<WT::System::Serialization::Con


	//QUESTION: is this really the best solution?  Remember that 
	//there will be history data that (arguably) should be saved, too.
	char StartingDir [MAX_PATH];
	GetCurrentDirectoryA (sizeof (StartingDir), StartingDir);
	strcat (StartingDir, "\\");

	char Version [30];
	strcpy (Version, "?");
	FILE *f = fopen ("version.txt", "r");
	if (f)
	{
		fgets (Version, sizeof (Version), f);
		Version [sizeof(Version)-1] = '\0';
		const size_t LastPos = strlen (Version) - 1;
		if (Version[LastPos] == '\n')
		{
			Version [LastPos] = '\0'; // remove the \n
		}
		fclose (f);
	}
	m_pUpdateManager = new WT::System::Net::CUpdateManager(StartingDir, Version); 
	return TRUE;
}


// CCUpdateManager_DemoApp message handlers



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CCUpdateManager_DemoApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CCUpdateManager_DemoApp message handlers

// The AUM timer is called every second (ish).  This code uses that
// timeslice to figure out what to do next.
//
//
void CCUpdateManager_DemoApp::OnAUMTimer ()
{
	m_pMainWnd->Invalidate (0);
}

void CCUpdateManager_DemoApp::OnFileCheckforupdates()
{
	if (!m_pUpdateManager)
	{
		return;
	}
	m_pUpdateManager->StartDownloadingManifest();
}


void CCUpdateManager_DemoApp::OnFileDownloadNewUpdate()
{
	using namespace WT::System::Net;
	if (!m_pUpdateManager)
	{
		return;
	}
	if (!m_pUpdateManager->IsPatchAvailable())
	{
		// TODO: handle this better.
		MessageBox (0, L"No new update to download", L"Nope!", MB_OK);
		return;
	}
	m_pUpdateManager->StartDownloadingPatch();
}


void CCUpdateManager_DemoApp::OnFileInstallNewUpdate()
{
	using namespace WT::System::Net;
	if (!m_pUpdateManager)
	{
		return;
	}

	if (!m_pUpdateManager->IsPatchReady())
	{
		// TODO: handle this better.
		MessageBox (0, L"No new update to install", L"Nope!", MB_OK);
		return;
	}
	m_pUpdateManager->StartInstallingPatch();
}

void CCUpdateManager_DemoApp::OnFileBadmanifest()
{
	if (!m_pUpdateManager)
	{
		return;
	}
	m_pUpdateManager->NotifiedUserOfBadManifest();
}

void CCUpdateManager_DemoApp::OnFileBadpatch()
{
	if (!m_pUpdateManager)
	{
		return;
	}
	m_pUpdateManager->NotifiedUserOfBadPatch();
}

void CCUpdateManager_DemoApp::OnFileCancelupdateinformationdownload()
{
	if (!m_pUpdateManager)
	{
		return;
	}
	m_pUpdateManager->CancelManifestDownload();
}

void CCUpdateManager_DemoApp::OnFileCancelupdatedownload()
{
	if (!m_pUpdateManager)
	{
		return;
	}
	m_pUpdateManager->CancelPatchDownload();
}

void CCUpdateManager_DemoApp::OnFileAbouttheupdate()
{
	if (!m_pUpdateManager)
	{
		return;
	}
	const bool IsAvail = m_pUpdateManager->IsPatchAvailable();
	if (IsAvail)
	{
		MessageBox (0, L"Sound is turned off", L"Sound", MB_OK);
		/*
		// Get the sound out
		using namespace WT::System::IO;
		using namespace WT::System::Net;
		CPatchManifest* pPM = m_pUpdateManager->GetPatchManifest();
		const bool SoundExists = pPM->ExpansionExists ("sound");
		if (SoundExists)
		{
			const std::string& Base64 = pPM->GetExpansionString ("sound").Get();
			CMemoryStream outf;
			CBase64Decoder::StringToStream (Base64, &outf);
			PlaySound ((LPCSTR)outf.GetHandle(), 0, SND_MEMORY | SND_SYNC);
			outf.Close();
		}
		*/
	}
}

//TODO: this is bad.
const wchar_t* const N2W(const char* const Str)
{
	static wchar_t WBuffer[1024];
	const int Len = (int)strlen (Str);
	for (int i=0; i<Len; i++)
	{
		WBuffer[i] = Str[i];
	}
	WBuffer[Len] = '\0';
	return WBuffer;
}
void CCUpdateManager_DemoApp::OnLocaleEnglishAmerica()
{
	setlocale (LC_ALL, "English_United States");
	const char* const NewLocale = setlocale(LC_ALL, 0);
	MessageBox(0, N2W(NewLocale), L"DBG: Locale set to: ", MB_OK | MB_ICONASTERISK);
}

void CCUpdateManager_DemoApp::OnLocaleEnglishCanada()
{
	setlocale (LC_ALL, "English_Canada");
	const char* const NewLocale = setlocale(LC_ALL, 0);
	MessageBox(0, N2W(NewLocale), L"DBG: Locale set to: ", MB_OK | MB_ICONASTERISK);
}

void CCUpdateManager_DemoApp::OnLocaleFrench()
{
	setlocale (LC_ALL, "French");
	const char* const NewLocale = setlocale(LC_ALL, 0);
	MessageBox(0, N2W(NewLocale), L"DBG: Locale set to: ", MB_OK | MB_ICONASTERISK);
}

void CCUpdateManager_DemoApp::OnLocaleFrenchFrance()
{
	setlocale (LC_ALL, "French_France");
	const char* const NewLocale = setlocale(LC_ALL, 0);
	MessageBox(0, N2W(NewLocale), L"DBG: Locale set to: ", MB_OK | MB_ICONASTERISK);
}

void CCUpdateManager_DemoApp::OnLocaleFrenchCanada()
{
	setlocale (LC_ALL, "French_Canada");
	const char* const NewLocale = setlocale(LC_ALL, 0);
	MessageBox(0, N2W(NewLocale), L"DBG: Locale set to: ", MB_OK | MB_ICONASTERISK);
}

void CCUpdateManager_DemoApp::OnLocaleShowlocale()
{
	const char* const NewLocale = setlocale(LC_ALL, 0);
	MessageBox(0, N2W(NewLocale), L"DBG: Locale set to: ", MB_OK | MB_ICONASTERISK);
}
