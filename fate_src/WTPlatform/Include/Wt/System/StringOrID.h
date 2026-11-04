//
// StringOrID.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// Overview:
//	The CStringOrID class was designed as a lightweight mechanism for 
//  associating a numeric and/or string identifier with an object. CStringOrID 
//  members can be added to other structures or containers as a way of uniquely
//  idenifiying them within a system.  
//  Runtime instances of CStringOrID may be marked with having valid numeric or 
//  string fields, neither, or both. The default CStringOrID constructor 
//  initializes the object with both numeric and string fields in the invalid 
//  state. 
//  The numeric portion of a CStringOrID instance takes precedence over the 
//  string portion when doing comparisons, such as with the Equals method. 
//  The CStringOrID class is a mutable class in that its numeric or string field
//  can be modified or invalidated at runtime. 
//  To help aid in efficiency, accessor methods are specified as inline. The 
//  CStringOrID class also provides the GetStringRef method to retrieve the 
//  string identifier field by reference.
//

#pragma once

#include "WT/System/Object.h"

namespace WT
{
namespace System
{

typedef SmartPointer<class CStringOrID> HStringOrID;
class CStringOrID : public System::CObject
{
public:

	WT_CLASS( CStringOrID , CObject )
	//TODO Add all derived interface base classes when supported in Type

	// Constructors / destructor
	CStringOrID();
	CStringOrID( const CStringOrID& Other );
	CStringOrID( int32 ID );
	CStringOrID( const std::string& String );
	CStringOrID( int32 ID, const std::string& String );

	virtual ~CStringOrID();

	
	//
	// Operator overloads
	//

	bool operator== (const CStringOrID& rhs) const
	{
		return Equals(&rhs);
	}

	bool operator!= (const CStringOrID& rhs) const
	{
		return !Equals(&rhs);
	}


	//
    // CStringOrID methods
    //

	// Sets the numeric identifier associated with the message object
	inline void SetID( int32 ID );

	// Retrieves the numeric identifier associated with the message object
	// This method will return -1 if no explicit numeric identifier was set
	inline int32 GetID() const;

	// Sets the string identifier associated with the object
	inline void SetString( const std::string& String );

	// Retrieves the string identifier associated with the object
	// This method will return the empty string if no explicit string identifier was set
	inline std::string GetString() const;

	// Retrieves a const reference to the string identifier associated with the object
	// This method will return the empty string if no explicit string identifier was set
	inline const std::string& GetStringRef() const;

	// Determines whether this object is logically equivalent to the provided CStringOrID object
	// Numeric identifiers will take precedence over string identifiers when doing comparison operations
	bool Equals( const CStringOrID* pOther ) const;

	// Determines whether the object has an explicitly-set numeric identifier
	inline bool HasValidID() const;

	// Determines whether the object has an explicitly-set string identifier
	inline bool HasValidString() const;

	// Determines whether the object has an explicitly-set numeric or string identifier
	inline bool IsValid() const;

	// Resets the interal storage members to invalid states
	void Invalidate();

private:	// data

	// User properties
	int32				m_ID;
	string				m_String;

	// Internal properties
	bool				m_IDSet;		// was the numeric identifier explicitly set by the user or through serialization?
	bool				m_StringSet;	// was the string identifier explicitly set by the user or through serialization?
};

#include "StringOrID.inl"

} // namespace System
} // namespace WT
