/*
	MotionSerializer.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Gary Hinger
	6/7/2005
*/

#include "WT/stdafx.h"
#include "WT/Math/Math.h"
#include "WT/System/IO/BitStream.h"
#include "WT/Graphics/Animation/MotionSerializer.h"
#include "WT/Graphics/Animation/Motion.h"
#include "WT/Graphics/Animation/Path.h"
#include "WT/Graphics/Animation/LinearInterpolator.h"
#include "WT/Graphics/Animation/HermiteInterpolator.h"
#include "WT/Graphics/Animation/BezierInterpolator.h"
#include "WT/Graphics/Animation/QuatLerpInterpolator.h"
#include "WT/Graphics/Animation/QuatSLerpInterpolator.h"
#include "WT/Graphics/Animation/QuatSQuadInterpolator.h"
#include "WT/Graphics/Animation/AnimationException.h"
#include <vector>

using namespace WT::System;
using namespace WT::System::IO;
using namespace WT::Math;
using std::vector;

namespace WT
{
namespace Graphics
{
namespace Animation
{


//
//	CMotionSerializer class
//

WT_DEFINE_CLASS(CMotionSerializer)

bool CMotionSerializer::Decode(CStream * pStream, CMotion * pMotion)
{
	if (pStream == NULL || pMotion == NULL)
	{
		return false;
	}

	try
	{
		m_pStream = pStream;
		ReadObject( pMotion );
	}
	catch(...)
	{
		// TODO : Log exception...
		return false;
	}

	return true;
}

int32 CMotionSerializer::Encode(CStream * pStream, const CMotion * pMotion)
{
	if (pStream == NULL || pMotion == NULL)
	{
		return 0;
	}

	try
	{
		m_pStream = pStream;
		WriteObject( pMotion );
	}
	catch(...)
	{
		// TODO : Log exception...
		return false;
	}

	return true;
}

//
//	Motion serialization helper functions
//

//
//	Constants, Enums
//

uint32 const geMotionRes_FileTag = 'WMOT';
uint32 const geMotionRes_FileVersion = 0x02;

//Number of bits of precision used to represent a float32, or a unit vector
uint32 const KHighPrecisionBits = 30;

//Number of bits used to represent an interpolation parameter (e.g. 0..1)
uint32 const KLowPrecisionBits = 15;

//Interpolator types
namespace EVecInterpolatorType
{
	enum Type
	{
		First,
		Linear = 0,
		Hermite,
		HermiteZeroDeriv,
		End
	};
}

namespace EQuatInterpolatorType
{
	enum Type
	{
		First,
		Linear = 0,
		Slerp,
		Squad,
		End
	};
}

//Path-related flag constants
uint32 const KPathFlagsHaveScale		= 1 << 0;
uint32 const KPathFlagsHaveRotation		= 1 << 1;
uint32 const KPathFlagsHaveTranslation	= 1 << 2;
uint32 const KPathFlagsScaleShift		= 3; // 3 bits shifted into bits 3..5
uint32 const KPathFlagsRotationShift	= 6; // 3 bits shifted into bits 6..8
uint32 const KPathFlagsTranslationShift	= 9; // 3 bits shifted into bits 9..11
uint32 const KPathFlagsHaveEvents       = 1 << 12;

//Interpolator-related flag constants
uint32 const KInterpFlagsLooping                    = 1 << 0; //A looping path?

uint32 const KInterpFlagsCompressionTimeTypeMask	= 1 << 1; //Mask to extract type of time data compression
uint32 const KInterpFlagsCompressionTimeTypeNone	= 0 << 1; //No compression of time data
uint32 const KInterpFlagsCompressionTimeTypeLinear  = 1 << 1; //Time data advances in constant steps

uint32 const KInterpFlagsCompressionKeyTypeMask	    = 7 << 2; //Mask to extract type of key data compression
uint32 const KInterpFlagsCompressionKeyTypeNone	    = 0 << 2; //No compression of key data
uint32 const KInterpFlagsCompressionKeyTypeDefault  = 1 << 2; //Key data is always the default (unit scale, no rotation or translation)
uint32 const KInterpFlagsCompressionKeyTypeConstant = 2 << 2; //Key data is constant
uint32 const KInterpFlagsCompressionKeyTypeUniform  = 3 << 2; //Uniform scale (may also be used for translation, but unlikely)
uint32 const KInterpFlagsCompressionKeyTypeLinear   = 4 << 2; //Key data advances linearly (applies to scale and translation keys)
uint32 const KInterpFlagsCompressionKeyTypeHinge    = 3 << 2; //Rotations are about a constant hinge axis (applies to rotation keys)

//A member of EVecInterpolatorType or EQuatInterpolatorType is shifted by this amount
//and bitwise-OR'd with the path flags.
uint32 const KInterpFlagsInterpolationTypeShift     = 5;

//Number of bits used to represent number of keys per interpolator
uint32 const KInterpCountBits = 24;



//
//	Motion object serializaton methods
//

// Read methods

CVector3f ReadUnitVector(CBitStream& _BitStream, int32 BitCount = 32)
{
	// Divide bits approximately evenly
	int const PBits = BitCount >> 1;
	int const TBits = BitCount - PBits;

	float32 phi, theta;
	phi = _BitStream.ReadFixedLittleEndian( 0.0f, KPi, PBits );
	_BitStream.FlushBits(); // Optional pad - results in bigger uncompressed file but actually helps compression
	theta = _BitStream.ReadFixedLittleEndian( -KPi, KPi, TBits );
	_BitStream.FlushBits(); // Optional pad - results in bigger uncompressed file but actually helps compression
	
	CVector3f Result( false );
	Result.z = cosf( phi );
	float32 h = sinf( phi );
	Result.x = h * cosf( theta );
	Result.y = h * sinf( theta );
	return Result;
}

CQuaternion ReadRotationQuaternion(CBitStream& _BitStream, int32 BitCount = 48)
{
	// Divide bits approximately evenly
	int RBits = BitCount / 3; // Allot the fewest bits for R.
	int PBits = (BitCount - RBits) >> 1;
	int TBits = BitCount - RBits - PBits;

	float r, phi, theta;
	r = _BitStream.ReadFixedLittleEndian( 0.0f, 0.5f * KPi, RBits );
	_BitStream.FlushBits(); // Optional pad - results in bigger uncompressed file but actually helps compression
	phi = _BitStream.ReadFixedLittleEndian( 0.0f, KPi, PBits );
	_BitStream.FlushBits(); // Optional pad - results in bigger uncompressed file but actually helps compression
	theta = _BitStream.ReadFixedLittleEndian( -KPi, KPi, TBits );
	_BitStream.FlushBits(); // Optional pad - results in bigger uncompressed file but actually helps compression

	// Compute pure imaginary logarithm
	CQuaternion LnQ( false );
	LnQ.w = 0.0f;
	LnQ.z = r * cosf( phi );
	float32 const h = r * sinf( phi );
	LnQ.x = h * cosf( theta );
	LnQ.y = h * sinf( theta );

	// Return the packed quaternion
	return LnQ.Exp();
}

void ReadTimes(CBitStream& _BitStream, uint32 count, uint32 InterpFlags, vector<float32>& Times)
{
	Times.resize( count );

	if ((InterpFlags & KInterpFlagsCompressionTimeTypeMask) == KInterpFlagsCompressionTimeTypeLinear)
	{
		float32 TimeFirst;
		float32 TimeDiff;
		TimeFirst = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits ); // First time
		TimeDiff = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits ); // Delta time

