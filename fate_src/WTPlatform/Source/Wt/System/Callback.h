#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Threading.h"
namespace WT
{
namespace System
{

template<class CCallback>
class CEventList
{
public:

	typedef CCallback *CCallbackPtr;

	inline CEventList() : m_Callbacks ( NULL ) , m_Count ( 0 )
	{
	}

	virtual ~CEventList()
	{
		Clear();
	}

	void Add ( const CCallbackPtr Callback )
	{
		CAutoCriticalSection auto_lock( m_CS() );
		CCallbackPtr* NewCallbacks = new CCallbackPtr[m_Count+1];
		for ( int i=0; i< m_Count ; i++ )
			NewCallbacks[i] = m_Callbacks[i];
		NewCallbacks[m_Count++] = Callback;
		delete[] m_Callbacks;
		m_Callbacks = NewCallbacks;
	}

	void Remove ( const CCallbackPtr Callback )
	{
		CAutoCriticalSection auto_lock( m_CS() );
		for ( int i=0; i<m_Count ; i++ )
		{
			if ( m_Callbacks[i] == Callback )
			{
				CCallbackPtr* NewCallbacks = new CCallbackPtr[m_Count-1];
				int k = 0;
				for ( int j=0 ; j<m_Count ; j++ )
				{
					if ( i != j )
						NewCallbacks[k++] = m_Callbacks[j];
				}
				delete[] m_Callbacks;
				m_Callbacks = NewCallbacks;
				m_Count--;
				delete Callback;
				return;
			}
		}
	}

	inline int32 GetCount() const
	{
		return m_Count;
	}

	void Clear ()
	{
		CAutoCriticalSection auto_lock( m_CS() );
		if ( m_Callbacks )
		{
			for ( int i=0 ; i<m_Count ; i++ )
				delete m_Callbacks[i];
			delete[] m_Callbacks;
			m_Callbacks = NULL;
		}
		m_Count = 0;
	}

	inline bool IsEmpty() const
	{
		return m_Count == 0;
	}

	inline CCallbackPtr* GetCallbacks() const
	{
		return m_Callbacks;
	}

	void operator+= ( const CCallbackPtr Callback )
	{
		Add ( Callback );
	}

	void operator-= ( const CCallbackPtr Callback )
	{
		Remove ( Callback );
	}

protected:

	CCallbackPtr* m_Callbacks;
	int32 m_Count;
	mutable WT::System::CCriticalSection m_CS;
};

//------------------------------------------------------------------------------
//no arguments no return
//------------------------------------------------------------------------------

template <class TargetClass>
class CMemberCallback0;

class CStaticCallback0;
extern CStaticCallback0* CreateStaticCallback0 ( void (* StaticMethod) () );

class CEvent0;

class CCallback0
{
public:

	typedef CEvent0 Event;

	virtual void operator()() = 0;

	template <class TargetClass>
	static inline CMemberCallback0<TargetClass >* WTCall
	Create ( TargetClass &Target , void (TargetClass::*MemberMethod) () )
	{
		return new CMemberCallback0<TargetClass> ( Target , MemberMethod );
	}

	static inline CStaticCallback0*  WTCall Create ( void (* StaticMethod) () )
	{
		return CreateStaticCallback0 ( StaticMethod );
	}
	virtual ~CCallback0()
	{
	}
};

template <class TargetClass>
class CMemberCallback0 : public CCallback0
{
public:

	typedef void (TargetClass::* PointerToMemberMethod) ();
	CMemberCallback0 ( TargetClass& ClassInstance , PointerToMemberMethod MemberMethod ) : Method ( MemberMethod ) , pTarget ( &ClassInstance )
	{
	}
	virtual void operator()()
	{
		(pTarget->*Method) ();
	}
	TargetClass* pTarget;
	PointerToMemberMethod Method;
	virtual ~CMemberCallback0()
	{
	}
};

class CStaticCallback0 : public CCallback0
{
public:

	typedef void (* PointerToStaticMethod) ();
	CStaticCallback0 ( PointerToStaticMethod StaticMethod ) : Method ( StaticMethod )
	{
	}
	virtual void operator()()
	{
		(*Method) ();
	}
	PointerToStaticMethod Method;
	virtual ~CStaticCallback0()
	{
	}

};

class CEvent0 : public CEventList< CCallback0 >
{
public:

	inline void operator()()
	{
		CAutoCriticalSection auto_lock( m_CS() );
		for ( int32 i=0 ; i < m_Count ; i++ )
			(*m_Callbacks[i])();
	}
};

//------------------------------------------------------------------------------
//no arguments with return
//------------------------------------------------------------------------------

template < typename RT , class TargetClass >
class CMemberCallback0r;

template < typename RT >
class CStaticCallback0r;

template < typename RT >
class CEvent0r;

template < typename RT >
class CCallback0r
{
public:

	typedef CEvent0r<RT> Event;

	virtual RT operator()() = 0;

	template <class TargetClass>
	static inline CMemberCallback0r< RT , TargetClass >* WTCall
	Create ( TargetClass &Target , RT (TargetClass::*MemberMethod) ( ) )
	{
		return new CMemberCallback0r< RT , TargetClass> ( Target , MemberMethod );
	}

	static inline CStaticCallback0r< RT >*  WTCall Create ( RT (* StaticMethod) () )
	{
		return new CStaticCallback0r< RT >( StaticMethod );
	}
	virtual ~CCallback0r()
	{
	}
};

template <typename RT , class TargetClass>
class CMemberCallback0r : public CCallback0r< RT >
{
public:

	typedef RT (TargetClass::* PointerToMemberMethod) ();
	CMemberCallback0r ( TargetClass& ClassInstance , PointerToMemberMethod MemberMethod ) : Method ( MemberMethod ) , pTarget ( &ClassInstance )
	{
	}
	virtual RT operator()()
	{
		return (pTarget->*Method) ();
	}
	virtual ~CMemberCallback0r()
	{
	}
	TargetClass* pTarget;
	PointerToMemberMethod Method;
};

template <typename RT>
class CStaticCallback0r : public CCallback0r< RT >
{
public:

	typedef RT (* PointerToStaticMethod) ();
	CStaticCallback0r ( PointerToStaticMethod StaticMethod ) : Method ( StaticMethod )
	{
	}
	virtual RT operator()()
	{
		return (*Method) ();
	}
	virtual ~CStaticCallback0r()
	{
	}
	PointerToStaticMethod Method;
};

template <typename RT>
class CEvent0r : public CEventList< CCallback0r< RT > >
{
public:

	inline RT operator()()
	{
		CAutoCriticalSection auto_lock( m_CS() );
		RT Result((RT())); //initialized both classes and primitive types
		for ( int32 i=0 ; i < m_Count ; i++ )
			Result = (*m_Callbacks[i])();
		return Result;
	}
};



//------------------------------------------------------------------------------
//one argument no return
//------------------------------------------------------------------------------

template <typename P1 , class TargetClass>
class CMemberCallback1;

template <typename P1 >
class CStaticCallback1;

template <typename P1 >
class CEvent1;

template <typename P1 >
class CCallback1
{
public:

	typedef CEvent1<P1> Event;

	virtual void operator()( P1 ) = 0;

	template <class TargetClass>
	static inline CMemberCallback1< P1 , TargetClass >* WTCall
	Create ( TargetClass &Target , void (TargetClass::*MemberMethod) ( P1 ) )
	{
		return new CMemberCallback1< P1 , TargetClass> ( Target , MemberMethod );
	}

	static inline CStaticCallback1< P1 >*  WTCall Create ( void (WTCall * StaticMethod) ( P1 ) )
	{
		return new CStaticCallback1< P1 >( StaticMethod );
	}
	virtual ~CCallback1()
	{
	}
};

template <typename P1 , class TargetClass>
class CMemberCallback1 : public CCallback1< P1 >
{
public:

	typedef void (TargetClass::* PointerToMemberMethod) ( P1 );
	CMemberCallback1 ( TargetClass& ClassInstance , PointerToMemberMethod MemberMethod ) : Method ( MemberMethod ) , pTarget ( &ClassInstance )
	{
	}
	virtual void operator()( P1 Param1 )
	{
		(pTarget->*Method) ( Param1 );
	}
	virtual ~CMemberCallback1()
	{
	}
	TargetClass* pTarget;
	PointerToMemberMethod Method;
};

template <typename P1 >
class CStaticCallback1 : public CCallback1< P1 >
{
public:

	typedef void (WTCall * PointerToStaticMethod) ( P1 );
	CStaticCallback1 ( PointerToStaticMethod StaticMethod ) : Method ( StaticMethod )
	{
	}
	virtual void operator()( P1 Param1 )
	{
		(*Method) ( Param1 );
	}
	virtual ~CStaticCallback1()
	{
	}
	PointerToStaticMethod Method;
};

template <typename P1 >
class CEvent1 : public CEventList< CCallback1< P1 > >
{
public:

	inline void operator()( P1 Param1 )
	{
		CAutoCriticalSection auto_lock( m_CS() );
		for ( int32 i=0 ; i < m_Count ; i++ )
			(*m_Callbacks[i])( Param1 );
	}
};

//------------------------------------------------------------------------------
//one arguments with return
//------------------------------------------------------------------------------

template <typename P1 , typename RT , class TargetClass>
class CMemberCallback1r;

template <typename P1 , typename RT >
class CStaticCallback1r;

template <typename P1 , typename RT >
class CEvent1r;

template <typename P1 , typename RT >
class CCallback1r
{
public:

	typedef CEvent1r<P1 , RT> Event;

	virtual RT operator()( P1 ) = 0;

	template <class TargetClass>
	static inline CMemberCallback1r< P1 , RT , TargetClass >* WTCall
	Create ( TargetClass &Target , RT (TargetClass::*MemberMethod) ( P1 ) )
	{
		return new CMemberCallback1r< P1 , RT , TargetClass> ( Target , MemberMethod );
	}
	static inline CStaticCallback1r< P1 , RT >*  WTCall Create ( RT (* StaticMethod) ( P1 ) )
	{
		return new CStaticCallback1r< P1 , RT >( StaticMethod );
	}
	virtual ~CCallback1r()
	{
	}
};

template <typename P1 , typename RT , class TargetClass>
class CMemberCallback1r : public CCallback1r< P1 , RT >
{
public:

	typedef RT (TargetClass::* PointerToMemberMethod) ( P1 );
	CMemberCallback1r ( TargetClass& ClassInstance , PointerToMemberMethod MemberMethod ) : Method ( MemberMethod ) , pTarget ( &ClassInstance )
	{
	}
	virtual RT operator()( P1 Param1 )
	{
		return (pTarget->*Method) ( Param1 );
	}
	virtual ~CMemberCallback1r()
	{
	}
	TargetClass* pTarget;
	PointerToMemberMethod Method;
};

template <typename P1 , typename RT>
class CStaticCallback1r : public CCallback1r< P1 , RT >
{
public:

	typedef RT (* PointerToStaticMethod) ( P1 );
	CStaticCallback1r ( PointerToStaticMethod StaticMethod ) : Method ( StaticMethod )
	{
	}
	virtual RT operator()( P1 Param1 )
	{
		return (*Method) ( Param1 );
	}
	virtual ~CStaticCallback1r()
	{
	}
	PointerToStaticMethod Method;
};

template <typename P1 , typename RT>
class CEvent1r : public CEventList< CCallback1r< P1 , RT > >
{
public:

	inline RT operator()( P1 Param1 )
	{
		RT Result((RT())); //initialized both classes and primitive types
		CAutoCriticalSection auto_lock( m_CS() );
		for ( int32 i=0 ; i < m_Count ; i++ )
			Result = (*m_Callbacks[i])( Param1 );
		return Result;
	}
};

//------------------------------------------------------------------------------
//two arguments no return
//------------------------------------------------------------------------------

template <typename P1 , typename P2 , class TargetClass>
class CMemberCallback2;

template <typename P1 , typename P2 >
class CStaticCallback2;

template <typename P1, typename P2 >
class CEvent2;

template <typename P1, typename P2 >
class CCallback2
{
public:

	typedef CEvent2<P1 , P2 > Event;

	virtual void operator()( P1 , P2 ) = 0;

	template <class TargetClass>
	static inline CMemberCallback2< P1 , P2 , TargetClass >* WTCall
	Create ( TargetClass &Target , void (TargetClass::*MemberMethod) ( P1 , P2 ) )
	{
		return new CMemberCallback2< P1 , P2 , TargetClass> ( Target , MemberMethod );
	}
	static inline CStaticCallback2< P1 , P2 >* 	WTCall Create ( void (* StaticMethod) ( P1 , P2 ) )
	{
		return new CStaticCallback2< P1, P2 >( StaticMethod );
	}
	virtual ~CCallback2()
	{
	}
};

template <typename P1 , typename P2 , class TargetClass>
class CMemberCallback2 : public CCallback2<P1,P2>
{
public:

	typedef void (TargetClass::* PointerToMemberMethod) ( P1 , P2 );
	CMemberCallback2 ( TargetClass& ClassInstance , PointerToMemberMethod MemberMethod ) : Method ( MemberMethod ) , pTarget ( &ClassInstance )
	{
	}
	virtual void operator()( P1 Param1 , P2 Param2 )
	{
		(pTarget->*Method) ( Param1 , Param2 );
	}
	virtual ~CMemberCallback2()
	{
	}
	TargetClass* pTarget;
	PointerToMemberMethod Method;
};

template <typename P1, typename P2 >
class CStaticCallback2 : public CCallback2<P1,P2>
{
public:

	typedef void (* PointerToStaticMethod) ( P1 , P2 );
	CStaticCallback2 ( PointerToStaticMethod StaticMethod ) : Method ( StaticMethod )
	{
	}
	virtual void operator()( P1 Param1 , P2 Param2 )
	{
		(*Method) ( Param1 , Param2 );
	}
	virtual ~CStaticCallback2()
	{
	}
	PointerToStaticMethod Method;
};

template <typename P1 , typename P2 >
class CEvent2 : public CEventList< CCallback2< P1 , P2 > >
{
public:

	inline void operator()( P1 Param1 , P2 Param2 )
	{
		CAutoCriticalSection auto_lock( m_CS() );
		for ( int32 i=0 ; i < m_Count ; i++ )
			(*m_Callbacks[i])( Param1 , Param2 );
	}
};

//------------------------------------------------------------------------------
//two arguments with return
//------------------------------------------------------------------------------

template <typename P1 , typename P2 , typename RT , class TargetClass>
class CMemberCallback2r;

template <typename P1 , typename P2 , typename RT >
class CStaticCallback2r;

template <typename P1, typename P2 , typename RT >
class CEvent2r;

template <typename P1, typename P2 , typename RT >
class CCallback2r
{
public:

	typedef CEvent2r<P1 , P2 , RT> Event;

	virtual RT operator()( P1 , P2 ) = 0;

