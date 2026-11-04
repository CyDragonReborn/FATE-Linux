/*
	effect.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _EFFECT_H
#define _EFFECT_H

#include "d3dx8_shim.h"

#include <d3dx8.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../GAMECLIENT/gamerules.h"

class CSpellDescription;
class CCharacter;

// effect description

class CEffect
{

public:

	CEffect( EEffectType Type,				// type of effect
			 bool Positive,					// a positive effect?
			 EEffectActivation Activation,	// activation circumstance
			 int32 Duration,				// duration in seconds
			 float32 Value,					// value change
			 float32 PriceMultiplier ) :
											m_Type( Type ),
											m_Positive( Positive ),
											m_Activation( Activation ),
											m_Duration( Duration ),
											m_DurationBonus( 0 ),
											m_DurationBonusPercent( 0 ),
											m_SFXDurationBonus( 0 ),
											m_Value( Value ),
											m_ValueBonus( 0 ),
											m_ValueBonusPercent( 0 ),
											m_Value2( Value ),
											m_Value2Bonus( 0 ),
											m_Value2BonusPercent( 0 ),
											m_Value3( Value ),
											m_Value3Bonus( 0 ),
											m_Value3BonusPercent( 0 ),
											m_PriceMultiplier( PriceMultiplier ),
											m_TimeActive( 0 ),
											m_ChanceOfSuccess( 100 ),
											m_ChanceOfSuccessBonus( 0 ),
											m_ChanceOfSuccessBonusPercent( 0 ),
											m_Message( "" ),
											m_Name( "" ),
											m_pOwner( NULL ),
											m_pParentSpell( NULL ),
											m_Exclusive( kFalse ),
											m_RangedValue( kFalse ),
											m_Direction( 0, 0, 0 ),
											m_DamageType( KDamageMagical )
	{
	}

	CEffect( CEffect* pEffect ) :			// effect to copy
			 m_Type( pEffect->Type() ),
			 m_Activation( pEffect->Activation() ),
			 m_Duration( pEffect->Duration() ),
			 m_DurationBonus( pEffect->DurationBonus() ),
			 m_DurationBonusPercent( pEffect->DurationBonusPercent() ),
			 m_SFXDurationBonus( pEffect->SFXDurationBonus() ),
			 m_Value( pEffect->Value() ),
			 m_ValueBonus( pEffect->ValueBonus() ),
			 m_ValueBonusPercent( pEffect->ValueBonusPercent() ),
			 m_Value2( pEffect->Value2() ),
			 m_Value2Bonus( pEffect->Value2Bonus() ),
			 m_Value2BonusPercent( pEffect->Value2BonusPercent() ),
			 m_Value3( pEffect->Value3() ),
			 m_Value3Bonus( pEffect->Value3Bonus() ),
			 m_Value3BonusPercent( pEffect->Value3BonusPercent() ),
			 m_Positive( pEffect->Positive() ),
			 m_PriceMultiplier( pEffect->PriceMultiplier() ),
			 m_ChanceOfSuccess( pEffect->ChanceOfSuccess() ),
			 m_ChanceOfSuccessBonus( pEffect->ChanceOfSuccessBonus() ),
			 m_ChanceOfSuccessBonusPercent( pEffect->ChanceOfSuccessBonusPercent() ),
			 m_pParentSpell( pEffect->ParentSpell() ),
			 m_TimeActive( 0 ),
			 m_Message( pEffect->Message() ),
			 m_Name( pEffect->Name() ),
			 m_Exclusive( pEffect->Exclusive() ),
			 m_Direction( pEffect->Direction() ),
			 m_DamageType( pEffect->DamageType() ),
			 m_pOwner( pEffect->Owner() ),
			 m_RangedValue( pEffect->RangedValue() )
	{
		for( uint32 i = 0; i < pEffect->SFXItems(); i++ )
		{
			m_pSFX.push_back( new CSFXDescription( pEffect->SFX( i ) ) );
		}
	}

	~CEffect( void )
	{
		for( uint32 i = 0; i < m_pSFX.size(); i++ )
		{
			DELETE_SAFELY( m_pSFX[i] );
		}
		m_pSFX.clear();
	}


	
////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
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

	void	SetType( EEffectType Type )			{	m_Type = Type;		};

	void	SetDamageType( EDamageType Type )	{	m_DamageType = Type;		};

	void	UpdateTime( float32 TimeElapsed )	// time elapsed in seconds
	{
		if( Duration() != KDurationInstant && 
			Duration() != KDurationAlways )
		{
			m_TimeActive += TimeElapsed;
		}
	}

	void	SetParentSpell( CSpellDescription* pSpellDescription )
	{
		m_pParentSpell = pSpellDescription;
	}

	void	SetChanceOfSuccess( int32 Value )	// percent chance of success
	{
		m_ChanceOfSuccess = Value;
	}

	void	SetChanceOfSuccessBonus( int32 Value )	// percent chance of success
	{
		m_ChanceOfSuccessBonus = Value;
	}

	void	SetChanceOfSuccessBonusPercent( int32 Value )	// percent chance of success
	{
		m_ChanceOfSuccessBonusPercent = Value;
	}

	void	SetRangedValue( bool State )				{	m_RangedValue = State;	};

	void	SetOwner( CCharacter* pCharacter )			{	m_pOwner = pCharacter;	};

	void	SetMessage( const std::string& Message )	{	m_Message = Message;	};

	void	SetName( const std::string& Name )			{	m_Name = Name;			};

	void	SetExclusive( bool State )					{	m_Exclusive = State;	};

	void	SetDirection( const D3DXVECTOR3& Direction )	{	m_Direction = Direction;};


	void	SetValue( float32 Value )		{	m_Value = Value;				};

	void	SetValueBonus( float32 Value )	{	m_ValueBonus = Value;			};

	void	SetValueBonusPercent( float32 Value )	{	m_ValueBonusPercent = Value;			};

	void	SetValue2( float32 Value )		{	m_Value2 = Value;				};

	void	SetValue2Bonus( float32 Value )	{	m_Value2Bonus = Value;			};

	void	SetValue2BonusPercent( float32 Value )	{	m_Value2BonusPercent = Value;			};

	void	SetValue3( float32 Value )		{	m_Value3 = Value;				};

	void	SetValue3Bonus( float32 Value )	{	m_Value3Bonus = Value;			};

	void	SetValue3BonusPercent( float32 Value )	{	m_Value3BonusPercent = Value;			};

	void	SetDuration( int32 Value )		{	m_Duration = Value;			};

	void	SetDurationBonus( int32 Value )	{	m_DurationBonus = Value;		};

	void	SetDurationBonusPercent( int32 Value )	{	m_DurationBonusPercent = Value;		};

	void	SetSFXDurationBonus( float32 Value )	{	m_SFXDurationBonus = Value;		};

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	CSpellDescription*	ParentSpell( void )				{	return m_pParentSpell;					};

	bool				RangedValue( void )				{	return m_RangedValue;					};

	CCharacter*			Owner( void )					{	return m_pOwner;						};

	uint32				SFXItems( void )				{	return m_pSFX.size();					};

	CSFXDescription*	SFX( uint32 Index )				{	return m_pSFX[Index];					};

	EDamageType			DamageType( void )				{	return m_DamageType;					};

	const D3DXVECTOR3& Direction( void )	{	return m_Direction;				};

	int32	ChanceOfSuccess( void )			{	return m_ChanceOfSuccess;		};

	int32	ChanceOfSuccessBonus( void )	{	return m_ChanceOfSuccessBonus;		};

	int32	ChanceOfSuccessBonusPercent( void )	{	return m_ChanceOfSuccessBonusPercent;		};

	bool	HasMessage( void )				{	return m_Message.length() > 0;	};

	bool	HasName( void )					{	return m_Name.length() > 0;		};

	bool				Expired( void )			
	{	
		if( Duration() != KDurationInstant && 
			Duration() != KDurationAlways )
		{
			return m_TimeActive >= Duration();
		}
		return kFalse;
	}

	const std::string&	Message( void )			{	return m_Message;			};

	const std::string&	Name( void )			{	return m_Name;				};

	float32				TimeActive( void )		{	return m_TimeActive;		};

	float32				PriceMultiplier( void )	{	return m_PriceMultiplier;	};

	bool				Positive( void )	{	return m_Positive;		};

	bool				Exclusive( void )	{	return m_Exclusive;		};

	EEffectType			Type( void )		{	return m_Type;			};
	EEffectActivation	Activation( void )	{	return m_Activation;	};

	int32				Duration( void )	{	return m_Duration;		};

	int32				DurationBonus( void )	{	return m_DurationBonus;		};
	int32				DurationBonusPercent( void )	{	return m_DurationBonusPercent;		};

	float32				SFXDurationBonus( void )		{	return m_SFXDurationBonus;		};

	float32				Value( void )		{	return m_Value;			};

	float32				ValueBonus( void )	{	return m_ValueBonus;	};

	float32				ValueBonusPercent( void )	{	return m_ValueBonusPercent;	};

	float32				Value2( void )		{	return m_Value2;		};

	float32				Value2Bonus( void )	{	return m_Value2Bonus;	};

	float32				Value2BonusPercent( void )	{	return m_Value2BonusPercent;	};

	float32				Value3( void )		{	return m_Value3;		};

	float32				Value3Bonus( void )	{	return m_Value3Bonus;	};

	float32				Value3BonusPercent( void )	{	return m_Value3BonusPercent;	};

private:
	EDamageType			m_DamageType;

	float32				m_TimeActive;

	EEffectType			m_Type;
	EEffectActivation	m_Activation;

	int32				m_Duration;
	float32				m_Value;
	float32				m_Value2;
	float32				m_Value3;

	int32				m_DurationBonus;
	float32				m_ValueBonus;
	float32				m_Value2Bonus;
	float32				m_Value3Bonus;
	int32				m_DurationBonusPercent;
	float32				m_ValueBonusPercent;
	float32				m_Value2BonusPercent;
	float32				m_Value3BonusPercent;

	float32				m_SFXDurationBonus;

	bool				m_Positive;

	bool				m_Exclusive;

	bool				m_RangedValue;

	float32				m_PriceMultiplier;

	int32				m_ChanceOfSuccess;

	int32				m_ChanceOfSuccessBonus;
	int32				m_ChanceOfSuccessBonusPercent;

	CCharacter*			m_pOwner;		// generator of the effect

	std::string			m_Message;
	std::string			m_Name;

	D3DXVECTOR3			m_Direction;

	std::vector< CSFXDescription* >		m_pSFX;

	CSpellDescription*	m_pParentSpell;

};
#endif