		if (count > 2)
		{
			TimeDiff /= (count - 1);
		}

		for (uint32 i = 0; i < count; ++i)
		{
			Times[i] = TimeFirst + TimeDiff * i;
		}
	} 
	else if (count > 4) 
	{   // This method is only worthwhile when there are more than 4 keys.
		float32 MinTime, MaxTime;
		MinTime = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
		MaxTime = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );

		for (uint32 i = 0; i < count; ++i)
		{
			Times[i] = _BitStream.ReadFixedLittleEndian( MinTime, MaxTime, KLowPrecisionBits );
		}
	} 
	else 
	{
		// Just read the times
		for (uint32 i = 0; i < count; ++i)
		{
			Times[i] = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
		}
	}
}

void ReadVectors(CBitStream& _BitStream, uint32 count, uint32 InterpFlags, 
				 CVector3f const& DefaultValue, vector<CVector3f>& Values)
{
	Values.resize( count );

	switch (InterpFlags & KInterpFlagsCompressionKeyTypeMask)
	{
		case KInterpFlagsCompressionKeyTypeDefault:
			{
				for (uint32 i = 0; i < count; ++i)
				{
					Values[i] = DefaultValue;
				}
			}
			break;

		case KInterpFlagsCompressionKeyTypeConstant:
			{
				CVector3f Value( false );
				Value.x = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				Value.y = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				Value.z = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				for (uint32 i = 0; i < count; ++i)
				{
					Values[i] = Value;
				}
			}
			break;

		case KInterpFlagsCompressionKeyTypeUniform:
			if (count > 4) // This method is only worthwhile when there are more than 4 keys.
			{
				float32 Min, Max;
				Min = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				Max = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );

				for (uint32 i = 0; i < count; ++i)
				{
					Values[i].x = _BitStream.ReadFixedLittleEndian( Min, Max, KLowPrecisionBits );
					Values[i].z = Values[i].y = Values[i].x;
				}
			} 
			else 
			{
				for (uint32 i = 0; i < count; ++i)
				{
					Values[i].x = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
					Values[i].z = Values[i].y = Values[i].x;
				}
			}
			break;

		case KInterpFlagsCompressionKeyTypeLinear:
			{
				// Read extreme values
				CVector3f P1( false );
				CVector3f P2( false );
				P1.x = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				P1.y = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				P1.z = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				P2.x = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				P2.y = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				P2.z = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				CVector3f Delta(P2 - P1);

				for (uint32 i = 0; i < count; ++i)
				{
					float32 Value;	
					Value = _BitStream.ReadFixedLittleEndian( 0.0f, 1.0f, KLowPrecisionBits );
					Values[i] = P1 + Delta * Value;
				}
			}
			break;

		case KInterpFlagsCompressionKeyTypeNone:
			if (count > 4) // This method is only worthwhile when there are more than 4 keys.
			{
				CVector3f Min( false ), Max( false );
				Min.x = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				Min.y = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				Min.z = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				Max.x = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				Max.y = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				Max.z = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );

				for (uint32 i = 0; i < count; ++i)
				{
					Values[i].x = _BitStream.ReadFixedLittleEndian( Min.x, Max.x, KLowPrecisionBits );
					Values[i].y = _BitStream.ReadFixedLittleEndian( Min.y, Max.y, KLowPrecisionBits );
					Values[i].z = _BitStream.ReadFixedLittleEndian( Min.z, Max.z, KLowPrecisionBits );
				}
			} 
			else 
			{
				for (uint32 i = 0; i < count; ++i)
				{
					Values[i].x = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
					Values[i].y = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
					Values[i].z = _BitStream.ReadFloatLittleEndian( KHighPrecisionBits );
				}
			}
			break;

		default:
			throw CAnimationException("CMotionSerializer::ReadVectors - Invalid compression type");
	}
}

void ReadQuats(CBitStream& _BitStream, uint32 count, uint32 InterpFlags, vector<CQuaternion>& Values)
{
	Values.resize( count );

	switch (InterpFlags & KInterpFlagsCompressionKeyTypeMask)
	{
		case KInterpFlagsCompressionKeyTypeDefault:
			for (uint32 i = 0; i < count; ++i)
			{
				Values[i] = CQuaternion();
			}
			break;

		case KInterpFlagsCompressionKeyTypeConstant:
			{
				CQuaternion const Q = ReadRotationQuaternion( _BitStream, 48 );
				for (uint32 i = 0; i < count; ++i)
				{
					Values[i] = Q;
				}
			}
			break;

		case KInterpFlagsCompressionKeyTypeHinge:
			{
				// Read the hinge axis
				CVector3f hinge = ReadUnitVector( _BitStream, KHighPrecisionBits );

				for (uint32 i = 0; i < count; ++i)
				{
					float32 Angle;
					Angle = _BitStream.ReadFixedLittleEndian( -KPi, KPi, KLowPrecisionBits );
					Values[i].SetFromAxisAngle( hinge, Angle );
				}
			}
			break;

		case KInterpFlagsCompressionKeyTypeNone:
			for (uint32 i = 0; i < count; ++i)
			{
				// Read each quaternion
				Values[i] = ReadRotationQuaternion( _BitStream, 48 );
			}
			break;

		default:
			throw CAnimationException("CMotionSerializer::ReadQuats - Invalid compression type");
	}

	//Correct the sign of quaternions so adjacent quaternions lie in same hemisphere
	for (uint32 i=1; i<count; ++i)
	{
		if (Values[i].Dot( Values[i - 1] ) < 0.0f)
		{
			Values[i] = -Values[i];
		}
	}
}

// ReadObject

