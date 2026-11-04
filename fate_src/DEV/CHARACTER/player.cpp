/*
	player.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004


*/

#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"

#include "../MEDIA/refmanager.h"

#include "../GAMEOBJECTS/soundbank.h"

#include "../LEVEL/level.h"

#include "charactertemplate.h"
#include "character.h"
#include "inventory.h"
#include "player.h"


CPlayer::CPlayer( LPDIRECT3DDEVICE8 pD3DDevice,						// direct3d device
				  CRefManager& pRefManager,							// pointer to the reference manager, for tracking submaterials
				  CSettings& pSettings,								// pointer to the settings manager, for keeping track of card/machine capabilities
				  CSpellTemplate& pSpellTemplate,					// spell template for casting
				  CParticleManager& pParticleManager,				// particle manager
				  CGlowManager& pGlowManager,						// particle manager
				  CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
				  EGender Gender,									// gender of player
				  int32 HeadIndex,									// head index
				  int32 HairIndex ) :								// hair index
				  CCharacter( pRefManager, 
							  pSettings, 
							  pSpellTemplate,
							  pParticleManager, 
							  pGlowManager,
							  pPositionalAudioManager ),
				  m_BaseHP( 18 )

{
	// we don't have to wait as long for players to re-path
	m_RepathPauseTime = .5f;
	m_HeadIndex = HeadIndex;
	m_HairIndex = HairIndex;
	m_Gender = Gender;

	// players are the only ones with quickslots
	m_pInventory->SetAllowQuickSlots( kTrue );
	SetBaseDescription( pD3DDevice );	
} // CPlayer::CPlayer()

void CPlayer::SetAIState( EAIState State )	// ai state to set
{
	// immediately let us repath after changing ai state
	m_TimeSincePath = 10;
	m_AIState = State;
} // CPlayer::SetAIState()

