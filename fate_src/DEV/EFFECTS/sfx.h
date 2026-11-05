/*
	sfx.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _SFX_H
#define _SFX_H

#include "d3dx8_shim.h"


#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/cullingbounds.h"
#include "../UTILITIES/frustum.h"
#include "../UTILITIES/settings.h"

#include "../MEDIA/refmanager.h"
#include "../GAMEOBJECTS/genericmodel.h"

#include "../PARTICLES/particlemanager.h"
#include "../GLOWS/glowmanager.h"
#include "../GLOWS/glowsystem.h"

#include "../CHARACTER/character.h"
#include "../ITEM/spelltemplate.h"

// sfx animation for spells/etc.

class CSFX
{

public:

	CSFX( CSFXDescription& pDescription,		// description of effect
		  LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
		  CRefManager& pRefManager,				// pointer to the reference manager, for tracking media
		  CSettings& pSettings,					// pointer to settings manager
		  CParticleManager& pParticleManager,	// particle manager
		  CGlowManager& pGlowManager,			// glow manager
		  CCharacter* pOwner,					// owning character
		  CCharacter* pTarget,					// target character
		  const D3DXVECTOR3& Position,			// worldspace position
		  const D3DXVECTOR3& TargetPosition,	// worldspace position
		  CSpellDescription* pPayload = NULL )	:		// payload spell
													m_Time( pDescription.m_Duration ),
													m_Looping( pDescription.m_Looping ),
													m_Type( pDescription.m_Type ),
													m_pCullingBounds( NULL ),
													m_pGenericModel( NULL ),
													m_Path( pDescription.m_Path ),
													m_Scale( 1 ),
													m_pOwner( pOwner ),
													m_pTarget( pTarget ),
													m_Position( Position ),
													m_TargetPosition( TargetPosition ),
													m_pParticleManager( pParticleManager ),
													m_pGlowManager( pGlowManager ),
													m_AnimationComplete( kFalse ),
													m_pPayloadSpell( pPayload ),
													m_Speed ( pDescription.m_Speed ),
													m_TurnRate ( pDescription.m_TurnRate )
	{
		ZeroMemory ( &m_Transformation, sizeof ( m_Transformation ));

		m_pGenericModel = new CGenericModel( pD3DDevice,
											 pRefManager,
											 pSettings,
											 pDescription.m_Path,
											 "" );

		switch( m_Type )
		{
		case KSFXFollowTarget :
			if( pTarget != NULL )
			{
				m_Scale = pTarget->Scale();
			}
			break;
		case KSFXFollowOwner :
			if( pOwner != NULL )
			{
				m_Scale = pOwner->Scale();
			}
			break;
		case KSFXStationaryTarget :
			if( pTarget != NULL )
			{
				m_Position = pTarget->Position();
			}
			break;
		case KSFXStationaryOwner :
			if( pOwner != NULL )
			{
				m_Position = pOwner->Position();
			}
			break;
		}

	
		D3DXMATRIX ScaleMatrix;

		D3DXMatrixScaling( &ScaleMatrix, 
						   m_Scale, 
						   m_Scale, 
						   m_Scale );

		D3DXMatrixTranslation( &m_Transformation, m_Position.x, m_Position.y, m_Position.z );

		if(	m_Type == KSFXProjectileCharacter ||
			m_Type == KSFXProjectileCharacterErratic )
		{
		  m_TargetPosition -= m_Position;
		  D3DXVec3Normalize( &m_TargetPosition, &m_TargetPosition );
		  m_TargetPosition *= 1000;
		  m_TargetPosition += m_Position;
		}

		if( m_Type == KSFXProjectileLocation ||
			m_Type == KSFXProjectileCharacter ||
			m_Type == KSFXProjectileLocationErratic ||
			m_Type == KSFXProjectileCharacterErratic )
		{
			/*if( m_pOwner != NULL )
			{
				m_Transformation = ScaleMatrix * m_pOwner->Orientation() * m_Transformation;
			}
			else*/
			{
				D3DXMATRIX Orientation;
				D3DXMatrixLookAtLH( &Orientation, &m_Position, &m_TargetPosition, &D3DXVECTOR3( 0, 1, 0 ) );
				D3DXMatrixTranspose( &Orientation, &Orientation );
				Orientation._41 = 0;
				Orientation._42 = 0;
				Orientation._43 = 0;
				Orientation._14 = 0;
				Orientation._24 = 0;
				Orientation._34 = 0;
				m_Transformation = ScaleMatrix * Orientation * m_Transformation;

			}

			if( m_Type == KSFXProjectileLocationErratic ||
				m_Type == KSFXProjectileCharacterErratic )
			{
				D3DXMATRIX RotationMatrix;
				// perform rotation about the Y axis
				D3DXMatrixRotationY( &RotationMatrix, (float32)KDeg2Rad * RandomBetween( -60, 60 ) );

				m_Transformation._41 = 0;
				m_Transformation._42 = 0;
				m_Transformation._43 = 0;
				m_Transformation *= RotationMatrix;
				m_Transformation._41 = m_Position.x;
				m_Transformation._42 = m_Position.y;
				m_Transformation._43 = m_Position.z;
			}
		}
		else
		{
			m_Transformation = ScaleMatrix * m_Transformation;
		}

		m_pCullingBounds = new CCullingBounds( D3DXVECTOR3( -10, -10, -10 ),
											   D3DXVECTOR3( 10, 10, 10 ) );
		m_pCullingBounds->Update( m_Transformation );
		m_pGenericModel->PlayAnimation( 0, m_Looping );
		FindParticles();
	}



	~CSFX( void )
	{
		ClearParticles();
		DELETE_SAFELY( m_pGenericModel );
		DELETE_SAFELY( m_pCullingBounds );
	}

	void ClearParticles( void )
	{
		m_ParticlePosition.clear();
		m_ParticleOrientation.clear();
		m_ParticleType.clear();

		m_TagParticleIndex.clear();
		m_TagParticleType.clear();

		for( uint32 i = 0; i < m_pGlowList.size(); i++ )
		{
			m_pGlowList[i]->m_RequestingRemoval = kTrue;
		}
		for( int32 i = 0; i < m_pModelGlowList.size(); i++ )
		{
			m_pModelGlowList[i]->m_RequestingRemoval = kTrue;
		}
		m_pGlowList.clear();
		m_pModelGlowList.clear();
		m_GlowIndex.clear();
		m_ModelGlowIndex.clear();

	} // CCharacter::ClearParticles()

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	const D3DXMATRIX&		Transformation( void )		{	return m_Transformation;	};

	// notify us if a character was deleted, in case we have them targeted
	void NotifyOfDeletion( CCharacter* pCharacter )	// character being deleted
	{
		if( m_pOwner == pCharacter )
		{
			m_pOwner = NULL;
		}
		if( m_pTarget == pCharacter )
		{
			m_pTarget = NULL;
		}
	}


	void	Update( CLevel& pLevel,
					float32 TimeElapsed )	// time elapsed in seconds
	{
		switch( m_Type )
		{
		case KSFXFollowOwner :
			if( m_pOwner != NULL )
			{
				m_Transformation._41 = m_pOwner->Position().x;
				m_Transformation._42 = m_pOwner->Position().y;
				m_Transformation._43 = m_pOwner->Position().z;
				if( m_pOwner->AIState() == KAIInTown )
				{
					m_Transformation._42 -= 10000;
				}
				m_pCullingBounds->Update( m_Transformation );
				if( !m_pOwner->Alive() )
				{
					m_AnimationComplete = kTrue;
				}
			}
			else
			{
				m_AnimationComplete = kTrue;
			}
			break;
		case KSFXFollowTarget :
			if( m_pTarget != NULL )
			{
				m_Transformation._41 = m_pTarget->Position().x;
				m_Transformation._42 = m_pTarget->Position().y;
				m_Transformation._43 = m_pTarget->Position().z;
				if( m_pTarget->AIState() == KAIInTown )
				{
					m_Transformation._42 -= 10000;
				}
				m_pCullingBounds->Update( m_Transformation );
				if( !m_pTarget->Alive() )
				{
					m_AnimationComplete = kTrue;
				}
			}
			else
			{
				m_AnimationComplete = kTrue;
			}
			break;

		}


		if( ( m_Type == KSFXProjectileCharacter ||
			  m_Type == KSFXProjectileLocation ||
			  m_Type == KSFXProjectileCharacterErratic ||
			  m_Type == KSFXProjectileLocationErratic ) &&
			  m_Time > 0 )
		{
			float32 DetonationRadius( 4 );
			if( m_pTarget != NULL )
			{
				m_TargetPosition = m_pTarget->Position();
				DetonationRadius = m_pTarget->CollisionRadius() * 2;
			}



			D3DXVECTOR3 Delta( m_Transformation._31, 0, m_Transformation._33 );
			D3DXVec3Normalize( &Delta, &Delta );

			Delta *= m_Speed * TimeElapsed;
			
			// TODO:: Collision with walls

			D3DXVECTOR3 TargetPosition = m_Position + Delta;
			D3DXVECTOR3 StartPosition = m_Position;
			TargetPosition.y += 3;
			StartPosition.y += 3;

			D3DXVECTOR3 Impact;
			D3DXVECTOR3 Normal;
			uint32 Material;
			D3DXVECTOR3 Color;
			CCharacter* pCharacter = NULL;

			bool DeliverPayload( kFalse );
			if( pLevel.RayCollision( StartPosition, TargetPosition, Impact, Normal, Material, Color, NULL, kFalse ) )
			{
				m_Time = 0;
				if( m_Type == KSFXProjectileLocation ||
					m_Type == KSFXProjectileLocationErratic )
				{
					DeliverPayload = kTrue;
				}
			}
			if( pLevel.CharacterRayCollision( StartPosition, TargetPosition, Impact, Normal, Material, &pCharacter ) )
			{
				if( pCharacter != NULL &&
					m_pPayloadSpell != NULL )
				{
					if( m_pOwner != NULL )
					{
						DeliverPayload = kTrue;
					}

				}
			}

			m_Position = m_Position + Delta;

			Delta = m_TargetPosition - m_Position;

			Delta.y = 0;
			if( ( m_Type == KSFXProjectileLocation ||
				  m_Type == KSFXProjectileLocationErratic ) &&
				  D3DXVec3Length( &Delta ) <= DetonationRadius )
			{
				if( m_pPayloadSpell != NULL )
				{
					if( m_pOwner != NULL )
					{
						DeliverPayload = kTrue;
					}
				}
			}

			if( m_pTarget != NULL &&
				D3DXVec3Length( &Delta ) <= DetonationRadius )
			{
				m_Time = 0;
				if( m_pPayloadSpell != NULL )
				{
					if( m_pOwner != NULL )
					{
						pCharacter = m_pTarget;
						DeliverPayload = kTrue;
					}
				}
			}

			if( m_Time > 0 &&
				m_Time - TimeElapsed <= 0 &&
				( m_Type == KSFXProjectileLocation ||
				  m_Type == KSFXProjectileLocationErratic ) )
			{
				DeliverPayload = kTrue;
			}


			if( DeliverPayload )
			{
				EAlignment TargetAlignment = m_pPayloadSpell->TargetAlignment();
				if( m_pOwner != NULL &&
					m_pOwner->Alignment() == KEvil )
				{
					if( TargetAlignment == KGood )
					{
						TargetAlignment = KEvil;
					}
					else if( TargetAlignment == KEvil )
					{
						TargetAlignment = KGood;
					}
				}			
				if( m_pPayloadSpell->Target() == KTargetArea ||
					m_pPayloadSpell->Target() == KTargetProjectile )
				{
					if( pCharacter == NULL ||
						( TargetAlignment == KAllAlignments ||
						  pCharacter->Alignment() == TargetAlignment ) )
					{
						m_Time = 0;
						D3DXVECTOR3 TargetPosition( m_Position );
						if( pCharacter != NULL )
						{
							TargetPosition = pCharacter->Position();
						}
						std::vector< CCharacter* > CharacterList;
						pLevel.FindCharactersInArea( TargetPosition, 
													m_pPayloadSpell->Range(),
													CharacterList );
						m_pPayloadSpell->PerformPayloadFX( pLevel, m_pOwner, TargetPosition );

						for( uint32 i = 0; i < CharacterList.size(); i++ )
						{
							if( CharacterList[i]->Alive() &&
								( CharacterList[i]->Alignment() == TargetAlignment ||
								TargetAlignment == KAllAlignments ) )
							{
								if( m_pPayloadSpell->CanCastOnTarget( m_pOwner, CharacterList[i] ) )
								{
									if( CharacterList[i] == pCharacter )
									{
										m_pPayloadSpell->CastPayload( pLevel, m_pOwner, CharacterList[i], kTrue );
									}
									else
									{
										m_pPayloadSpell->CastPayload( pLevel, m_pOwner, CharacterList[i], kTrue );
									}
								}
								else
								{
									pLevel.AddEventText( m_pOwner,
														CharacterList[i],
														CharacterList[i]->Position() + D3DXVECTOR3( 0, 9, 0 ),
														"Failed!",
														.75f,
														kFalse,
														1 );
								}

								if( m_pOwner != NULL &&
									CharacterList[i]->Alignment() != m_pOwner->Alignment() )
								{
									CharacterList[i]->RespondToAttack( m_pOwner );
								}

							}
						}
					}
				}
				else if( pCharacter != NULL )
				{
					if( TargetAlignment == KAllAlignments ||
						pCharacter->Alignment() == TargetAlignment )
					{
						m_Time = 0;
						// deliver payload!
						if( m_pPayloadSpell->CanCastOnTarget( m_pOwner, pCharacter ) )
						{
							m_pPayloadSpell->CastPayload( pLevel, m_pOwner, pCharacter, kTrue );
						}
						else
						{
							pLevel.AddEventText( m_pOwner,
												pCharacter,
												pCharacter->Position() + D3DXVECTOR3( 0, 9, 0 ),
												"Failed!",
												.75f,
												kFalse,
												1 );
						}
						if( m_pOwner != NULL &&
							pCharacter->Alignment() != m_pOwner->Alignment() )
						{
							pCharacter->RespondToAttack( m_pOwner );
						}
					}
				}
			}

			D3DXVec3Normalize( &Delta, &Delta );
			TurnToward( Delta, TimeElapsed );
			m_pCullingBounds->Update( m_Transformation );


		}

		m_pGenericModel->UpdateAnimation( TimeElapsed );
		if( m_Time > 0 )
		{
			m_Time -= TimeElapsed;
			if( m_Time <= 0 )
			{
				m_Time = 0;
			}
		}

		if( !m_Looping &&
			( m_pGenericModel->ActiveAnimations() == 0 ||
			 !m_pGenericModel->AnimationPlaying( 0 ) ) )
		{
			m_AnimationComplete = kTrue;
		}
		EmitParticles();
	}
	void TurnToward( const D3DXVECTOR3& TargetDirection,	// direction to face
					 float32 TimeElapsed )					// time elapsed in seconds
	{
		D3DXMATRIX  RotationMatrix;

		D3DXVECTOR3 Forward( m_Transformation._31, 0, m_Transformation._33 );
		D3DXVec3Normalize( &Forward, &Forward );
		
		// find the angle delta between direction currently facing, 
		// and desired direction
		float32 CurrentAngle = (float32)KRad2Deg * ( (float32)atan2( Forward.x, Forward.z ) );
		float32 TargetAngle  = (float32)KRad2Deg * ( (float32)atan2( TargetDirection.x, TargetDirection.z ) );

		TargetAngle -= CurrentAngle;

		// make sure this is halved into left/right turns
		if (TargetAngle > 180)
		{
			TargetAngle = TargetAngle - 360;
		}
		if (TargetAngle < -180) 
		{
			TargetAngle = TargetAngle + 360;
		}
		
		float32 DegreesToTurn( 0 );


		// turn faster the further from the destination angle we are
		if( TargetAngle < 0 )
		{
			float32 Turn = -m_TurnRate * fabs( TargetAngle );
			if( Turn < -KMaxTurnRate )
			{
				Turn = -KMaxTurnRate;
			}
			DegreesToTurn = Turn * TimeElapsed;
			if( DegreesToTurn < TargetAngle )
			{
				DegreesToTurn = TargetAngle;
			}
		}
		else if( TargetAngle > 0 )
		{
			float32 Turn = m_TurnRate * fabs( TargetAngle );
			if( Turn > KMaxTurnRate )
			{
				Turn = KMaxTurnRate;
			}
			DegreesToTurn = Turn * TimeElapsed;
			if( DegreesToTurn > TargetAngle )
			{
				DegreesToTurn = TargetAngle;
			}

		}

		// perform rotation about the Y axis
		D3DXMatrixRotationY( &RotationMatrix, (float32)KDeg2Rad * DegreesToTurn );

		m_Transformation._41 = 0;
		m_Transformation._42 = 0;
		m_Transformation._43 = 0;
		m_Transformation *= RotationMatrix;
		m_Transformation._41 = m_Position.x;
		m_Transformation._42 = m_Position.y;
		m_Transformation._43 = m_Position.z;

	}


	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
					const D3DXMATRIX& ViewportMatrix )	// viewport matrix
	{

		m_pGenericModel->Render( pD3DDevice, m_Transformation * ViewportMatrix );
	}

	void				FindParticles( void )
	{
		for( uint32 i = 0; i < m_pGenericModel->GetPointCount(); i++ )
		{
			CDataPoint& Point = m_pGenericModel->GetPoint( i );
			std::string Name = StringUpper( Point.Name() );
			if( Name.length() >= 8 && Name.substr( 0, 8 ) == "PARTICLE" )
			{
				int32 ParticleIndex = m_pParticleManager.GetParticleType( Point.UserData() );
				if( ParticleIndex != -1 )
				{
					D3DXVECTOR3 Position = Point.Position();
					m_ParticlePosition.push_back( Position );
					m_ParticleOrientation.push_back( Point.Orientation() );
					m_ParticleType.push_back( ParticleIndex );
				}
			}
			else if( Name.length() >= 4 && Name.substr( 0, 4 ) == "GLOW" )
			{
				int32 GlowIndex = m_pGlowManager.GetGlowType( Point.UserData() );
				if( GlowIndex != -1 )
				{
					m_pModelGlowList.push_back( m_pGlowManager.AddGlow( GlowIndex, Point.Position() ) );					
					m_ModelGlowIndex.push_back( i );

				}
			}
		}


		for( int32 i = 0; i < m_pGenericModel->GetTagCount(); i++ )
		{
			std::string Name = StringUpper( m_pGenericModel->GetTagName( i ) );
			if( Name.length() >= 8 && Name.substr( 0, 8 ) == "PARTICLE" )
			{
				int32 ParticleIndex = m_pParticleManager.GetParticleType( m_pGenericModel->GetTagUserData( i )  );
				if( ParticleIndex != -1 )
				{
					m_TagParticleIndex.push_back( i );
					m_TagParticleType.push_back( ParticleIndex );
				}
			}
			else if( Name.length() >= 4 && Name.substr( 0, 4 ) == "GLOW" )
			{
				int32 GlowIndex = m_pGlowManager.GetGlowType( m_pGenericModel->GetTagUserData( i ) );
				if( GlowIndex != -1 )
				{
					m_pGlowList.push_back( m_pGlowManager.AddGlow( GlowIndex, D3DXVECTOR3( 0, -10000, 0 ) ) );					
					m_GlowIndex.push_back( i );
				}
			}
		}

	}

	void				EmitParticles( void )
	{
		for( uint32 i = 0; i < m_ParticleType.size(); i++ )
		{
			D3DXVECTOR3 Position;
			D3DXVec3TransformCoord( &Position,  &m_ParticlePosition[i], &m_Transformation );
			D3DXMATRIX Transform = m_ParticleOrientation[i];
			Transform._41 = Position.x;
			Transform._42 = Position.y;
			Transform._43 = Position.z;
			m_pParticleManager.AddParticle( m_ParticleType[i], Transform );
		}

		for( int32 i = 0; i < m_TagParticleType.size(); i++ )
		{
			D3DXMATRIX Transform = m_pGenericModel->GetTransformedTag( m_TagParticleIndex[i] );
			Transform *= m_Transformation;
			m_pParticleManager.AddParticle( m_TagParticleType[i], Transform );
		}

		for( int32 i = 0; i < m_pGlowList.size(); i++ )
		{
			D3DXVECTOR3 Position = m_pGenericModel->GetTagPosition( m_GlowIndex[i] );
			D3DXVec3TransformCoord( &Position, &Position, &m_Transformation );
			m_pGlowList[i]->m_Position = Position;
		}
		for( int32 i = 0; i < m_pModelGlowList.size(); i++ )
		{
			D3DXVECTOR3 Position = m_pGenericModel->GetPoint( m_ModelGlowIndex[i] ).Position();
			D3DXVec3TransformCoord( &Position, &Position, &m_Transformation );
			m_pModelGlowList[i]->m_Position = Position;
		}
	}

	void		SetPayloadSpell( CSpellDescription* pSpell )			{	m_pPayloadSpell = pSpell;			};

	void		SetOwner( CCharacter* pCharacter )						{	m_pOwner = pCharacter;				};

	void		SetTarget( CCharacter* pCharacter )						{	m_pTarget = pCharacter;				};

	void		SetTargetPosition( const D3DXVECTOR3& Position )		{	m_TargetPosition = Position;		};

	void		SetTime( float32 Time )									{	m_Time = Time;					};

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	ESFXType	Type( void )			{	return m_Type;		};

	bool		Complete( void )		{	return m_Time == 0 || m_AnimationComplete;	};

	bool		InFrustum( CFrustum& pFrustum )			// frustum to cull to
	{
		// if we're not in the frustum, don't render at all
		if( pFrustum.Intersects( *m_pCullingBounds ) == KFrustumOutside )
		{
			return kFalse;
		}

		return kTrue;
	} 

	std::string&	Path( void )		{	return m_Path;		};

	CCharacter*		Target( void )		{	return m_pTarget;	};
	CCharacter*		Owner( void )		{	return m_pOwner;	};

private:

	CSpellDescription* m_pPayloadSpell;

	CCharacter*	m_pOwner;
	CCharacter*	m_pTarget;

	float32		m_Time;
	D3DXMATRIX	m_Transformation;

	D3DXVECTOR3	m_Position;
	D3DXVECTOR3	m_TargetPosition;

	float32		m_Speed;
	float32		m_TurnRate;
	float32		m_Scale;

	CCullingBounds*		m_pCullingBounds;

	CGenericModel*		m_pGenericModel;

	bool		m_AnimationComplete;

	bool		m_Looping;


	ESFXType	m_Type;

	std::vector< D3DXVECTOR3 >	m_ParticlePosition;
	std::vector< D3DXMATRIX >	m_ParticleOrientation;
	std::vector< int32 >		m_ParticleType;

	std::vector< uint32 >		m_TagParticleIndex;
	std::vector< int32 >		m_TagParticleType;

	CParticleManager&	m_pParticleManager;
	CGlowManager&		m_pGlowManager;

	std::vector< CGlow* >		m_pGlowList;
	std::vector< CGlow* >		m_pModelGlowList;
	std::vector< uint32 >		m_GlowIndex;
	std::vector< uint32 >		m_ModelGlowIndex;

	std::string					m_Path;

};
#endif