void CMotionSerializer::ReadObject(CMotion* pMotion)
{
	if (pMotion == NULL || m_pStream == NULL)
	{
		throw CArgumentException("CMotionSerializer::ReadObject - Invalid Stream or Motion object handle");
	}

	uint32 i;
	Read( &i );
	if (i != geMotionRes_FileTag)
	{
		throw CAnimationException("CMotionSerializer::ReadObject - Invalid Motion file tag");
	}

	uint32 FileVersion;
	Read( &FileVersion );
	if (FileVersion < 1 || FileVersion > geMotionRes_FileVersion) 
	{
		throw CAnimationException("CMotionSerializer::ReadObject - Invalid Motion file version");
	}

	uint32 namelength;
	Read( &namelength );

	// Read the motion name
	{
		vector<char> Name;

		if (namelength == std::numeric_limits<uint32>::max())
		{
			throw CAnimationException("CMotionSerializer::ReadObject - corrupt motion file");
		}

		Name.resize( namelength + 1 );
		Read( &Name[0], namelength );
		Name[namelength] = '\0';

		if ( m_MotionVersion > 1 )
		{
			pMotion->GetName() = &Name[0];
		}
	}

	uint32 PathCount;
	Read( &PathCount );

	{
		uint32 EventsFlag;
		Read( &EventsFlag );
		if (EventsFlag)
		{
			throw CAnimationException("CMotionSerializer::ReadObject - Invalid event flags");
		}
	}

	// Read path names
	uint32 StringCount;
	uint32 BlockSize;
	Read( &StringCount );
	Read( &BlockSize );
	
	vector<uint32> Offsets( StringCount );
	Read( &Offsets[0], StringCount );

	vector<string> PathNames;
	PathNames.resize( StringCount );
	for (uint32 j = 0; j < StringCount; ++j)
	{
		uint32 StringLength = ((j + 1 == StringCount) ? BlockSize : Offsets[j + 1]) - Offsets[j];
		
		PathNames[j].resize( StringLength );
		Read( &PathNames[j][0], StringLength );

		//Handle obsolete NULL-terminated data
		if (*PathNames[j].rbegin() == '\0')
		{
			PathNames[j].resize( PathNames[j].size() - 1 );
		}
	}

	// Start with a clean slate - we may be filling an object
	pMotion->SetPathCount( 0 );
	pMotion->SetMorphPathCount( 0 );

	// Read the paths
	CBitStream _BitStream( m_pStream );
	for (uint32 j = 0; j < PathCount; ++j)
	{
		vector<float32> STimes;
		vector<CVector3f> SValues;
		vector<float32> RTimes;
		vector<CQuaternion> RValues;
		vector<float32> TTimes;
		vector<CVector3f> TValues;

		// Read the jth path

		uint16 PathFlags;
		PathFlags = static_cast<uint16>( _BitStream.ReadIntegerLittleEndian( 16 ) );

		uint32 const bHaveScaleKeys = PathFlags & KPathFlagsHaveScale;
		uint32 const bHaveRotationKeys = PathFlags & KPathFlagsHaveRotation;
		uint32 const bHaveTranslationKeys = PathFlags & KPathFlagsHaveTranslation;
		uint32 const SInterpType = (PathFlags >> KPathFlagsScaleShift) & 3;
		uint32 const RInterpType = (PathFlags >> KPathFlagsRotationShift) & 3;
		uint32 const TInterpType = (PathFlags >> KPathFlagsTranslationShift) & 3;
		uint32 const bHaveEvents = PathFlags & KPathFlagsHaveEvents;

		if (bHaveScaleKeys)
		{
			if ( SInterpType < EVecInterpolatorType::First || 
				 SInterpType >= EVecInterpolatorType::End )
			{
				throw CAnimationException("CMotionSerializer::ReadObject - Invalid SInterpType");
			}

			uint8 InterpFlags;
			uint32 count;
			InterpFlags = static_cast<uint8>( _BitStream.ReadIntegerLittleEndian( 8 ) );
			count = _BitStream.ReadIntegerLittleEndian( KInterpCountBits );

			// Check interpolation type for errors
			uint32 const InterpolationType = InterpFlags >> KInterpFlagsInterpolationTypeShift;
			if ( InterpolationType < EVecInterpolatorType::First || 
				 InterpolationType >= EVecInterpolatorType::End )
			{
				throw CAnimationException("CMotionSerializer::ReadObject - Invalid InterpolationType");
			}

			ReadTimes( _BitStream, count, InterpFlags, STimes );
			ReadVectors( _BitStream, count, InterpFlags, CVector3f(1.0f, 1.0f, 1.0f), SValues );
		}

		if (bHaveRotationKeys)
		{
			if ( RInterpType < EQuatInterpolatorType::First || 
				 RInterpType >= EQuatInterpolatorType::End )
			{
				throw CAnimationException("CMotionSerializer::ReadObject - Invalid RInterpType");
			}

			uint8 InterpFlags;
			uint32 count;
			InterpFlags = static_cast<uint8>( _BitStream.ReadIntegerLittleEndian( 8 ) );
			count = _BitStream.ReadIntegerLittleEndian( KInterpCountBits );

			// Check interpolation type for errors
			uint32 const InterpolationType = InterpFlags >> KInterpFlagsInterpolationTypeShift;
			if ( InterpolationType < EQuatInterpolatorType::First || 
				 InterpolationType >= EQuatInterpolatorType::End )
			{
				throw CAnimationException("CMotionSerializer::ReadObject - Invalid InterpolationType");
			}

			ReadTimes( _BitStream, count, InterpFlags, RTimes );
			ReadQuats( _BitStream, count, InterpFlags, RValues );
		}

		if (bHaveTranslationKeys)
		{
			if ( TInterpType < EVecInterpolatorType::First || 
				 TInterpType >= EVecInterpolatorType::End )
			{
				throw CAnimationException("CMotionSerializer::ReadObject - Invalid TInterpType");
			}

			uint8 InterpFlags;
			uint32 count;
			InterpFlags = static_cast<uint8>( _BitStream.ReadIntegerLittleEndian( 8 ) );
			count = _BitStream.ReadIntegerLittleEndian( KInterpCountBits );

			// Check interpolation type for errors
			uint32 const InterpolationType = InterpFlags >> KInterpFlagsInterpolationTypeShift;
			if ( InterpolationType < EVecInterpolatorType::First || 
				 InterpolationType >= EVecInterpolatorType::End )
			{
				throw CAnimationException("CMotionSerializer::ReadObject - Invalid InterpolationType");
			}

			ReadTimes( _BitStream, count, InterpFlags, TTimes );
			ReadVectors( _BitStream, count, InterpFlags, CVector3f(), TValues );
		}

		HPath hPath( new CPath( PathNames[j] ) );

		if (!STimes.empty())
		{
			IVector3fKeyInterpolator * pInterp;

			switch (SInterpType)
			{
				default:
					assert(0);
				case EVecInterpolatorType::Linear:
					pInterp = new CVector3fLinearInterpolator();
					break;

				case EVecInterpolatorType::Hermite:
					pInterp = new CVector3fHermiteInterpolator();
					break;

				case EVecInterpolatorType::HermiteZeroDeriv:
					pInterp = new CVector3fBezierInterpolator();
					break;
			}

			for (uint32 i = 0; i < STimes.size(); ++i)
			{
				pInterp->AddKey(STimes[i], SValues[i]);
			}
			pInterp->SetRecomputeAsAClosedLoop( true );
			pInterp->Recompute();
			hPath->SetScaleInterpolator( pInterp );
		}

		if (!RTimes.empty())
		{
			IQuatKeyInterpolator * pInterp;

			switch (RInterpType)
			{
				default:
					assert(0);
				case EQuatInterpolatorType::Linear:
					pInterp = new CQuatLinearInterpolator();
					break;

				case EQuatInterpolatorType::Slerp:
					pInterp = new CQuatSLerpInterpolator();
					break;

				case EQuatInterpolatorType::Squad:
					pInterp = new CQuatSQuadInterpolator();
					break;
			}

			for (uint32 i = 0; i < RTimes.size(); ++i)
			{
				pInterp->AddKey( RTimes[i], RValues[i] );
			}
			pInterp->SetRecomputeAsAClosedLoop( true );
			pInterp->Recompute();
			hPath->SetRotationInterpolator( pInterp );
		}

		if (!TTimes.empty())
		{
			IVector3fKeyInterpolator* pInterp;

			switch (TInterpType)
			{
				default:
					assert(0);
				case EVecInterpolatorType::Linear:
					pInterp = new CVector3fLinearInterpolator();
					break;

				case EVecInterpolatorType::Hermite:
					pInterp = new CVector3fHermiteInterpolator();
					break;

				case EVecInterpolatorType::HermiteZeroDeriv:
					pInterp = new CVector3fBezierInterpolator();
					break;
			}

			for (uint32 i = 0; i < TTimes.size(); ++i)
			{
				pInterp->AddKey( TTimes[i], TValues[i] );
			}
			pInterp->SetRecomputeAsAClosedLoop( true );
			pInterp->Recompute();
			hPath->SetTranslationInterpolator( pInterp );
		}

		if (bHaveEvents)
		{
			uint32 EventCount = _BitStream.ReadIntegerLittleEndian( 32 );

			std::vector<float32> Times( EventCount );
			for (uint32 i = 0; i < EventCount; ++i)
			{
				Times[i] = _BitStream.ReadFloatLittleEndian( 32 );
			}

			for (uint32 i = 0; i < EventCount; ++i)
			{
				string Data;

				for (;;)
				{
					char c = static_cast<char>( _BitStream.ReadIntegerLittleEndian( 8 ) );

					//Check for NUL terminator
					if (c == '\0')
					{
						break;
					}

					Data.push_back( c );
				}

				hPath->GetEvents().push_back( CPath::Event( Times[i], Data ) ); 
			}
			
			std::stable_sort( hPath->GetEvents().begin(), hPath->GetEvents().end() );
		}

		pMotion->AddPath( *hPath );
	} // end for loop over Paths
	_BitStream.FlushBits();
}

