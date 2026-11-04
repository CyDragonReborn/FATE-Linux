/*
	Path.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/

#include "WT/stdafx.h"
#include "WT/Graphics/Animation/Path.h"

using namespace WT::Math;


namespace WT
{
namespace Graphics
{
namespace Animation
{

WT_DEFINE_CLASS(CPath)
WT_DEFINE_CLASS(CSimplePath<float32>)

CPath::CPath(const string& Name)
	: m_Name(Name)
	, m_pScaleInterpolator(0)
	, m_pRotationInterpolator(0)
	, m_pTranslationInterpolator(0)
{
}

CPath::CPath(const string& Name,
		   IVector3fKeyInterpolator * pScaleInterpolator,
		   IQuatKeyInterpolator * pRotationInterpolator,
		   IVector3fKeyInterpolator * pTranslationInterpolator)
		   : m_Name(Name)
		   , m_pScaleInterpolator(pScaleInterpolator)
		   , m_pRotationInterpolator(pRotationInterpolator)
		   , m_pTranslationInterpolator(pTranslationInterpolator)
{
}

CPath::~CPath()
{
	SafeDelete( m_pScaleInterpolator );
	SafeDelete( m_pRotationInterpolator );
	SafeDelete( m_pTranslationInterpolator );
}

void CPath::Sample(float32 Time, bool Looping, CMatrix4f& Result) const
{
	// Scale, then rotate, then translate  == [Scale]*[Rotate]+[Translate]
	if (m_pRotationInterpolator)
	{
		CQuaternion Rotation( false );
		m_pRotationInterpolator->Sample(Time, Looping, Rotation);
		Rotation.ToMatrix(&Result);
	}
	else
	{
		Result.SetIdentity();
	}

	if (m_pTranslationInterpolator)
	{
		m_pTranslationInterpolator->Sample(Time, Looping, Result.RowT);
	}

	// optimized for no scale channel
	if (m_pScaleInterpolator)
	{
		CVector3f Scale( false );
		m_pScaleInterpolator->Sample(Time, Looping, Scale);
		Result.PreScale( Scale );
	}
}

void CPath::Sample(float32 Time, bool Looping, PathComponents& Result) const
{
	if (m_pScaleInterpolator)
	{
		m_pScaleInterpolator->Sample(Time, Looping, Result.Scale);
		Result.ApplyScale = true;
	}
	else
	{
		Result.ApplyScale = false;
	}

	if (m_pRotationInterpolator) 
	{
		m_pRotationInterpolator->Sample(Time, Looping, Result.Rotation);
	}
	else
	{
		Result.Rotation.SetNoRotation();
	}

	if (m_pTranslationInterpolator)
	{
		m_pTranslationInterpolator->Sample(Time, Looping, Result.Translation);
	}
	else
	{
		Result.Translation.Clear();
	}
}

bool CPath::GetTimeExtents(float32& StartTime, float32& EndTime) const
{
	bool EndIsSet = false;
	StartTime = KMaxFloat32;
	EndTime   = KMinFloat32;
	if (m_pScaleInterpolator)
	{
		StartTime = std::min(StartTime, m_pScaleInterpolator->GetKeyTime(0));
		if (m_pScaleInterpolator->GetKeyCount() > 1)
		{
			EndTime = std::max(EndTime, m_pScaleInterpolator->GetKeyTime(m_pScaleInterpolator->GetKeyCount()-1));
			EndIsSet = true;
		}
	}

	if (m_pRotationInterpolator)
	{
		StartTime = std::min(StartTime, m_pRotationInterpolator->GetKeyTime(0));
		if (m_pRotationInterpolator->GetKeyCount() > 1)
		{
			EndTime = std::max(EndTime, m_pRotationInterpolator->GetKeyTime(m_pRotationInterpolator->GetKeyCount()-1));
			EndIsSet = true;
		}
	}

	if (m_pTranslationInterpolator)
	{
		StartTime = std::min(StartTime, m_pTranslationInterpolator->GetKeyTime(0));
		if (m_pTranslationInterpolator->GetKeyCount() > 1)
		{
			EndTime = std::max(EndTime, m_pTranslationInterpolator->GetKeyTime(m_pTranslationInterpolator->GetKeyCount()-1));
			EndIsSet = true;
		}
	}

	if (!EndIsSet)  // all channels have only 1 key
		EndTime = StartTime;

	if (StartTime > EndTime)
	{
		StartTime = EndTime = 0.0f;
		return false;
	}

	return true;
}


//TODO: Optimization routines
/*
bool ValueComparison( const cmVector3f &A,const cmVector3f &B, float32 Tolerance)
{
    return A.Compare(B,Tolerance);
}

bool ValueComparison( const cmQuaternion &A, const cmQuaternion &B, float32 Tolerance )
{
    return A.Compare(B,Tolerance);
}

template<typename VT>
bool InterpolatorComparison(const cmIKeyInterpolator<VT> *Long,const cmIKeyInterpolator<VT> *Short, float32 Tolerance)
{
	uint32 keysInLong = Long->GetKeyCount();
	VT ValueLong, ValueShort;

	for (int32 i=keysInLong-1; i>=0; --i)
	{
		float32 Time = Long->GetKeyTime(i);
		Long->Sample(Time,false,ValueLong);
		Short->Sample(Time,false,ValueShort);
		if (ValueComparison(ValueLong,ValueShort,Tolerance) == false)
		{
			return false;
		}
    }
    return true;
}

template<typename VT>
bool InterpolatorComparison(const cmIKeyInterpolator<VT> *Long, 
                               const cmIKeyInterpolator<VT> *Short, 
                               int ShortIndex,  // index of deleted spot: (effectively index of spot 'after' deleted spot)
                               float32 Tolerance)
{
    VT ValueLong, ValueShort;

    //  I'm assuming the highest order curve is cubic
    //  for a cubic curve, deleting a key in short at index N causes a potential change 
    //  in the curve between keys N-2 and N+2
    //  note that the deletion causes the keys in the source (Short) list to be 
    //  renumbered such that (N+1) becomes N, and (N+2) becomes (N+1)
    //  ... so deleting N can effect the source 'Short' curve between N-2 and N+1
    //  that time interval is located in the 'Long' curve is located by index, and comparisons
    //  are made for all 'Long' keys in that interval.

    int32 LongStartingIndex = Long->Search( Short->TimeAtIndex( cmMax(ShortIndex-2, 0) ) );
    int32 LongEndingIndex   = Long->Search( Short->TimeAtIndex( cmMin(ShortIndex+1, Long->GetSize()-1) ) );

    for (int32 i=LongEndingIndex; i>=LongStartingIndex; i--)
    {
	    float32 Time = Long->TimeAtIndex(i);
        Long->Sample(Time,false,&ValueLong);
        Short->Sample(Time,false,&ValueShort);
        if (ValueComparison(ValueLong,ValueShort,Tolerance) == false)
        {
            return false;
        }
    }
    return true;
}

template<typename VT>
bool TryToReduceTo2Keys(cmIKeyInterpolator<VT> *Interpolator, float32 Tolerance)
{
    VT FirstValue, TestValue;

    if (Interpolator->GetSize() <2)
        return true;

    if (Interpolator->GetSize() ==2)
        return true;

    Interpolator->ValueAtIndex(0,&FirstValue);
    for (int32 i=0; i<Interpolator->GetSize(); i++)
		{
            Interpolator->ValueAtIndex(i,&TestValue);
            if ( ValueComparison(FirstValue,TestValue,Tolerance) == false)
            {
                return false;
            }
        }
    // if this point is reached, all keys are equal within tolerance, so remove all but first and last
    float32 FirstTime, LastTime;
    FirstTime = Interpolator->TimeAtIndex(0);
    LastTime = Interpolator->TimeAtIndex(Interpolator->GetSize()-1);
    Interpolator->ClearKeys();
    Interpolator->AppendKey(FirstTime,FirstValue);
    Interpolator->AppendKey(LastTime,FirstValue);
    return true;
}

template<typename VT>
bool InterpolatorHasEmptyData( const cmIKeyInterpolator<VT> *Interpolator, const VT &Identity, float32 Tolerance)
{
    if ( Interpolator->GetSize() == 0 )
        return true;

    if ( Interpolator->GetSize() == 1 ) 
    {
        VT Value;
        Interpolator->ValueAtIndex(0,&Value);
        if (ValueComparison(Value,Identity,Tolerance)==true)
        {
            return true;
        }
    }
    
    if ( Interpolator->GetSize() == 2 ) 
    {
        VT Value;
        Interpolator->ValueAtIndex(0,&Value);
        if (ValueComparison(Value,Identity,Tolerance)==true)
        {
            Interpolator->ValueAtIndex(1,&Value);
            if (ValueComparison(Value,Identity,Tolerance)==true)
            {
                return true;
            }
        }
    }
    return false;
}

template<typename VT>
cmOwnerHandle<CKeyInterpolator<VT> > CopyInterpolator ( const cmIKeyInterpolator<VT> *Interpolator )
{
    cmIFactoryConstructor* pConstructor = g_pComponentBroker->Get( cmExtId(Interpolator->GetID()) );
    if (pConstructor == NULL)
    {
        return NULL;  
    }
    cmIKeyInterpolatorHnd(VT) Copy =  cmIKeyInterpolator<VT>::Promote( pConstructor->Create() );
   
    uint32 Size = Interpolator->GetSize();

    for (uint32 i=0; i<Size; i++)
	{
		float32 Time = Interpolator->TimeAtIndex(i);
        VT Value;
        Interpolator->ValueAtIndex(i,&Value);

        Copy->AppendKey(Time,Value);
        Copy->SetRecomputeAsAClosedLoop(Interpolator->GetRecomputeAsAClosedLoop());
        Copy->SetSerializeEntireKey(Interpolator->GetSerializeEntireKey());
        Copy->Recompute();
    }
    cmTAssert( InterpolatorComparison(Interpolator, (cmIKeyInterpolator<VT> *)Copy, 0.0f) == true);
    return Copy;   
}

template<typename VT>
bool KeyReduce( const cmIKeyInterpolator<VT> *Interpolator, cmIKeyInterpolator<VT> *Copy, float32 Tolerance)
{
    int32 Size = Interpolator->GetSize();
    float32 Time;
    VT Value;
    bool bAnyRemoved = false;

    for (int32 i=Size-2; i>=1; i--)
    {
        Time = Copy->TimeAtIndex(i);
        Copy->ValueAtIndex(i,&Value);
        Copy->RemoveKey(i);
        Copy->Recompute();
        if (  InterpolatorComparison(Interpolator, Copy, i, Tolerance) == false)
        {
            Copy->InsertKey(Time,Value);
            Copy->Recompute();
            #ifndef NDEBUG
            VT Value2;
            Copy->ValueAtIndex(i,&Value2);
            cmTAssert( ValueComparison(Value,Value2,0.0f) );
            cmTAssert( Copy->TimeAtIndex(i) == Time );
            #endif
        }
        else
        {
            bAnyRemoved = true;
        }
        cmTAssert( InterpolatorComparison(Interpolator, Copy, Tolerance) == true);
    }
    return bAnyRemoved;
}
        

template<typename VT>
cmIKeyInterpolatorHnd(VT) ChannelOptimize( const cmIKeyInterpolator<VT> *Interpolator, const VT &Identity, float32 Tolerance)
{
    if (Interpolator)
    {
        cmIKeyInterpolatorHnd(VT) Copy = CopyInterpolator(Interpolator);
        cmIKeyInterpolator<VT> * pCopy = Copy;

        if (TryToReduceTo2Keys(pCopy,Tolerance))
        {
            if (InterpolatorHasEmptyData(pCopy,Identity,Tolerance))
                return NULL;
            else
                return Copy;
        }
        else
        {
            if (KeyReduce(Interpolator,pCopy,Tolerance) ==true)
                return Copy;
        }
    }
    return const_cast< cmIKeyInterpolator<VT> * >(Interpolator);
}
*/

void CPath::Optimize(float32 ScaleTolerance, float32 RotationTolerance, float32 TranslationTolerance)
{
	//TODO: expose
	/*
	cmVector3f IdentityScaleVector(1.0f,1.0f,1.0f);
	cmVector3f IdentityVector(0.0f,0.0f,0.0f);
	cmQuaternion IdentityQuat;
	IdentityQuat.SetNoRotation();

	SetScaleInterpolator(ChannelOptimize(GetScaleInterpolator(), IdentityScaleVector, ScaleTolerance));
	SetTranslationInterpolator(ChannelOptimize( GetTranslationInterpolator(), IdentityVector, TranslationTolerance));
	SetRotationInterpolator(ChannelOptimize( GetRotationInterpolator(), IdentityQuat, RotationTolerance));
	*/
}

} // Animation
} // Graphics
} // WT
