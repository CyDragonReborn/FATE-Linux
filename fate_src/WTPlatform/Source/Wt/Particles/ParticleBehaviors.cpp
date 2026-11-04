//
// ParticleBehaviors.cpp
//
// Copyright (C) 2005 WildTangent, Inc.
// All Rights Reserved
//
//

#include "WT/stdafx.h"
#include "WT/Particles/ParticleBehaviors.h"
#include "WT/Particles/Particle.h"
#include "WT/Particles/ParticleSystemData.h"
#include "WT/Math/MathSerialization.h"

namespace WT
{

using namespace Math;

namespace Particles
{

WT_DEFINE_CLASS( CEmitParticleBehavior )
WT_DEFINE_CLASS( CGravityParticleBehavior )
WT_DEFINE_CLASS( CKillOldParticleBehavior )
WT_DEFINE_CLASS( CKillInsideParticleBehavior )
WT_DEFINE_CLASS( CMoveParticleBehavior )
WT_DEFINE_CLASS( CTargetSizeParticleBehavior )
WT_DEFINE_CLASS( CScaleParticleBehavior )
WT_DEFINE_CLASS( CRotateParticleBehavior )
WT_DEFINE_CLASS( CFadeParticleBehavior )
WT_DEFINE_CLASS( CAnimateParticleBehavior )
WT_DEFINE_CLASS( CRandomMoveParticleBehavior )
WT_DEFINE_CLASS( CBounceParticleBehavior )
WT_DEFINE_CLASS( CPointAttractParticleBehavior )
WT_DEFINE_CLASS( CImageEmitParticleBehavior )

// -------------------------------------------------------------------------------------
// CEmitParticleBehavior
// -------------------------------------------------------------------------------------

CEmitParticleBehavior::CEmitParticleBehavior()
{
	m_MinAlpha = 0x0;
	m_MaxAlpha = 0x0;
	m_MinFrame = 0;
	m_MaxFrame = 0;
	m_ParticlesPerSecond = 0.0f;
	m_Age = 0.0f;
	m_AgeStdDv = 0.0f;
}

CEmitParticleBehavior::~CEmitParticleBehavior()
{
}

void CEmitParticleBehavior::Init ( int32 ID, CShape3d* pPosition, CShape3d* pVelocity, CShape3d* pSize,
	                              CShape3d* pColor, uint8 MinAlpha, uint8 MaxAlpha,
						          CShape3d* pRotation,
						          uint32 MinFrame, uint32 MaxFrame,
						          float32 ParticlesPerSecond,
						          float32 Age, float32 AgeStdDv )
{
	m_ID = ID;
	m_hPosition = pPosition;
	m_hVelocity = pVelocity;
	m_hSize = pSize;
	m_hColor = pColor;
	m_hRotation = pRotation;
	m_MinAlpha = MinAlpha;
	m_MaxAlpha = MaxAlpha;
	m_MinFrame = MinFrame;
	m_MaxFrame = MaxFrame;
	m_ParticlesPerSecond = ParticlesPerSecond;
	m_Age = Age;
	m_AgeStdDv = AgeStdDv;
}

void CEmitParticleBehavior::Update ( CParticleSystemData* pData ,  float32 FrameTime )
{
	uint32 Rate = Float32ToInt32 ( Math_Floor ( m_ParticlesPerSecond * FrameTime ));
	
	if ( Math_Randf() < m_ParticlesPerSecond * FrameTime - float32 ( Rate ))
		Rate++;
	
	CVector3f Position , Velocity , Size;
	
	//start adding particles to the list

	CParticle NewParticle;

	for ( uint32 i = 0 ; i < Rate ; i++ )
	{
		//generate random vectors based on our objects

		NewParticle.Age = m_Age + NRand ( m_AgeStdDv );
		NewParticle.Position = m_hPosition->GenerateRandom ();
		NewParticle.Velocity = m_hVelocity->GenerateRandom ();
		NewParticle.Size = m_hSize->GenerateRandom ();
		NewParticle.Color = m_hColor->GenerateRandom();
		NewParticle.Opacity = Math_Randf ( m_MinAlpha , m_MaxAlpha );
		NewParticle.Frame = Math_Randf ( (float32) m_MinFrame , (float32) m_MaxFrame );
		NewParticle.Rotation = m_hRotation->GenerateRandom ();
		pData->AddParticle ( NewParticle );
	}
}

void CEmitParticleBehavior::SetPosition ( CShape3d* pPosition )
{
	m_hPosition = pPosition;
}

CShape3d* CEmitParticleBehavior::GetPosition( ) const
{
	return m_hPosition;
}

void CEmitParticleBehavior::SetVelocity ( CShape3d* pVelocity )
{
	m_hVelocity = pVelocity;
}

CShape3d* CEmitParticleBehavior::GetVelocity( ) const
{
	return m_hVelocity;
}

void CEmitParticleBehavior::SetSize ( CShape3d* pSize )
{
	m_hSize = pSize;
}

CShape3d* CEmitParticleBehavior::GetSize( ) const
{
	return m_hSize;
}

void CEmitParticleBehavior::SetColor ( CShape3d* pColor )
{
	m_hColor = pColor;
}

CShape3d* CEmitParticleBehavior::GetColor( ) const
{
	return m_hColor;
}

void CEmitParticleBehavior::SetRotation ( CShape3d* pRotation )
{
	m_hRotation = pRotation;
}

CShape3d* CEmitParticleBehavior::GetRotation( ) const
{
	return m_hRotation;
}

void CEmitParticleBehavior::SetMinAlpha ( uint8 MinAlpha )
{
	m_MinAlpha = MinAlpha;
}

uint8 CEmitParticleBehavior::GetMinAlpha ( ) const
{
	return m_MinAlpha;
}

void CEmitParticleBehavior::SetMaxAlpha ( uint8 MaxAlpha )
{
	m_MaxAlpha = MaxAlpha;
}

uint8 CEmitParticleBehavior::GetMaxAlpha ( ) const
{
	return m_MaxAlpha;
}

void CEmitParticleBehavior::SetMinFrame ( uint32 MinFrame )
{
	m_MinFrame = MinFrame;
}

uint32 CEmitParticleBehavior::GetMinFrame ( ) const
{
	return m_MinFrame;
}

void CEmitParticleBehavior::SetMaxFrame ( uint32 MaxFrame )
{
	m_MaxFrame = MaxFrame;
}

uint32 CEmitParticleBehavior::GetMaxFrame ( ) const
{
	return m_MaxFrame;
}

void CEmitParticleBehavior::SetParticlesPerSecond ( float32 ParticlesPerSecond )
{
	m_ParticlesPerSecond = ParticlesPerSecond;
}

float32 CEmitParticleBehavior::GetParticlesPerSecond ( ) const
{
	return m_ParticlesPerSecond;
}

void CEmitParticleBehavior::SetAge ( float32 Age )
{
	m_Age = Age;
}

float32 CEmitParticleBehavior::GetAge ( ) const
{
	return m_Age;
}

void CEmitParticleBehavior::SetAgeStdDv ( float32 AgeStdDv )
{
	m_AgeStdDv = AgeStdDv;
}

float32 CEmitParticleBehavior::GetAgeStdDv ( ) const
{
	return m_AgeStdDv;
}

//
// ISerializable
//

void CEmitParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "Position", m_hPosition.get() );
	pInfo->AddValue( "Velocity", m_hVelocity.get() );
	pInfo->AddValue( "Size", m_hSize.get() );
	pInfo->AddValue( "Color", m_hColor.get() );
	pInfo->AddValue( "Rotation", m_hRotation.get() );
	pInfo->AddValue( "MinAlpha", m_MinAlpha );
	pInfo->AddValue( "MaxAlpha", m_MaxAlpha );
	pInfo->AddValue( "MinFrame", m_MinFrame );
	pInfo->AddValue( "MaxFrame", m_MaxFrame );
	pInfo->AddValue( "ParticlesPerSecond", m_ParticlesPerSecond );
	pInfo->AddValue( "Age", m_Age );
	pInfo->AddValue( "AgeStdDv", m_AgeStdDv );
}

void CEmitParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );

	CShape3d* pPosition; pInfo->GetValue( "Position", pPosition ); m_hPosition = pPosition;
	CShape3d* pVelocity; pInfo->GetValue( "Velocity", pVelocity ); m_hVelocity = pVelocity;
	CShape3d* pSize; pInfo->GetValue( "Size", pSize ); m_hSize = pSize;
	CShape3d* pColor; pInfo->GetValue( "Color", pColor ); m_hColor = pColor;
	CShape3d* pRotation; pInfo->GetValue( "Rotation", pRotation ); m_hRotation = pRotation;

	pInfo->GetValue( "MinAlpha", m_MinAlpha );
	pInfo->GetValue( "MaxAlpha", m_MaxAlpha );
	pInfo->GetValue( "MinFrame", m_MinFrame );
	pInfo->GetValue( "MaxFrame", m_MaxFrame );
	pInfo->GetValue( "ParticlesPerSecond", m_ParticlesPerSecond );
	pInfo->GetValue( "Age", m_Age );
	pInfo->GetValue( "AgeStdDv", m_AgeStdDv );
}


// -------------------------------------------------------------------------------------
// CGravityParticleBehavior
// -------------------------------------------------------------------------------------

void CGravityParticleBehavior::Update( CParticleSystemData* pData ,  float32 FrameTime )
{
	CVector3f Gravity ( m_Direction * FrameTime );

	CParticle* pParticle = pData->GetParticles();
	for (uint32 i = 0; i < pData->GetParticleCount(); i++)
	{
		pParticle->Velocity += Gravity;
		pParticle++;
	}
}