	template <class TargetClass>
	static inline CMemberCallback2r< P1 , P2 , RT , TargetClass >* WTCall
	Create ( TargetClass &Target , RT (TargetClass::*MemberMethod) ( P1 , P2 ) )
	{
		return new CMemberCallback2r< P1 , P2 , RT , TargetClass> ( Target , MemberMethod );
	}
	static inline CStaticCallback2r< P1 , P2 , RT >* WTCall
	Create ( RT (* StaticMethod) ( P1 , P2 ) )
	{
		return new CStaticCallback2r< P1, P2, RT >( StaticMethod );
	}
	virtual ~CCallback2r()
	{
	}
};

template <typename P1 , typename P2 , typename RT , class TargetClass>
class CMemberCallback2r : public CCallback2r<P1,P2,RT>
{
public:

	typedef RT (TargetClass::* PointerToMemberMethod) ( P1 , P2 );
	CMemberCallback2r ( TargetClass& ClassInstance , PointerToMemberMethod MemberMethod ) : Method ( MemberMethod ) , pTarget ( &ClassInstance )
	{
	}
	virtual RT operator()( P1 Param1 , P2 Param2 )
	{
		return (pTarget->*Method) ( Param1 , Param2 );
	}
	virtual ~CMemberCallback2r()
	{
	}
	TargetClass* pTarget;
	PointerToMemberMethod Method;
};

template <typename P1, typename P2 , typename RT>
class CStaticCallback2r : public CCallback2r<P1,P2,RT>
{
public:

	typedef RT (* PointerToStaticMethod) ( P1 , P2 );
	CStaticCallback2r ( PointerToStaticMethod StaticMethod ) : Method ( StaticMethod )
	{
	}
	virtual RT operator()( P1 Param1 , P2 Param2 )
	{
		return (*Method) ( Param1 , Param2 );
	}
	virtual ~CStaticCallback2r()
	{
	}
	PointerToStaticMethod Method;
};

template <typename P1 , typename P2 , typename RT>
class CEvent2r : public CEventList< CCallback2r< P1 , P2 , RT > >
{
public:

	inline RT operator()( P1 Param1 , P2 Param2 )
	{
		RT Result((RT())); //initialized both classes and primitive types
		CAutoCriticalSection auto_lock( m_CS() );
		for ( int32 i=0 ; i < m_Count ; i++ )
			Result = (*m_Callbacks[i])( Param1 , Param2 );
		return Result;
	}
};

//------------------------------------------------------------------------------
//three arguments no return
//------------------------------------------------------------------------------

template <typename P1 , typename P2 , typename P3 , class TargetClass>
class CMemberCallback3;

template <typename P1 , typename P2 , typename P3 >
class CStaticCallback3;

template <typename P1 , typename P2 , typename P3 >
class CEvent3;

template <typename P1, typename P2 , typename P3 >
class CCallback3
{
public:

	typedef CEvent3<P1 , P2 , P3> Event;

	virtual void operator()( P1 , P2 , P3 ) = 0;

	template <class TargetClass>
	static inline CMemberCallback3< P1 , P2 , P3 , TargetClass >* WTCall
	Create ( TargetClass &Target , void (TargetClass::*MemberMethod) ( P1 , P2 , P3 ) )
	{
		return new CMemberCallback3< P1 , P2 , P3 , TargetClass> ( Target , MemberMethod );
	}
	static inline CStaticCallback3< P1 , P2 , P3 >*  WTCall Create ( void (* StaticMethod) ( P1 , P2 , P3 ) )
	{
		return new CStaticCallback3< P1, P2 , P3 >( StaticMethod );
	}
	virtual ~CCallback3()
	{
	}
};

template <typename P1 , typename P2 , typename P3 , class TargetClass>
class CMemberCallback3 : public CCallback3< P1 , P2 , P3 >
{
public:

	typedef void (TargetClass::* PointerToMemberMethod) ( P1 , P2 , P3 );
	CMemberCallback3 ( TargetClass& ClassInstance , PointerToMemberMethod MemberMethod ) : Method ( MemberMethod ) , pTarget ( &ClassInstance )
	{
	}
	virtual void operator()( P1 Param1 , P2 Param2 , P3 Param3 )
	{
		(pTarget->*Method) ( Param1 , Param2 , Param3 );
	}
	virtual ~CMemberCallback3()
	{
	}
	TargetClass* pTarget;
	PointerToMemberMethod Method;
};

template <typename P1, typename P2 , typename P3 >
class CStaticCallback3 : public CCallback3< P1 , P2 , P3 >
{
public:

