/*
	ObjectCollection.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Ira Small
	10/31/2005
*/

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/System/Object.h"
#include "WT/System/ObjectPropertyCollection.h"

namespace WT
{
	namespace System
	{
		namespace Collections
		{
			// -------------------------------------------------------------------------------------
			// Interfaces
			// -------------------------------------------------------------------------------------

			typedef SmartPointer<class CObjectCollection> HObjectCollection;
			class CObjectCollection : public CObject
			{
				public:
					WT_CLASS( CObjectCollection, CObject );

					CObjectCollection();
					virtual ~CObjectCollection();

					virtual void Clear()
					{
						m_Objects.clear();
					}

					virtual void AddObject( CObject& Object );
					virtual void RemoveObject( CObject const& Object );

					bool ContainsObject( CObject const* pObject )
					{
						ObjectsMap::iterator it = m_Objects.find( CObject::GetClassType() );

						if (it != m_Objects.end())
						{
							//There are CObject's
							std::vector<HObject>& Objects = it->second;

							std::vector<HObject>::iterator ObjectIt = std::find( Objects.begin(), Objects.end(), pObject );
							if (ObjectIt != Objects.end())
							{
								//The object is in this collection
								return true;
							}
						}

						return false;
					}

					template < typename T >
					uint32 GetObjectCount() const
					{
						ObjectsMap::const_iterator I = m_Objects.find( WT::TypeTraits< T >::GetType() );
						
						if( I != m_Objects.end() )
						{
							return (uint32)( I->second.size() );
						}

						return 0;
					}

					template < typename T >
					T *GetObject( uint32 Index ) const
					{
						ObjectsMap::const_iterator I = m_Objects.find( WT::TypeTraits< T >::GetType() );

						if( I != m_Objects.end() )
						{
							if( Index < I->second.size() )
							{
								return WT::DynamicCast< T >( I->second[ Index ].get() );
							}
						}
						
						return NULL;
					}

					//Find an object of type ObjectType with a property named PropertyName
					//taking on a value of type ValueType equal to Value. NULL is returned if 
					//no satisfactory object is found.
					//For example, FindObject<CPerson>( "Name", "Bob" ) finds a CPerson named "Bob".
					//Currently only the class properties (those defined in WT_BEGIN_PROPERTIES)
					//for the class and its ancestors are searchable - per-object user defined properties 
					//are not.
					template <typename ObjectType, typename ValueType> 
						ObjectType* FindObject( std::string const& PropertyName, ValueType const& Value )
					{
						ObjectsMap::const_iterator I = m_Objects.find( ObjectType::GetClassType() );
						if (I == m_Objects.end())
						{
							//No objects of that type
							return NULL;
						}

						std::vector<HObject> const& Objects = I->second;
						for (int32 i = 0; i < static_cast<int32>( Objects.size() ); ++i)
						{
							CObject* pObject = Objects[i];
							assert( pObject != NULL );

							CObjectPropertyCollection ObjectProperties( *pObject );

							CProperty const* pProp = ObjectProperties.GetItem( PropertyName );
							if (pProp != NULL 
								&& pProp->IsReadable() 
								&& pProp->GetPropertyType() == WT::TypeTraits<ValueType>::GetType())
							{
								CValueTypedProperty<ValueType> const* pTypedProp = WT::DynamicCast< CValueTypedProperty<ValueType> >( pProp );
								if (pProp == NULL)
								{
									throw std::runtime_error( "not CTypedProperty" );
								}

								ValueType ObjectValue;
								pTypedProp->GetValue( pObject, ObjectValue );

								if (ObjectValue == Value)
								{
									return WT::DynamicCast< ObjectType >( pObject ); //Successful match
								}
							}
						}

						return NULL;
					}

			private:
				typedef std::map< CType const*, vector<HObject> > ObjectsMap;
				ObjectsMap m_Objects;
			};
		}
	}
}