//
// ISerializable
//

void CGravityParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "Direction", m_Direction );
}

void CGravityParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
	pInfo->GetValue( "Direction", m_Direction );
}


// -------------------------------------------------------------------------------------
// CKillOldParticleBehavior
// -------------------------------------------------------------------------------------

void CKillOldParticleBehavior::Update( CParticleSystemData* pData,  float32 FrameTime )
{
	CParticle* pParticle = pData->GetParticles() + pData->GetParticleCount();
	for (int i = pData->GetParticleCount(); i >= 0; i--)
	{
		if ( pParticle->Age >= m_Age )
			pData->RemoveParticle( i );

		--pParticle;
	}
}

//
// ISerializable
//

void CKillOldParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "Age", m_Age );
}

void CKillOldParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
	pInfo->GetValue( "Age", m_Age );
}


// -------------------------------------------------------------------------------------
// CKillInsideParticleBehavior
// -------------------------------------------------------------------------------------

void CKillInsideParticleBehavior::Update( CParticleSystemData* pData,  float32 FrameTime )
{
	if ( m_hObject == NULL )
		return;

	// get the actual pointer first to speed things up a bit
	CShape3d* pKillInsideObject = m_hObject;

	CParticle* pParticle = pData->GetParticles() + pData->GetParticleCount();
	for ( int i = pData->GetParticleCount(); i >= 0; i-- )
	{
		if ( pKillInsideObject->IsInside( pParticle->Position ) )
			pData->RemoveParticle( i );

		--pParticle;
	}
}

//
// ISerializable
//

void CKillInsideParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "Object", m_hObject.get() );
}

void CKillInsideParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
	CShape3d* pObject; pInfo->GetValue( "Object", pObject ); m_hObject = pObject;
}


// -------------------------------------------------------------------------------------
// CMoveParticleBehavior
// -------------------------------------------------------------------------------------

void CMoveParticleBehavior::Update( CParticleSystemData* pData,  float32 FrameTime )
{
	CParticle* pParticle = pData->GetParticles();

	// move all the particle according to their respective positions and accelerations
	for ( uint32 i = 0; i < pData->GetParticleCount(); i++ )
	{
		pParticle->Position += ( pParticle->Velocity * FrameTime );
		pParticle->Age += FrameTime;

		++pParticle;
	}
}

//
// ISerializable
//

void CMoveParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
}

void CMoveParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
}


// -------------------------------------------------------------------------------------
// CTargetSizeParticleBehavior
// -------------------------------------------------------------------------------------

