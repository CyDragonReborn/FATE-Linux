/*
	Scene.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Ira Small
	10/20/2005
 */

#include "WT/stdafx.h"
#include "WT/System/Logging/Log.h"
#include "WT/Graphics/Scene.h"
#include "WT/Graphics/Level.h"
#include "WT/Graphics/Actor.h"

using namespace WT::System;

namespace WT
{
	namespace Graphics
	{
		namespace
		{
			//Tolerances used to compare planes - increase these if art
			//defects result in too many reflection planes.
			float32 const KNEpsilon = 0.05f; //Normal component tolerance
			float32 const KDEpsilon = 1.0f; //Distance tolerance

			int32 const KMirrorTextureSize = 512;

			D3DCOLOR const KDefaultBackgroundColor = D3DCOLOR_ARGB( 0xff, 0xff, 0xff, 0xff );
		}

		WT_DEFINE_CLASS( CScene );

		CScene::CScene()
			: m_pDevice( NULL )
			, m_BackgroundColor( KDefaultBackgroundColor )
			, m_ResourcesValid( false )
		{
		}

		CScene::CScene( LPDIRECT3DDEVICE9 pDevice )
			: m_pDevice( pDevice )
			, m_BackgroundColor( KDefaultBackgroundColor )
			, m_ResourcesValid( false )
		{
		}

		CScene::~CScene()
		{
			Clear();
		}

		/*virtual*/ void CScene::Clear()
		{
			CObjectCollection::Clear();

			m_ActorDefaultMotions.clear();
			m_ActorDefaultLooping.clear();
			m_ActorDefaultTimeIntoMotion.clear();

			m_pDevice = NULL;

			ClearMirrors();
			ClearShadowMaps();
		}

		void CScene::Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
		{
			if (!Context.ShouldRender( *this ))
			{
				return;
			}

			uint32 const RenderableCount = GetObjectCount<IRenderable>();
			for (uint32 I = 0; I < RenderableCount; ++I)
			{
				IRenderable *Renderable = GetObject<IRenderable>( I );
				assert( Renderable != NULL );
				Renderable->Render( pDevice, Context );
			}
		}

		void CScene::Update( float32 ElapsedTimeSeconds )
		{
			uint32 const UpdateableCount = GetObjectCount<IUpdateable>();
			for (uint32 I = 0; I < UpdateableCount; ++I)
			{
				IUpdateable *Updateable = GetObject<IUpdateable>( I );
				assert( Updateable != NULL );
				Updateable->Update( ElapsedTimeSeconds );
			}
		}

		uint32 WTCall CScene::GetClassFormatVersion() {return 5;}

		void CScene::GetObjectData( Serialization::CSerializationInfo* SerialInfo, Serialization::CSerializationContext* sContext ) const
		{
			GetClassType()->GetProperties()->SerializePropertyValues( this, SerialInfo, sContext );

			SerialInfo->AddValue( "ObjectCount", static_cast<uint32>( GetObjectCount<CObject>() ) );

			int32 ActorIndex = 0;

			uint32 const ObjectCount = GetObjectCount<CObject>();
			for (uint32 I = 0; I < ObjectCount; ++I)
			{
				CObject		&Object = *GetObject<CObject>( I );
				SerialInfo->AddValue( "Object", &Object );

				CActor* Actor = WT::DynamicCast<CActor>( &Object );
				if( Actor != NULL )
				{
					SerialInfo->AddValue( "DefaultMotionName", m_ActorDefaultMotions[ActorIndex] );
					SerialInfo->AddValue( "DefaultLooping", m_ActorDefaultLooping[ActorIndex] );
					SerialInfo->AddValue( "DefaultTimeIntoMotion", m_ActorDefaultTimeIntoMotion[ActorIndex] );
					++ActorIndex;
				}
			}

			SerialInfo->AddValue<uint32>( "BackgroundColor", m_BackgroundColor );
		
		}

