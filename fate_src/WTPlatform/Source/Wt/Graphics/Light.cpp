/*
	Light.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Ira Small
	10/27/2005
 */

#include "WT/stdafx.h"
#include "WT/Graphics/Light.h"

namespace WT
{
	namespace Graphics
	{
		WT_DEFINE_CLASS( CLight );

		CLight::CLight()
		{
			memset( &DxLight, 0, sizeof( DxLight ) );
		}

		CLight::CLight( D3DLIGHT9 &Light )
		{
			*this = Light;
		}

		CLight::CLight( CLight &Light )
		{
			*this = Light;
		}

		CLight::~CLight()
		{
			return;
		}

		void CLight::GetObjectData( System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext ) const
		{
			SerialInfo->AddValue( "Type", (uint32)DxLight.Type );

			SerialInfo->AddValue( "Diffuse.r", (float32)DxLight.Diffuse.r );
			SerialInfo->AddValue( "Diffuse.g", (float32)DxLight.Diffuse.g );
			SerialInfo->AddValue( "Diffuse.b", (float32)DxLight.Diffuse.b );
			SerialInfo->AddValue( "Diffuse.a", (float32)DxLight.Diffuse.a );

			SerialInfo->AddValue( "Specular.r", (float32)DxLight.Specular.r );
			SerialInfo->AddValue( "Specular.g", (float32)DxLight.Specular.g );
			SerialInfo->AddValue( "Specular.b", (float32)DxLight.Specular.b );
			SerialInfo->AddValue( "Specular.a", (float32)DxLight.Specular.a );

			SerialInfo->AddValue( "Ambient.r", (float32)DxLight.Ambient.r );
			SerialInfo->AddValue( "Ambient.g", (float32)DxLight.Ambient.g );
			SerialInfo->AddValue( "Ambient.b", (float32)DxLight.Ambient.b );
			SerialInfo->AddValue( "Ambient.a", (float32)DxLight.Ambient.a );

			SerialInfo->AddValue( "Position.x", (float32)DxLight.Position.x );
			SerialInfo->AddValue( "Position.y", (float32)DxLight.Position.y );
			SerialInfo->AddValue( "Position.z", (float32)DxLight.Position.z );

			SerialInfo->AddValue( "Direction.x", (float32)DxLight.Direction.x );
			SerialInfo->AddValue( "Direction.y", (float32)DxLight.Direction.y );
			SerialInfo->AddValue( "Direction.z", (float32)DxLight.Direction.z );

			SerialInfo->AddValue( "Range", (float32)DxLight.Range );
			SerialInfo->AddValue( "Falloff", (float32)DxLight.Falloff );
			SerialInfo->AddValue( "Attenuation0", (float32)DxLight.Attenuation0 );
			SerialInfo->AddValue( "Attenuation1", (float32)DxLight.Attenuation1 );
			SerialInfo->AddValue( "Attenuation2", (float32)DxLight.Attenuation2 );
			SerialInfo->AddValue( "Theta", (float32)DxLight.Theta );
			SerialInfo->AddValue( "Phi", (float32)DxLight.Phi );

			return;
		}

		void CLight::SetObjectData( System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext )
		{
			uint32 TempInt = 0;
			float32 TempFloat = 0;

			SerialInfo->GetValue( "Type", TempInt );
			DxLight.Type = (D3DLIGHTTYPE)TempInt;

			SerialInfo->GetValue( "Diffuse.r", TempFloat );
			DxLight.Diffuse.r = TempFloat;
			SerialInfo->GetValue( "Diffuse.g", TempFloat );
			DxLight.Diffuse.g = TempFloat;
			SerialInfo->GetValue( "Diffuse.b", TempFloat );
			DxLight.Diffuse.b = TempFloat;
			SerialInfo->GetValue( "Diffuse.a", TempFloat );
			DxLight.Diffuse.a = TempFloat;

			SerialInfo->GetValue( "Specular.r", TempFloat );
			DxLight.Specular.r = TempFloat;
			SerialInfo->GetValue( "Specular.g", TempFloat );
			DxLight.Specular.g = TempFloat;
			SerialInfo->GetValue( "Specular.b", TempFloat );
			DxLight.Specular.b = TempFloat;
			SerialInfo->GetValue( "Specular.a", TempFloat );
			DxLight.Specular.a = TempFloat;

			SerialInfo->GetValue( "Ambient.r", TempFloat );
			DxLight.Ambient.r = TempFloat;
			SerialInfo->GetValue( "Ambient.g", TempFloat );
			DxLight.Ambient.g = TempFloat;
			SerialInfo->GetValue( "Ambient.b", TempFloat );
			DxLight.Ambient.b = TempFloat;
			SerialInfo->GetValue( "Ambient.a", TempFloat );
			DxLight.Ambient.a = TempFloat;

			SerialInfo->GetValue( "Position.x", TempFloat );
			DxLight.Position.x = TempFloat;
			SerialInfo->GetValue( "Position.y", TempFloat );
			DxLight.Position.y = TempFloat;
			SerialInfo->GetValue( "Position.z", TempFloat );
			DxLight.Position.z = TempFloat;

			SerialInfo->GetValue( "Direction.x", TempFloat );
			DxLight.Direction.x = TempFloat;
			SerialInfo->GetValue( "Direction.y", TempFloat );
			DxLight.Direction.y = TempFloat;
			SerialInfo->GetValue( "Direction.z", TempFloat );
			DxLight.Direction.z = TempFloat;

			SerialInfo->GetValue( "Range", TempFloat );
			DxLight.Range = TempFloat;
			SerialInfo->GetValue( "Falloff", TempFloat );
			DxLight.Falloff = TempFloat;
			SerialInfo->GetValue( "Attenuation0", TempFloat );
			DxLight.Attenuation0 = TempFloat;
			SerialInfo->GetValue( "Attenuation1", TempFloat );
			DxLight.Attenuation1 = TempFloat;
			SerialInfo->GetValue( "Attenuation2", TempFloat );
			DxLight.Attenuation2 = TempFloat;
			SerialInfo->GetValue( "Theta", TempFloat );
			DxLight.Theta = TempFloat;
			SerialInfo->GetValue( "Phi", TempFloat );
			DxLight.Phi = TempFloat;

			return;
		}

		void CLight::FinalizeDeserialization( System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext )
		{
			//_BaseClass::FinalizeDeserialization( SerialInfo, sContext );
		}

		CLight &CLight::operator=( const D3DLIGHT9 &rhs )
		{
			memcpy( &DxLight, &rhs, sizeof( rhs ) );
			return *this;
		}

		CLight::operator D3DLIGHT9 &()
		{
			return DxLight;
		}

		CLight &CLight::operator=( const CLight &rhs )
		{
			memcpy( &DxLight, &rhs.DxLight, sizeof( rhs.DxLight ) );
			return *this;
		}
	}
}