void CTargetSizeParticleBehavior::Update( CParticleSystemData* pData,  float32 FrameTime )
{
	CParticle* pParticle = pData->GetParticles();

	// Resize all particel torward the supplied size given the scale rate

	CVector3f ScaleFactor = m_Scale * FrameTime;
	
	for ( uint32 i = 0; i < pData->GetParticleCount(); i++ )
	{
		CVector3f DiffVector ( m_Size - pParticle->Size );
		DiffVector.x *= ScaleFactor.x;
		DiffVector.y *= ScaleFactor.y;
		DiffVector.z *= ScaleFactor.z;
		pParticle->Size += DiffVector;

		++pParticle;
	}
}

//
// ISerializable
//

void CTargetSizeParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "Size", m_Size );
	pInfo->AddValue( "Scale", m_Scale );
}

void CTargetSizeParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
	pInfo->GetValue( "Size", m_Size );
	pInfo->GetValue( "Scale", m_Scale );
}


// -------------------------------------------------------------------------------------
// CScaleParticleBehavior
// -------------------------------------------------------------------------------------

void CScaleParticleBehavior::Update( CParticleSystemData* pData, float32 FrameTime )
{
	CParticle* pParticle = pData->GetParticles();

	CVector3f ScaleFactor = m_Scale * FrameTime;
	
	for ( uint32 i = 0; i < pData->GetParticleCount(); i++ )
	{
		pParticle->Size += ScaleFactor;
		++pParticle;
	}
}

//
// ISerializable
//

void CScaleParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "Scale", m_Scale );
}

void CScaleParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
	pInfo->GetValue( "Scale", m_Scale );
}


// -------------------------------------------------------------------------------------
// CRotateParticleBehavior
// -------------------------------------------------------------------------------------

void CRotateParticleBehavior::Update( CParticleSystemData* pData, float32 FrameTime )
{
	CParticle* pParticle = pData->GetParticles();

	CVector3f Rotation = m_Rotation * FrameTime;
	
	for ( uint32 i = 0; i < pData->GetParticleCount(); i++ )
	{
		pParticle->Rotation += Rotation;
		++pParticle;
	}
}

//
// ISerializable
//

void CRotateParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "Rotation", m_Rotation );
}

void CRotateParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
	pInfo->GetValue( "Rotation", m_Rotation );
}


// -------------------------------------------------------------------------------------
// CFadeParticleBehavior
// -------------------------------------------------------------------------------------

void CFadeParticleBehavior::Update( CParticleSystemData* pData,  float32 FrameTime )
{
	CParticle* pParticle = pData->GetParticles();
	float32 FadeFactor = m_Fade * FrameTime;
	bool ClampLow = (m_Fade < 0.0f);

	if ( pData->GetAdditive() )
	{
		for ( uint32 i = 0; i < pData->GetParticleCount(); i++ )
		{
			pParticle->Color += FadeFactor;
			pParticle->Opacity += FadeFactor;

			if( m_Clamp )
			{
				if( ClampLow )
				{
					if( pParticle->Color.x < 0.0f ) pParticle->Color.x = 0.0f;
					if( pParticle->Color.y < 0.0f ) pParticle->Color.y = 0.0f;
					if( pParticle->Color.z < 0.0f ) pParticle->Color.z = 0.0f;
					if( pParticle->Opacity < 0.0f ) pParticle->Opacity = 0.0f;
				}
				else
				{
					if( pParticle->Color.x > 1.0f ) pParticle->Color.x = 1.0f;
					if( pParticle->Color.y > 1.0f ) pParticle->Color.y = 1.0f;
					if( pParticle->Color.z > 1.0f ) pParticle->Color.z = 1.0f;
					if( pParticle->Opacity > 1.0f ) pParticle->Opacity = 1.0f;
				}
			}
			++pParticle;
		}
	}
	else
	{
		for ( uint32 i = 0; i < pData->GetParticleCount(); i++ )
		{
			pParticle->Opacity += FadeFactor;

			if( m_Clamp )
			{
				if( ClampLow )
				{
					if( pParticle->Opacity < 0.0f ) pParticle->Opacity = 0.0f;
				}
				else
				{
					if( pParticle->Opacity > 1.0f ) pParticle->Opacity = 1.0f;
				}
			}

			++pParticle;
		}
	}
}

//
// ISerializable
//

void CFadeParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "Fade", m_Fade );
}

void CFadeParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
	pInfo->GetValue( "Fade", m_Fade );
}


// -------------------------------------------------------------------------------------
// CAnimateParticleBehavior
// -------------------------------------------------------------------------------------

void CAnimateParticleBehavior::Update( CParticleSystemData* pData, float32 FrameTime )
{
	CParticle* pParticle = pData->GetParticles();
	float32 FrameAdvance = FrameTime / m_SecondsPerFrame;

	for ( uint32 i = 0; i < pData->GetParticleCount(); i++ )
	{
		pParticle->Frame += FrameAdvance;
		if ( pParticle->Frame >= m_StartFrame + m_FrameCount )
		{
			if ( m_Loop )
				pParticle->Frame = static_cast<float32>( m_StartFrame );
			else
				pParticle->Frame -= FrameAdvance;
		}

		++pParticle;
	}
}

//
// ISerializable
//

void CAnimateParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "SecondsPerFrame", m_SecondsPerFrame );
	pInfo->AddValue( "Loop", m_Loop );
	pInfo->AddValue( "StartFrame", m_StartFrame );
	pInfo->AddValue( "FrameCount", m_FrameCount );
}

void CAnimateParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
	pInfo->GetValue( "SecondsPerFrame", m_SecondsPerFrame );
	pInfo->GetValue( "Loop", m_Loop );
	pInfo->GetValue( "StartFrame", m_StartFrame );
	pInfo->GetValue( "FrameCount", m_FrameCount );
}


// -------------------------------------------------------------------------------------
// CRandomMoveParticleBehavior
// -------------------------------------------------------------------------------------

void CRandomMoveParticleBehavior::Update( CParticleSystemData* pData, float32 FrameTime )
{
	CParticle* pParticle = pData->GetParticles();

	CVector3f Displace;
	for ( uint32 i = 0; i < pData->GetParticleCount(); i++ )
	{
		Displace = m_hRandomMove->GenerateRandom() * FrameTime;
		pParticle->Position += Displace;
		++pParticle;
	}
}

//
// ISerializable
//

void CRandomMoveParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "RandomMove", m_hRandomMove.get() );
}

void CRandomMoveParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
	CShape3d* pRandomMove; pInfo->GetValue( "RandomMove", pRandomMove ); m_hRandomMove = pRandomMove;
}


// -------------------------------------------------------------------------------------
// CBounceParticleBehavior
// -------------------------------------------------------------------------------------

void CBounceParticleBehavior::Update( CParticleSystemData* pData, float32 FrameTime )
{
	CParticle* pParticle = pData->GetParticles();
	CVector3f NormalVelocity , TangentVelocity;

	for ( uint32 i = 0; i < pData->GetParticleCount(); i++ )
	{
		if ( m_hBounce->MovingPointCollision( pParticle->Position, pParticle->Velocity, FrameTime, TangentVelocity, NormalVelocity ) )
		{
			if ( TangentVelocity.LengthSquared() <= m_CutoffSqr )
				pParticle->Velocity = TangentVelocity - NormalVelocity * m_Resilience;
			else
				pParticle->Velocity = TangentVelocity * m_OneMinusFriction - NormalVelocity * m_Resilience;
		}

		++pParticle;
	}
}

//
// ISerializable
//

void CBounceParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "Bounce", m_hBounce.get() );
	pInfo->AddValue( "Friction", m_Friction );
	pInfo->AddValue( "Resilience", m_Resilience );
	pInfo->AddValue( "CutOff", m_CutOff );
	pInfo->AddValue( "OneMinusFriction", m_OneMinusFriction );
	pInfo->AddValue( "CuttoffSqr", m_CutoffSqr );
}

void CBounceParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
	CShape3d* pBounce; pInfo->GetValue( "Bounce", pBounce ); m_hBounce = pBounce;
	pInfo->GetValue( "Friction", m_Friction );
	pInfo->GetValue( "Resilience", m_Resilience );
	pInfo->GetValue( "CutOff", m_CutOff );
	pInfo->GetValue( "OneMinusFriction", m_OneMinusFriction );
	pInfo->GetValue( "CuttoffSqr", m_CutoffSqr );
}