	typedef void (* PointerToStaticMethod) ( P1 , P2 , P3 );
	CStaticCallback3 ( PointerToStaticMethod StaticMethod ) : Method ( StaticMethod )
	{
	}
	virtual void operator()( P1 Param1 , P2 Param2 , P3 Param3 )
	{
		(*Method) ( Param1 , Param2 , Param3 );
	}
	virtual ~CStaticCallback3()
	{
	}
	PointerToStaticMethod Method;
};

template <typename P1 , typename P2 , typename P3 >
class CEvent3 : public CEventList< CCallback3< P1 , P2 , P3 > >
{
public:

	inline void operator()( P1 Param1 , P2 Param2 , P3 Param3 )
	{
		CAutoCriticalSection auto_lock( m_CS() );
		for ( int32 i=0 ; i < m_Count ; i++ )
			(*m_Callbacks[i])( Param1 , Param2 , Param3 );
	}
};

//------------------------------------------------------------------------------
//three arguments with return
//------------------------------------------------------------------------------

template <typename P1 , typename P2 , typename P3 , typename RT , class TargetClass>
class CMemberCallback3r;

template <typename P1 , typename P2 , typename P3 , typename RT >
class CStaticCallback3r;

template <typename P1, typename P2 , typename P3 , typename RT >
class CEvent3r;

template <typename P1, typename P2 , typename P3 , typename RT >
class CCallback3r
{
public:

	typedef CEvent3r<P1 , P2 , P3 , RT> Event;

	virtual RT operator()( P1 , P2 , P3 ) = 0;

	template <class TargetClass>
	static inline CMemberCallback3r< P1 , P2 , P3 , RT , TargetClass >* WTCall
	Create ( TargetClass &Target , RT (TargetClass::*MemberMethod) ( P1 , P2 , P3 ) )
	{
		return new CMemberCallback3r< P1 , P2 , P3 , RT , TargetClass> ( Target , MemberMethod );
	}
	static inline CStaticCallback3r< P1 , P2 , P3 , RT >* WTCall Create ( RT (* StaticMethod) ( P1 , P2 , P3 ) )
	{
		return new CStaticCallback3r< P1 , P2 , P3 , RT >( StaticMethod );
	}
	virtual ~CCallback3r()
	{
	}
};

template <typename P1 , typename P2 , typename P3 , typename RT , class TargetClass>
class CMemberCallback3r : public CCallback3r< P1 , P2 , P3 , RT >
{
public:

	typedef RT (TargetClass::* PointerToMemberMethod) ( P1 , P2 , P3 );
	CMemberCallback3r ( TargetClass& ClassInstance , PointerToMemberMethod MemberMethod ) : Method ( MemberMethod ) , pTarget ( &ClassInstance )
	{
	}
	virtual RT operator()( P1 Param1 , P2 Param2 , P3 Param3 )
	{
		return (pTarget->*Method) ( Param1 , Param2 , Param3 );
	}
	virtual ~CMemberCallback3r()
	{
	}
	TargetClass* pTarget;
	PointerToMemberMethod Method;
};

template <typename P1, typename P2 , typename P3 , typename RT>
class CStaticCallback3r : public CCallback3r< P1 , P2 , P3 , RT >
{
public:

	typedef RT (* PointerToStaticMethod) ( P1 , P2 , P3 );
	CStaticCallback3r ( PointerToStaticMethod StaticMethod ) : Method ( StaticMethod )
	{
	}
	virtual RT operator()( P1 Param1 , P2 Param2 , P3 Param3 )
	{
		return (*Method) ( Param1 , Param2 , Param3 );
	}
	virtual ~CStaticCallback3r()
	{
	}
	PointerToStaticMethod Method;
};

template <typename P1 , typename P2 , typename P3 , typename RT>
class CEvent3r : public CEventList< CCallback3r< P1 , P2 , P3 , RT > >
{
public:

