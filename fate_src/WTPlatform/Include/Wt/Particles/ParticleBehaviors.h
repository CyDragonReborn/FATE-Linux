//
// ParticleBehaviors.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// Overview:
//	This file contains a number of actual behavior classes that derive from the CParticleBehavior abstract base
//	class.
//

#pragma once

#include "WT/Particles/ParticleBehavior.h"
#include "WT/Image/Image.h"
#include "WT/Math/Math.h"
#include "WT/System/Serialization/ISerializable.h"

namespace WT
{
using namespace Math;
using namespace Graphics;

namespace Particles
{

class Math::CShape3d;
class CParticleSystemData;

// -------------------------------------------------------------------------------------
// Emit particle behavior - emit particles from a given source, size, color, age and
// direction.
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CEmitParticleBehavior> HEmitParticleBehavior;
class CEmitParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CEmitParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()

	CEmitParticleBehavior();
	~CEmitParticleBehavior();

	void Init ( int32 ID, CShape3d* pPosition, CShape3d* pVelocity, CShape3d* pSize,
	            CShape3d* pColor, uint8 MinAlpha, uint8 MaxAlpha,
			    CShape3d* pRotation,
				uint32 MinFrame, uint32 MaxFrame,
				float32 ParticlesPerSecond,
				float32 Age, float32 AgeStdDv = 1.0f );

	void SetPosition ( CShape3d* pPosition );
	CShape3d* GetPosition( ) const;

	void SetVelocity ( CShape3d* pVelocity );
	CShape3d* GetVelocity( ) const;

	void SetSize ( CShape3d* pSize );
	CShape3d* GetSize( ) const;

	void SetColor ( CShape3d* pColor );
	CShape3d* GetColor( ) const;

	void SetMinAlpha ( uint8 MinAlpha );
	uint8 GetMinAlpha ( ) const;

	void SetMaxAlpha ( uint8 MaxAlpha );
	uint8 GetMaxAlpha ( ) const;

	void SetRotation ( CShape3d* pRotation );
	CShape3d* GetRotation( ) const;

	void SetMinFrame ( uint32 MinFrame );
	uint32 GetMinFrame ( ) const;

	void SetMaxFrame ( uint32 MaxFrame );
	uint32 GetMaxFrame ( ) const;

	void SetParticlesPerSecond ( float32 ParticlesPerSecond );
	float32 GetParticlesPerSecond ( ) const;

	void SetAge ( float32 Age );
	float32 GetAge ( ) const;

	void SetAgeStdDv ( float32 AgeStdDv );
	float32 GetAgeStdDv ( ) const;

	void Update ( CParticleSystemData* pData ,  float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );


private:	// data

	HShape3d 	m_hPosition;
	HShape3d 	m_hVelocity;
	HShape3d 	m_hSize;
	HShape3d 	m_hColor;
	HShape3d 	m_hRotation;
	uint8		m_MinAlpha;
	uint8		m_MaxAlpha;
	uint32		m_MinFrame;
	uint32		m_MaxFrame;
	float32		m_ParticlesPerSecond;
	float32		m_Age;
	float32		m_AgeStdDv;
};


// -------------------------------------------------------------------------------------
// Gravity particle behavior - apply constant force on a certain direction to the
// particles.
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CGravityParticleBehavior> HGravityParticleBehavior;
class CGravityParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CGravityParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()
	
	CGravityParticleBehavior() {}
	~CGravityParticleBehavior() {}

	void Init( int32 ID, const CVector3f& Direction )
	{
		m_ID = ID;
		m_Direction = Direction;
	}

	void SetDirection( CVector3f Direction )
	{
		m_Direction = Direction;
	}
	CVector3f GetDirection() const
	{
		return m_Direction;
	}

	void Update( CParticleSystemData* pData , float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private: // data

	CVector3f m_Direction;
};


// -------------------------------------------------------------------------------------
// Kill old particle behavior - kill particles once they have reached a certain age
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CKillOldParticleBehavior> HKillOldParticleBehavior;
class CKillOldParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CKillOldParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()

	CKillOldParticleBehavior() : m_Age(1.0f)
	{
	}
	~CKillOldParticleBehavior() {}

	void Init( int32 ID, float32 Age )
	{
		m_ID = ID;
		m_Age = Age;
	}