// -------------------------------------------------------------------------------------
// CPointAttractParticleBehavior Methods
// -------------------------------------------------------------------------------------

void CPointAttractParticleBehavior::Update( CParticleSystemData* pData, float32 FrameTime )
{
	CParticle* pParticle = pData->GetParticles();

	// first calculate how much of the magnitude we need to apply relative to the frame time
	float MagnitudeDT = m_Magnitude * FrameTime;

	for ( uint32 i = 0; i < pData->GetParticleCount(); i++ )
	{
		CVector3f ParticleDir( m_Point - pParticle->Position );
		
		// calculate force based on distance between the two points (force drops as 1/r^2 )
		// reduced by the drop off parameter

		float32 RSqr = ParticleDir.LengthSquared();

		// check to see if this particle is within affected range of the point
		if ( RSqr < m_RadiusSqr )
			pParticle->Velocity += ParticleDir * ( MagnitudeDT / ( Math_Sqrt( RSqr ) + ( RSqr  + m_DropOff )));

		++pParticle;
	}
}

//
// ISerializable
//

void CPointAttractParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "Point", m_Point );
	pInfo->AddValue( "Radius", m_Radius );
	pInfo->AddValue( "Magnitude", m_Magnitude );
	pInfo->AddValue( "DropOff", m_DropOff );
	pInfo->AddValue( "RadiusSqr", m_RadiusSqr );
}

void CPointAttractParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
	pInfo->GetValue( "Point", m_Point );
	pInfo->GetValue( "Radius", m_Radius );
	pInfo->GetValue( "Magnitude", m_Magnitude );
	pInfo->GetValue( "DropOff", m_DropOff );
	pInfo->GetValue( "RadiusSqr", m_RadiusSqr );
}


// -------------------------------------------------------------------------------------
// CImageEmitParticleBehavior
// -------------------------------------------------------------------------------------

void CImageEmitParticleBehavior::Update( CParticleSystemData* pData, float32 FrameTime )
{
	if ( m_OneTime && m_Emitted )
		return;

	m_Emitted = true;

	if ( m_hImage == NULL )
		return;

	if ( m_hPosition == NULL )
		return;

	CVector3f Position , Velocity , Size;
	CParticle Particle;

	//start adding particles to the list
	uint32 ImageWidth = m_hImage->GetWidth();
	uint32 ImageHeight = m_hImage->GetHeight();

	for ( uint32 y = 0; y < ImageHeight; y++ )
	{
		for ( uint32 x = 0; x < ImageWidth; x++ )
		{
			uint32 PixelColor = m_hImage->GetPixelColor( x, y );
			if ( (PixelColor & 0xFF000000) == 0 )		// Alpha channel
				continue;

			float32 U = (float32)x / (float32)ImageWidth;
			float32 V = (float32)(ImageHeight-y) / (float32)ImageHeight;
			Particle.Age = m_Age + NRand( m_AgeStdDv );

			//map the position onto the rect from 0..1 on both directions
			Particle.Position = m_hPosition->GetPointOn( U , V );
			if ( m_SingleVelocity )
			{
				Particle.Velocity = m_hVelocity->GenerateRandom();
			}
			else
			{
				//generate 4 speeds for 4 corners
				CVector3f BottomLeftSpeed = m_hBottomLeftVelocity->GenerateRandom();
				CVector3f BottomRightSpeed = m_hBottomRightVelocity->GenerateRandom();
				CVector3f TopLeftSpeed = m_hTopLeftVelocity->GenerateRandom();
				CVector3f TopRightSpeed = m_hTopRightVelocity->GenerateRandom();

				//do bilinear interpolation on all 4 speeds to come up with the particle speed
				CVector3f Velocity1 = BottomLeftSpeed * (1-U) + BottomRightSpeed * U;
				CVector3f Velocity2 = TopLeftSpeed * (1-U) + TopRightSpeed * U;
				Particle.Velocity = Velocity1 * (1-V) + V*Velocity2;
			}

			Particle.Size = m_hSize->GenerateRandom();
			Particle.Color.x = (float) ( (PixelColor & 0x00FF0000) >> 16 );	// Red
			Particle.Color.y = (float) ( (PixelColor & 0x0000FF00) >> 8 );	// Green
			Particle.Color.z = (float) ( (PixelColor & 0x000000FF) );		// Blue
			Particle.Opacity = Math_Randf ( m_MinAlpha , m_MaxAlpha );
			Particle.Frame = Math_Randf ( (float32)m_MinFrame, (float32)m_MaxFrame );
			Particle.Rotation = m_hRotation->GenerateRandom();

			pData->AddParticle( Particle );
		}
	}
}

