#pragma once

#include "WT/System/Object.h"

namespace WT
{
namespace System
{


// This helper class will clean up objects when it goes out of scope.
// It is passed a pointer to an Object. When this class goes out of scope it
// will delete the object and clear the pointer to NULL (this class is given
// a reference to an Object pointer).
//
// This class is particularly useful for ensuring that we don't have memory
// leaks even when exceptions are thrown.
template <class T>
class CScopedDeleteAndClear
{
public:
	CScopedDeleteAndClear(T*& rpObj) :
	  m_rpObject(rpObj)
	{
	}

	~CScopedDeleteAndClear()
	{
		delete m_rpObject;
		m_rpObject = NULL;
	}

	// to appease compiler warnings:
	void operator =(CScopedDeleteAndClear const & )
	{
		assert(0);
	}
private:
	T*& m_rpObject;
};


// constuctor is passed a reference to a pointer.
// When this class goes out of scope the pointer will be
// cleared to NULL. The pointer will NOT be deleted.
template <class T>
class CScopedClear
{
public:
	CScopedClear(T*& rpObj) :
	  m_rpObject(rpObj)
	{
	}

	~CScopedClear()
	{
		m_rpObject = NULL;
	}

	// to appease compiler warnings:
	void operator =(CScopedClear const & )
	{
		assert(0);
	}
private:
	T*& m_rpObject;
};

template <class T>
class CScopedDeleteArray 
{
public:
	CScopedDeleteArray(T* pObj) :
	  m_pObject(pObj)
	{
	}

	~CScopedDeleteArray()
	{
		delete [] m_pObject;
	}

	// to appease compiler warnings:
	void operator =(CScopedDeleteArray const & )
	{
		assert(0);
	}
private:
	T* m_pObject;
};

} // namespace System
} // namespace WT
