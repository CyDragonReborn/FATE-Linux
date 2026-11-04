/*
	Thread.h

	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	4/20/2006

*/

#pragma once

#include "WT/System/Object.h"
#include "WT/System/Threading.h"
#include "windows.h"

namespace WT
{
namespace System
{

enum EThreadState
{
	KThreadStateSuspended,
	KThreadStateRunning,
	KThreadStateExited,
	KThreadStateCount //Number of thread states
};

class CThread;

//
// CThreadFunction represents a function to be executed on a thread
//
typedef SmartPointer<class CThreadFunction> HThreadFunction;
class CThreadFunction : public CObject
{
public:
	WT_ABSTRACT_CLASS( CThreadFunction, CObject );

	//Execute the function. The running thread is passed in; it can be
	//used to check whether quick finishing is requested (Thread.GetShouldFinish()).
	virtual uint32 Execute( CThread& Thread ) = 0;
};

//
// CThread executes a CThreadFunction in its own thread.
//
typedef SmartPointer<class CThread> HThread;
class CThread : public CObject
{
public:
	WT_CLASS_NO_CREATOR( CThread, CObject );

	//Constructor. ThreadProc points to the function that will be executed;
	//it must not be NULL. The function will receive as an argument a CThread*
	//which is not necessarily castable to a derived type. If Suspended is true, 
	//then the thread will begin suspended. If no explicit stack size (in bytes) 
	//is specified, then the OS will use the stack size of the main thread. 
	CThread( CThreadFunction* pThreadFunction, bool Suspended = false, uint32 StackSizeBytes = 0 );

	//Destructor. If the thread is still running, this function requests 
	//that it finish and then waits for it to finish.
	virtual ~CThread();

	//Get the thread handle.
	HANDLE GetHandle();

	//Get the state of the thread.
	EThreadState GetState() const;

	//Get the result of the thread function. An exception occurs if the thread is not finished yet.
	uint32 GetResult() const;

	//Returns true if a request was made for thread to finish soon. 
	//This may mean that CThread::~CThread() is waiting on the thread to 
	//finish.
	bool GetShouldFinish() const;

	//Sets whether it is requested that thread finish soon.
	void SetShouldFinish( bool ShouldFinish );

	//Get the thread priority.
	int GetPriority() const;

	//Set the thread priority.
	void SetPriority( int Priority );

	//Decrement the suspend count, possibly leading to thread resuming.
	void Resume();

	//Increment the suspend count, possibly suspending the thread.
	void Suspend();

	//Yield the current thread.
	static void YieldCurrentThread();

private:
	static unsigned __stdcall StaticThreadProc( void* Argument );

private:
	CThreadFunction*	m_pThreadFunction;
	CCriticalSection	m_StateAccess;
	HANDLE				m_hThread;
	unsigned			m_ThreadID;
	uint32				m_Result;
	EThreadState		m_State;
	bool				m_ShouldFinish : 1;
};

}
}
