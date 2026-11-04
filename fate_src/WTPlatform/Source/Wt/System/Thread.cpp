/*
	Thread.cpp

	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	4/20/2006

*/

#include "WT/stdafx.h"

#include <process.h>
#include <sstream>
#include "WT/System/Thread.h"
#include "WT/System/SystemException.h"

namespace WT
{
namespace System
{

CThread::CThread( CThreadFunction* pThreadFunction, bool Suspended, uint32 StackSizeBytes )
	: m_pThreadFunction( pThreadFunction )
	, m_ShouldFinish( false )
{
	if (pThreadFunction == NULL)
	{
		WT_THROW( CArgumentException, "pThreadFunction" );
	}

	unsigned int InitFlag;
	if (Suspended)
	{
		InitFlag = CREATE_SUSPENDED;
		m_State = KThreadStateSuspended;
	} else {
		InitFlag = 0;
		m_State = KThreadStateRunning;
	}

	m_hThread = reinterpret_cast<HANDLE>(
		::_beginthreadex( NULL, StackSizeBytes, &CThread::StaticThreadProc, this, InitFlag, &m_ThreadID ) );
	if (!m_hThread)
	{
		std::ostringstream o;
		o << "_beginthreadex failure; errno = " << errno;
		WT_THROW( CRuntimeException, o.str() );
	}
}

/*virtual*/ CThread::~CThread()
{
	m_ShouldFinish = true;

	while (m_State == KThreadStateSuspended)
	{
		Resume();
	}

	::WaitForSingleObject( m_hThread, INFINITE );
	::CloseHandle( m_hThread );
}

void CThread::Suspend()
{
	CAutoClassLock<CCriticalSection> Lock( &m_StateAccess );

	if (m_State == KThreadStateExited)
	{
		return;
	}
	
	DWORD const Result = ::SuspendThread( m_hThread );
	if (Result == 0xffffffff)
	{
		WT_THROW( CRuntimeException, "SuspendThread failure" );
	}

	m_State = KThreadStateSuspended;
}

void CThread::Resume()
{
	CAutoClassLock<CCriticalSection> Lock( &m_StateAccess );

	if (m_State == KThreadStateExited)
	{
		return;
	}

	DWORD const Result = ::ResumeThread( m_hThread );
	if (Result == 0xffffffff)
	{
		WT_THROW( CRuntimeException, "ResumeThread failure" );
	}

	if (Result <= 1)
	{
		//The thread is now running. 0 means it was never suspended; 1 means it was 
		//suspended with a count of 1.
		m_State = KThreadStateRunning;
	}

	if (Result == 0)
	{
		WT_THROW( CRuntimeException, "Thread already resumed" ); 
	}
}

/*static*/ unsigned __stdcall CThread::StaticThreadProc( void* Argument )
{
	CThread* const pThread = static_cast<CThread*>( Argument );
	assert( pThread != NULL );

	pThread->m_Result = pThread->m_pThreadFunction->Execute( *pThread );

	CAutoClassLock<CCriticalSection> Lock( &pThread->m_StateAccess );
	pThread->m_State = KThreadStateExited;

	return S_OK;
}

//Get the thread handle.
HANDLE CThread::GetHandle()
{
	return m_hThread;
}

EThreadState CThread::GetState() const
{
	return m_State;
}

bool CThread::GetShouldFinish() const
{
	return m_ShouldFinish;
}

void CThread::SetShouldFinish( bool ShouldFinish )
{
	m_ShouldFinish = ShouldFinish;
}

uint32 CThread::GetResult() const
{
	if (m_State != KThreadStateExited)
	{
		WT_THROW( CRuntimeException, "not finished yet" );
	}

	return m_Result;
}

int CThread::GetPriority() const
{
	int Priority = GetThreadPriority( m_hThread );
	if (Priority == THREAD_PRIORITY_ERROR_RETURN)
	{
		DWORD dwError = ::GetLastError();
		std::ostringstream o;
		o << "GetThreadPriority() failure; GetLastError() == " << dwError;
		WT_THROW( System::CRuntimeException, o.str() );
	}
	return Priority;
}

void CThread::SetPriority( int Priority )
{
	BOOL Result = SetThreadPriority( m_hThread, Priority );
	if (!Result)
	{
		DWORD dwError = ::GetLastError();
		std::ostringstream o;
		o << "SetThreadPriority() failure; GetLastError() == " << dwError;
		WT_THROW( System::CRuntimeException, o.str() );
	}
}

/*static*/ void CThread::YieldCurrentThread()
{
	::Sleep( 0 );
}

}
}
