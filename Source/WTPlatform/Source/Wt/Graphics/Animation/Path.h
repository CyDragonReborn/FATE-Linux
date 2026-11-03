/*
	Path.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved
*/

#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/Math.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/Graphics/Animation/IKeyInterpolator.h"
#include "WT/Graphics/Animation/HermiteInterpolator.h"

namespace WT
{
namespace Graphics
{
namespace Animation
{

//
// A path based on a single interpolator
// TODO: Add properties support
//
template <typename T> class CSimplePath : public System::CObject, public System::Serialization::ISerializable
{
public:
	WT_BEGIN_CLASS( CSimplePath<T>, System::CObject )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable )
	WT_END_CLASS()

	CSimplePath() 
		: m_pInterpolator( NULL )
	{
	}

	CSimplePath( const string& Name, IKeyInterpolator<T>* pInterpolator = NULL )
		: m_Name( Name )
		, m_pInterpolator( pInterpolator )
	{
	}

	virtual ~CSimplePath()
	{
		delete m_pInterpolator;
	}

	//
	// Accessors
	//

	// SimplePath class will delete this pointer on destruction or assignment.
	void SetInterpolator( IKeyInterpolator<T>* pInterpolator )
	{
		delete m_pInterpolator;
		m_pInterpolator = pInterpolator;
	}

	const IKeyInterpolator<T>* GetInterpolator() const {return m_pInterpolator;}

	string const& GetName() const {return m_Name;}
	
	void SetName( string const& Name ) {m_Name = Name;}

	//
	// Public methods
	//

	void Sample( float32 Time, bool Looping, T& Result ) const
	{
		if (m_pInterpolator == NULL)
		{
			Result = 0.0f;
		} else {
			m_pInterpolator->Sample( Time, Looping, Result );
		}
	}

	bool GetTimeExtents( float32& StartTime, float32& EndTime ) const
	{
		if (m_pInterpolator == NULL)
		{
			StartTime = EndTime = 0.0f;
			return false;
		}

		StartTime = m_pInterpolator->GetKeyTime( 0 );
		if (m_pInterpolator->GetKeyCount() > 1)
		{
			EndTime = m_pInterpolator->GetKeyTime( m_pInterpolator->GetKeyCount() - 1 );
		} else {
			EndTime = StartTime;
		}

		return true;
	}

	//
    // IProperty interface
    //
	//TODO: Add properties support
	/*
	WT_CLASS_PROPERTY_START2( SimplePath )
		WT_FUNC_RW_PROPERTY( string, Name )
		WT_FUNC_HANDLE_RW_PROPERTY( IKeyInterpolator<T>, Interpolator )
	WT_CLASS_PROPERTY_END2
	*/

	//
	// ISerializable methods
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pSerialInfo, 
								System::Serialization::CSerializationContext* pContext ) const
	{
		pContext;

		pSerialInfo->AddValue( "Name", m_Name );

		pSerialInfo->AddValue( "bHaveInterpolator", m_pInterpolator != NULL ? true : false );

		if (m_pInterpolator != NULL)
		{
			pSerialInfo->AddContainerBeginWithCount( "Keys", m_pInterpolator->GetKeyCount() );

			for (int32 i = 0; i < static_cast<int32>( m_pInterpolator->GetKeyCount() ); ++i)
			{
				float32 const Time = m_pInterpolator->GetKeyTime( i );
				pSerialInfo->AddValue( "Time", Time );
				T Value;
				m_pInterpolator->Sample( Time, false, Value );
				pSerialInfo->AddValue( "Value", Value );
			}

			pSerialInfo->AddContainerEnd();
		}
	}

	void SetObjectData( System::Serialization::CSerializationInfo* pSerialInfo, 
								System::Serialization::CSerializationContext* pContext )
	{
		pContext;

		SetInterpolator( NULL );

		pSerialInfo->GetValue( "Name", m_Name );

		bool bHaveInterpolator;
		pSerialInfo->GetValue( "bHaveInterpolator", bHaveInterpolator );

		if (bHaveInterpolator)
		{
			m_pInterpolator = new CKeyInterpolator<T, CHermiteKey<T> >();

			int32 const KeyCount = pSerialInfo->GetContainerBeginWithCount( "Keys" );

			for (int32 i = 0; i < KeyCount; ++i)
			{
				float32 Time;
				T Value;
				pSerialInfo->GetValue( "Time", Time );
				pSerialInfo->GetValue( "Value", Value );
				m_pInterpolator->AddKey( Time, Value );
			}

			pSerialInfo->GetContainerEnd();

			m_pInterpolator->SetRecomputeAsAClosedLoop( true );
			m_pInterpolator->Recompute();
		}
	}

private: // data

