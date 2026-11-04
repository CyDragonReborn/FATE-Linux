//
// Particle.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// Overview:
//	The CParticle class contains all the attributes for a single renderable particle.  The ParticleSystemData
//	class holds a collection of CParticle objects.
//

#pragma once

#include "WT/System/Object.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/Math/MathSerialization.h"
#include "WT/Math/Math.h"

namespace WT
{
namespace Particles
{

typedef System::SmartPointer<class CParticle> HParticle;
class CParticle : public System::CObject, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CParticle, System::CObject )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()

	CParticle() : Age ( 0 ) , Color ( Math::CVector3f(255,255,255) ) , Opacity(255) , Frame ( 0 )
	{
	}

	// ISerializable

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const
	{
		pContext;

		pInfo->AddValue( "Position", Position );
		pInfo->AddValue( "Velocity", Velocity );
		pInfo->AddValue( "Size", Size );
		pInfo->AddValue( "Rotation", Rotation );
		pInfo->AddValue( "Age", Age );
		pInfo->AddValue( "Color", Color );
		pInfo->AddValue( "Opacity", Opacity );
		pInfo->AddValue( "Frame", Frame );
	}

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext )
	{
		pContext;

		pInfo->GetValue( "Position", Position );
		pInfo->GetValue( "Velocity", Velocity );
		pInfo->GetValue( "Size", Size );
		pInfo->GetValue( "Rotation", Rotation );
		pInfo->GetValue( "Age", Age );
		pInfo->GetValue( "Color", Color );
		pInfo->GetValue( "Opacity", Opacity );
		pInfo->GetValue( "Frame", Frame );
	}

public:

	// data

	Math::CVector3f	Position;	// Position of the particle in space
	Math::CVector3f	Velocity;	// The velocity vector of the particle
	Math::CVector3f	Size;		// Size of the particle
	Math::CVector3f	Rotation;	// Rotation of the particle (most important the z rotation)
	Math::CVector3f	Color;		// Color of the particle
	float32			Age;		// Age of particle in seconds
	float32			Opacity;	// Opacity of the particle
	float32			Frame;		// Flipbook frame of this particle (could be fractional number to
								// indicate relative time into the frame)
};

} //namespace Particles
} //namespace WT
