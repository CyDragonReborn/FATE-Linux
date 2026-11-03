/***************************************************************************************
 *
 * HTMLWindow.cpp
 *
 * Copyright (C) 2006 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          02/23/2006
 * Notes:         
 *
 ***************************************************************************************/

#include "WT/stdafx.h"
#include <windows.h>
#include <atlbase.h>
#include <atlwin.h>
#include <exdispid.h>
#include "HTMLWindow.h"

namespace WT
{
namespace System
{

WT_DEFINE_CLASS( CHTMLWindow )

CHTMLWindow::CHTMLWindow( HWND hParentWindow, Math::CRect2i const& Rect,
	std::string const& URL, DWORD dwStyle, DWORD dwExStyle )
{
	InitATLModule();

	Init( hParentWindow, Rect, URL, dwStyle, dwExStyle );
}

CHTMLWindow::~CHTMLWindow()	
{
}

/*virtual*/ void CHTMLWindow::Close()
{
	m_Window.DestroyWindow();
}
	
}
}