void CPlayer::SetBaseDescription( LPDIRECT3DDEVICE8 pD3DDevice )					// direct3d device
{
	m_TemplateName = "PLAYER";

	m_NaturalArmor = KBaseArmorClass;

	m_Name = "Player"; // game code might key off these, leaving non-translatable for the time being
	m_Family = "Human";

	m_Vitality = 25;
	m_Dexterity = 20;
	m_Magic = 10;
	m_Strength = 25;

	CalculateMaxHP();
	m_HP = (float32)m_MaxHP;

	CalculateMaxMana();
	m_Mana = (float32)m_MaxMana;

	m_ToHitBonus = 20;

	LoadPlayerModels( pD3DDevice );

	m_pCharacterSounds->AddSample( KSoundCast,
								   "SOUNDS/cast.wav" );

	m_pCharacterSounds->AddSample( KSoundCastSplash,
								   "SOUNDS/castsplash.wav" );

	m_pCharacterSounds->AddSample( KSoundPullSplash,
								   "SOUNDS/pullsplash.wav" );

	m_pCharacterSounds->AddSample( KSoundBiteSplash,
								   "SOUNDS/fishsplash.wav" );

	m_pCharacterSounds->AddSample( KSoundDie,
								   "SOUNDS/DEATH/maledeath.wav" );

	m_pCharacterSounds->AddSample( KSoundStep,
								   "SOUNDS/step1.wav" );

	m_pCharacterSounds->AddSample( KSoundStep,
								   "SOUNDS/step2.wav" );

	m_pCharacterSounds->AddSample( KSoundStep,
								   "SOUNDS/step3.wav" );

	m_pStrikeSounds->AddSample( m_pModel->GetAnimationIndex( "PUNCH" ),
								"SOUNDS/BATTLE/woodflesh1.wav" );

	m_pStrikeSounds->AddSample( m_pModel->GetAnimationIndex( "PUNCH" ),
								"SOUNDS/BATTLE/woodflesh2.wav" );

	m_pStrikeSounds->AddSample( m_pModel->GetAnimationIndex( "PUNCH" ),
								"SOUNDS/BATTLE/woodflesh3.wav" );

	m_pAttackSounds->AddSample( m_pModel->GetAnimationIndex( "PUNCH" ),
								"SOUNDS/BATTLE/miss.wav" );

	AddUnarmedAttack( new CAttackDescription( "PUNCH", kFalse, 6, 8, 1, 2, 0 ) );
	UnarmedAttack( 0 )->SetAnimationIndex( m_pModel->GetAnimationIndex( "PUNCH" ) );
	UnarmedAttack( 0 )->SetDamageType( KDamageCrushing );

	m_RunningSpeed = 22;
	m_WalkingSpeed = 13;

	// players can use EVERYTHING by default
	for( uint32 i = 0; i < KItemTypes; i++ )
	{
		m_pInventory->SetSupportsItemType( (EItemType)i, kTrue );
	}

	m_pFishingPole = new CGenericModel( pD3DDevice,
										m_pRefManager,
										m_pSettings,
										"PLAYER//POLE//pole.sms",
										"" );
	m_pFish = new CGenericModel( pD3DDevice,
								 m_pRefManager,
								 m_pSettings,
								 "PLAYER//FISH//fish.sms",
								 "" );


	// add all the helper sounds

	m_pCharacterSounds->AddSample( KSoundHelpManaLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/mana1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpManaLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/mana2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpManaLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/mana3.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpStaminaLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/Stamina1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpStaminaLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/Stamina2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpStaminaLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/Stamina3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpStaminaLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/Stamina4.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpStaminaLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/Stamina5.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpHealthLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/Health1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpHealthLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/Health2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpHealthLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/Health3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpHealthLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/Health4.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpHealthLow,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/Health5.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpDie,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/die1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpDie,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/die2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpDie,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/die3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpDie,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/die4.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpDie,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/die5.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpPoison,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/poison1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPoison,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/poison2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPoison,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/poison3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPoison,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/poison4.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpTrap,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/trap1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpTrap,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/trap2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpTrap,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/trap3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpTrap,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/trap4.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpTrap,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/trap5.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpLowSkill,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/lowskill1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpLowSkill,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/lowskill2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpLowSkill,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/lowskill3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpLowSkill,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/lowskill4.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpLowPower,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/lowpower1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpLowPower,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/lowpower2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpLowPower,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/lowpower3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpLowPower,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/lowpower4.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpLevelUp,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/levelup1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpLevelUp,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/levelup2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpLevelUp,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/levelup3.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpJourneyBegin,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/journeybegin1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpJourneyBegin,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/journeybegin2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpJourneyBegin,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/journeybegin3.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpJourneyEnd,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/journeycomplete1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpJourneyEnd,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/journeycomplete2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpJourneyEnd,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/journeycomplete3.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpFateSmiles,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/fategood1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpFateSmiles,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/fategood2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpFateSmiles,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/fategood3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpFateSmiles,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/fategood4.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpFateSmiles,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/fategood5.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpPortal,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/portal1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPortal,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/portal2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPortal,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/portal3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPortal,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/portal4.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPortal,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/portal5.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPortal,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/portal6.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpChooseFate,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/choosefate1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpChooseFate,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/choosefate2.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpLowGold,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/lowgold1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpLowGold,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/lowgold2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpLowGold,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/lowgold3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpLowGold,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/lowgold4.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpHealthy,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/healthy1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpHealthy,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/healthy2.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpRefreshed,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/refreshed1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpRefreshed,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/refreshed2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpRefreshed,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/refreshed3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpRefreshed,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/refreshed4.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpRefreshed,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/refreshed5.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpRefreshed,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/refreshed6.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpStronger,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/stronger1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpStronger,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/stronger2.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpFortified,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/fortified1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpFortified,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/fortified2.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpPackFull,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/packfull1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPackFull,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/packfull2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPackFull,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/packfull3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPackFull,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/packfull4.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPackFull,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/packfull5.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPackFull,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/packfull6.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPackFull,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/packfull7.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpFateChosen,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/choice1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpFateChosen,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/choice2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpFateChosen,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/choice3.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpTemptFate,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/temptfate1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpTemptFate,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/temptfate2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpTemptFate,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/temptfate3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpTemptFate,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/temptfate4.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpFameUp,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/fame1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpFameUp,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/fame2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpFameUp,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/fame3.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpPetDepart,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petleft1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPetDepart,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petleft2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPetDepart,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petleft3.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpPetReturn,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petreturn1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPetReturn,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petreturn2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPetReturn,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petreturn3.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpImpossible,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/impossible1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpImpossible,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/impossible2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpImpossible,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/impossible3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpImpossible,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/impossible4.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpImpossible,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/impossible5.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpImpossible,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/impossible6.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpPetTransform,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/pettransform1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPetTransform,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/pettransform2.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpPetFlee,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petflee1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPetFlee,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petflee2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPetFlee,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petflee3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPetFlee,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petflee4.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpPetLevelUp,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petlevelup1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPetLevelUp,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petlevelup2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPetLevelUp,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petlevelup3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpPetLevelUp,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/petlevelup4.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpNewQuest,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/newquest1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpNewQuest,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/newquest2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpNewQuest,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/newquest3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpNewQuest,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/newquest4.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpNewQuest,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/newquest5.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpNewQuest,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/newquest6.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpQuestComplete,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/questcomplete1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpQuestComplete,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/questcomplete2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpQuestComplete,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/questcomplete3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpQuestComplete,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/questcomplete4.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpNewSpell,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/newspell1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpNewSpell,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/newspell2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpNewSpell,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/newspell3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpNewSpell,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/newspell4.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpNewSpell,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/newspell5.ogg" );

	m_pCharacterSounds->AddSample( KSoundHelpSpellFail,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/spellfail1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpSpellFail,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/spellfail2.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpSpellFail,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/spellfail3.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpSpellFail,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/spellfail4.ogg" );


	m_pCharacterSounds->AddSample( KSoundHelpSpellMiscast,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/spellmiscast1.ogg" );
	m_pCharacterSounds->AddSample( KSoundHelpSpellMiscast,
								   "SOUNDS/VOICES/" + CGame::GetLocaleString() + "/NARRATOR/spellmiscast2.ogg" );
	
	PlayAnimation( "IDLE", kTrue );
} // CPlayer::CPlayer()



void CPlayer::CalculateMaxHP( void )
{
	m_MaxHP = 4 * Vitality() +
			  4 * m_Level +
			  m_BaseHP;

	if( m_HP > MaxHP() )
	{
		m_HP = (float32)MaxHP();
	}
} // CPlayer::CalculateMaxHP()

int32 CPlayer::BaseMaxHP( void )
{
	return	  4 * BaseVitality() +
			  4 * m_Level +
			  m_BaseHP;
} // CPlayer::BaseMaxHP()