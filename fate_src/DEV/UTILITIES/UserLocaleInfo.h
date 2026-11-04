/*
	UserLocaleInfo.h

	Copyright (C) 2007 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	10/01/2007

*/

#ifndef WT_SYSTEM_USERLOCALEINFO_H
#define WT_SYSTEM_USERLOCALEINFO_H

#include <string>

//namespace WT		//commented by TDC for Fate
//{		//commented by TDC for Fate
//namespace System		//commented by TDC for Fate
//{		//commented by TDC for Fate

class CUserLocaleInfo
{
public:
	CUserLocaleInfo();
	~CUserLocaleInfo();

	std::wstring GetLocaleName() const;

private:
	std::wstring m_wsLocaleName;
};
//
//} //namespace System		//commented by TDC for Fate
//} //namespace WT		//commented by TDC for Fate

#endif //WT_SYSTEM_USERLOCALEINFO_H