// Write Methods

//Pack a unit vector into a 32-bit value.
void WriteUnitVector(const CVector3f& V, CBitStream& _BitStream, int32 BitCount = 32)
{
	// Divide bits approximately evenly
	int const PBits = BitCount >> 1;
	int const TBits = BitCount - PBits;

	// Convert to spherical coordinates
	float phi;
	float theta;
	phi = V.z <= -1.0f ? KPi : V.z >= 1.0f ? 0.0f : acosf( V.z );
	theta = atan2f( V.y, V.x );

	/*
	phi ranges from 0 to pi
	theta ranges from -pi to +pi
	*/

	_BitStream.WriteFixedLittleEndian( phi, 0.0f, KPi, PBits );
	_BitStream.FlushBits(); // Optional pad - results in bigger uncompressed file but actually helps compression
	_BitStream.WriteFixedLittleEndian( theta, -KPi, KPi, TBits );
	_BitStream.FlushBits(); // Optional pad - results in bigger uncompressed file but actually helps compression
}

/*
Pack a unit quaternion into a sequence of bits. A factor of -1 may be discarded.
With Bits = 32, after unpacking, the maximum error in rotation matrix components 
will be about 0.0033.
*/
void WriteRotationQuaternion(const CQuaternion& Q, CBitStream& _BitStream, int32 BitCount = 32)
{
	// Divide bits approximately evenly
	int RBits = BitCount / 3; // Allot the fewest bits for R.
	int PBits = (BitCount - RBits) >> 1;
	int TBits = BitCount - RBits - PBits;

	// From the 4-component unit quaternion, compute a 3-component pure imaginary 
	// logarithm. Note: Ln() automatically negates the input if w < 0.
	CQuaternion LnQ = Q.Ln();
	
	// The logarithm is a 3D vector with magnitude 0 to pi/2. This point rests in a 
	// sphere of radius pi/2. Points outside of the sphere but in a cube enclosing 
	// the sphere can't be used and are being wasted! Convert the vector to spherical 
	// coordinates to avoid this waste and allow us to decrease precision, thus
	// improving compression.

	float r = LnQ.x * LnQ.x + LnQ.y * LnQ.y + LnQ.z * LnQ.z;
	float phi;
	float theta;
	if (r == 0.0f)
	{
		phi = 0.0f;
		theta = -KPi;
	} 
	else 
	{
		r = sqrtf( r );
		phi = LnQ.z / r;
		phi = phi <= -1.0f ? KPi : phi >= 1.0f ? 0.0f : acosf( phi );
		theta = atan2f( LnQ.y, LnQ.x );
	}

	/*
	r ranges from 0 to pi/2.
	phi ranges from pi.
	theta ranges from -pi to +pi.
	*/

	_BitStream.WriteFixedLittleEndian( r, 0.0f, 0.5f * KPi, RBits );
	_BitStream.FlushBits(); // Optional pad - results in bigger uncompressed file but actually helps compression
	_BitStream.WriteFixedLittleEndian( phi, 0.0f, KPi, PBits );
	_BitStream.FlushBits(); // Optional pad - results in bigger uncompressed file but actually helps compression
	_BitStream.WriteFixedLittleEndian( theta, -KPi, KPi, TBits );
	_BitStream.FlushBits(); // Optional pad - results in bigger uncompressed file but actually helps compression
}

template <typename T>
bool InterpolatorHasLinearTime(const IKeyInterpolator<T> * pInterp)
{
	float32 const KTolerance = 0.0001f;
	
	uint32 const KeyCount = pInterp->GetKeyCount();
	if (KeyCount <= 2)
	{
		return true;
	}

    float32 const First = pInterp->GetKeyTime( 0 );
    float32 const Delta = (pInterp->GetKeyTime(KeyCount - 1) - First) / (KeyCount - 1);
    
    for (uint32 i=2; i<KeyCount - 1; ++i)
    {
		float32 const Expected = First + Delta * (i - 1);
        if ( fabsf( pInterp->GetKeyTime(i) - Expected ) > KTolerance )
        {
            return false;
        }
    }

    return true;
}