		void CScene::SetObjectData( Serialization::CSerializationInfo* SerialInfo, Serialization::CSerializationContext* sContext )
		{
			Clear();

			//Initialize property values
			CRenderableImpl::InitPropertyValues();

			if (SerialInfo->GetObjectFormatVersion() >= 5)
			{
				GetClassType()->GetProperties()->DeserializePropertyValues( this, SerialInfo, sContext );
			} else if (SerialInfo->GetObjectFormatVersion() == 4) { //Obsolete version
				bool b; SerialInfo->GetValue( "Visible", b ); SetVisible( b );
			}
	
			m_pDevice = reinterpret_cast<LPDIRECT3DDEVICE9>( sContext->GetProperties()->GetValue<uint64>( "D3DDevice" ) );
			if (m_pDevice == NULL)
			{
				throw std::invalid_argument( "NULL D3DDevice in serialization context" );
			}

			int32 ActorIndex = 0;

			uint32 ObjectCount;
			SerialInfo->GetValue( "ObjectCount", ObjectCount );
			for( uint32 I = 0; I < ObjectCount; ++I )
			{
				if (SerialInfo->GetObjectFormatVersion() <= 2)
				{
					//Old version
					string ObjectName;
					SerialInfo->GetValue( "ObjectName", ObjectName );
				}

				CObject* pObject;
				SerialInfo->GetValue( "Object", pObject );
				AddObject( *pObject );

				CActor* Actor = WT::DynamicCast<CActor>( pObject );
				if( Actor != NULL )
				{
					string	DefaultMotionName;
					bool	DefaultLooping = false;
					float32	DefaultTimeIntoMotion = 0;

					SerialInfo->GetValue( "DefaultMotionName", DefaultMotionName );
					SerialInfo->GetValue( "DefaultLooping", DefaultLooping );
					SerialInfo->GetValue( "DefaultTimeIntoMotion", DefaultTimeIntoMotion );

					m_ActorDefaultMotions[ActorIndex] = DefaultMotionName;
					m_ActorDefaultLooping[ActorIndex] = DefaultLooping;
					m_ActorDefaultTimeIntoMotion[ActorIndex] = DefaultTimeIntoMotion;
					++ActorIndex;
				}

				CLevel* Level = WT::DynamicCast<CLevel>( pObject );
				if (Level != NULL)
				{
					Level->SetScene( this );
				}
			}

			if (SerialInfo->GetObjectFormatVersion() >= 2)
			{
				SerialInfo->GetValue<uint32>( "BackgroundColor", m_BackgroundColor );
			} else {
				//Old version
				m_BackgroundColor = KDefaultBackgroundColor;
			}
		}

		void CScene::FinalizeDeserialization( Serialization::CSerializationInfo* SerialInfo, Serialization::CSerializationContext* sContext )
		{
			//_BaseClass::FinalizeDeserialization( SerialInfo, sContext );

			uint32 ActorCount = GetObjectCount<CActor>();
			for( uint32 I = 0; I < ActorCount; I++ )
			{
				CActor &Actor = *GetObject<CActor>( I );

				for( uint32 J = 0; J < Actor.GetMotionCount(); J++ )
				{
					MotionInfo MI = Actor.GetMotion( J );
					
					if( MI.hMotion != NULL )
					{
						string MotionName = MI.hMotion->GetName();

						if( MotionName == GetActorDefaultMotion( I ) )
						{
							Actor.PlayMotion( J, GetActorTimeIntoMotion( I ) );
						}
					}
				}
			}
		}

		/*virtual*/ void CScene::AddObject( CObject& Object )
		{
			CObjectCollection::AddObject( Object );

			CActor* pActor = WT::DynamicCast<CActor>( &Object );
			if (pActor != NULL)
			{
				m_ActorDefaultMotions.push_back( "" );
				m_ActorDefaultLooping.push_back( false );
				m_ActorDefaultTimeIntoMotion.push_back( 0 );
			}
		}

		void CScene::SetActorDefaultMotion( uint32 Index, string MotionName )
		{
			if( Index >= GetObjectCount<CActor>() )
			{
				throw CException( "CScene::SetActorDefaultMotion(), Non-existent Index" );
			}

			m_ActorDefaultMotions[ Index ] = MotionName;
		}

		string CScene::GetActorDefaultMotion( uint32 Index ) const
		{			
			if( Index >= GetObjectCount<CActor>() )
			{
				throw CException( "CScene::GetActorDefaultMotion(), Non-existent Index" );
			}

			return m_ActorDefaultMotions[ Index ];
		}

		void CScene::SetActorDefaultLooping( uint32 Index, bool Looping )
		{			
			if( Index >= GetObjectCount<CActor>() )
			{
				throw CException( "CScene::SetActorDefaultLooping(), Non-existent Index" );
			}

			m_ActorDefaultLooping[ Index ] = Looping;
		}

		bool CScene::GetActorDefaultLooping( uint32 Index ) const
		{
			if( Index >= GetObjectCount<CActor>() )
			{
				throw CException( "CScene::SetActorDefaultLooping(), Non-existent Index" );
			}

			return m_ActorDefaultLooping[ Index ];
		}

		void CScene::SetActorTimeIntoMotion( uint32 Index, float32 TimeIntoMotion )
		{
			if( Index >= GetObjectCount<CActor>() )
			{
				throw CException( "CScene::SetActorTimeIntoMotion(), Non-existent Index" );
			}

			m_ActorDefaultTimeIntoMotion[ Index ] = TimeIntoMotion;
		}

		float32 CScene::GetActorTimeIntoMotion( uint32 Index ) const
		{
			if( Index >= GetObjectCount<CActor>() )
			{
				throw CException( "CScene::GetActorTimeIntoMotion(), Non-existent Index" );
			}

			return m_ActorDefaultTimeIntoMotion[ Index ];
		}

		void CScene::RenderToTexture( CRenderToTexture const& RenderToTexture )
		{
			m_RenderToTextureEvent( RenderToTexture );
		}

