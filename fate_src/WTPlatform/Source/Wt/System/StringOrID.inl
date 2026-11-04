//
//	StringOrID.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

// Sets the numeric identifier associated with the message object
inline void CStringOrID::SetID( int32 ID )
{
	m_ID = ID;
	m_IDSet = true;
}

// Retrieves the numeric identifier associated with the message object
// This method will return -1 if no explicit numeric identifier was set
inline int32 CStringOrID::GetID() const
{
	return m_ID;
}

// Sets the string identifier associated with the object
inline void CStringOrID::SetString( const std::string& String )
{
	m_String = String;
	m_StringSet = true;
}

// Retrieves the string identifier associated with the object
// This method will return the empty string if no explicit string identifier was set
inline std::string CStringOrID::GetString() const
{
	return m_String;
}

// Retrieves a const reference to the string identifier associated with the object
// This method will return the empty string if no explicit string identifier was set
inline const std::string& CStringOrID::GetStringRef() const
{
	return m_String;
}

// Determines whether the object has an explicitly-set numeric identifier
inline bool CStringOrID::HasValidID() const
{
	return m_IDSet;
}

// Determines whether the object has an explicitly-set string identifier
inline bool CStringOrID::HasValidString() const
{
	return m_StringSet;
}

// Determines whether the object has an explicitly-set numeric or string identifier
inline bool CStringOrID::IsValid() const
{
	return HasValidID() || HasValidString();
}