template<typename T> 
void WriteTimes(CBitStream& _BitStream, uint32 count, uint32 InterpFlags, const IKeyInterpolator<T> * pInterp)
{
	if ((InterpFlags & KInterpFlagsCompressionTimeTypeMask) == KInterpFlagsCompressionTimeTypeLinear)
	{
		assert( count >= 2 );
		float32 const TimeFirst = pInterp->GetKeyTime( 0 );
		float32 const TimeDiff = pInterp->GetKeyTime( count - 1 ) - TimeFirst;
		_BitStream.WriteFloatLittleEndian( TimeFirst, KHighPrecisionBits ); // First element time
		_BitStream.WriteFloatLittleEndian( TimeDiff, KHighPrecisionBits ); // Delta time
	} 
	else if (count > 4) 
	{	// This method is only worthwhile when there are more than 4 keys.
		float32 MinTime = 0.0f;
		float32 MaxTime = 0.0f;

		if (count)
		{
			float32 time = pInterp->GetKeyTime( 0 );
			MinTime = time;
			MaxTime = time;
		}

		for (uint32 i = 1; i < count; ++i)
		{
			float32 time = pInterp->GetKeyTime( i );
			if (MinTime > time) { MinTime = time; }
			else if (MaxTime < time) { MaxTime = time; }
		}

		_BitStream.WriteFloatLittleEndian( MinTime, KHighPrecisionBits );
		_BitStream.WriteFloatLittleEndian( MaxTime, KHighPrecisionBits );

		for (uint32 i = 0; i < count; ++i)
		{
			float32 time = pInterp->GetKeyTime( i );
			_BitStream.WriteFixedLittleEndian( time, MinTime, MaxTime, KLowPrecisionBits ); //Write ith element time
		}
	} 
	else 
	{
		// Just write the times
		for (uint32 i = 0; i < count; ++i)
		{
			float32 time = pInterp->GetKeyTime( i );
			_BitStream.WriteFloatLittleEndian( time, KHighPrecisionBits );
		}
	}
}

template <typename T>
bool InterpolatorIsDefault(const IKeyInterpolator<T> * pInterp, T const& DefaultValue)
{
	float32 const KTolerance = 0.0001f;

	uint32 const KeyCount = pInterp->GetKeyCount();
	for (uint32 i = 0; i < KeyCount; ++i)
	{
		T Value( false );
		pInterp->GetValueByIndex( i, &Value );
		if (!Value.Compare( DefaultValue, KTolerance ))
		{
			return false;
		}
	}

	return true;
}

template <typename T>
bool InterpolatorIsConstant(const IKeyInterpolator<T> * pInterp)
{
	float32 const KTolerance = 0.0001f;

	uint32 const KeyCount = pInterp->GetKeyCount();
	if (KeyCount < 2)
	{
		return true;
	}

	T Value( false );
	pInterp->GetValueByIndex( 0, &Value );

	for (uint32 i = 1; i < KeyCount; ++i)
	{
		T Trial( false );
		pInterp->GetValueByIndex( i, &Trial );
		if (!Value.Compare( Trial, KTolerance ))
		{
			return false;
		}
	}

	return true;
}

bool InterpolatorIsUniform(const IKeyInterpolator<CVector3f> * pInterp)
{
	float32 const KTolerance = 0.0001f;

	uint32 const KeyCount = pInterp->GetKeyCount();
	for (uint32 i = 0; i < KeyCount; ++i)
	{
		CVector3f Value( false );
		pInterp->GetValueByIndex( i, &Value );
		if (fabsf( Value.y - Value.x ) > KTolerance ||
			fabsf( Value.z - Value.x ) > KTolerance)
		{
			return false;
		}
	}

	return true;
}

bool InterpolatorIsLinear(const IKeyInterpolator<CVector3f> * pInterp)
{
	float32 const KTolerance = 0.0001f;

	uint32 const KeyCount = pInterp->GetKeyCount();
	if (KeyCount < 2)
	{
		return true;
	}

	//Choose an arbitrary key
	int32 NearKey = 0;
	CVector3f Near( false );
	pInterp->GetValueByIndex( NearKey, &Near );

	//Choose the farthest key (to maintain maximum precision)
	int32 FarKey = 0;
	CVector3f Far( Near );
	CVector3f Delta;
	float32 LengthSquared = 0.0f;

	for (uint32 i = 1; i < KeyCount; ++i)
	{
		CVector3f TrialFar( false );
		pInterp->GetValueByIndex( i, &TrialFar );
		CVector3f TrialDelta = TrialFar - Near;
		float32 TrialLengthSquared = TrialDelta.LengthSquared();

		if (TrialLengthSquared > LengthSquared)
		{
			FarKey = i;
			Far = TrialFar;
			Delta = TrialDelta;
			LengthSquared = TrialLengthSquared;
		}
	}

	if (LengthSquared == 0.0f)
	{
		return true; // All points are coincident
	}

	for (uint32 i = 1; i < KeyCount; ++i)
	{
		CVector3f Temp( false );
		pInterp->GetValueByIndex( i, &Temp );

		CVector3f Projected = Near + (Temp - Near).DotProduct( Delta ) / LengthSquared * Delta;

		if (!Temp.Compare( Projected, KTolerance ))
		{
			return false;
		}
	}

	return true;
}

bool InterpolatorIsHinged(const IKeyInterpolator<CQuaternion> * pInterp )
{
	float32 const KTolerance = 0.0001f;

	uint32 const KeyCount = pInterp->GetKeyCount();
	if (KeyCount < 2)
	{
		return true;
	}

    CVector3f Axis( false );
    float32 Angle;
	CQuaternion Q( false );
	pInterp->GetValueByIndex( 0, &Q );
	if (!Q.GetAxisAngle( &Axis, &Angle ))
	{
		return false;
	}

	// The axis can be used with or without a negative sign.
	// Find the component of largest magnitude in the Axis. We'll demand that this 
	// be positive to enable comparisons between hinge rotations.
	int ComponentIndex = 0;
	{
		float Max = fabsf( Axis.x );
		if (fabsf( Axis.y ) > Max) { Max = fabsf( Axis.y ); ComponentIndex = 1; }
		if (fabsf( Axis.z ) > Max) { ComponentIndex = 2; }
	}

	if (Axis.e[ComponentIndex] < 0.0f)
	{
		Axis = -Axis;
		// Angle = -Angle;
	}
	        
    for (uint32 i=1; i<KeyCount; i++)
    {
		CVector3f NextAxis( false );
		CQuaternion Q( false );
		pInterp->GetValueByIndex( i, &Q );
		if (!Q.GetAxisAngle( &NextAxis, &Angle ))
		{
			return false;
		}

		if (NextAxis.e[ComponentIndex] < 0.0f)
		{
			NextAxis = -NextAxis;
			// Angle = -Angle;
		}
            
        if (!Axis.Compare( NextAxis, KTolerance ))
        {
            return false;
        }
    }

    return true;
}

