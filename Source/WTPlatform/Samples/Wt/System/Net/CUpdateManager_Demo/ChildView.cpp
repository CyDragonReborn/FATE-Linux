// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif
#include "CUpdateManager_Demo.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "WT/System/Net/Net.h"
#include ".\childview.h"

// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}
using namespace WT::System::Net;
extern const wchar_t* const N2W(const char* const Str);

void CChildView::DrawI18N (CPaintDC& dc, RECT& r, const CStringI18N& Text)
{
	CFont cFont;
	const int H=Text.GetHeight() ? Text.GetHeight() : 20;
	const int CharSet = Text.GetCharset();
	const wchar_t* const WFont = N2W(Text.GetFontname().c_str());
	cFont.CreateFont (H, 0, 0, 0, FW_DONTCARE, 0, 0, 0, 
		CharSet,
		OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		VARIABLE_PITCH | FF_DONTCARE, WFont);

	CFont* OrigFont = dc.SelectObject (&cFont);

	TEXTMETRIC Metrics;
	dc.GetTextMetrics (&Metrics);
	const int CH = Metrics.tmHeight + Metrics.tmExternalLeading;
	r.bottom = r.top + CH;
	const std::wstring& Value = Text.Get();
	dc.DrawText (Value.c_str(), (int)Value.length(), &r, DT_SINGLELINE);
	r.top += CH; r.bottom += CH;

	dc.SelectObject (OrigFont);
	cFont.DeleteObject ();
}

void CChildView::OnPaint() 
{
	using namespace WT::System::Net;
	const int KCH = 20; // character height
	CPaintDC dc(this); // device context for painting
	RECT r;
	GetClientRect (&r);
	r.bottom = r.top + KCH;



	dc.DrawText (L"WT::System::Net::CUpdater demonstation program: patch ability", -1, &r, DT_CENTER | DT_SINGLELINE );
	r.top += KCH; r.bottom += KCH;

	TCHAR buffer [256];
	CUpdateManager* pUM = theApp.m_pUpdateManager;
	if (pUM)
	{
		swprintf (buffer, L"Download Percent (0..100) %6.2f%%            ", pUM->GetPercentDone());
		dc.DrawText (buffer, -1, &r, DT_SINGLELINE);
		r.top += KCH; r.bottom += KCH;

		swprintf (buffer, L"Version %S                               ", pUM->GetVersion().c_str());
		dc.DrawText (buffer, -1, &r, DT_SINGLELINE);
		r.top += KCH; r.bottom += KCH;

		CPatchManifest* pPM = pUM->GetPatchManifest();
		if (pPM)
		{
			const CStringI18N& Short = pPM->GetAboutShort();
			DrawI18N (dc, r, Short);

			const CStringI18N& Long = pPM->GetAboutLong();
			DrawI18N (dc, r, Long);
		}
		else
		{
			dc.DrawText (L"no patch manifest to get about information                       ", -1, &r, DT_SINGLELINE);
			r.top += KCH; r.bottom += KCH;
		}

	
	}

	const EUpdateManagerState::Enum State = (EUpdateManagerState::Enum)theApp.GetPatchState();
	switch (State)
	{
	case EUpdateManagerState::KCanDownloadManifest:
		swprintf (buffer, L"Can download manifest                       ");
		break;
	case EUpdateManagerState::KAmDownloadingManifest:
		swprintf (buffer, L"Am downloading manifest                       ");
		break;
	case EUpdateManagerState::KAmNotifyingBadManifest:
		swprintf (buffer, L"Am notifying the user of a bad manifest                       ");
		break;
	case EUpdateManagerState::KCanDownloadPatch:
		swprintf (buffer, L"Can download patch                       ");
		break;
	case EUpdateManagerState::KAmDownloadingPatch:
		swprintf (buffer, L"Am downloading patch                       ");
		break;
	case EUpdateManagerState::KAmNotifyingBadPatch:
		swprintf (buffer, L"Am notifying the user of a bad patch.exe                       ");
		break;
	case EUpdateManagerState::KCanInstallPatch:
		swprintf (buffer, L"Can install patch now!                       ");
		break;
	default:
		swprintf (buffer, L"Error: unknown state %d            ", State);
		break;
	}
	//swprintf (buffer, L"Patch State %d                       ", State);
	dc.DrawText (buffer, -1, &r, DT_SINGLELINE);
	r.top += KCH; r.bottom += KCH;

	GetClientRect (&r);
	int CX = (r.left + r.right) / 2;
	int CY = (r.top + r.bottom) / 2;
	if (pUM)
	{
		// Draw the percent-done "target"
		float Pct = pUM->GetPercentDone(); 
		//if (Pct > 0.0f)
		{

			int R = min (CX, CY) * 8 / 10;
			const RECT BB = { CX-R, CY-R, CX+R, CY+R };
			const POINT Start = { CX, BB.top };
			const float Angle = (float)(Pct * 2 * M_PI / 100);
			const POINT End = { CX+(int)(R * sin (Angle)) , CY-(int)(R*cos(Angle)) };

			dc.MoveTo (CX-R, CY); dc.LineTo (CX+R, CY);
			dc.MoveTo (CX, CY-R); dc.LineTo (CX, CY+R);

			CGdiObject OldPen;
			CPen Black (PS_SOLID, 10, RGB (0, 0, 0));
			CPen White (PS_SOLID, 10, RGB (255, 255, 255));

			OldPen.FromHandle (dc.SelectObject (&Black));
			dc.Arc (&BB, End, Start);

			dc.SelectObject (&White);
			dc.Arc (&BB, Start, End);

			dc.SelectObject (&OldPen);
		}
	}

	HBITMAP hBitmap = (HBITMAP)LoadImage (0, L"updates\\download\\picture.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE); 
	CDC bmpdc;
	bmpdc.CreateCompatibleDC(&dc);
	HBITMAP hOld = (HBITMAP)bmpdc.SelectObject (hBitmap);
	BITMAP bm;
	GetObject (hBitmap, sizeof (bm), &bm);
	dc.BitBlt (CX-bm.bmWidth/2, CY-bm.bmHeight/2, bm.bmWidth, bm.bmHeight, &bmpdc, 0, 0, SRCCOPY);
	bmpdc.SelectObject (hOld);
	DeleteObject (hOld);
	DeleteObject (hBitmap);
	bmpdc.DeleteDC();

}


void CChildView::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnTimer(nIDEvent);
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	//DEMO: never erase the background
	//return CWnd::OnEraseBkgnd(pDC);
	return 0;
}