	void SetAge( float32 Age )
	{
		m_Age = Age;
	}
	float32 GetAge() const
	{
		return m_Age;
	}

	void Update( CParticleSystemData* pData , float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private: // data

	float32	m_Age;
};


// -------------------------------------------------------------------------------------
// Kill Inside particle behavior - kill particles once they reached inside a bounding
// object
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CKillInsideParticleBehavior> HKillInsideParticleBehavior;
class CKillInsideParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CKillInsideParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()
	
	CKillInsideParticleBehavior ( )
	{
	}

	~CKillInsideParticleBehavior ( )
	{
	}

	void Init( int32 ID, CShape3d* pObject )
	{
		m_ID = ID;
		m_hObject = pObject;
	}

	void SetObject( CShape3d* pObject )
	{
		m_hObject = pObject;
	}

	CShape3d* GetObject ( ) const
	{
		return m_hObject;
	}

	void Update( CParticleSystemData* pData , float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private: // data

	HShape3d m_hObject;
};


// -------------------------------------------------------------------------------------
// Move particle behavior - move the particles according to their velocity directions
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CMoveParticleBehavior> HMoveParticleBehavior;
class CMoveParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CMoveParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()

	CMoveParticleBehavior ( ) {}
	~CMoveParticleBehavior ( ) {}

	void Update( CParticleSystemData* pData , float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );
};


// -------------------------------------------------------------------------------------
// Target size particle behavior - resize all particles toward a specific size
// with a specific rate of change
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CTargetSizeParticleBehavior> HTargetSizeParticleBehavior;
class CTargetSizeParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CTargetSizeParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()
	
 	CTargetSizeParticleBehavior ( )
	{
	}
	~CTargetSizeParticleBehavior ( )
	{
	}

	void Init( int32 ID, const CVector3f& Size , const CVector3f& Scale )
	{
		m_ID = ID;
		m_Size = Size;
		m_Scale = Scale;
	}

	void SetSize( CVector3f Size )
	{
		m_Size = Size;
	}
	CVector3f GetSize() const
	{
		return m_Size;
	}

	void SetScale( CVector3f Scale )
	{
		m_Scale = Scale;
	}
	CVector3f GetScale() const
	{
		return m_Scale;
	}

	void Update( CParticleSystemData* pData , float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private: // data

	CVector3f m_Size;
	CVector3f m_Scale;
};


// -------------------------------------------------------------------------------------
// Scale particle behavior - resize all particles by the given scale (units per frame
// time.
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CScaleParticleBehavior> HScaleParticleBehavior;
class CScaleParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CScaleParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()
	
 	CScaleParticleBehavior() {}
	~CScaleParticleBehavior() {}

	void Init( int32 ID, const CVector3f& Scale )
	{
		m_ID = ID;
		m_Scale = Scale;
	}

	void SetScale( CVector3f Scale )
	{
		m_Scale = Scale;
	}
	CVector3f GetScale() const
	{
		return m_Scale;
	}

	void Update( CParticleSystemData* pData , float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private: // data

	CVector3f m_Scale;
};


// -------------------------------------------------------------------------------------
// Rotate particle behavior - rotate all particles by the given angle (radians per frame
// time)
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CRotateParticleBehavior> HRotateParticleBehavior;
class CRotateParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CRotateParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()
	
 	CRotateParticleBehavior() {}
	~CRotateParticleBehavior() {}

	void Init( int32 ID, const CVector3f& Rotation )
	{
		m_ID = ID;
		m_Rotation = Rotation;
	}

	void SetRotation( CVector3f Rotation )
	{
		m_Rotation = Rotation;
	}
	CVector3f GetRotation( ) const
	{
		return m_Rotation;
	}

	void Update( CParticleSystemData* pData, float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private: // data

	CVector3f m_Rotation;
};


// -------------------------------------------------------------------------------------
// Fade particle behavior - Fade particles by given amount (units per seconds)
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CFadeParticleBehavior> HFadeParticleBehavior;
class CFadeParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CFadeParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()
	
	CFadeParticleBehavior() {}
	~CFadeParticleBehavior() {}

	void Init( int32 ID, float32 Fade, bool Clamp = false )
	{
		m_ID = ID;
		m_Fade = Fade;
		m_Clamp = Clamp;
	}

	void SetFade( float32 Fade )
	{
		m_Fade = Fade;
	}
	float32 GetFade( ) const
	{
		return m_Fade;
	}

	void Update( CParticleSystemData* pData , float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private: // data

	float32 m_Fade;
	bool m_Clamp;
};


