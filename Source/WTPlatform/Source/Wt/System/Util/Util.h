/******************************************************************************
	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved

	History:
	2/13/2006	Rui Jiang	Createed

	Description:  Utility functions/classes. Please note, these functions
	should be platform dependant. That is, they should not contain 
	any operating system specific functions. Put them in WinUtil instead.
******************************************************************************/
#pragma once

#include <iostream>
#include <strstream>
#include <vector>

namespace WT
{
namespace System
{
namespace Util
{

/////////////////////////////////////////////////////////////////////////////
// This template class has function to automatically release object.
// Use it to declare things like: 
//   autoreleaselist<vector<CSomeClass*> > m_MyList;
// Then this list will automatically delete the pointers while erase or clear.
template<class stlclass> class autoreleaselist : public stlclass
{
public:
	virtual ~autoreleaselist()
	{
		//We have to manually override the base class's destructor because
		//it will call base class's CLEAR instead of ours.
		clear();
	}
	typedef typename stlclass::iterator stliterator;
	stliterator erase(stliterator it)
	{
		delete (*it);
		return stlclass::erase(it);
	}
	stliterator erase(stliterator first, stliterator last)
	{
		for (stliterator i = first; i != last; i++)
			delete (*i);
		return stlclass::erase(first, last);
	}
	void clear()
	{
		erase( begin(), end());
	}
};

}}} //namespace
