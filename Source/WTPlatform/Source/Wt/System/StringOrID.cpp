/*
	StringOrID.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved
*/

#include "WT/stdafx.h"
#include "WT/System/StringOrID.h"


namespace WT
{
namespace System
{

WT_DEFINE_CLASS(CStringOrID)

//
// Constructors / destructor
//

CStringOrID::CStringOrID()
	: m_ID( -1 )
	, m_String( "" )
	, m_IDSet( false )
	, m_StringSet( false )
{
}

CStringOrID::CStringOrID( const CStringOrID& Other )
	: m_ID( Other.m_ID )
	, m_String( Other.m_String )
	, m_IDSet( Other.m_IDSet )
	, m_StringSet( Other.m_StringSet )
{
}

CStringOrID::CStringOrID( int32 ID )
	: m_ID( ID )
	, m_String( "" )
	, m_IDSet( true )
	, m_StringSet( false )
{
}

CStringOrID::CStringOrID( const std::string& String )
	: m_ID( -1 )
	, m_String( String )
	, m_IDSet( false )
	, m_StringSet( true )
{
}

CStringOrID::CStringOrID( int32 ID, const std::string& String )
	: m_ID( ID )
	, m_String( String )
	, m_IDSet( true )
	, m_StringSet( true )
{
}

CStringOrID::~CStringOrID()
{
}

// Determines whether this object is logically equivalent to the provided CStringOrID object
// Numeric identifiers will take precedence over string identifiers when doing comparison operations
bool CStringOrID::Equals( const CStringOrID* pOther ) const
{
	if( pOther == NULL )
	{
		return false;
	}

	if( m_IDSet && pOther->m_IDSet )
	{
		return (m_ID == pOther->GetID());
	}
	if( m_StringSet && pOther->m_StringSet )
	{
		return (m_String == pOther->GetString());
	}

	return false;
}

// Resets the interal storage members to invalid states
void CStringOrID::Invalidate()
{
	m_ID = -1;
	m_String = "";
	m_IDSet = false;
	m_StringSet = false;
}

} // namespace System
} // namespace WT