// -------------------------------------------------------------------------------------
// Animate particle behavior - animate particles based on flipbook texture animation,
// the frame for each particle is coming from the texture associated with the particle
// system itself which hold multiple frames with the same width and height stacked
// right to each other, left to right, top to bottom
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CAnimateParticleBehavior> HAnimateParticleBehavior;
class CAnimateParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CAnimateParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()
	
	CAnimateParticleBehavior() : m_SecondsPerFrame(1.0f) , m_Loop (true) , m_StartFrame(0) , m_FrameCount(0)
	{
	}
	~CAnimateParticleBehavior()
	{
	}

	void Init( int32 ID, float32 SecondsPerFrame, uint32 StartFrame, uint32 FrameCount, bool Loop )
	{
		m_ID = ID;
		m_SecondsPerFrame = SecondsPerFrame;
		m_Loop = Loop;
		m_StartFrame = StartFrame;
		m_FrameCount = FrameCount;
	}

	void SetSecondsPerFrame( float32 SecondsPerFrame )
	{
		m_SecondsPerFrame = SecondsPerFrame;
	}
	float32 GetSecondsPerFrame( ) const
	{
		return m_SecondsPerFrame;
	}

	void SetStartFrame( uint32 StartFrame )
	{
		m_StartFrame = StartFrame;
	}
	uint32 GetStartFrame( ) const
	{
		return m_StartFrame;
	}

	void SetFrameCount( uint32 FrameCount )
	{
		m_FrameCount = FrameCount;
	}
	uint32 GetFrameCount( ) const
	{
		return m_FrameCount;
	}

	void SetLoop( bool Loop )
	{
		m_Loop = Loop;
	}
	bool GetLoop( ) const
	{
		return m_Loop;
	}

	void Update( CParticleSystemData* pData , float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private: // data

	float32	m_SecondsPerFrame;
	bool	m_Loop;
	uint32	m_StartFrame;
	uint32	m_FrameCount;
};


// -------------------------------------------------------------------------------------
// Random Move particle behavior - Randomly move the particles controlled
// by the given bounding object
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CRandomMoveParticleBehavior> HRandomMoveParticleBehavior;
class CRandomMoveParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CRandomMoveParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()
	
	CRandomMoveParticleBehavior()
	{
	}

	~CRandomMoveParticleBehavior()
	{
	}

	void Init( int32 ID, CShape3d* pRandomMove )
	{
		m_ID = ID;

		m_hRandomMove = pRandomMove;
	}

	void SetRandomMove( CShape3d* pRandomMove )
	{
		m_hRandomMove = pRandomMove;
	}

	CShape3d* GetRandomMove() const
	{
		return m_hRandomMove;
	}

	void Update( CParticleSystemData* pData , float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private: // data

	HShape3d 	m_hRandomMove;
};


// -------------------------------------------------------------------------------------
// Bounce particle behavior - resize all particles by the given Bounce (units per frame
// time.
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CBounceParticleBehavior> HBounceParticleBehavior;
class CBounceParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CBounceParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()
	
 	CBounceParticleBehavior()
	{
	}

	~CBounceParticleBehavior()
	{
	}

	void Init( int32 ID, CShape3d* pBounce, float32 Friction, float32 Resilience, float32 CutOff )
	{
		m_ID = ID;
		m_hBounce = pBounce;
		m_Friction = Friction;
		m_Resilience = Resilience;
		m_CutOff = CutOff;
		m_OneMinusFriction = 1 - Friction;
		m_CutoffSqr  = CutOff * CutOff;
	}

	void SetBounce( CShape3d* pBounce )
	{
		m_hBounce = pBounce;
	}

	CShape3d* GetBounce( ) const
	{
		return m_hBounce;
	}

	void SetFriction( float32 Friction )
	{
		m_Friction = Friction;
		m_OneMinusFriction = 1 - Friction;
	}

	float32 GetFriction( ) const
	{
		return m_Friction;
	}

	void SetResilience( float32 Resilience )
	{
		m_Resilience = Resilience;
	}
	float32 GetResilience( ) const
	{
		return m_Resilience;
	}

	void SetCutOff( float32 CutOff )
	{
		m_CutOff = CutOff;
		m_CutoffSqr  = CutOff * CutOff;
	}
	float32 GetCutOff( ) const
	{
		return m_CutOff;
	}

	void Update( CParticleSystemData* pData, float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private:	// data

	HShape3d	m_hBounce;
	float32		m_Friction;
	float32		m_Resilience;
	float32		m_CutOff;
	float32		m_OneMinusFriction;
	float32		m_CutoffSqr;
};