void WriteVectors(CBitStream& _BitStream, uint32 count, uint32 InterpFlags, const IKeyInterpolator<CVector3f> * pInterp)
{
	switch (InterpFlags & KInterpFlagsCompressionKeyTypeMask)
	{
	case KInterpFlagsCompressionKeyTypeDefault:
		break;

	case KInterpFlagsCompressionKeyTypeConstant:
		{
			CVector3f Value( false );
			pInterp->GetValueByIndex( 0, &Value );
			_BitStream.WriteFloatLittleEndian( Value.x, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Value.y, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Value.z, KHighPrecisionBits );
		}
		break;

	case KInterpFlagsCompressionKeyTypeUniform:
		if (count > 4) // This method is only worthwhile when there are more than 4 keys.
		{
			CVector3f Value( false );
			pInterp->GetValueByIndex( 0, &Value );
			float32 Min = Value.x, Max = Value.x;

			for (uint32 i = 1; i < count; i++)
			{
				pInterp->GetValueByIndex( i, &Value );
				if (Min > Value.x) { Min = Value.x; }
				else if (Max < Value.x) { Max = Value.x; }
			}

			_BitStream.WriteFloatLittleEndian( Min, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Max, KHighPrecisionBits );
			for (uint32 i = 0; i < count; ++i)
			{
				CVector3f Value( false );
				pInterp->GetValueByIndex( i, &Value );
				_BitStream.WriteFixedLittleEndian( Value.x, Min, Max, KLowPrecisionBits );
			}
		} 
		else 
		{
			for (uint32 i = 0; i < count; ++i)
			{
				CVector3f Value( false );
				pInterp->GetValueByIndex( i, &Value );
				_BitStream.WriteFloatLittleEndian( Value.x, KHighPrecisionBits );
			}
		}
		break;

	case KInterpFlagsCompressionKeyTypeLinear:
		{
			// Choose an arbitrary key
			int32 NearKey = 0;
			CVector3f Near( false );
			pInterp->GetValueByIndex( NearKey, &Near );

			// Choose the farthest key (to maintain maximum precision)
			int32 FarKey = 0;
			CVector3f Far( Near );
			CVector3f Delta;
			float32 LengthSquared = 0.0f;

			for (uint32 i=1; i<count; ++i)
			{
				CVector3f TrialFar( false );
				pInterp->GetValueByIndex( i, &TrialFar );
				CVector3f TrialDelta = TrialFar - Near;
				float32 TrialLengthSquared = TrialDelta.LengthSquared();

				if (TrialLengthSquared > LengthSquared)
				{
					FarKey = i;
					Far = TrialFar;
					Delta = TrialDelta;
					LengthSquared = TrialLengthSquared;
				}
			}

			// Now choose NearKey to be the farthest key from FarKey, so we have the endpoints of a 
			// line segment that all keys line on.
			
			for (uint32 i=1; i<count; ++i)
			{
				CVector3f TrialNear( false );
				pInterp->GetValueByIndex( i, &TrialNear );
				CVector3f TrialDelta = Far - TrialNear;
				float32 TrialLengthSquared = TrialDelta.LengthSquared();

				if (TrialLengthSquared > LengthSquared)
				{
					NearKey = i;
					Near = TrialNear;
					Delta = TrialDelta;
					LengthSquared = TrialLengthSquared;
				}
			}

			//Write the two endpoint vectors
			_BitStream.WriteFloatLittleEndian( Near.x, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Near.y, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Near.z, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Far.x, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Far.y, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Far.z, KHighPrecisionBits );

			//For each key, write a number from 0 to 1 indicating the position along the line segment.
			for (uint32 i = 0; i < count; ++i)
			{
				CVector3f Temp( false );
				pInterp->GetValueByIndex( i, &Temp );

				float32 Value = (Temp - Near).DotProduct( Delta );
				_BitStream.WriteFixedLittleEndian( Value, 0.0f, LengthSquared, KLowPrecisionBits );	
			}
		}
		break;

	case KInterpFlagsCompressionKeyTypeNone:
		if (count > 4) //This method is only worthwhile when there are more than 4 keys.
		{
			CVector3f Value( false );
			pInterp->GetValueByIndex( 0, &Value );
			CVector3f Min = Value, Max = Value;

			for (uint32 i = 1; i < count; i++)
			{
				pInterp->GetValueByIndex( i, &Value );
				if (Min.x > Value.x) { Min.x = Value.x; }
				else if (Max.x < Value.x) { Max.x = Value.x; }
				if (Min.y > Value.y) { Min.y = Value.y; }
				else if (Max.y < Value.y) { Max.y = Value.y; }
				if (Min.z > Value.z) { Min.z = Value.z; }
				else if (Max.z < Value.z) { Max.z = Value.z; }
			}

			_BitStream.WriteFloatLittleEndian( Min.x, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Min.y, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Min.z, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Max.x, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Max.y, KHighPrecisionBits );
			_BitStream.WriteFloatLittleEndian( Max.z, KHighPrecisionBits );
			for (uint32 i = 0; i < count; ++i)
			{
				CVector3f Value( false );
				pInterp->GetValueByIndex( i, &Value );
				_BitStream.WriteFixedLittleEndian( Value.x, Min.x, Max.x, KLowPrecisionBits );
				_BitStream.WriteFixedLittleEndian( Value.y, Min.y, Max.y, KLowPrecisionBits );
				_BitStream.WriteFixedLittleEndian( Value.z, Min.z, Max.z, KLowPrecisionBits );
			}
		} 
		else 
		{
			for (uint32 i = 0; i < count; ++i)
			{
				CVector3f Value( false );
				pInterp->GetValueByIndex( i, &Value );
				_BitStream.WriteFloatLittleEndian( Value.x, KHighPrecisionBits );
				_BitStream.WriteFloatLittleEndian( Value.y, KHighPrecisionBits );
				_BitStream.WriteFloatLittleEndian( Value.z, KHighPrecisionBits );
			}
		}
		break;

	default:
		throw CAnimationException("CMotionSerializer::WriteVectors - Invalid compression type");
	}
}

