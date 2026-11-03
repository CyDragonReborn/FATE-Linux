/*
	BlendState.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Gary Hinger
	4/14/2005
*/
#pragma once

#include "WT/System/Object.h"
#include <vector>


namespace WT
{
namespace Graphics
{
namespace Animation
{

typedef System::SmartPointer<class CBlendState> HBlendState;
class CBlendState : public System::CObject
{
public:
	WT_CLASS(CBlendState, System::CObject)


	CBlendState();
	CBlendState(CBlendState const& Other);
	virtual ~CBlendState();


	//
	//	CBlendState methods
	//

	virtual uint32 GetIndexCount() const;
	virtual void SetIndexCount( uint32 Count );

	virtual int32 GetIndex( uint32 Index ) const;
	virtual void SetIndex( uint32 Index, int32 Value );

	virtual uint32 GetWeightCount() const;
	virtual void SetWeightCount( uint32 Count );

	virtual float32 GetWeight( uint32 Index ) const;
	virtual void SetWeight( uint32 Index, float32 Value );


	//
	// IPropertyClass interface
	//
	//TODO  Implement
	/*
	WT_CLASS_PROPERTY_START2( CBlendState )
		WT_ARRAY_FUNC_RW_PROPERTY2( int32, Index )
		WT_ARRAY_FUNC_RW_PROPERTY2( float32, Weight );
	WT_CLASS_PROPERTY_END2
	*/


private: // data

	std::vector<int32>		m_Indices;
	std::vector<float32>	m_Weights;
};


} // Animation
} // Graphics
} // WT