// -------------------------------------------------------------------------------------
// Attract to point particle behavior - attract the particles to a specific point in
// space
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CPointAttractParticleBehavior> HPointAttractParticleBehavior;
class CPointAttractParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CPointAttractParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()
	
 	CPointAttractParticleBehavior() : m_Radius(0), m_Magnitude(0), m_DropOff(0), m_RadiusSqr(0)
	{
	}
	~CPointAttractParticleBehavior()
	{
	}

	void Init( int32 ID, const CVector3f& Point, float32 Radius, float32 Magnitude, float32 DropOff=0 )
	{
		m_ID = ID;
		m_Point = Point;
		m_Radius = Radius;
		m_RadiusSqr = Radius * Radius;
		m_Magnitude = Magnitude;
		m_DropOff = DropOff;
	}

	void SetPoint( const CVector3f& Point )
	{
		m_Point = Point;
	}
	CVector3f GetPoint( ) const
	{
		return m_Point;
	}

	void SetRadius( float32 Radius )
	{
		m_Radius = Radius;
		m_RadiusSqr = Radius * Radius;
	}
	float32 GetRadius( ) const
	{
		return m_Radius;
	}

	void SetMagnitude( float32 Magnitude )
	{
		m_Magnitude = Magnitude;
	}
	float32 GetMagnitude( ) const
	{
		return m_Magnitude;
	}

	void SetDropOff( float32 DropOff )
	{
		m_DropOff = DropOff;
	}
	float32 GetDropOff( ) const
	{
		return m_DropOff;
	}

	void Update( CParticleSystemData* pData , float32 FrameTime );

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private: // data

	CVector3f	m_Point;
	float32		m_Radius;
	float32		m_Magnitude;
	float32		m_DropOff;
	float32		m_RadiusSqr;
};