void WriteQuats(CBitStream& _BitStream, uint32 count, uint32 InterpFlags, const IKeyInterpolator<CQuaternion> * pInterp)
{
	switch (InterpFlags & KInterpFlagsCompressionKeyTypeMask)
	{
		case KInterpFlagsCompressionKeyTypeDefault:
			break;

		case KInterpFlagsCompressionKeyTypeConstant:
			{
				CQuaternion Value( false );
				pInterp->GetValueByIndex( 0, &Value );
				WriteRotationQuaternion( Value, _BitStream, 48 );
			}
			break;

		case KInterpFlagsCompressionKeyTypeHinge:
			{
				float Angle;
				CVector3f hinge( false );

				CQuaternion Value( false );
				pInterp->GetValueByIndex( 0, &Value );
				Value.GetAxisAngle( &hinge, &Angle );
				hinge.Normalize();

				// The axis can be used with or without a negative sign.
				// Find the component of largest magnitude in the Axis. We'll demand that this 
				// be positive to enable comparisons between hinge rotations.
				int ComponentIndex = 0;
				{
					float Max = fabsf( hinge.x );
					if (fabsf( hinge.y ) > Max) { Max = fabsf( hinge.y ); ComponentIndex = 1; }
					if (fabsf( hinge.z ) > Max) { ComponentIndex = 2; }
				}

				if (hinge.e[ComponentIndex] < 0.0f)
				{
					hinge = -hinge;
					// Angle = -Angle;
				}

				// Write the hinge axis
				WriteUnitVector( hinge, _BitStream, KHighPrecisionBits );

				for (uint32 i = 0; i < count; ++i)
				{
					pInterp->GetValueByIndex( i, &Value );
					Value.GetAxisAngle( &hinge, &Angle );

					if (hinge.e[ComponentIndex] < 0.0f)
					{
						// hinge = -hinge;
						Angle = -Angle;
					}

					// Normalize the angle to be in the (-Pi,+Pi] range.
					if (Angle <= -KPi)
					{
						Angle += 2.0f * KPi;
					} 
					else if (Angle > KPi)
					{
						Angle -= 2.0f * KPi;
					}

					_BitStream.WriteFixedLittleEndian( Angle, -KPi, KPi, KLowPrecisionBits );
				}
			}
			break;

	case KInterpFlagsCompressionKeyTypeNone:
			for (uint32 i = 0; i < count; ++i)
			{
				CQuaternion Value( false );
				pInterp->GetValueByIndex( i, &Value );
				WriteRotationQuaternion( Value, _BitStream, 48 );
			}
			break;

	default:
		throw CAnimationException("CMotionSerializer::WriteQuats - Invalid compression type");
	}
}

// WriteObject