	string						m_Name;
	IKeyInterpolator<T>*		m_pInterpolator;
};

//
// An object containing scale, rotation, and translation information that can
// be assembled to form a matrix.
//
class PathComponents
{
public:
	PathComponents() 
		: Scale( CVector3f::gm_UnitScale )
		, ApplyScale(false)
	{
	}

	void __fastcall ToMatrix4f( CMatrix4f * Result ) const
    {
		Rotation.ToMatrix(Result);
		Result->RowT = Translation;
		if (ApplyScale)
		{
			Result->PreScale( Scale );
		}
    }

    void __fastcall SetIdentity()
    {
		ApplyScale = false;
		Scale.x = Scale.y = Scale.z = 1.0f;
		Rotation.SetNoRotation();
		Translation.x = Translation.y = Translation.z = 0.0f;
    }

	Math::CVector3f		Scale;
	Math::CQuaternion	Rotation;
	Math::CVector3f		Translation;
	bool				ApplyScale;
};


//
// A path that is based on a scale, a rotation, and a translation interpolator and yields a 
// PathComponents object when sampled.
//
typedef System::SmartPointer<class CPath> HPath;
class CPath : public System::CObject	//TODO: Add properties support
{
public:
	struct Event
	{
		Event( float32 Time, string Data )
			: m_Time( Time )
			, m_Data( Data )
		{
		}

		bool operator<( Event const& E ) const {return m_Time < E.m_Time;}

		float32 m_Time;
		string m_Data;
	};

	WT_CLASS(CPath, System::CObject)

	CPath() : m_pScaleInterpolator( NULL ), m_pRotationInterpolator( NULL ), m_pTranslationInterpolator( NULL )
	{
	}
	virtual ~CPath();

	CPath(const string& Name);
	CPath(const string& Name,
		 IVector3fKeyInterpolator * pScaleInterpolator,
		 IQuatKeyInterpolator * pRotationInterpolator,
		 IVector3fKeyInterpolator * pTranslationInterpolator);


	//
	// Accessors
	//

	// Path class will delete this pointer on destruction or assignment.
	void SetScaleInterpolator(IVector3fKeyInterpolator * pInterpolator)
	{
		delete m_pScaleInterpolator;
		m_pScaleInterpolator = pInterpolator;
	}

	// Path class will delete this pointer on destruction or assignment.
    void SetRotationInterpolator(IQuatKeyInterpolator * pInterpolator)
	{
		delete m_pRotationInterpolator;
		m_pRotationInterpolator = pInterpolator;
	}

	// Path class will delete this pointer on destruction or assignment.
    void SetTranslationInterpolator(IVector3fKeyInterpolator * pInterpolator)
	{
		delete m_pTranslationInterpolator;
		m_pTranslationInterpolator = pInterpolator;
	}

	IVector3fKeyInterpolator * GetScaleInterpolator() const
	{
		return m_pScaleInterpolator;
	}
    IQuatKeyInterpolator * GetRotationInterpolator() const
	{
		return m_pRotationInterpolator;
	}
    IVector3fKeyInterpolator * GetTranslationInterpolator() const
	{
		return m_pTranslationInterpolator;
	}

	string const& GetName() const
	{
		return m_Name;
	}

	void SetName(string const& Name)
	{
		m_Name = Name;
	}

	//
	// Public methods
	//

	void Sample(float32 Time, bool Looping, Math::CMatrix4f& Result) const;
	void Sample(float32 Time, bool Looping, PathComponents& Result) const;

	bool GetTimeExtents(float32& StartEnd, float32& EndTime) const;

	void Optimize(float32 ScaleTolerance, float32 RotationTolerance, float32 TranslationTolerance);

	vector<Event>& GetEvents() {return m_Events;}
	vector<Event> const& GetEvents() const {return m_Events;}

	//
    // IProperty interface
    //
	//TODO: Add properties support
	/*
	WT_CLASS_PROPERTY_START2( cmPath )
		WT_FUNC_RW_PROPERTY( cmString, Name )
		WT_FUNC_HANDLE_RW_PROPERTY( cmIKeyInterpolator<cmVector3f>, ScaleInterpolator )
		WT_FUNC_HANDLE_RW_PROPERTY( cmIKeyInterpolator<cmQuaternion>, RotationInterpolator )
		WT_FUNC_HANDLE_RW_PROPERTY( cmIKeyInterpolator<cmVector3f>, TranslationInterpolator )
	WT_CLASS_PROPERTY_END2
	*/


private: // data

	string						m_Name;
	IVector3fKeyInterpolator*	m_pScaleInterpolator;
	IQuatKeyInterpolator*		m_pRotationInterpolator;
	IVector3fKeyInterpolator*	m_pTranslationInterpolator;

	//For a deserialized CPath, these will be sorted with ascending time.
	vector<Event>				m_Events;
};

} // Animation
} // Graphics
} // WT