// -------------------------------------------------------------------------------------
// Image emit particle behavior - emit particles from a source image
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CImageEmitParticleBehavior> HImageEmitParticleBehavior;
class CImageEmitParticleBehavior : public CParticleBehavior, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CImageEmitParticleBehavior, CParticleBehavior )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()

	CImageEmitParticleBehavior() : m_Age( 0.0f ) , m_OneTime(true) , m_Emitted(false)
	{
	}

	virtual ~CImageEmitParticleBehavior()
	{
	}

	void Init ( int32 ID, CImage* pImage, CRect* pPosition, CShape3d* pVelocity,
				CShape3d* pTopLeftVelocity, CShape3d* pTopRightVelocity,
				CShape3d* pBottomLeftVelocity, CShape3d* pBottomRightVelocity,
				CShape3d* pSize,
	            CShape3d* pColor, uint8 MinAlpha, uint8 MaxAlpha,
				CShape3d* pRotation,
				uint32 MinFrame, uint32 MaxFrame,
				bool SingleVelocity,
				float32 Age, float32 AgeStdDv = 1.0f )
	{
		m_ID = ID;
		m_hImage = pImage;
		m_hPosition = pPosition;
		m_hVelocity = pVelocity;
		m_hTopLeftVelocity = pTopLeftVelocity;
		m_hTopRightVelocity = pTopRightVelocity;
		m_hBottomLeftVelocity = pBottomLeftVelocity;
		m_hBottomRightVelocity = pBottomRightVelocity;
		m_hSize = pSize;
		m_hColor = pColor;
		m_hRotation = pRotation;
		m_MinAlpha = MinAlpha;
		m_MaxAlpha = MaxAlpha;
		m_MinFrame = MinFrame;
		m_MaxFrame = MaxFrame;
		m_SingleVelocity = SingleVelocity;
		m_Age = Age;
		m_AgeStdDv = AgeStdDv;
	}

	virtual void Update( CParticleSystemData* pData, float32 FrameTime );

	void SetImage( CImage* pImage )
	{
		m_hImage = pImage;
	}

	CImage* GetImage( ) const
	{
		return m_hImage;
	}

	void SetPosition ( CRect* pPosition )
	{
		m_hPosition = pPosition;
	}

	CRect* GetPosition( ) const
	{
		return m_hPosition;
	}

	void SetVelocity( CShape3d* pVelocity )
	{
		m_hVelocity = pVelocity;
	}

	CShape3d* GetVelocity( ) const
	{
		return m_hVelocity;
	}

	void SetTopLeftVelocity ( CShape3d* pVelocity )
	{
		m_hTopLeftVelocity = pVelocity;
	}

	CShape3d* GetTopLeftVelocity( ) const
	{
		return m_hTopLeftVelocity;
	}

	void SetTopRightVelocity ( CShape3d* pVelocity )
	{
		m_hTopRightVelocity = pVelocity;
	}

	CShape3d* GetTopRightVelocity( ) const
	{
		return m_hTopRightVelocity;
	}

	void SetBottomLeftVelocity( CShape3d* pVelocity )
	{
		m_hBottomLeftVelocity = pVelocity;
	}

	CShape3d* GetBottomLeftVelocity( ) const
	{
		return m_hBottomLeftVelocity;
	}

	void SetBottomRightVelocity( CShape3d* pVelocity )
	{
		m_hBottomRightVelocity = pVelocity;
	}

	CShape3d* GetBottomRightVelocity( ) const
	{
		return m_hBottomRightVelocity;
	}

	void SetSize( CShape3d* pSize )
	{
		m_hSize = pSize;
	}

	CShape3d* GetSize( ) const
	{
		return m_hSize;
	}

	void SetColor( CShape3d* pColor )
	{
		m_hColor = pColor;
	}

	CShape3d* GetColor( ) const
	{
		return m_hColor;
	}

	void SetRotation ( CShape3d* pRotation )
	{
		m_hRotation = pRotation;
	}

	CShape3d* GetRotation( ) const
	{
		return m_hRotation;
	}

	void SetMinAlpha ( uint8 MinAlpha )
	{
		m_MinAlpha = MinAlpha;
	}

	uint8 GetMinAlpha ( ) const
	{
		return m_MinAlpha;
	}

	void SetMaxAlpha ( uint8 MaxAlpha )
	{
		m_MaxAlpha = MaxAlpha;
	}

	uint8 GetMaxAlpha ( ) const
	{
		return m_MaxAlpha;
	}

	void SetMinFrame ( uint32 MinFrame )
	{
		m_MinFrame = MinFrame;
	}

	uint32 GetMinFrame ( ) const
	{
		return m_MinFrame;
	}

	void SetMaxFrame ( uint32 MaxFrame )
	{
		m_MaxFrame = MaxFrame;
	}
	uint32 GetMaxFrame ( ) const
	{
		return m_MaxFrame;
	}

	void SetEmitted ( bool Emitted )
	{
		m_Emitted = Emitted;
	}
	bool GetEmitted ( ) const
	{
		return m_Emitted;
	}

	void SetAge ( float32 Age )
	{
		m_Age = Age;
	}

	float32 GetAge ( ) const
	{
		return m_Age;
	}

	void SetAgeStdDv ( float32 AgeStdDv )
	{
		m_AgeStdDv = AgeStdDv;
	}

	float32 GetAgeStdDv ( ) const
	{
		return m_AgeStdDv;
	}

	//
	// ISerializable
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private: // data

	HShape3d 	m_hVelocity;
	HShape3d 	m_hTopLeftVelocity;
	HShape3d 	m_hTopRightVelocity;
	HShape3d 	m_hBottomLeftVelocity;
	HShape3d 	m_hBottomRightVelocity;
	HShape3d 	m_hSize;
	HShape3d 	m_hColor;
	HShape3d 	m_hRotation;

	HImage		m_hImage;
	HRect		m_hPosition;

	uint32		m_MinFrame;
	uint32		m_MaxFrame;
	float32		m_Age;
	float32		m_AgeStdDv;
	bool		m_OneTime;
	bool		m_Emitted;
	bool		m_SingleVelocity;
	uint8		m_MinAlpha;
	uint8		m_MaxAlpha;
};


} //namespace Particle
} //namespace WT