void CMotionSerializer::WriteObject(const CMotion * pMotion)
{
	if ( pMotion == NULL || m_pStream == NULL )
	{
		throw CArgumentException("CMotionSerializer::WriteObject - Invalid Stream or Motion object handle");
	}

	uint32 i;
	i = geMotionRes_FileTag; Write( &i );
	i = geMotionRes_FileVersion; Write( &i );

	// Write the motion name
	uint32 namelength = static_cast<uint32>( pMotion->GetName().size() );
	Write( &namelength );
	Write( pMotion->GetName().c_str(), namelength );

	// Write the path count
	int32 PathCount = pMotion->GetPathCount();
	Write( &PathCount );

	uint32 EventsFlag = 0;
	Write( &EventsFlag );

	// Write path names
	uint32 StringCount = 0;
	uint32 BlockSize = 0;
	for (int32 i = 0; i < PathCount; ++i)
	{
		const CPath* pPath = pMotion->GetPath( i );
		if ( pPath != NULL )
		{
			BlockSize += static_cast<uint32>( pPath->GetName().size() );
		}
		++StringCount;
	}

	Write( &StringCount );
	Write( &BlockSize );

	uint32 Offset = 0;
	for (int32 i = 0; i < PathCount; ++i)
	{
		Write( &Offset );
		CPath const* pPath = pMotion->GetPath(i);
		if ( pPath != NULL )
		{
			Offset += static_cast<uint32>( pPath->GetName().size() );
		}
	}

	for (int32 i = 0; i < PathCount; ++i)
	{
		const CPath* pPath = pMotion->GetPath(i);
		if ( pPath != NULL )
		{
			const string BoneName = pPath->GetName();
			Write( &BoneName[0], static_cast<uint32>(BoneName.size()) );
		}
	}

	// Write the paths
	CBitStream _BitStream( m_pStream );
	for (int32 j = 0; j < PathCount; ++j)
	{
		// Write the jth path

		CPath const* pPath = pMotion->GetPath( j );
		if (pPath == NULL)
		{
			continue;
		}

		// Compute path flags
		uint16 PathFlags = 0;
		uint32 SInterpType = EVecInterpolatorType::Linear;
		uint32 RInterpType = EQuatInterpolatorType::Linear;
		uint32 TInterpType = EVecInterpolatorType::Linear;

		// For each interpolator that exists, update path flags
		const IVector3fKeyInterpolator * pScaleInterp = pPath->GetScaleInterpolator();
		if (pScaleInterp != NULL && pScaleInterp->GetKeyCount())
		{
			PathFlags |= KPathFlagsHaveScale;

			const CType * pScaleInterpType = pScaleInterp->GetType();
			if ( CVector3fLinearInterpolator::GetClassType()->IsSubclassOf(*pScaleInterpType) )
			{
				SInterpType = EVecInterpolatorType::Linear;
			}
			else if ( CVector3fHermiteInterpolator::GetClassType()->IsSubclassOf(*pScaleInterpType) )
			{
				SInterpType = EVecInterpolatorType::Hermite;
			}
			else if ( CVector3fBezierInterpolator::GetClassType()->IsSubclassOf(*pScaleInterpType) )
			{
				SInterpType = EVecInterpolatorType::HermiteZeroDeriv;
			}
			else
			{
				throw CAnimationException("CMotionSerializer::WriteObject - Unknown scale interpolator type");
			}
		}
		
		const IQuatKeyInterpolator * pRotInterp = pPath->GetRotationInterpolator();
		if (pRotInterp != NULL && pRotInterp->GetKeyCount())
		{
			PathFlags |= KPathFlagsHaveRotation;

			const CType * pRotInterpType = pRotInterp->GetType();
			if ( CQuatLinearInterpolator::GetClassType()->IsSubclassOf(*pRotInterpType) )
			{
				RInterpType = EQuatInterpolatorType::Linear;
			}
			else if ( CQuatSLerpInterpolator::GetClassType()->IsSubclassOf(*pRotInterpType) )
			{
				RInterpType = EQuatInterpolatorType::Slerp;
			}
			else if ( CQuatSQuadInterpolator::GetClassType()->IsSubclassOf(*pRotInterpType) )
			{
				RInterpType = EQuatInterpolatorType::Squad;
			}
			else
			{
				throw CAnimationException("CMotionSerializer::WriteObject - Unknown rotation interpolator type");
			}
		}

		const IVector3fKeyInterpolator * pTransInterp = pPath->GetTranslationInterpolator();
		if (pTransInterp != NULL && pTransInterp->GetKeyCount())
		{
			PathFlags |= KPathFlagsHaveTranslation;

			const CType * pTransInterpType = pTransInterp->GetType();
			if ( CVector3fLinearInterpolator::GetClassType()->IsSubclassOf(*pTransInterpType) )
			{
				TInterpType = EVecInterpolatorType::Linear;
			}
			else if ( CVector3fHermiteInterpolator::GetClassType()->IsSubclassOf(*pTransInterpType) )
			{
				TInterpType = EVecInterpolatorType::Hermite;
			}
			else if ( CVector3fBezierInterpolator::GetClassType()->IsSubclassOf(*pTransInterpType) )
			{
				TInterpType = EVecInterpolatorType::HermiteZeroDeriv;
			}
			else
			{
				throw CAnimationException("CMotionSerializer::WriteObject - Unknown translation interpolator type");
			}
		}

		PathFlags |= SInterpType << KPathFlagsScaleShift;
		PathFlags |= RInterpType << KPathFlagsRotationShift;
		PathFlags |= TInterpType << KPathFlagsTranslationShift;

		if (pPath->GetEvents().size())
		{
			PathFlags |= KPathFlagsHaveEvents;
		}

		_BitStream.WriteIntegerLittleEndian( PathFlags, 16 );

		if (pScaleInterp != NULL && pScaleInterp->GetKeyCount())
		{
			uint8 InterpFlags = static_cast<uint8>(SInterpType << KInterpFlagsInterpolationTypeShift);
			uint32 count = pScaleInterp->GetKeyCount();

			if (count >= (1 << KInterpCountBits))
			{
				throw CAnimationException("CMotionSerializer::WriteObject - Interpolator key count exceeds max allowed");
			}

			if ( pScaleInterp->GetKeyCount() > 1 && InterpolatorHasLinearTime(pScaleInterp) )
			{
				InterpFlags |= KInterpFlagsCompressionTimeTypeLinear;
			}

			if ( InterpolatorIsDefault(pScaleInterp, CVector3f(1.0f, 1.0f, 1.0f)) )
			{
				InterpFlags |= KInterpFlagsCompressionKeyTypeDefault;
			} 
			else if ( pScaleInterp->GetKeyCount() >= 2 && InterpolatorIsConstant(pScaleInterp) ) 
			{
				InterpFlags |= KInterpFlagsCompressionKeyTypeConstant;
			} 
			else if ( InterpolatorIsUniform(pScaleInterp) ) 
			{
				InterpFlags |= KInterpFlagsCompressionKeyTypeUniform;
			} 
			else if ( pScaleInterp->GetKeyCount() >= 3 && InterpolatorIsLinear(pScaleInterp) ) 
			{
				InterpFlags |= KInterpFlagsCompressionKeyTypeLinear;
			}

			//InterpFlags |= KInterpFlagsLooping; // Looping

			_BitStream.WriteIntegerLittleEndian( InterpFlags, 8 );
			_BitStream.WriteIntegerLittleEndian( count, KInterpCountBits );

			WriteTimes( _BitStream, count, InterpFlags, pScaleInterp );
			WriteVectors( _BitStream, count, InterpFlags, pScaleInterp );
		}

		if (pRotInterp != NULL && pRotInterp->GetKeyCount())
		{
			uint8 InterpFlags = static_cast<uint8>(RInterpType << KInterpFlagsInterpolationTypeShift);
			uint32 count = pRotInterp->GetKeyCount();

			if (count >= (1 << KInterpCountBits))
			{
				throw CAnimationException("CMotionSerializer::WriteObject - Interpolator key count exceeds max allowed");
			}

			if ( pRotInterp->GetKeyCount() > 1 && InterpolatorHasLinearTime(pRotInterp) )
			{
				InterpFlags |= KInterpFlagsCompressionTimeTypeLinear;
			}

			if ( InterpolatorIsDefault( pRotInterp, CQuaternion()) )
			{
				InterpFlags |= KInterpFlagsCompressionKeyTypeDefault;
			} 
			else if ( pRotInterp->GetKeyCount() >= 2 && InterpolatorIsConstant(pRotInterp) ) 
			{
				InterpFlags |= KInterpFlagsCompressionKeyTypeConstant;
			} 
			else if ( pRotInterp->GetKeyCount() >= 3 && InterpolatorIsHinged( pRotInterp )) 
			{
				InterpFlags |= KInterpFlagsCompressionKeyTypeHinge;
			}
			
			//InterpFlags |= KInterpFlagsLooping; // Looping

			_BitStream.WriteIntegerLittleEndian( InterpFlags, 8 );
			_BitStream.WriteIntegerLittleEndian( count, KInterpCountBits );

			WriteTimes( _BitStream, count, InterpFlags, pRotInterp );
			WriteQuats( _BitStream, count, InterpFlags, pRotInterp );
		}

		if (pTransInterp != NULL && pTransInterp->GetKeyCount())
		{
			uint8 InterpFlags = static_cast<uint8>(TInterpType << KInterpFlagsInterpolationTypeShift);
			uint32 count = pTransInterp->GetKeyCount();

			if (count >= (1 << KInterpCountBits))
			{
				throw CAnimationException("CMotionSerializer::WriteObject - Key count exceeds max");
			}

			if (pTransInterp->GetKeyCount() > 1 && InterpolatorHasLinearTime( pTransInterp ))
			{
				InterpFlags |= KInterpFlagsCompressionTimeTypeLinear;
			}

			if (InterpolatorIsDefault( pTransInterp, CVector3f() ))
			{
				InterpFlags |= KInterpFlagsCompressionKeyTypeDefault;
			} 
			else if (pTransInterp->GetKeyCount() >= 1 && InterpolatorIsConstant( pTransInterp )) 
			{
				InterpFlags |= KInterpFlagsCompressionKeyTypeConstant;
			} 
			else if (InterpolatorIsUniform( pTransInterp )) 
			{
				InterpFlags |= KInterpFlagsCompressionKeyTypeUniform;
			} 
			else if (pTransInterp->GetKeyCount() >= 3 && InterpolatorIsLinear( pTransInterp )) 
			{
				InterpFlags |= KInterpFlagsCompressionKeyTypeLinear;
			}

			//InterpFlags |= KInterpFlagsLooping; // Looping

			_BitStream.WriteIntegerLittleEndian( InterpFlags, 8 );
			_BitStream.WriteIntegerLittleEndian( count, KInterpCountBits );

			WriteTimes( _BitStream, count, InterpFlags, pTransInterp );
			WriteVectors( _BitStream, count, InterpFlags, pTransInterp );
		}

		if (pPath->GetEvents().size())
		{
			uint32 EventCount = static_cast<uint32>( pPath->GetEvents().size() );
			_BitStream.WriteIntegerLittleEndian( EventCount, 32 );

			for (uint32 i = 0; i < EventCount; ++i)
			{
				_BitStream.WriteFloatLittleEndian( pPath->GetEvents()[i].m_Time, 32 );
			}

			for (uint32 i = 0; i < EventCount; ++i)
			{
				char const* s = pPath->GetEvents()[i].m_Data.c_str();
				for (int32 j = static_cast<int32>( strlen( s ) ); j; --j)
				{
					_BitStream.WriteIntegerLittleEndian( *(s++), 8 );
				}

				//Write NUL terminator
				char const c = '\0';
				_BitStream.WriteIntegerLittleEndian( c, 8 );
			}
		}
	}
	_BitStream.FlushBits();
}


} // Animation
} // Graphics
} // WT
