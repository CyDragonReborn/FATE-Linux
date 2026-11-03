/*
	Light.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Ira Small
	10/27/2005
*/

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/System/Object.h"
#include "WT/System/Serialization/ISerializable.h"

namespace WT
{
	namespace Graphics
	{
		// -------------------------------------------------------------------------------------
		// Interfaces
		// -------------------------------------------------------------------------------------

		typedef System::SmartPointer<class CLight> HLight;
		class CLight : public WT::System::CObject, public WT::System::Serialization::ISerializable
		{
			public:
				CLight();
				CLight( D3DLIGHT9 &Light );
				CLight( CLight &Light );
				virtual ~CLight();

				WT_BEGIN_CLASS( CLight, WT::System::CObject );
					WT_INHERITS_INTERFACE(WT::System::Serialization::ISerializable);
				WT_END_CLASS();

				virtual void GetObjectData( WT::System::Serialization::CSerializationInfo* SerialInfo, WT::System::Serialization::CSerializationContext* sContext ) const;
				virtual void SetObjectData( WT::System::Serialization::CSerializationInfo* SerialInfo, WT::System::Serialization::CSerializationContext* sContext );
				virtual void FinalizeDeserialization( WT::System::Serialization::CSerializationInfo* SerialInfo, WT::System::Serialization::CSerializationContext* sContext );

				CLight &operator=( const D3DLIGHT9 &rhs );
				operator D3DLIGHT9 &();
				CLight &operator=( const CLight &rhs );

			private:
				D3DLIGHT9 DxLight;
		};
	}
}