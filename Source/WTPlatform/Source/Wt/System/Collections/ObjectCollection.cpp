/*
	ObjectCollection.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Ira Small
	10/31/2005
 */

#include "WT/stdafx.h"
#include "WT/System/SmartPointer.h"
#include "WT/System/Collections/ObjectCollection.h"

namespace WT
{
	namespace System
	{
		namespace Collections
		{
			WT_DEFINE_CLASS( CObjectCollection );

			CObjectCollection::CObjectCollection()
			{
			}

			CObjectCollection::~CObjectCollection()
			{
			}

			void CObjectCollection::AddObject( CObject& Object )
			{
				if (ContainsObject( &Object ))
				{
					WT_THROW( System::CRuntimeException, "Object already in collection" );
				}

				assert( Object.GetType() != NULL );
				std::vector<CType const*> Types;
				Object.GetType()->GetSupportedTypes( Types );
				assert( !Types.empty() );

				for (std::vector<CType const*>::iterator it = Types.begin(); it != Types.end(); ++it)
				{
					m_Objects[ *it ].push_back( &Object );
				}
			}

			void CObjectCollection::RemoveObject( CObject const& Object )
			{
				if (!ContainsObject( &Object ))
				{
					WT_THROW( System::CRuntimeException, "Object not in collection" );
				}

				assert( Object.GetType() != NULL );
				std::vector<CType const*> Types;
				Object.GetType()->GetSupportedTypes( Types );
				assert( !Types.empty() );

				for (std::vector<CType const*>::iterator TypeIt = Types.begin(); TypeIt != Types.end(); ++TypeIt)
				{
					ObjectsMap::iterator it = m_Objects.find( *TypeIt );

					if (it == m_Objects.end())
					{
						//Object not registered under that type
						continue;
					}

					std::vector<HObject>& Objects = it->second;

					std::vector<HObject>::iterator ObjectIt = std::find( Objects.begin(), Objects.end(), &Object );
					if (ObjectIt != Objects.end())
					{
						Objects.erase( ObjectIt );
					}
				}
			}
		}
	}
}
