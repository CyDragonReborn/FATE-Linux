/*
	Sspelltemplate.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _SPELLTEMPLATE_H
#define _SPELLTEMPLATE_H

#include "d3dx8_shim.h"


#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../GAMECLIENT/gamerules.h"
#include "../UTILITIES/loadlocalizedstring.h"

#include "../LEVEL/level.h"
#include "../CHARACTER/character.h"
#include "../MEDIA/refmanager.h"

#include "effect.h"


#include "../GAMEOBJECTS/soundbank.h"

class CDataFile;
class CSettings;
class CParticleManager;

const enum ECastSound
{
	KCastSoundSuccess,
	KCastSoundFail,
	KCastSoundStrike,
	KCastSoundChant,
	KCastSounds
};

const std::string KCastSoundName[KCastSounds] =
{
	"SUCCESS",
	"FAIL",
	"STRIKE",
	"CHANT"
};

// spell description contains spell value and property description information

class CSpellDescription
{

public:

	CSpellDescription( CRefManager& pRefManager,							// pointer to the reference manager, for tracking media
					   CSettings& pSettings,								// settings( for audio management )
					   CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager for cast sounds
					   const std::string& Name,								// name of Spell
					   CMaterial* pIconMaterial,							// icon material
					   EMagicSphere Sphere,									// spell type
					   ETarget Target,										// spell target
					   EAlignment TargetAlignment,							// target alignment
					   float32 Range,										// range of area effect
					   int32 ManaCost,										// cost to cast
					   float32 CastTime ) :									// seconds to cast
					   m_Name( Name ),
					   m_HealSpell( kFalse ),
					   m_SummonSpell( kFalse ),
					   m_Range( Range ),
					   m_pRefManager( pRefManager ),
					   m_Sphere( Sphere ),
					   m_Target( Target ),
					   m_ManaCost( ManaCost ),
					   m_CastTime( CastTime ),
					   m_DurationBonus( 0 ),
					   m_pIconMaterial( pIconMaterial ),
					   m_TargetAlignment( TargetAlignment ),
					   m_pSoundBank( NULL ),
					   m_UseDescription( "" )

	{
		m_pSoundBank = new CSoundBank( pRefManager,
									   pSettings,
									   pPositionalAudioManager );
	}

	~CSpellDescription( void )
	{
		DELETE_SAFELY( m_pSoundBank );

		m_pRefManager.RemoveMaterial( m_pIconMaterial );
		for( uint32 i = 0; i < m_pEffects.size(); i++ )
		{
			DELETE_SAFELY( m_pEffects[i] );
		}
		m_pEffects.clear();

		for( i = 0; i < m_pPayloadEffects.size(); i++ )
		{
			DELETE_SAFELY( m_pPayloadEffects[i] );
		}
		m_pPayloadEffects.clear();

		for( i = 0; i < m_pSFX.size(); i++ )
		{
			DELETE_SAFELY( m_pSFX[i] );
		}
		m_pSFX.clear();

		for( i = 0; i < m_pPayloadSFX.size(); i++ )
		{
			DELETE_SAFELY( m_pPayloadSFX[i] );
		}
		m_pPayloadSFX.clear();
	}

	
////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	void	AddSound( ECastSound Type,				// type of sound
					  const std::string& Path )		// path to sound
	{	
		m_pSoundBank->AddSample( Type,
							     Path );
	};

	void	SetUseDescription( const std::string& Text )		{	m_UseDescription = Text;			};

	void	AddSFX( ESFXType Type,
					const std::string& Path,
					float32 Duration,
					bool Looping,
					float32 Speed,
					float32 TurnRate,
					bool Exclusive,
					bool RespectBonuses )					
			{
				m_pSFX.push_back( new CSFXDescription( Type,
													   Path,
													   Duration,
													   Looping,
													   Speed,
													   TurnRate,
													   Exclusive,
													   RespectBonuses ) );
			};

	void	AddPayloadSFX( ESFXType Type,
					const std::string& Path,
					float32 Duration,
					bool Looping,
					float32 Speed,
					float32 TurnRate,
					bool Exclusive,
					bool RespectBonuses )					
			{
				m_pPayloadSFX.push_back( new CSFXDescription( Type,
													   Path,
													   Duration,
													   Looping,
													   Speed,
													   TurnRate,
													   Exclusive,
													   RespectBonuses ) );
			};

	void	SetTarget( ETarget Target )							{	m_Target = Target;			};

	void	SetDurationBonus( float32 Bonus )					{	m_DurationBonus = Bonus;	};

	void	AddEffect( CEffect* pEffect )			
			{	
				// detect if this is a healing spell
				if( Sphere() == KMagicDefense &&
					pEffect->Type() == KEffectHP )
				{
					m_HealSpell = kTrue;
				}
				// detect if this is a summon spell
				if( Sphere() == KMagicCharm &&
					pEffect->Type() == KEffectSummon )
				{
					m_SummonSpell = kTrue;
				}

				m_pEffects.push_back( pEffect );	
			};

	void	AddPayloadEffect( CEffect* pEffect )			
			{	
				// detect if this is a healing spell
				if( Sphere() == KMagicDefense &&
					pEffect->Type() == KEffectHP )
				{
					m_HealSpell = kTrue;
				}
				// detect if this is a summon spell
				if( Sphere() == KMagicCharm &&
					pEffect->Type() == KEffectSummon )
				{
					m_SummonSpell = kTrue;
				}
				m_pPayloadEffects.push_back( pEffect );	
			};


	void	AddRequirement( EStatistic Statistic,	// statistic
							uint32 Value )			// requirement value
	{
		m_RequirementStatistic.push_back( Statistic );
		m_RequirementValue.push_back( Value );
	}

	void	AddCastParticle( const D3DXVECTOR3& Velocity,
							 int32 Particle )
	{
		m_ParticleVelocity.push_back( Velocity );
		m_Particle.push_back( Particle );
	}

	bool RequirementsMet( CCharacter& pCharacter )
	{
		bool RequirementsMet( kTrue );
		for( uint32 i = 0; i < Requirements(); i++ )
		{
			if( !pCharacter.RequirementMet( RequirementStat( i ),
											RequirementValue( i ) ) )
			{
				RequirementsMet = kFalse;
			}
		}
		return RequirementsMet;
	}

	void PerformFX( CLevel& pLevel,			// level to add effects to
					CCharacter* pUser,		// using character
					CCharacter* pTarget )	// target character
	{
		if( pUser == NULL )
		{
			return;
		}

		m_pSoundBank->PlaySample( KCastSoundSuccess,
								  pUser->Position(),
								  255,
								  48 );


		int32 Bonus( 0 );
		switch( m_Sphere )
		{
		case KMagicAttack :
			Bonus = pUser->SkillPoints( KSkillAttackMagic );
			break;
		case KMagicDefense :
			Bonus = pUser->SkillPoints( KSkillDefenseMagic );
			break;
		case KMagicCharm :
			Bonus = pUser->SkillPoints( KSkillCharmMagic );
			break;
		}

		float32 DurationBonus = Bonus * m_DurationBonus;
		for( uint32 i = 0; i < SFXItems(); i++ )
		{
			if( pTarget != NULL &&
				pUser != NULL )
			{
				pLevel.AddSFX( *SFX( i ), 
								pUser,
								pTarget,
								pUser->Position(),
								pTarget->Position(),
								DurationBonus,
								this );
			}
		}

	}

	void PerformFX( CLevel& pLevel,			// level to add effects to
					CCharacter* pUser,		// using character
					const D3DXVECTOR3& Position )	// target position
	{
		if( pUser == NULL )
		{
			return;
		}

		m_pSoundBank->PlaySample( KCastSoundSuccess,
								  pUser->Position(),
								  255,
								  48 );


		int32 Bonus( 0 );
		switch( m_Sphere )
		{
		case KMagicAttack :
			Bonus = pUser->SkillPoints( KSkillAttackMagic );
			break;
		case KMagicDefense :
			Bonus = pUser->SkillPoints( KSkillDefenseMagic );
			break;
		case KMagicCharm :
			Bonus = pUser->SkillPoints( KSkillCharmMagic );
			break;
		}

		float32 DurationBonus = Bonus * m_DurationBonus;
		for( uint32 i = 0; i < SFXItems(); i++ )
		{
			pLevel.AddSFX( *SFX( i ), 
							pUser,
							NULL,
							pUser->Position(),
							Position,
							DurationBonus,
							this );
		}

	}

	void PerformPayloadFX( CLevel& pLevel,					// level to add effects to
						   CCharacter* pUser,				// using character
						   const D3DXVECTOR3& Position )	// target position
	{
		if( pUser == NULL )
		{
			return;
		}

		m_pSoundBank->PlaySample( KCastSoundStrike,
								  Position,
								  255,
								  48 );

		for( uint32 i = 0; i < PayloadSFXItems(); i++ )
		{
			pLevel.AddSFX( *PayloadSFX( i ), 
						   pUser,
						   NULL,
						   Position,
						   Position,
						   0,
						   this  );
		}
	}

	void Cast( CLevel& pLevel,			// level to add effects to
			   CCharacter* pUser,		// using character
			   CCharacter* pTarget,		// target character
			   bool DoFX = kTrue )	// which of the targets is this?
	{
	
		if( pUser == NULL )
		{
			return;
		}	
		if( DoFX )
		{
			PerformFX( pLevel, pUser, pTarget );
		}

		int32 ChanceRoll = RandomIntegerBetween( 0, 100 );

		bool Failed( kFalse );
		bool AddedSpellIcon( kFalse );
		for( uint32 i = 0; i < m_pEffects.size(); i++ )
		{
			Failed = kTrue;
			// bonuses
			int32 Bonus( 0 );
			switch( m_Sphere )
			{
			case KMagicAttack :
				Bonus = pUser->SkillPoints( KSkillAttackMagic );
				break;
			case KMagicDefense :
				Bonus = pUser->SkillPoints( KSkillDefenseMagic );
				break;
			case KMagicCharm :
				Bonus = pUser->SkillPoints( KSkillCharmMagic );
				break;
			}
			if( pTarget != NULL )
			{
				
				int32 Chance = (int32)( m_pEffects[i]->ChanceOfSuccess() + m_pEffects[i]->ChanceOfSuccessBonus() * (float32)Bonus );
				Chance += Chance * (int32)( m_pEffects[i]->ChanceOfSuccessBonusPercent() / 100.0f * (float32)Bonus );

				if( ChanceRoll <= Chance )
				{
					CEffect* pEffect = new CEffect( m_pEffects[i] );
		
		

					if( pUser != NULL && pTarget != NULL )
					{
						D3DXVECTOR3 Direction = pTarget->Position() - pUser->Position();
						Direction.y = 0;
						D3DXVec3Normalize( &Direction, &Direction );
						
						pEffect->SetDirection( Direction );
					}
					if( pEffect->Duration() != KDurationInstant &&
						pEffect->Type() != KEffectKnockbackEffect &&
						!AddedSpellIcon )
					{
						pEffect->SetParentSpell( this );
						AddedSpellIcon = kTrue;
					}
					CEffect* pFinalEffect = pTarget->AddNewEffect( pLevel, pEffect, Bonus, pUser );
					// boost HP damages based upon Magic stat
					if( pFinalEffect->Type() == KEffectHP &&
						pFinalEffect->Value() < 0 &&
						TargetAlignment() == KEvil &&
						pUser != NULL &&
						pUser->IsPlayer() )
					{
						if( pTarget != pUser )
						{
							float32 Value = pFinalEffect->Value();
							Value = ( Value * ( pUser->Magic() + 100 ) / 100.0f );


							Value -= (int32)ceil( Value * (float32)pUser->EffectValue( KEffectPercentDamageBonus ) / -100.0f );
							Value -= (int32)pUser->EffectValue( KEffectDamageBonus );
							pFinalEffect->SetValue( Value );

							float32 Value2 = pFinalEffect->Value2();
							Value2 = ( Value2 * ( pUser->Magic() + 100 ) / 100.0f );


							Value2 -= (int32)ceil( Value2 * (float32)pUser->EffectValue( KEffectPercentDamageBonus ) / -100.0f );
							Value2 -= (int32)pUser->EffectValue( KEffectDamageBonus );
							pFinalEffect->SetValue2( Value2 );
						}
					}

					Failed = kFalse;
				}
			}

		}
		if( Failed &&
			pTarget != NULL )
		{
			pLevel.AddEventText( pUser,
								pTarget,
								pTarget->Position() + D3DXVECTOR3( 0, 9, 0 ),
								LoadLocalizedString(995),
								.75f,
								kTrue,
								1 );
		}

	}


	void Cast( CLevel& pLevel,			// level to add effects to
			   CCharacter* pUser,		// using character
			   const D3DXVECTOR3& Position,	// position to cast to
			   bool DoFX = kTrue )	// which of the targets is this?
	{
		
		if( DoFX )
		{
			PerformFX( pLevel, pUser, Position );
		}

	}


	
	void CastPayload( CLevel& pLevel,			// level to add effects to
					  CCharacter* pUser,		// using character
					  CCharacter* pTarget,		// target character
					  bool DoFX = kTrue )		// which of the targets is this?
	{
		if( pUser == NULL )
		{
			return;
		}
		if( pTarget != NULL )
		{
			m_pSoundBank->PlaySample( KCastSoundStrike,
									  pTarget->Position(),
									  255,
									  48 );
		}
		int32 ChanceRoll = RandomIntegerBetween( 0, 100 );
		bool Failed( kFalse );
		bool AddedSpellIcon( kFalse );
		// the only payload effects are delivered by the effect themselves when they strike
		// their target
		for( uint32 i = 0; i < m_pPayloadEffects.size(); i++ )
		{
			Failed = kTrue;
			// bonuses
			int32 Bonus( 0 );
			switch( m_Sphere )
			{
			case KMagicAttack :
				Bonus = pUser->SkillPoints( KSkillAttackMagic );
				break;
			case KMagicDefense :
				Bonus = pUser->SkillPoints( KSkillDefenseMagic );
				break;
			case KMagicCharm :
				Bonus = pUser->SkillPoints( KSkillCharmMagic );
				break;
			}
			if( pTarget != NULL )
			{
				int32 Chance = (int32)( m_pPayloadEffects[i]->ChanceOfSuccess() + m_pPayloadEffects[i]->ChanceOfSuccessBonus() * (float32)Bonus );
				Chance += Chance * (int32)( m_pPayloadEffects[i]->ChanceOfSuccessBonusPercent() / 100.0f * (float32)Bonus );

				if( ChanceRoll <= Chance )
				{
					CEffect* pEffect = pTarget->AddEffect( pLevel, m_pPayloadEffects[i], Bonus, pUser, DoFX );

					// boost HP damages based upon Magic stat
					if( pEffect->Type() == KEffectHP &&
						pEffect->Value() < 0 &&
						TargetAlignment() == KEvil &&
						pUser != NULL &&
						pUser->IsPlayer() )
					{
						if( pTarget != pUser )
						{
							float32 Value = pEffect->Value();
							Value = ( Value * ( pUser->Magic() + 100 ) / 100.0f );


							Value -= (int32)ceil( Value * (float32)pUser->EffectValue( KEffectPercentDamageBonus ) / -100.0f );
							Value -= (int32)pUser->EffectValue( KEffectDamageBonus );
							pEffect->SetValue( Value );

							float32 Value2 = pEffect->Value2();
							Value2 = ( Value2 * ( pUser->Magic() + 100 ) / 100.0f );


							Value2 -= (int32)ceil( Value2 * (float32)pUser->EffectValue( KEffectPercentDamageBonus ) / -100.0f );
							Value2 -= (int32)pUser->EffectValue( KEffectDamageBonus );
							pEffect->SetValue2( Value2 );

						}
					}


					if( m_pPayloadEffects[i]->Duration() != KDurationInstant &&
						m_pPayloadEffects[i]->Type() != KEffectKnockbackEffect &&
						!AddedSpellIcon )
					{
						pEffect->SetParentSpell( this );
						AddedSpellIcon = kTrue;
					}
					Failed = kFalse;
				}
			}

		}

		if( Failed &&
			pTarget != NULL )
		{
			pLevel.AddEventText( pUser,
								pTarget,
								pTarget->Position() + D3DXVECTOR3( 0, 9, 0 ),
								LoadLocalizedString(995),
								.75f,
								kTrue,
								1 );
		}
	}


	void Cast( CLevel& pLevel,		// level to add effects to
			   CCharacter* pUser,	// using character
			   CItem* pTarget )		// target item
	{
		if( pUser == NULL )
		{
			return;
		}

		if( pTarget == NULL )
		{
			return;
		}

		for( uint32 i = 0; i < m_pEffects.size(); i++ )
		{
			switch( m_pEffects[i]->Type() )
			{
			case KEffectIdentify :
				pTarget->SetIdentified( kTrue );
				break;
			}
		}

		m_pSoundBank->PlaySample( KCastSoundSuccess,
								  pUser->Position(),
								  255,
								  48 );

		int32 Bonus( 0 );
		switch( m_Sphere )
		{
		case KMagicAttack :
			Bonus = pUser->SkillPoints( KSkillAttackMagic );
			break;
		case KMagicDefense :
			Bonus = pUser->SkillPoints( KSkillDefenseMagic );
			break;
		case KMagicCharm :
			Bonus = pUser->SkillPoints( KSkillCharmMagic );
			break;
		}

		float32 DurationBonus = Bonus * m_DurationBonus;


		for( i = 0; i < SFXItems(); i++ )
		{
			pLevel.AddSFX( *SFX( i ), 
						   pUser,
						   NULL,
						   pUser->Position(),
						   pTarget->Position(),
						   DurationBonus,
						   this  );
		}

	} 

	void FailCast( CLevel& pLevel,			// level to add effects to
				   CCharacter* pUser,		// using character
				   CCharacter* pTarget )	// target character
	{

		if( pUser == NULL )
		{
			return;
		}
		m_pSoundBank->PlaySample( KCastSoundFail,
								  pUser->Position(),
								  255,
								  48 );
	}

	void Chant( CLevel& pLevel,				// level to add effects to
				   CCharacter* pUser,		// using character
				   CCharacter* pTarget )	// target character
	{

		if( pUser == NULL )
		{
			return;
		}
		m_pSoundBank->PlaySample( KCastSoundChant,
								  pUser->Position(),
								  255,
								  48 );
	}

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	float32				Range( void )						{	return m_Range;							};

	bool				IsHealSpell( void )					{	return m_HealSpell;						};

	bool				IsSummonSpell( void )				{	return m_SummonSpell;						};

	uint32				Particles( void )					{	return m_Particle.size();				};

	int32				Particle( uint32 Index )			{	return m_Particle[Index];				};

	const D3DXVECTOR3&	ParticleVelocity( uint32 Index )	{	return m_ParticleVelocity[Index];		};


	uint32				SFXItems( void )					{	return m_pSFX.size();					};

	uint32				PayloadSFXItems( void )				{	return m_pPayloadSFX.size();					};

	int32				ManaCost( void )					{	return m_ManaCost;						};

	float32				CastTime( void )					{	return m_CastTime;						};

	ETarget				Target( void )						{	return m_Target;						};

	EAlignment			TargetAlignment( void )				{	return m_TargetAlignment;				};


	CSFXDescription*	SFX( uint32 Index )					{	return m_pSFX[Index];					};

	CSFXDescription*	PayloadSFX( uint32 Index )			{	return m_pPayloadSFX[Index];					};

	const std::string&	UseDescription( void )				{	return m_UseDescription;				};


	uint32				Requirements( void )				{	return m_RequirementValue.size();	};

	EStatistic			RequirementStat( uint32 Index )		{	return m_RequirementStatistic[Index];	};

	uint32				RequirementValue( uint32 Index )	{	return m_RequirementValue[Index];		};

	std::vector< EStatistic >&	RequirementStats( void )	{	return m_RequirementStatistic;			};

	std::vector< uint32 >&		RequirementValues( void )	{	return m_RequirementValue;				};

	uint32				Effects( void )				{	return m_pEffects.size();			};

	CEffect*			Effect( uint32 Index )		{	return m_pEffects[Index];			};

	uint32				PayloadEffects( void )				{	return m_pPayloadEffects.size();			};

	CEffect*			PayloadEffect( uint32 Index )		{	return m_pPayloadEffects[Index];			};

	EMagicSphere		Sphere( void )				{	return m_Sphere;		};

	const std::string&	Name( void )				{	return m_Name;			};

	CMaterial*			IconMaterial( void )		{	return m_pIconMaterial;	};

	bool CanCastOnItem( CItem* pTarget ) // target item
	{
		bool CanCast( kFalse );
		for( uint32 i = 0; i < m_pEffects.size(); i++ )
		{
			switch( m_pEffects[i]->Type() )
			{
			case KEffectIdentify :
				if( !pTarget->Identified() &&
					( pTarget->Effects( KActivationPassive ) > 0  ||
					pTarget->DamageBonuses() > 0 ) )
				{
					CanCast = kTrue;
				}
				break;
			}
		}
		for( i = 0; i < m_pPayloadEffects.size(); i++ )
		{
			switch( m_pPayloadEffects[i]->Type() )
			{
			case KEffectIdentify :
				if( !pTarget->Identified() &&
					( pTarget->Effects( KActivationPassive ) > 0  ||
					pTarget->DamageBonuses() > 0 ) )
				{
					CanCast = kTrue;
				}
				break;
			}
		}
		return CanCast;
	} 

	bool CanCastOnTarget( CCharacter* pUser,	// casting character
						  CCharacter* pTarget ) // target character
	{
		if( pTarget == NULL ||
			pUser == NULL )
		{
			return kFalse;
		}
		// bonuses
		float32 Bonus( 0 );
		switch( m_Sphere )
		{
		case KMagicAttack :
			Bonus = (float32)pUser->SkillPoints( KSkillAttackMagic );
			break;
		case KMagicDefense :
			Bonus = (float32)pUser->SkillPoints( KSkillDefenseMagic );
			break;
		case KMagicCharm :
			Bonus = (float32)pUser->SkillPoints( KSkillCharmMagic );
			break;
		}

		bool CanCast( kTrue );
		for( uint32 i = 0; i < m_pEffects.size(); i++ )
		{
			float32 Value = m_pEffects[i]->Value();
			Value += m_pEffects[i]->ValueBonus() * (float32)Bonus;

			Value += Value * m_pEffects[i]->ValueBonusPercent() / 100.0f * (float32)Bonus;

			switch( m_pEffects[i]->Type() )
			{
			case KEffectTurnAlignment :
				if( pTarget->Level() > Value )
				{
					CanCast = kFalse;
				}
				break;
			}
		}
		for( i = 0; i < m_pPayloadEffects.size(); i++ )
		{
			float32 Value = m_pPayloadEffects[i]->Value();
			Value += m_pPayloadEffects[i]->ValueBonus() * (float32)Bonus;

			Value += Value * m_pPayloadEffects[i]->ValueBonusPercent() / 100.0f * (float32)Bonus;

			switch( m_pPayloadEffects[i]->Type() )
			{
			case KEffectTurnAlignment :
				if( pTarget->Level() > Value )
				{
					CanCast = kFalse;
				}
				break;
			}
		}
		return CanCast;
	} 

	bool SufficientLevelToCastOnItem( CCharacter* pUser,
									  CItem* pTarget )	// target item
	{
		bool CanCast( kTrue );

		// bonuses
		float32 Bonus( 0 );
		switch( m_Sphere )
		{
		case KMagicAttack :
			Bonus = (float32)pUser->SkillPoints( KSkillAttackMagic );
			break;
		case KMagicDefense :
			Bonus = (float32)pUser->SkillPoints( KSkillDefenseMagic );
			break;
		case KMagicCharm :
			Bonus = (float32)pUser->SkillPoints( KSkillCharmMagic );
			break;
		}

		for( uint32 i = 0; i < m_pEffects.size(); i++ )
		{
			float32 Value = ( m_pEffects[i]->Value() +m_pEffects[i]->ValueBonus() * Bonus );
			Value = Value + Value * m_pEffects[i]->ValueBonusPercent() / 100.0f * Bonus;
	
			switch( m_pEffects[i]->Type() )
			{
			case KEffectIdentify :
				if( !pTarget->Identified() &&
					pTarget->Effects( KActivationPassive ) > 0 )
				{
					if( Value <
						pTarget->BaseLevel() )
					{						 
						CanCast = kFalse;
					}
				}
				break;
			}
		}
		for( i = 0; i < m_pPayloadEffects.size(); i++ )
		{
			float32 Value = ( m_pPayloadEffects[i]->Value() +m_pPayloadEffects[i]->ValueBonus() * Bonus );
			Value = Value + Value * m_pPayloadEffects[i]->ValueBonusPercent() / 100.0f * Bonus;
	
			switch( m_pPayloadEffects[i]->Type() )
			{
			case KEffectIdentify :
				if( !pTarget->Identified() &&
					pTarget->Effects( KActivationPassive ) > 0 )
				{
					if( Value <
						pTarget->BaseLevel() )
					{						 
						CanCast = kFalse;
					}
				}
				break;
			}
		}
		return CanCast;
	} 

	bool SufficientLevelToCast( CLevel& pLevel,
								CCharacter* pUser )
	{
		bool CanCast( kTrue );

		// bonuses
		float32 Bonus( 0 );
		switch( m_Sphere )
		{
		case KMagicAttack :
			Bonus = (float32)pUser->SkillPoints( KSkillAttackMagic );
			break;
		case KMagicDefense :
			Bonus = (float32)pUser->SkillPoints( KSkillDefenseMagic );
			break;
		case KMagicCharm :
			Bonus = (float32)pUser->SkillPoints( KSkillCharmMagic );
			break;
		}

		for( uint32 i = 0; i < m_pEffects.size(); i++ )
		{
			float32 Value = ( m_pEffects[i]->Value() +m_pEffects[i]->ValueBonus() * Bonus );
			Value = Value + Value * m_pEffects[i]->ValueBonusPercent() / 100.0f * Bonus;
	
			switch( m_pEffects[i]->Type() )
			{
			case KEffectDiscover :
				if( Value < pLevel.LevelDepth())
				{						 
					CanCast = kFalse;
				}
				break;
			case KEffectOpenPortal :
				if( Value < pLevel.LevelDepth())
				{						 
					CanCast = kFalse;
				}
				break;
			}
		}
		for( i = 0; i < m_pPayloadEffects.size(); i++ )
		{
			float32 Value = ( m_pPayloadEffects[i]->Value() +m_pPayloadEffects[i]->ValueBonus() * Bonus );
			Value = Value + Value * m_pPayloadEffects[i]->ValueBonusPercent() / 100.0f * Bonus;
	
			switch( m_pPayloadEffects[i]->Type() )
			{
			case KEffectDiscover :
				if( Value < pLevel.LevelDepth())
				{						 
					CanCast = kFalse;
				}
				break;
			}
		}
		return CanCast;
	} 


private:

	bool				m_HealSpell;

	bool				m_SummonSpell;

	CSoundBank*			m_pSoundBank;

	CRefManager&		m_pRefManager;

	std::string			m_Name;

	std::string			m_UseDescription;

	EMagicSphere		m_Sphere;

	EAlignment			m_TargetAlignment;

	ETarget				m_Target;

	float32				m_Range;

	float32				m_CastTime;

	float32				m_DurationBonus;

	int32				m_ManaCost;

	std::vector< CEffect* >			m_pEffects;
	std::vector< CEffect* >			m_pPayloadEffects;

	std::vector< EStatistic >		m_RequirementStatistic;
	std::vector< uint32 >			m_RequirementValue;

	CMaterial*						m_pIconMaterial;

	std::vector< CSFXDescription* >	m_pSFX;

	std::vector< CSFXDescription* >	m_pPayloadSFX;

	std::vector< int32 >			m_Particle;

	std::vector< D3DXVECTOR3 >		m_ParticleVelocity;
};



class CSpellTemplate
{

public:

	CSpellTemplate( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
					CRefManager& pRefManager,		// pointer to the reference manager, for tracking media
					CSettings& pSettings,			// settings( for audio management )
					CParticleManager& pParticleManager,		// particle manager
					CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
					const std::string& Path );		// path to template
	~CSpellTemplate( void );
	
////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////


	uint32					Spells( void )							{	return m_pSpellDescription.size();			};

	uint32					Spells( EMagicSphere Sphere )			{	return m_pSpellsBySphere[Sphere].size();	};

	CSpellDescription*		SpellDescription( std::string Name );	// name of spell to retrieve

	CSpellDescription*		SpellDescription( uint32 Index )			{	return m_pSpellDescription[Index];	};

	CSpellDescription*		SpellDescription( EMagicSphere Sphere, 
											  uint32 Index )			{	return m_pSpellsBySphere[Sphere][Index];	};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

private:

	void	ParseTemplate( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
						   CRefManager& pRefManager,		// pointer to the reference manager, for tracking media
						   CSettings& pSettings,			// settings( for audio management )
						   CParticleManager& pParticleManager,		// particle manager
						   CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
						   const std::string& Path );		// path to template


	std::vector< CSpellDescription* >	m_pSpellDescription;

	std::vector< CSpellDescription* >	m_pSpellsBySphere[KMagicSpheres];
};

#endif