//
// ISerializable
//

void CImageEmitParticleBehavior::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "ID", m_ID );
	pInfo->AddValue( "Velocity", m_hVelocity.get() );
	pInfo->AddValue( "TopLeftVelocity", m_hTopLeftVelocity.get() );
	pInfo->AddValue( "TopRightVelocity", m_hTopRightVelocity.get() );
	pInfo->AddValue( "BottomLeftVelocity", m_hBottomLeftVelocity.get() );
	pInfo->AddValue( "BottomRightVelocity", m_hBottomRightVelocity.get() );

	pInfo->AddValue( "Size", m_hSize.get() );
	pInfo->AddValue( "Color", m_hColor.get() );
	pInfo->AddValue( "Rotation", m_hRotation.get() );

	// TODO - Image must be serializable
	pInfo->AddValue( "Image", m_hImage.get() );
	pInfo->AddValue( "Position", m_hPosition.get() );

	pInfo->AddValue( "MinFrame", m_MinFrame );
	pInfo->AddValue( "MaxFrame", m_MaxFrame );
	pInfo->AddValue( "Age", m_Age );
	pInfo->AddValue( "AgeStdDv", m_AgeStdDv );
	pInfo->AddValue( "OneTime", m_OneTime );
	pInfo->AddValue( "Emitted", m_Emitted );
	pInfo->AddValue( "SingleVelocity", m_SingleVelocity );
	pInfo->AddValue( "MinAlpha", m_MinAlpha );
	pInfo->AddValue( "MaxAlpha", m_MaxAlpha );
}

void CImageEmitParticleBehavior::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "ID", m_ID );
	CShape3d* pVelocity; pInfo->GetValue( "Velocity", pVelocity ); m_hVelocity = pVelocity;
	CShape3d* pTopLeftVelocity; pInfo->GetValue( "TopLeftVelocity", pTopLeftVelocity ); m_hTopLeftVelocity = pTopLeftVelocity;
	CShape3d* pTopRightVelocity; pInfo->GetValue( "TopRightVelocity", pTopRightVelocity ); m_hTopRightVelocity = pTopRightVelocity;
	CShape3d* pBottomLeftVelocity; pInfo->GetValue( "BottomLeftVelocity", pBottomLeftVelocity ); m_hBottomLeftVelocity = pBottomLeftVelocity;
	CShape3d* pBottomRightVelocity; pInfo->GetValue( "BottomRightVelocity", pBottomRightVelocity ); m_hBottomRightVelocity = pBottomRightVelocity;

	CShape3d* pSize; pInfo->GetValue( "Size", pSize ); m_hSize = pSize;
	CShape3d* pColor; pInfo->GetValue( "Color", pColor ); m_hColor = pColor;
	CShape3d* pRotation; pInfo->GetValue( "Rotation", pRotation ); m_hRotation = pRotation;

	// TODO - Image must be serializable
	CImage* pImage; pInfo->GetValue( "Image", pImage ); m_hImage = pImage;
	CRect* pPosition; pInfo->GetValue( "Position", pPosition ); m_hPosition = pPosition;

	pInfo->GetValue( "MinFrame", m_MinFrame );
	pInfo->GetValue( "MaxFrame", m_MaxFrame );
	pInfo->GetValue( "Age", m_Age );
	pInfo->GetValue( "AgeStdDv", m_AgeStdDv );
	pInfo->GetValue( "OneTime", m_OneTime );
	pInfo->GetValue( "Emitted", m_Emitted );
	pInfo->GetValue( "SingleVelocity", m_SingleVelocity );
	pInfo->GetValue( "MinAlpha", m_MinAlpha );
	pInfo->GetValue( "MaxAlpha", m_MaxAlpha );
}


} //namespace Particles
} //namespace WT