		void CScene::RenderToTextures( CCamera const& RealCamera )
		{
			int32 const MirrorCount = static_cast<int32>( m_hMirrors.size() );
			for (int32 i = 0; i < MirrorCount; ++i)
			{
				m_hMirrors[i]->Update( RealCamera );
			}

			int32 const ShadowMapCount = static_cast<int32>( m_hShadowMaps.size() );
			for (int32 i = 0; i < ShadowMapCount; ++i)
			{
				m_hShadowMaps[i]->Update( RealCamera, *this );
			}
		}

		void CScene::AddShadowMap( CShadowMap* pShadowMap )
		{
			if (pShadowMap == NULL)
			{
				throw std::invalid_argument( "shadow map" );
			}

			m_hShadowMaps.push_back( pShadowMap );
			pShadowMap->OnUpdate() += CRenderToTexture::UpdateCallback::Create( *this, &CScene::RenderToTexture );
		}


		void CScene::ClearShadowMaps()
		{
			int32 const ShadowMapCount = static_cast<int32>( m_hShadowMaps.size() );
			for (int32 i = 0; i < ShadowMapCount; ++i)
			{
				m_hShadowMaps[i]->OnUpdate().Clear();
			}
			m_hShadowMaps.clear();
		}

		CMirror* CScene::AddMirror( CPlane const& Plane )
		{
			//Find or create a Mirror object that will be used to update a reflection texture
			//for this plane.
			CMirror* pMirror = NULL;
			int32 const MirrorCount = static_cast<int32>( m_hMirrors.size() );
			for (int32 i = 0; i < MirrorCount; ++i)
			{
				CMirror* pExistingMirror = m_hMirrors[i];

				assert( pExistingMirror != NULL );

				if (pExistingMirror->GetPlane().Compare( Plane, KNEpsilon, KDEpsilon ))
				{
					pMirror = pExistingMirror;
					break;
				}
			}

			//If no mirror exists for this plane, then create it.
			if (pMirror == NULL)
			{
				HMirror hMirror( new CMirror( 
					m_pDevice, KMirrorTextureSize, m_BackgroundColor, Plane ) );
				m_hMirrors.push_back( hMirror );

				pMirror = hMirror;
				pMirror->OnUpdate() += CRenderToTexture::UpdateCallback::Create( *this, &CScene::RenderToTexture );

				LOG_GREEN( "Info: CScene creating CMirror for plane %fx + %fy + %fz + %f = 0\n",
					Plane.Normal.x, Plane.Normal.y, Plane.Normal.z, Plane.Distance );
			}

			return pMirror;
		}

		void CScene::ClearMirrors()
		{
			int32 const MirrorCount = static_cast<int32>( m_hMirrors.size() );
			for (int32 i = 0; i < MirrorCount; ++i)
			{
				m_hMirrors[i]->OnUpdate().Clear();
			}
			m_hMirrors.clear();
		}

		void CScene::SetBackgroundColor( D3DCOLOR const& BackgroundColor )
		{
			m_BackgroundColor = BackgroundColor;

			int32 const MirrorCount = static_cast<int32>( m_hMirrors.size() );
			for (int32 i = 0; i < MirrorCount; ++i)
			{
				m_hMirrors[i]->SetBackgroundColor( BackgroundColor );
			}
		}

		void CScene::InvalidateDeviceObjects()
		{
			m_ResourcesValid = false;

			int32 const MirrorCount = static_cast<int32>( m_hMirrors.size() );
			for (int32 i = 0; i < MirrorCount; ++i)
			{
				m_hMirrors[i]->InvalidateDeviceObjects();
			}

			int32 const ShadowMapCount = static_cast<int32>( m_hShadowMaps.size() );
			for (int32 i = 0; i < ShadowMapCount; ++i)
			{
				m_hShadowMaps[i]->InvalidateDeviceObjects();
			}

			uint32 const DeviceObjectCount = GetObjectCount<IDeviceObject>();
			for (uint32 I = 0; I < DeviceObjectCount; ++I)
			{
				IDeviceObject *pDeviceObject = GetObject<IDeviceObject>( I );
				assert( pDeviceObject != NULL );
				pDeviceObject->InvalidateDeviceObjects();
			}
		}

		void CScene::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
		{
			int32 const MirrorCount = static_cast<int32>( m_hMirrors.size() );
			for (int32 i = 0; i < MirrorCount; ++i)
			{
				m_hMirrors[i]->RestoreDeviceObjects( pDevice );
			}

			int32 const ShadowMapCount = static_cast<int32>( m_hShadowMaps.size() );
			for (int32 i = 0; i < ShadowMapCount; ++i)
			{
				m_hShadowMaps[i]->RestoreDeviceObjects( pDevice );
			}

			uint32 const DeviceObjectCount = GetObjectCount<IDeviceObject>();
			for (uint32 I = 0; I < DeviceObjectCount; ++I)
			{
				IDeviceObject *pDeviceObject = GetObject<IDeviceObject>( I );
				assert( pDeviceObject != NULL );
				pDeviceObject->RestoreDeviceObjects( pDevice );
			}

			m_ResourcesValid = true;
		}

		bool CScene::IsValid() const
		{
			return m_ResourcesValid;
		}

	} //namespace Graphics
} //namespace WT
