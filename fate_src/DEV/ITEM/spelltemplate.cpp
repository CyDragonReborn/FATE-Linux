/*
	spelltemplate.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/datafile.h"
#include "../UTILITIES/settings.h"

#include "../MEDIA/refmanager.h"

#include "../PARTICLES/particlemanager.h"

#include "effect.h"

#include "spelltemplate.h"

CSpellTemplate::CSpellTemplate( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
								CRefManager& pRefManager,		// pointer to the reference manager, for tracking media
								CSettings& pSettings,			// settings( for audio management )
								CParticleManager& pParticleManager,		// particle manager
								CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
								const std::string& Path ) 		// path to template
{
	ParseTemplate( pD3DDevice, pRefManager, pSettings, pParticleManager, pPositionalAudioManager, Path );
} // CSpellTemplate::CSpellTemplate()

CSpellTemplate::~CSpellTemplate( void )
{
	for( uint32 i = 0; i < m_pSpellDescription.size(); i++ )
	{
		DELETE_SAFELY( m_pSpellDescription[i] );
	}
	m_pSpellDescription.clear();

	// the rest do not have to be deleted, as they were pointers
	// to objects in the base spell description list
	for( int32 i = 0; i < KMagicSpheres; i++ )
	{
		m_pSpellsBySphere[i].clear();
	}
	
} // CSpellTemplate::~CSpellTemplate()


CSpellDescription* CSpellTemplate::SpellDescription( std::string Name ) // name of spell to retrieve
{

	Name = StringUpper( Name );
	for( uint32 i = 0; i < m_pSpellDescription.size(); i++ )
	{
		if( StringUpper( m_pSpellDescription[i]->Name() ) == Name )
		{
			return m_pSpellDescription[i];
		}
	}
	return NULL;
} // CSpellTemplate::SpellDescription()

void CSpellTemplate::ParseTemplate( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
									CRefManager& pRefManager,		// pointer to the reference manager, for tracking media
									CSettings& pSettings,			// settings( for audio management )
									CParticleManager& pParticleManager,		// particle manager
									CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
									const std::string& Path )		// path to template
{
	std::string BasePath = GetLocalPath();
	CDataFile* pDataFile = new CDataFile( Path );


	uint32 Spells = pDataFile->DataGroups( "SPELL" );

	for( uint32 w = 0; w < Spells; w++ )
	{
		std::string			Name( "" );

		float32				CastTime( 0 );

		float32				Range( 0 );

		EMagicSphere		Sphere( KMagicCharm );

		int32				ManaCost( 10 );

		ETarget				Target( KTargetUser );

		EAlignment			TargetAlignment( KAllAlignments );

		CMaterial*			pIconMaterial = NULL;

		// parse tile widths
		CDataGroup* pMasterGroup = pDataFile->GetDataGroup( "SPELL", w );


		CDataGroup* pSubGroup = pMasterGroup->GetSubDataGroup( "SPHERE" );
		if( pSubGroup != NULL )
		{
			std::string TypeVal = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 i = 0; i < KMagicSpheres; i++ )
			{
				if( KMagicSphereName[i] == TypeVal )
				{
					Sphere = (EMagicSphere)i;
				}
			}
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "TARGET" );
		if( pSubGroup != NULL )
		{
			std::string TypeVal = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 i = 0; i < KTargets; i++ )
			{
				if( KTargetNames[i] == TypeVal )
				{
					Target = (ETarget)i;
					break;
				}
			}
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "TARGET_ALIGNMENT" );
		if( pSubGroup != NULL )
		{
			std::string TypeVal = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 i = 0; i < KAlignments; i++ )
			{
				if( KAlignmentName[i] == TypeVal )
				{
					TargetAlignment = (EAlignment)i;
				}
			}
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "ICON" );
		if( pSubGroup != NULL )
		{
			std::string IconPath( pSubGroup->Item( 0 ) );
			std::string AlphaPath( "" );
			if( pSubGroup->Items() > 1 )
			{
				AlphaPath = pSubGroup->Item( 1 );
				pIconMaterial = pRefManager.AddMaterialAlpha( pD3DDevice, IconPath, AlphaPath );
			}
			else
			{
				pIconMaterial = pRefManager.AddMaterial( pD3DDevice, IconPath );
			}
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "NAME" );
		if( pSubGroup != NULL )
		{
			Name = pSubGroup->Item( 0 );
		}


		pSubGroup = pMasterGroup->GetSubDataGroup( "MANA" );
		if( pSubGroup != NULL )
		{
			ManaCost = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "CAST_TIME" );
		if( pSubGroup != NULL )
		{
			CastTime = (float32)atof( pSubGroup->Item( 0 ).c_str() );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "RANGE" );
		if( pSubGroup != NULL )
		{
			Range = (float32)atof( pSubGroup->Item( 0 ).c_str() );
		}

		CSpellDescription* pSpell = new CSpellDescription( pRefManager,
														   pSettings,
														   pPositionalAudioManager,
														   Name,
														   pIconMaterial,
														   Sphere,
														   Target,
														   TargetAlignment,
														   Range,
														   ManaCost,
														   CastTime );

		m_pSpellDescription.push_back( pSpell );

		m_pSpellsBySphere[Sphere].push_back( pSpell );

		uint32 Particles = pMasterGroup->SubDataGroups( "CAST_PARTICLE" );
		for( uint32 b = 0; b < Particles; b++ )
		{
			CDataGroup* pSubDataGroup = pMasterGroup->GetSubDataGroup( "CAST_PARTICLE", b );
			int32 Particle = pParticleManager.GetParticleType( pSubDataGroup->Item( 0 ) );
			D3DXVECTOR3 Velocity( 0, 0, 0 );
			Velocity.x = (float32)atof( pSubDataGroup->Item( 1 ).c_str() );
			Velocity.y = (float32)atof( pSubDataGroup->Item( 2 ).c_str() );
			Velocity.z = (float32)atof( pSubDataGroup->Item( 3 ).c_str() );

			pSpell->AddCastParticle( Velocity, Particle );
		}


		uint32 Effects = pMasterGroup->SubDataGroups( "EFFECT" );
		for( uint32 e = 0; e < Effects; e++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "EFFECT", e );


			CDataGroup* pEffectGroup;
			float32			Value( 0 );
			int32			Duration( KDurationInstant );
			EEffectType		Type( KEffectHP );
			EEffectActivation Activation( KActivationUsage );
 
			uint32 SubGroups = pSubGroup->SubDataGroups();

			pEffectGroup = pSubGroup->GetSubDataGroup( "TYPE" );

			if( pEffectGroup != NULL )
			{
				std::string EffectTypeString = StringUpper( pEffectGroup->Item( 0 ) );
				for( uint32 a = 0; a < KAllEffectTypes; a++ )
				{
					if( EffectTypeString == KEffectNames[a] )
					{
						Type = (EEffectType)a;
						break;
					}
				}
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "ACTIVATION" );

			if( pEffectGroup != NULL )
			{
				std::string ActivationString = StringUpper( pEffectGroup->Item( 0 ) );
				for( uint32 a = 0; a < KActivationTypes; a++ )
				{
					if( ActivationString == KActivationNames[a] )
					{
						Activation = (EEffectActivation)a;
						break;
					}
				}

			}


			pEffectGroup = pSubGroup->GetSubDataGroup( "DURATION" );

			if( pEffectGroup != NULL )
			{
				std::string DurationString = StringUpper( pEffectGroup->Item( 0 ) );
				if( DurationString == "INSTANT" )
				{
					Duration = KDurationInstant;
				}
				else if( DurationString == "ALWAYS" )
				{
					Duration = KDurationAlways;
				}
				else
				{
					Duration = (int32)atoi( DurationString.c_str() );
				}
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE" );

			if( pEffectGroup != NULL )
			{
				Value = (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() );
			}



			CEffect* pSubEffect = new CEffect( Type,
											kTrue,
											Activation,
											Duration,
											Value,
											0 );


			pEffectGroup = pSubGroup->GetSubDataGroup( "SFX_DURATION_BONUS" );
			if( pEffectGroup != NULL )
			{
				pSubEffect->SetSFXDurationBonus( (float32)atof( pEffectGroup->Item( 0 ).c_str() ) );
			}

			uint32 SFXItems = pSubGroup->SubDataGroups( "SFX" );
			for( uint32 s = 0; s < SFXItems; s++ )
			{
				pEffectGroup = pSubGroup->GetSubDataGroup( "SFX", s );

				std::string TypeString = StringUpper( pEffectGroup->Item( 0 ) );
				ESFXType Type( KSFXStationaryTarget );
				for( uint32 a = 0; a < KSFXTypes; a++ )
				{
					if( TypeString == KSFXTypeName[a] )
					{
						Type = (ESFXType)a;
						break;
					}
				}
				float32 Duration = (float32)atof( pEffectGroup->Item( 1 ).c_str() );
				bool Looping = atoi( pEffectGroup->Item( 2 ).c_str() ) == 1;
				bool Exclusive = kFalse;
				if( pEffectGroup->Items() >= 7 )
				{
					Exclusive = atoi( pEffectGroup->Item( 6 ).c_str() ) == 1;
				}
				bool RespectBonuses = kFalse;
				if( pEffectGroup->Items() >= 8 )
				{
					RespectBonuses = atoi( pEffectGroup->Item( 7 ).c_str() ) == 1;
				}
				pSubEffect->AddSFX( Type, 
							     pEffectGroup->Item( 5 ),
							     Duration,
							     Looping,
								 (float32)atof( pEffectGroup->Item( 3 ).c_str() ),
								 (float32)atof( pEffectGroup->Item( 4 ).c_str() ),
								 Exclusive,
								 RespectBonuses );

			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "DAMAGE_TYPE" );

			if( pEffectGroup != NULL )
			{
				EDamageType DamageType( KDamageSlashing );
				std::string TypeString = StringUpper( pEffectGroup->Item( 0 ) );
				for( uint32 a = 0; a < KDamageTypes; a++ )
				{
					if( TypeString == KDamageNames[a] )
					{
						DamageType = (EDamageType)a;
						break;
					}
				}
				pSubEffect->SetDamageType( DamageType );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE3" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetValue3( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE3_BONUS" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetValue3Bonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE3_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetValue3BonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE2" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetValue2( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
				if( pSubEffect->Value2() != pSubEffect->Value() )
				{
					pSubEffect->SetRangedValue( kTrue );
				}
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE2_BONUS" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetValue2Bonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE2_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetValue2BonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE_BONUS" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetValueBonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetValueBonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "DURATION_BONUS" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetDurationBonus( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "DURATION_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetDurationBonusPercent( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "CHANCE_OF_SUCCESS" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetChanceOfSuccess( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "CHANCE_OF_SUCCESS_BONUS" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetChanceOfSuccessBonus( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "CHANCE_OF_SUCCESS_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetChanceOfSuccessBonusPercent( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			}


			pEffectGroup = pSubGroup->GetSubDataGroup( "MESSAGE" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetMessage( pEffectGroup->Item( 0 ) );
			}


			pEffectGroup = pSubGroup->GetSubDataGroup( "NAME" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetName( pEffectGroup->Item( 0 ) );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "EXCLUSIVE" );

			if( pEffectGroup != NULL )
			{
				pSubEffect->SetExclusive( atoi( pEffectGroup->Item( 0 ).c_str() ) == 1 );
			}


			pSpell->AddEffect( pSubEffect );
		}

// payload effects
		Effects = pMasterGroup->SubDataGroups( "PAYLOADEFFECT" );
		for( int32 e = 0; e < Effects; e++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "PAYLOADEFFECT", e );


			CDataGroup* pEffectGroup;
			float32			Value( 0 );
			int32			Duration( KDurationInstant );
			EEffectType		Type( KEffectHP );
			EEffectActivation Activation( KActivationUsage );

			uint32 SubGroups = pSubGroup->SubDataGroups();

			pEffectGroup = pSubGroup->GetSubDataGroup( "TYPE" );

			if( pEffectGroup != NULL )
			{
				std::string EffectTypeString = StringUpper( pEffectGroup->Item( 0 ) );
				for( uint32 a = 0; a < KAllEffectTypes; a++ )
				{
					if( EffectTypeString == KEffectNames[a] )
					{
						Type = (EEffectType)a;
						break;
					}
				}
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "ACTIVATION" );

			if( pEffectGroup != NULL )
			{
				std::string ActivationString = StringUpper( pEffectGroup->Item( 0 ) );
				for( uint32 a = 0; a < KActivationTypes; a++ )
				{
					if( ActivationString == KActivationNames[a] )
					{
						Activation = (EEffectActivation)a;
						break;
					}
				}

			}


			pEffectGroup = pSubGroup->GetSubDataGroup( "DURATION" );

			if( pEffectGroup != NULL )
			{
				std::string DurationString = StringUpper( pEffectGroup->Item( 0 ) );
				if( DurationString == "INSTANT" )
				{
					Duration = KDurationInstant;
				}
				else if( DurationString == "ALWAYS" )
				{
					Duration = KDurationAlways;
				}
				else
				{
					Duration = (int32)atoi( DurationString.c_str() );
				}
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE" );

			if( pEffectGroup != NULL )
			{
				Value = (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() );
			}



			CEffect* pSpellEffect = new CEffect( Type,
											kTrue,
											Activation,
											Duration,
											Value,
											0 );

			uint32 SFXItems = pSubGroup->SubDataGroups( "SFX" );
			for( uint32 s = 0; s < SFXItems; s++ )
			{
				pEffectGroup = pSubGroup->GetSubDataGroup( "SFX", s );

				std::string TypeString = StringUpper( pEffectGroup->Item( 0 ) );
				ESFXType Type( KSFXStationaryTarget );
				for( uint32 a = 0; a < KSFXTypes; a++ )
				{
					if( TypeString == KSFXTypeName[a] )
					{
						Type = (ESFXType)a;
						break;
					}
				}
				float32 Duration = (float32)atof( pEffectGroup->Item( 1 ).c_str() );
				bool Looping = atoi( pEffectGroup->Item( 2 ).c_str() ) == 1;
				bool Exclusive = kFalse;
				if( pEffectGroup->Items() >= 7 )
				{
					Exclusive = atoi( pEffectGroup->Item( 6 ).c_str() ) == 1;
				}
				bool RespectBonuses = kFalse;
				if( pEffectGroup->Items() >= 8 )
				{
					RespectBonuses = atoi( pEffectGroup->Item( 7 ).c_str() ) == 1;
				}
				pSpellEffect->AddSFX( Type, 
							     pEffectGroup->Item( 5 ),
							     Duration,
							     Looping,
								 (float32)atof( pEffectGroup->Item( 3 ).c_str() ),
								 (float32)atof( pEffectGroup->Item( 4 ).c_str() ),
								 Exclusive,
								 RespectBonuses );

			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "DAMAGE_TYPE" );

			if( pEffectGroup != NULL )
			{
				EDamageType DamageType( KDamageSlashing );
				std::string TypeString = StringUpper( pEffectGroup->Item( 0 ) );
				for( uint32 a = 0; a < KDamageTypes; a++ )
				{
					if( TypeString == KDamageNames[a] )
					{
						DamageType = (EDamageType)a;
						break;
					}
				}
				pSpellEffect->SetDamageType( DamageType );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE3" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetValue3( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE3_BONUS" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetValue3Bonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE3_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetValue3BonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE2" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetValue2( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
				if( pSpellEffect->Value2() != pSpellEffect->Value() )
				{
					pSpellEffect->SetRangedValue( kTrue );
				}
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE2_BONUS" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetValue2Bonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE2_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetValue2BonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE_BONUS" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetValueBonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetValueBonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "DURATION_BONUS" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetDurationBonus( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "DURATION_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetDurationBonusPercent( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "CHANCE_OF_SUCCESS" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetChanceOfSuccess( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "CHANCE_OF_SUCCESS_BONUS" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetChanceOfSuccessBonus( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "CHANCE_OF_SUCCESS_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetChanceOfSuccessBonusPercent( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			}


			pEffectGroup = pSubGroup->GetSubDataGroup( "MESSAGE" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetMessage( pEffectGroup->Item( 0 ) );
			}


			pEffectGroup = pSubGroup->GetSubDataGroup( "NAME" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetName( pEffectGroup->Item( 0 ) );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "EXCLUSIVE" );

			if( pEffectGroup != NULL )
			{
				pSpellEffect->SetExclusive( atoi( pEffectGroup->Item( 0 ).c_str() ) == 1 );
			}


			pSpell->AddPayloadEffect( pSpellEffect );
		}


		uint32 SFXItems = pMasterGroup->SubDataGroups( "SFX" );
		for( uint32 s = 0; s < SFXItems; s++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "SFX", s );

			std::string TypeString = StringUpper( pSubGroup->Item( 0 ) );
			ESFXType Type( KSFXStationaryTarget );
			for( uint32 a = 0; a < KSFXTypes; a++ )
			{
				if( TypeString == KSFXTypeName[a] )
				{
					Type = (ESFXType)a;
					break;
				}
			}
			float32 Duration = (float32)atof( pSubGroup->Item( 1 ).c_str() );
			bool Looping = atoi( pSubGroup->Item( 2 ).c_str() ) == 1;
			bool Exclusive = kFalse;
			if( pSubGroup->Items() >= 7 )
			{
				Exclusive = atoi( pSubGroup->Item( 6 ).c_str() ) == 1;
			}
			bool RespectBonuses = kFalse;
			if( pSubGroup->Items() >= 8 )
			{
				RespectBonuses = atoi( pSubGroup->Item( 7 ).c_str() ) == 1;
			}
			pSpell->AddSFX( Type, 
						    pSubGroup->Item( 5 ),
						    Duration,
						    Looping,
							(float32)atof( pSubGroup->Item( 3 ).c_str() ),
							(float32)atof( pSubGroup->Item( 4 ).c_str() ),
							Exclusive,
							RespectBonuses );

		}

		SFXItems = pMasterGroup->SubDataGroups( "PAYLOADSFX" );
		for( int32 s = 0; s < SFXItems; s++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "PAYLOADSFX", s );

			std::string TypeString = StringUpper( pSubGroup->Item( 0 ) );
			ESFXType Type( KSFXStationaryTarget );
			for( uint32 a = 0; a < KSFXTypes; a++ )
			{
				if( TypeString == KSFXTypeName[a] )
				{
					Type = (ESFXType)a;
					break;
				}
			}
			float32 Duration = (float32)atof( pSubGroup->Item( 1 ).c_str() );
			bool Looping = atoi( pSubGroup->Item( 2 ).c_str() ) == 1;
			bool Exclusive = kFalse;
			if( pSubGroup->Items() >= 7 )
			{
				Exclusive = atoi( pSubGroup->Item( 6 ).c_str() ) == 1;
			}
			bool RespectBonuses = kFalse;
			if( pSubGroup->Items() >= 8 )
			{
				RespectBonuses = atoi( pSubGroup->Item( 7 ).c_str() ) == 1;
			}
			pSpell->AddPayloadSFX( Type, 
						    pSubGroup->Item( 5 ),
						    Duration,
						    Looping,
							(float32)atof( pSubGroup->Item( 3 ).c_str() ),
							(float32)atof( pSubGroup->Item( 4 ).c_str() ),
							Exclusive,
							RespectBonuses );

		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "SFX_DURATION_BONUS" );
		if( pSubGroup != NULL )
		{
			pSpell->SetDurationBonus( (float32)atof( pSubGroup->Item( 0 ).c_str() ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "DESCRIPTION" );
		if( pSubGroup != NULL )
		{
			std::string Description = pSubGroup->Item( 0 );

			while( Description.find( "\\n" ) != -1 )
			{
				int32 Index = Description.find( "\\n" );
				Description.replace( Index, 2, "\n" );
			}
			pSpell->SetUseDescription( Description );
		}


		uint32 Requirements = pMasterGroup->SubDataGroups( "REQUIRES" );

		for( uint32 Requirement = 0; Requirement < Requirements; Requirement++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "REQUIRES", Requirement );
			EStatistic Statistic = KStatStrength;
			std::string Stat = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 i = 0; i < KStatistics; i++ )
			{
				if( Stat == StringUpper( KStatisticName[i] ) )
				{
					Statistic = (EStatistic)i;
				}
			}
			pSpell->AddRequirement( Statistic, (uint32)atoi( pSubGroup->Item( 1 ).c_str() ) );
		}


		uint32 Sounds = pMasterGroup->SubDataGroups( "SOUND" );
		for( uint32 i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "SOUND", i );

			std::string Name = StringUpper( pSubGroup->Item( 0 ) );
			ECastSound Type( KCastSoundSuccess );
			for( uint32 j = 0; j < KCastSounds; j++ )
			{
				if( Name == KCastSoundName[j] )
				{
					Type = (ECastSound)j;
					break;
				}
			}

			pSpell->AddSound( Type,
							  pSubGroup->Item( 1 ) );
		}

	}

	DELETE_SAFELY( pDataFile );
} // CSpellTemplate::ParseTemplate()
