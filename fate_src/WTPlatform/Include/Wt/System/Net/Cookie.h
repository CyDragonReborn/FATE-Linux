/*
	Cookie.h

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005
 */
#pragma once

#include "WT/System/Uri.h"
#include "WT/System/DateTime.h"
namespace WT
{
namespace System
{
namespace Net
{

typedef SmartPointer<class CCookie> HCookie;
class CCookie : public CObject
{
public:
	WT_CLASS( CCookie, CObject );

	CCookie() { }
	CCookie(const string& Name, const string& Value):m_Name(Name),m_Value(Value) { }
	//Cookie(const Uri& Uri);
	virtual ~CCookie() { }

	const string& GetName() const { return m_Name; }
	const string& GetValue() const { return m_Value; }

	void SetName(const string Value) { m_Name = Value; }
	void SetValue(const string Value) { m_Value = Value; }

private:
	string m_Name;
	string m_Value;

	// There are lots of potential extras for cookies that .NET
	// supports; for now (Sep-2005) our library doesn't support
	// those features
	//string m_Comment;
	//bool m_Discard;
	//string m_Domain;
	//bool m_Expired;
	//WT::System::DateTime m_Expires;
	//string m_Path;
	//string m_Port;
	//bool m_Secure;
	//WT::System::DateTime m_TimeStamp;
	//int32 m_Version;
};

} // end namespace Net
} // end namespace System
} // end namespace WT
