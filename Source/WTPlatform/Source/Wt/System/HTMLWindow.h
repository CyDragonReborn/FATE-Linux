/***************************************************************************************
 *
 * HTMLWindow.h
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

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/System/Object.h"
#include "WT/System/HTMLBase.h"

namespace WT
{
namespace System
{

/*
CHTMLWindow is a window that hosts HTML documents and is ideal for in-game use. 
*/
typedef SmartPointer<class CHTMLWindow> HHTMLWindow;
class CHTMLWindow : public CObject, public CHTMLBase
{
public:
	WT_CLASS_NO_CREATOR( CHTMLWindow, CObject );

	CHTMLWindow( HWND hParentWindow, Math::CRect2i const& Rect, std::string const& URL = "about:blank",
		DWORD dwStyle = WS_VISIBLE, DWORD dwExStyle = 0 );
	virtual ~CHTMLWindow();

	//
	//
	// window.external methods
	//
	//

	//Close the window.
	virtual void Close();
};

}
}