	inline RT operator()( P1 Param1 , P2 Param2 , P3 Param3 )
	{
		RT Result((RT())); //initialized both classes and primitive types
		CAutoCriticalSection auto_lock( m_CS() );
		for ( int32 i=0 ; i < m_Count ; i++ )
			Result = (*m_Callbacks[i])( Param1 , Param2 , Param3 );
		return Result;
	}
};

//------------------------------------------------------------------------------
//four arguments no return
//------------------------------------------------------------------------------

template <typename P1 , typename P2 , typename P3 , typename P4 , class TargetClass>
class CMemberCallback4;

template <typename P1 , typename P2 , typename P3 , typename P4 >
class CStaticCallback4;

template <typename P1 , typename P2 , typename P3 , typename P4 >
class CEvent4;

template <typename P1, typename P2 , typename P3 , typename P4 >
class CCallback4
{
public:

	typedef CEvent4<P1 , P2 , P3 , P4 > Event;

	virtual void operator()( P1 , P2 , P3 , P4 ) = 0;

	template <class TargetClass>
	static inline CMemberCallback4< P1 , P2 , P3 , P4 , TargetClass >* WTCall
	Create ( TargetClass &Target , void (TargetClass::*MemberMethod) ( P1 , P2 , P3 , P4 ) )
	{
		return new CMemberCallback4< P1 , P2 , P3 , P4 , TargetClass> ( Target , MemberMethod );
	}
	static inline CStaticCallback4< P1 , P2 , P3 ,P4 >* WTCall Create ( void (* StaticMethod) ( P1 , P2 , P3 , P4 ) )
	{
		return new CStaticCallback4< P1, P2 , P3 , P4 >( StaticMethod );
	}
	virtual ~CCallback4()
	{
	}
};

template <typename P1 , typename P2 , typename P3 , typename P4 , class TargetClass>
class CMemberCallback4 : public CCallback4< P1 , P2 , P3 , P4 >
{
public:

	typedef void (TargetClass::* PointerToMemberMethod) ( P1 , P2 , P3 , P4 );
	CMemberCallback4 ( TargetClass& ClassInstance , PointerToMemberMethod MemberMethod ) : Method ( MemberMethod ) , pTarget ( &ClassInstance )
	{
	}
	virtual void operator()( P1 Param1 , P2 Param2 , P3 Param3 , P4 Param4 )
	{
		(pTarget->*Method) ( Param1 , Param2 , Param3 , Param4 );
	}
	virtual ~CMemberCallback4()
	{
	}
	TargetClass* pTarget;
	PointerToMemberMethod Method;
};

template <typename P1, typename P2 , typename P3 , typename P4 >
class CStaticCallback4 : public CCallback4< P1 , P2 , P3 , P4 >
{
public:

	typedef void (* PointerToStaticMethod) ( P1 , P2 , P3 , P4);
	CStaticCallback4 ( PointerToStaticMethod StaticMethod ) : Method ( StaticMethod )
	{
	}
	virtual void operator()( P1 Param1 , P2 Param2 , P3 Param3 , P4 Param4 )
	{
		(*Method) ( Param1 , Param2 , Param3 , Param4 );
	}
	virtual ~CStaticCallback4()
	{
	}
	PointerToStaticMethod Method;
};

template <typename P1 , typename P2 , typename P3 , typename P4 >
class CEvent4 : public CEventList< CCallback4< P1 , P2 , P3 , P4 > >
{
public:

