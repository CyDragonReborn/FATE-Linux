/*
	CookieCollection.h

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005
 */
#pragma once

#include <vector>
#include "WT/System/Net/Cookie.h"
namespace WT
{
namespace System
{
namespace Net
{

typedef SmartPointer<class CCookieCollection> HCookieCollection;
class CCookieCollection  : public CObject 
{
public:
	WT_CLASS( CCookieCollection, CObject );

	CCookieCollection() { }
	virtual ~CCookieCollection()
	{
		const int32 N = Count();
		for (int32 i=0; i<N; i++)
		{
			delete m_Data[i];
			m_Data[i] = 0;
		}
	}

	// Adds a cookie value.  The collection will then take ownership of the 
	// object and will delete it when the collection is deleted.
	void Add (CCookie* Value)
	{
		m_Data.push_back (Value);
	}

	int32 Count() const 
	{
		return (int)m_Data.size();
	}

	CCookie* Item (const int32 Index)
	{
		const int32 N = Count();
		if (Index < 0 || Index >= N)
		{
			// TODO: should throw something here
			assert (false);
			return 0;
		}
		return m_Data[Index];
	}


private:
	vector<CCookie*> m_Data;
};

} // end namespace Net
} // end namespace System
} // end namespace WT