	inline void operator()( P1 Param1 , P2 Param2 , P3 Param3 , P4 Param4 )
	{
		CAutoCriticalSection auto_lock( m_CS() );
		for ( int32 i=0 ; i < m_Count ; i++ )
			(*m_Callbacks[i])( Param1 , Param2 , Param3 , Param4 );
	}
};

//------------------------------------------------------------------------------
//four arguments with return
//------------------------------------------------------------------------------

template <typename P1 , typename P2 , typename P3 , typename P4 , typename RT , class TargetClass>
class CMemberCallback4r;

template <typename P1 , typename P2 , typename P3 , typename P4 , typename RT >
class CStaticCallback4r;

template <typename P1, typename P2 , typename P3 , typename P4 , typename RT >
class CEvent4r;

template <typename P1, typename P2 , typename P3 , typename P4 , typename RT >
class CCallback4r
{
public:

	typedef CEvent4r<P1 , P2 , P3 , P4 , RT> Event;

	virtual RT operator()( P1 , P2 , P3 , P4 ) = 0;

	template <class TargetClass>
		static inline CMemberCallback4r< P1 , P2 , P3 , P4 , RT , TargetClass >* WTCall
		Create ( TargetClass &Target , RT (TargetClass::*MemberMethod) ( P1 , P2 , P3 , P4 ) )
	{
		return new CMemberCallback4r< P1 , P2 , P3 , P4 , RT , TargetClass> ( Target , MemberMethod );
	}
	static inline CStaticCallback4r< P1 , P2 , P3 , P4 , RT >*  WTCall Create ( RT (* StaticMethod) ( P1 , P2 , P3 , P4 ) )
	{
		return new CStaticCallback4r< P1 , P2 , P3 , P4 , RT >( StaticMethod );
	}
	virtual ~CCallback4r()
	{
	}
};

template <typename P1 , typename P2 , typename P3 , typename P4 , typename RT , class TargetClass>
class CMemberCallback4r : public CCallback4r< P1 , P2 , P3 , P4 , RT >
{
public:

	typedef RT (TargetClass::* PointerToMemberMethod) ( P1 , P2 , P3 , P4 );
	CMemberCallback4r ( TargetClass& ClassInstance , PointerToMemberMethod MemberMethod ) : Method ( MemberMethod ) , pTarget ( &ClassInstance )
	{
	}
	virtual RT operator()( P1 Param1 , P2 Param2 , P3 Param3 , P4 Param4 )
	{
		return (pTarget->*Method) ( Param1 , Param2 , Param3 , Param4 );
	}
	virtual ~CMemberCallback4r()
	{
	}
	TargetClass* pTarget;
	PointerToMemberMethod Method;
};

template <typename P1, typename P2 , typename P3 , typename P4 , typename RT>
class CStaticCallback4r : public CCallback4r< P1 , P2 , P3 , P4 , RT >
{
public:

	typedef RT (* PointerToStaticMethod) ( P1 , P2 , P3 , P4 );
	CStaticCallback4r ( PointerToStaticMethod StaticMethod ) : Method ( StaticMethod )
	{
	}
	virtual RT operator()( P1 Param1 , P2 Param2 , P3 Param3 , P4 Param4 )
	{
		return (*Method) ( Param1 , Param2 , Param3 , Param4 );
	}
	virtual ~CStaticCallback4r()
	{
	}
	PointerToStaticMethod Method;
};

template <typename P1 , typename P2 , typename P3 , typename P4 , typename RT>
class CEvent4r : public CEventList< CCallback4r< P1 , P2 , P3 , P4 , RT > >
{
public:

	inline RT operator()( P1 Param1 , P2 Param2 , P3 Param3 , P4 Param4 )
	{
		RT Result((RT())); //initialized both classes and primitive types
		CAutoCriticalSection auto_lock( m_CS() );
		for ( int32 i=0 ; i < m_Count ; i++ )
			Result = (*m_Callbacks[i])( Param1 , Param2 , Param3 , Param4 );
		return Result;
	}
};

}
}