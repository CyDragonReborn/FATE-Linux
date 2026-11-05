/*
	gamerules.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/
#ifndef _GAMERULES_H
#define _GAMERULES_H

#pragma warning(disable: 4786) 

#include <math.h>
#include "../UTILITIES/constants.h"
#include "../UTILITIES/loadlocalizedstring.h"
#include "../GAME/game.h"

const std::string KSaveExtension = "FFD";

enum ERank
{
	KRankNormal,
	KRankElite,
	KRankLegendary,
	KRanks
};

const std::string KRankName[KRanks] =
{
	"",
	LoadLocalizedString(215),
	LoadLocalizedString(216),
};

const uint32 KRankLevelOffset[KRanks] =
{
	0,
	12,
	30
};

const float32 KRankPowerMultiplier[KRanks] =
{
	1,
	2,
	4
};

const float32 KRankToHitMultiplier[KRanks] =
{
	1,
	1.25f,
	2
};

const float32 KRankExperienceMultiplier[KRanks] =
{
	1,
	1.1f,
	1.2f
};

const float32 KRankItemRarityMultiplier[KRanks] =
{
	1,
	5,
	10
};

const float32 KRankMonsterRarityMultiplier[KRanks] =
{
	1,
	10,
	20
};

const float32 KRankRequirementMultiplier[KRanks] =
{
	1,
	1.5,
	2
};

const float32 KRankPriceMultiplier[KRanks] =
{
	1,
	3,
	6
};

enum EHighlightColor
{
	KHighlightLightPurple,
	KHighlightRed,
	KHighlightGreen,
	KHighlightDarkRed,
	KHighlightYellow,
	KHighlightDarkPurple,
	KHighlightBlue,
	KHighlightGray,
	KHighlightTeal,
	KHighlightBlack,
	KHighlightColors
};

enum EGrade
{
	KGradeNormal,
	KGradeSuperior,
	KGradeExceptional,
	KGradeFlawless,
	KGrades
};

enum EJournalStatistic
{
	KJournalStatTimePlayed,
	KJournalStatStepsTaken,
	KJournalStatGoldGathered,
	KJournalStatQuestsCompleted,
	KJournalStatLevelsExplored,
	KJournalStatDeaths,
	KJournalStatMonstersKilled,
	KJournalStatFamousMonstersKilled,
	KJournalStatSpellsCast,
	KJournalStatChestsOpened,
	KJournalStatLargeChestsOpened,
	KJournalStatTrapsSprung,
	KJournalStatBarrelsBroken,
	KJournalStatPotionsUsed,
	KJournalStatPortalsUsed,
	KJournalStatFishCaught,
	KJournalStatTimesGambled,
	KJournalStatistics
};

const std::string KJournalStatName[KJournalStatistics] =
{
	LoadLocalizedString(217),
	LoadLocalizedString(218),
	LoadLocalizedString(219),
	LoadLocalizedString(220),
	LoadLocalizedString(221),
	LoadLocalizedString(222),
	LoadLocalizedString(223),
	LoadLocalizedString(224),
	LoadLocalizedString(225),
	LoadLocalizedString(226),
	"LargeChest",
	LoadLocalizedString(227),
	LoadLocalizedString(228),
	LoadLocalizedString(229),
	LoadLocalizedString(230),
	LoadLocalizedString(231),
	LoadLocalizedString(232)
};

const std::string KGradeName[KGrades] = 
{
	"",
	LoadLocalizedString(233),
	LoadLocalizedString(234),
	LoadLocalizedString(235)
};

const float32 KGradeBonus[KGrades] =
{
	0,
	20,
	40,
	60
};

const uint32 KFameLevels( 21 );
const uint32 KFameGate[KFameLevels] =
{
	0,
	500,
	1000,
	2000,
	4000,
	8000,
	12000,
	20000,
	36000,
	68000,
	100000,
	164000,
	292000,
	548000,
	1188000,
	3236000,
	7332000,
	15524000,
	23716000,
	40100000,
	40100000
};

const std::string KFameName[KFameLevels] =
{
	LoadLocalizedString(236),
	LoadLocalizedString(236),
	LoadLocalizedString(237),
	LoadLocalizedString(238),
	LoadLocalizedString(239),
	LoadLocalizedString(240),
	LoadLocalizedString(241),
	LoadLocalizedString(242),
	LoadLocalizedString(243),
	LoadLocalizedString(244),
	LoadLocalizedString(245),
	LoadLocalizedString(246),
	LoadLocalizedString(247),
	LoadLocalizedString(248),
	LoadLocalizedString(249),
	LoadLocalizedString(250),
	LoadLocalizedString(251),
	LoadLocalizedString(252),
	LoadLocalizedString(253),
	LoadLocalizedString(254),
	LoadLocalizedString(255)
};

enum EGender
{
	KMale,
	KFemale,
	KGenders
};

enum EDifficulty
{
	KDifficultyPage,
	KDifficultyAdventurer,
	KDifficultyHero,
	KDifficultyLegend,
	KDifficulties
};

const float32 KDifficultyLevelBase[KDifficulties] =
{
	-5,
	0,
	1,
	3
};

const float32 KDifficultyMultiplier[KDifficulties] =
{
	.75f,
	1,
	1.5f,
	2
};

const float32 KDifficultyAppearanceMultiplier[KDifficulties] =
{
	.75f,
	1,
	1.5f,
	3
};

const float32 KDifficultyItemMultiplier[KDifficulties] =
{
	.75f,
	1,
	1.2f,
	1.4f
};

const std::string KDifficultyName[KDifficulties] =
{
	LoadLocalizedString(256),
	LoadLocalizedString(257),
	LoadLocalizedString(258),
	LoadLocalizedString(259)
};

enum EQuestType 
{
	KQuestKillMonster,
	KQuestKillMonsters,
	KQuestMonsterItem,
	KQuestFindItem,
	KQuestKillArmy,
	KQuestArmyItem,
//	KQuestFindItems,
//	KQuestFindLost,
	KQuestMasterQuest,
	KQuestTypes
//	KQuestClearLevel,
};

enum EMiniMapDolly
{
	KMapNormal,
	KMapHigh,
	KMapHigher,
	KMapHighest,
	KMiniMapHeights
};

const std::string KQuestName[KQuestTypes] =
{
	"KILLMONSTER",
	"KILLMONSTERS",
	"MONSTERITEM",
	"FINDITEM",
	"KILLARMY",
	"ARMYITEM",
	"MASTERQUEST"
//	"CLEARLEVEL"
};

enum ETrapType
{
	KTrapNone,
	KTrapExplosion,
	KTrapPoison,
	KTrapMonster,
	KTrapSword,
	KTrapTypes
};

enum EAlignment
{
	KNeutral,
	KGood,
	KEvil,
	KAllAlignments,
	KAlignments
};

const std::string KAlignmentName[KAlignments] = 
{
	"NEUTRAL",
	"GOOD",
	"EVIL",
	"ALL"
};

enum ESFXType
{
	KSFXStationaryTarget,
	KSFXStationaryOwner,
	KSFXStationary,
	KSFXStationaryPortal,
	KSFXBeam,
	KSFXFollowOwner,
	KSFXFollowTarget,
	KSFXProjectileLocation,
	KSFXProjectileCharacter,
	KSFXProjectileLocationErratic,
	KSFXProjectileCharacterErratic,
	KSFXTypes
};

const std::string KSFXTypeName[KSFXTypes] =
{
	"STATIONARYTARGET",
	"STATIONARYOWNER",
	"STATIONARY",
	"STATIONARYPORTAL",
	"BEAM",
	"FOLLOWOWNER",
	"FOLLOWTARGET",
	"PROJECTILELOCATION",
	"PROJECTILECHARACTER",
	"PROJECTILELOCATIONERRATIC",
	"PROJECTILECHARACTERERRATIC",
};

enum ETactic
{
	KTacticsFighter,
	KTacticsMage,
	KTacticsFighterMage,
	KTacticsDarter,
	KTactics
};

const std::string KTacticNames[KTactics] =
{
	"FIGHTER",
	"MAGE",
	"FIGHTERMAGE",
	"DARTER"
};


enum EMagicSphere
{
	KMagicAttack,
	KMagicDefense,
	KMagicCharm,
	KMagicSpheres
};


const std::string KMagicSphereName[KMagicSpheres] = 
{
	"ATTACK",
	"DEFENSE",
	"CHARM"
};

const std::string KMagicSphereShortName[KMagicSpheres] = 
{
	LoadLocalizedString(260),
	LoadLocalizedString(261),
	LoadLocalizedString(262)
};

enum EAttackSpeed
{
	KAttackSlowest,
	KAttackSlow,
	KAttackNormal,
	KAttackFast,
	KAttackFastest,
	KAttackSpeeds
};

enum EDamageType
{
	KDamagePiercing,
	KDamageSlashing,
	KDamageCrushing,
	KDamageMagical,
	KDamageFire,
	KDamageIce,
	KDamageElectric,
	KDamageUndead,
	KDamageTypes
};

const std::string KDamageTypeDescription[KDamageTypes] =
{
	LoadLocalizedString(263),
	LoadLocalizedString(264),
	LoadLocalizedString(265),
	LoadLocalizedString(266),
	LoadLocalizedString(267),
	LoadLocalizedString(268),
	LoadLocalizedString(269),
	LoadLocalizedString(270)
};

const std::string KDamageResistanceDescription[KDamageTypes] =
{
	LoadLocalizedString(271),
	LoadLocalizedString(272),
	LoadLocalizedString(273),
	LoadLocalizedString(274),
	LoadLocalizedString(275),
	LoadLocalizedString(276),
	LoadLocalizedString(277),
	LoadLocalizedString(278)
};

const std::string KDamageWeaknessDescription[KDamageTypes] =
{
	LoadLocalizedString(279),
	LoadLocalizedString(280),
	LoadLocalizedString(281),
	LoadLocalizedString(282),
	LoadLocalizedString(283),
	LoadLocalizedString(284),
	LoadLocalizedString(285),
	LoadLocalizedString(286)
};

const std::string KDamageNames[KDamageTypes] =
{
	"PIERCING",
	"SLASHING",
	"CRUSHING",
	"MAGIC",
	"FIRE",
	"ICE",
	"ELECTRIC",
	"UNDEAD"
};

const std::string KDamageWeaponPrefix[KDamageTypes] = 
{
	"",
	"",
	"",
	"",
	LoadLocalizedString(287),
	LoadLocalizedString(288),
	LoadLocalizedString(289),
	LoadLocalizedString(290)
};

const std::string KDamageParticles[KDamageTypes] =
{
	"",
	"",
	"",
	"",
	"FIRE",
	"ICE",
	"ELECTRIC",
	"TINYFASTGLEAM"
};

const std::string KDamageParticlesSecondary[KDamageTypes] =
{
	"",
	"",
	"",
	"",
	"BIGFIRESPARK",
	"SMALLSNOWFLAKE",
	"",
	""
};

enum EStatistic
{
	KStatStrength,
	KStatDexterity,
	KStatVitality,
	KStatMagic,
	KStatLevel,
	KStatRenown,
	KStatLineage,
	KStatSkillSword,
	KStatSkillClub,
	KStatSkillHammer,
	KStatSkillAxe,
	KStatSkillSpear,
	KStatSkillStaff,
	KStatSkillPolearm,
	KStatSkillBow,
	KStatSkillCriticalStrike,
	KStatSkillSpellcasting,
	KStatSkillDualWield,
	KStatSkillShield,
	KStatSkillAttackMagic,
	KStatSkillDefenseMagic,
	KStatSkillCharmMagic,
	KStatistics
};

const std::string KStatisticName[KStatistics] =
{
	"STRENGTH",
	"DEXTERITY",
	"VITALITY",
	"MAGIC",
	"LEVEL",
	"RENOWN",
	"LINEAGE",
	"SKILLSWORD",
	"SKILLCLUB",
	"SKILLHAMMER",
	"SKILLAXE",
	"SKILLSPEAR",
	"SKILLSTAFF",
	"SKILLPOLEARM",
	"SKILLBOW",
	"SKILLCRITICALSTRIKE",
	"SKILLSPELLCASTING",
	"SKILLDUALWIELD",
	"SKILLSHIELD",
	"SKILLATTACKMAGIC",
	"SKILLDEFENSEMAGIC",
	"SKILLCHARMMAGIC"
};

const std::string KStatisticDescription[KStatistics] =
{
	LoadLocalizedString(291), // 291 - 312
	LoadLocalizedString(292),
	LoadLocalizedString(293),
	LoadLocalizedString(294),
	LoadLocalizedString(295),
	LoadLocalizedString(296),
	LoadLocalizedString(297),
	LoadLocalizedString(298),
	LoadLocalizedString(299),
	LoadLocalizedString(300),
	LoadLocalizedString(301),
	LoadLocalizedString(302),
	LoadLocalizedString(303),
	LoadLocalizedString(304),
	LoadLocalizedString(305),
	LoadLocalizedString(306),
	LoadLocalizedString(307),
	LoadLocalizedString(308),
	LoadLocalizedString(309),
	LoadLocalizedString(310),
	LoadLocalizedString(311),
	LoadLocalizedString(312)
};

const float32 KAttackSpeedValue[KAttackSpeeds] =
{
	.6f,
	.8f,
	1.0f,
	1.1f,
	1.2f
};

const std::string KAttackSpeedName[KAttackSpeeds] =
{
	LoadLocalizedString(313),
	LoadLocalizedString(314),
	LoadLocalizedString(315),
	LoadLocalizedString(316),
	LoadLocalizedString(317)
};


enum ESkill
{
	KSkillSword,
	KSkillClub,
	KSkillHammer,
	KSkillAxe,
	KSkillSpear,
	KSkillStaff,
	KSkillPolearm,
	KSkillBow,
	KSkillCriticalStrike,
	KSkillSpellcasting,
	KSkillDualWield,
	KSkillShield,
	KSkillAttackMagic,
	KSkillDefenseMagic,
	KSkillCharmMagic,
	KSkills
};

const std::string KSkillNames[KSkills] =
{
	"SKILLSWORD",
	"SKILLCLUB",
	"SKILLHAMMER",
	"SKILLAXE",
	"SKILLSPEAR",
	"SKILLSTAFF",
	"SKILLPOLEARM",
	"SKILLBOW",
	"SKILLCRITICALSTRIKE",
	"SKILLSPELLCASTING",
	"SKILLDUALWIELD",
	"SKILLSHIELD",
	"SKILLATTACKMAGIC",
	"SKILLDEFENSEMAGIC",
	"SKILLCHARMMAGIC"
};

const std::string KSkillDescription[KSkills] =
{
	LoadLocalizedString(298),
	LoadLocalizedString(299),
	LoadLocalizedString(300),
	LoadLocalizedString(301),
	LoadLocalizedString(302),
	LoadLocalizedString(303),
	LoadLocalizedString(304),
	LoadLocalizedString(305),
	LoadLocalizedString(306),
	LoadLocalizedString(307),
	LoadLocalizedString(308),
	LoadLocalizedString(309),
	LoadLocalizedString(310),
	LoadLocalizedString(311),
	LoadLocalizedString(312)
};

const std::string KSkillExplanation[KSkills] =
{
	LoadLocalizedString(318),
	LoadLocalizedString(319),
	LoadLocalizedString(320),
	LoadLocalizedString(321),
	LoadLocalizedString(322),
	LoadLocalizedString(323),
	LoadLocalizedString(324),
	LoadLocalizedString(325),
	LoadLocalizedString(326),
	LoadLocalizedString(327),
	LoadLocalizedString(328),
	LoadLocalizedString(329),
	LoadLocalizedString(330),
	LoadLocalizedString(331),
	LoadLocalizedString(332)
};

enum EItemCategory
{
	KCategoryBlank,
	KCategoryGold,
	KCategoryItem,
	KCategoryContainer,
	KCategoryWeapon,
	KCategoryArmor,
	KCategoryJewelry,
	KCategoryEvent,
	KItemCategories
};

enum EItemType
{
	KItemNone,
	KItemStairsUp,
	KItemStairsDown,
	KItemTownPortal,
	KItemDungeonPortal,
	KItemFishingHole,
	KItemGold,
	KItemGeneric,
	KItemPotion,
	KItemWeaponRack,
	KItemContainer,
	KItemChestSmall,
	KItemChestMedium,
	KItemChestLarge,
	KItemSword,
	KItemClub,
	KItemHelmet,
	KItemShield,
	KItemSpear,
	KItemBow,
	KItemBelt,
	KItemNecklace,
	KItemRing,
	KItemGloves,
	KItemBoots,
	KItemShirt,
	KItemPolearm,
	KItemScroll,
	KItemBook,
	KItemSpell,
	KItemAxe,
	KItemHammer,
	KItemStaff,
	KItemCrossbow,
	KItemGem,
	KItemPetFood,
	KItemFishingPole,
	KItemMagicAnvil,
	KItemShrine,
	KItemFountain,
	KItemThrone,
	KItemStatue,
	KItemHealFountain,
	KItemManaFountain,
	KItemStaminaFountain,
	KItemWellnessFountain,
	KItemTypes
};

enum EMerchant
{
	KMerchantTraveling,
	KMerchantGoods,
	KMerchantWeapons,
	KMerchantArmor,
	KMerchantWeaponsArmor,
	KMerchantMagic,
	KMerchantPotions,
	KMerchantGambler,
	KMerchantStash,
	KMerchantBreakGems,
	KMerchantBreakWeapons,
	KMerchantMinstrel,
	KMerchantHealer,
	KMerchantFish,
	KMerchantEnchanter,
	KMerchantTypes
};

const std::string KMerchantName[KMerchantTypes] = 
{
	"TRAVELING",
	"GOODS",
	"WEAPONS",
	"ARMOR",
	"WEAPONSARMOR",
	"MAGIC",
	"POTIONS",
	"GAMBLER",
	"STASH",
	"BREAKGEMS",
	"BREAKWEAPONS",
	"MINSTREL",
	"HEALER",
	"FISH",
	"ENCHANTER"
};

const std::string KItemTypeName[KItemTypes] = 
{
	"NONE",
	"STAIRSUP",
	"STAIRSDOWN",
	"TOWNPORTAL",
	"DUNGEONPORTAL",
	"FISHINGHOLE",
	"GOLD",
	"GENERIC",
	"POTION",
	"WEAPONRACK",
	"CONTAINER",
	"CHESTSMALL",
	"CHESTMEDIUM",
	"CHESTLARGE",
	"SWORD",
	"CLUB",
	"HELMET",
	"SHIELD",
	"SPEAR",
	"BOW",
	"BELT",
	"NECKLACE",
	"RING",
	"GLOVES",
	"BOOTS",
	"SHIRT",
	"POLEARM",
	"SCROLL",
	"BOOK",
	"SPELL",
	"AXE",
	"HAMMER",
	"STAFF",
	"CROSSBOW",
	"GEM",
	"PETFOOD",
	"FISHINGPOLE",
	"MAGICANVIL",
	"SHRINE",
	"FOUNTAIN",
	"THRONE",
	"STATUE",
	"HEALFOUNTAIN",
	"MANAFOUNTAIN",
	"STAMINAFOUNTAIN",
	"WELLNESSFOUNTAIN"
};

const EDamageType KItemDamageType[KItemTypes] =
{
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageCrushing,
	KDamageSlashing,
	KDamageSlashing,
	KDamagePiercing,
	KDamagePiercing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamagePiercing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageCrushing,
	KDamageCrushing,
	KDamagePiercing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing,
	KDamageSlashing
};

const bool KItemTwoHanded[KItemTypes] = 
{
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kTrue,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kTrue,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kTrue,
	kTrue,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse
};

const bool KItemRightHanded[KItemTypes] = 
{
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kFalse,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kFalse,
	kFalse,
	kFalse,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse
};

const std::string KItemTypeDescription[KItemTypes] = 
{
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	LoadLocalizedString(333),
	LoadLocalizedString(334),
	LoadLocalizedString(335),
	LoadLocalizedString(336),
	LoadLocalizedString(337),
	LoadLocalizedString(338),
	LoadLocalizedString(339),
	LoadLocalizedString(340),
	LoadLocalizedString(340),
	LoadLocalizedString(341),
	LoadLocalizedString(342),
	LoadLocalizedString(343),
	LoadLocalizedString(344),
	"",
	"",
	"",
	LoadLocalizedString(345),
	LoadLocalizedString(346),
	LoadLocalizedString(347),
	LoadLocalizedString(348),
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	""
};

const EItemCategory	KItemCategory[KItemTypes] =
{
	KCategoryBlank,
	KCategoryBlank,
	KCategoryBlank,
	KCategoryBlank,
	KCategoryBlank,
	KCategoryBlank,
	KCategoryGold,
	KCategoryItem,
	KCategoryItem,
	KCategoryContainer,
	KCategoryContainer,
	KCategoryContainer,
	KCategoryContainer,
	KCategoryContainer,
	KCategoryWeapon,
	KCategoryWeapon,
	KCategoryArmor,
	KCategoryArmor,
	KCategoryWeapon,
	KCategoryWeapon,
	KCategoryArmor,
	KCategoryJewelry,
	KCategoryJewelry,
	KCategoryArmor,
	KCategoryArmor,
	KCategoryArmor,
	KCategoryWeapon,
	KCategoryItem,
	KCategoryItem,
	KCategoryItem,
	KCategoryWeapon,
	KCategoryWeapon,
	KCategoryWeapon,
	KCategoryWeapon,
	KCategoryItem,
	KCategoryItem,
	KCategoryItem,
	KCategoryEvent,
	KCategoryEvent,
	KCategoryEvent,
	KCategoryEvent,
	KCategoryEvent,
	KCategoryEvent,
	KCategoryEvent,
	KCategoryEvent,
	KCategoryEvent,
};

const ESkill KSkillModifier[KItemTypes] =
{
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillClub,
	KSkillSword,
	KSkillSword,
	KSkillSpear,
	KSkillBow,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillPolearm,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillAxe,
	KSkillHammer,
	KSkillStaff,
	KSkillBow,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword,
	KSkillSword
};

const std::string KWeaponAnimationRight[KItemTypes] =
{
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"RIGHTSLASH",
	"RIGHTSLASH",
	"NONE",
	"NONE",
	"RIGHTSTAB",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"RIGHTPIKESTAB",
	"NONE",
	"NONE",
	"NONE",
	"RIGHTSLASH",
	"RIGHTSLASH",
	"RIGHTPIKESTAB",
	"RIGHTCROSSBOW",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE"
};

const std::string KWeaponAnimationLeft[KItemTypes] =
{
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"LEFTSLASH",
	"LEFTSLASH",
	"NONE",
	"NONE",
	"LEFTSTAB",
	"BOW",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"LEFTPIKESTAB",
	"NONE",
	"NONE",
	"NONE",
	"LEFTSLASH",
	"LEFTSLASH",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE",
	"NONE"
};

enum EEffectActivation
{
	KActivationPassive,
	KActivationUsage,
	KActivationTypes
};

const std::string KActivationNames[KActivationTypes] =
{
	"PASSIVE",
	"USAGE"
};

enum ETarget
{
	KTargetUser,
	KTargetCharacter,
	KTargetArea,
	KTargetItem,
	KTargetCharacterArea,
	KTargetUserArea,
	KTargetUserPets,
	KTargetCharacterPets,
	KTargetUserAndPets,
	KTargetCharacterAndPets,
	KTargetProjectile,
	KTargets
};

const std::string KTargetDescription[KTargets] =
{
	LoadLocalizedString(349),
	LoadLocalizedString(350),
	LoadLocalizedString(351),
	LoadLocalizedString(352),
	LoadLocalizedString(353),
	LoadLocalizedString(354),
	LoadLocalizedString(355),
	LoadLocalizedString(356),
	LoadLocalizedString(357),
	LoadLocalizedString(358),
	LoadLocalizedString(359)
};

const std::string KTargetNames[KTargets] =
{
	"USER",
	"CHARACTER",
	"AREA",
	"ITEM",
	"CHARACTERAREA",
	"USERAREA",
	"USERPETS",
	"CHARACTERPETS",
	"USERANDPETS",
	"CHARACTERANDPETS",
	"PROJECTILE"
};


enum EEffectType
{
	KEffectStrength,
	KEffectDexterity,
	KEffectVitality,
	KEffectMagic,
	KEffectMaxMana,
	KEffectMaxHP,
	KEffectMaxStamina,
	KEffectMana,
	KEffectHP,
	KEffectStamina,
	KEffectArmorBonus,
	KEffectToHitBonus,
	KEffectDamageBonus,
	KEffectDamageTaken,
	KEffectKnockback,
	KEffectSkillSword,
	KEffectSkillClub,
	KEffectSkillHammer,
	KEffectSkillAxe,
	KEffectSkillSpear,
	KEffectSkillStaff,
	KEffectSkillPolearm,
	KEffectSkillBow,
	KEffectSkillCrossbow,
	KEffectSkillThrown,
	KEffectSkillDualWield,
	KEffectSkillShield,
	KEffectSkillAttackMagic,
	KEffectSkillDefenseMagic,
	KEffectSkillCharmMagic,
	KEffectPercentStrength,
	KEffectPercentDexterity,
	KEffectPercentVitality,
	KEffectPercentMagic,
	KEffectPercentMana,
	KEffectPercentHP,
	KEffectPercentStamina,
	KEffectPercentSpeed,
	KEffectPercentAttackSpeed,
	KEffectPercentArmorBonus,
	KEffectPercentToHitBonus,
	KEffectPercentDamageBonus,
	KEffectPercentDamageTaken,
	KEffectPercentMagicalDrop,
	KEffectPercentGoldDrop,
	KEffectPercentCastSpeed,
	KEffectPercentLifeStolen,
	KEffectPercentManaStolen,
	KEffectPercentDamageReflected,
	KEffectPercentBlockChance,
	KEffectPercentItemRequirements,
	KEffectPercentPiercingResistance,
	KEffectPercentSlashingResistance,
	KEffectPercentCrushingResistance,
	KEffectPercentMagicalResistance,
	KEffectPercentFireResistance,
	KEffectPercentIceResistance,
	KEffectPercentElectricResistance,
	KEffectTypes,
	KEffectRemove,
	KEffectKnockbackEffect,
	KEffectIdentify,
	KEffectWriteSpell,
	KEffectSummon,
	KEffectAddDamageType,
	KEffectTransform,
	KEffectRevertTransform,
	KEffectOpenPortal,
	KEffectDiscover,
	KEffectFlee,
	KEffectTurnAlignment,
	KEffectDispel,
	KEffectDispelEnemy,
	KAllEffectTypes
};

const EEffectType KDamageResistance[KDamageTypes] =
{
	KEffectPercentPiercingResistance,
	KEffectPercentSlashingResistance,
	KEffectPercentCrushingResistance,
	KEffectPercentMagicalResistance,
	KEffectPercentFireResistance,
	KEffectPercentIceResistance,
	KEffectPercentElectricResistance,
	KEffectPercentFireResistance
};

enum EBonusType
{
	KBonusValue,
	KBonusPercentage,
	KBonusTypes
};

enum EBonusPositive
{
	KBonusPositive,
	KBonusNegative
};

const EBonusType KEffectBonusType[KEffectTypes] =
{
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusValue,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage,
	KBonusPercentage
};
const bool KEffectHasValue[KAllEffectTypes] =
{
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kFalse,
	kFalse,
	kTrue,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse
};

const bool KEffectIsRecharge[KAllEffectTypes] =
{
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kTrue,
	kTrue,
	kTrue,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse
};

const std::string KEffectNames[KAllEffectTypes] =
{
	"STRENGTH",
	"DEXTERITY",
	"VITALITY",
	"MAGIC",
	"MANA",
	"HP",
	"STAMINA",
	"MANARECHARGE",
	"HPRECHARGE",
	"STAMINARECHARGE",
	"ARMORBONUS",
	"TOHITBONUS",
	"DAMAGEBONUS",
	"DAMAGETAKEN",
	"KNOCKBACK",
	"SKILLSWORD",
	"SKILLCLUB",
	"SKILLHAMMER",
	"SKILLAXE",
	"SKILLSPEAR",
	"SKILLSTAFF",
	"SKILLPOLEARM",
	"SKILLBOW",
	"SKILLCRITICALSTRIKE",
	"SKILLSPELLCASTING",
	"SKILLDUALWIELD",
	"SKILLSHIELD",
	"SKILLATTACKMAGIC",
	"SKILLDEFENSEMAGIC",
	"SKILLCHARMMAGIC",
	"PERCENTSTRENGTH",
	"PERCENTDEXTERITY",
	"PERCENTVITALITY",
	"PERCENTMAGIC",
	"PERCENTMANA",
	"PERCENTHP",
	"PERCENTSTAMINA",
	"PERCENTSPEED",
	"PERCENTATTACKSPEED",
	"PERCENTARMORBONUS",
	"PERCENTTOHITBONUS",
	"PERCENTDAMAGEBONUS",
	"PERCENTDAMAGETAKEN",
	"PERCENTMAGICALDROP",
	"PERCENTGOLDDROP",
	"PERCENTCASTSPEED",
	"PERCENTLIFESTOLEN",
	"PERCENTMANASTOLEN",
	"PERCENTDAMAGEREFLECTED",
	"PERCENTBLOCKCHANCE",
	"PERCENTITEMREQUIREMENTS",
	"PERCENTPIERCINGRESISTANCE",
	"PERCENTSLASHINGRESISTANCE",
	"PERCENTCRUSHINGRESISTANCE",
	"PERCENTMAGICALRESISTANCE",
	"PERCENTFIRERESISTANCE",
	"PERCENTICERESISTANCE",
	"PERCENTELECTRICRESISTANCE",
	"NA",
	"REMOVEEFFECT",
	"KNOCKBACKEFFECT",
	"IDENTIFY",
	"WRITESPELL",
	"SUMMON",
	"ADDDAMAGETYPE",
	"TRANSFORM",
	"REVERTTRANSFORM",
	"OPENPORTAL",
	"DISCOVER",
	"FLEE",
	"TURNALIGNMENT",
	"DISPEL",
	"DISPELENEMY"
};

// is susceptible to magic resistance?
const bool KEffectIsMagic[KAllEffectTypes] =
{
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kTrue,
	kFalse,
	kFalse,
	kTrue,
	kFalse,
	kFalse,
	kFalse,
	kTrue,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse,
	kFalse
};

const std::string KEffectDescriptions[KAllEffectTypes] =
{
	LoadLocalizedString(291),
	LoadLocalizedString(292),
	LoadLocalizedString(293),
	LoadLocalizedString(294),
	LoadLocalizedString(925),
	LoadLocalizedString(923),
	LoadLocalizedString(922),
	LoadLocalizedString(1126),
	LoadLocalizedString(1127),
	LoadLocalizedString(1128),
	LoadLocalizedString(1129),
	LoadLocalizedString(920),
	LoadLocalizedString(1130),
	LoadLocalizedString(1131),
	LoadLocalizedString(417),
	LoadLocalizedString(298),
	LoadLocalizedString(299),
	LoadLocalizedString(300),
	LoadLocalizedString(301),
	LoadLocalizedString(302),
	LoadLocalizedString(303),
	LoadLocalizedString(304),
	LoadLocalizedString(305),
	LoadLocalizedString(306),
	LoadLocalizedString(307),
	LoadLocalizedString(308),
	LoadLocalizedString(309),
	LoadLocalizedString(310),
	LoadLocalizedString(311),
	LoadLocalizedString(312),
	LoadLocalizedString(360),
	LoadLocalizedString(361),
	LoadLocalizedString(362),
	LoadLocalizedString(363),
	LoadLocalizedString(364),
	LoadLocalizedString(365),
	LoadLocalizedString(366),
	LoadLocalizedString(367),
	LoadLocalizedString(368),
	LoadLocalizedString(369),
	LoadLocalizedString(370),
	LoadLocalizedString(371),
	LoadLocalizedString(372),
	LoadLocalizedString(373),
	LoadLocalizedString(374),
	LoadLocalizedString(375),
	LoadLocalizedString(376),
	LoadLocalizedString(377),
	LoadLocalizedString(378),
	LoadLocalizedString(379),
	LoadLocalizedString(380),
	LoadLocalizedString(381),
	LoadLocalizedString(382),
	LoadLocalizedString(383),
	LoadLocalizedString(384),
	LoadLocalizedString(385),
	LoadLocalizedString(386),
	LoadLocalizedString(387),
	LoadLocalizedString(388),
	LoadLocalizedString(389),
	LoadLocalizedString(390),
	LoadLocalizedString(391),
	LoadLocalizedString(392),
	LoadLocalizedString(393),
	LoadLocalizedString(394),
	LoadLocalizedString(395),
	LoadLocalizedString(396),
	LoadLocalizedString(397),
	LoadLocalizedString(398),
	LoadLocalizedString(399),
	LoadLocalizedString(400),
	LoadLocalizedString(401),
	LoadLocalizedString(402)
};

const std::string KEffectSpellPositiveDescriptions[KAllEffectTypes] =
{
	LoadLocalizedString(403),
	LoadLocalizedString(404),
	LoadLocalizedString(405),
	LoadLocalizedString(406),
	LoadLocalizedString(407),
	LoadLocalizedString(408),
	LoadLocalizedString(409),
	LoadLocalizedString(410),
	LoadLocalizedString(411),
	LoadLocalizedString(412),
	LoadLocalizedString(413),
	LoadLocalizedString(414),
	LoadLocalizedString(415),
	LoadLocalizedString(416),
	LoadLocalizedString(417),
	LoadLocalizedString(418),
	LoadLocalizedString(419),
	LoadLocalizedString(420),
	LoadLocalizedString(421),
	LoadLocalizedString(422),
	LoadLocalizedString(423),
	LoadLocalizedString(424),
	LoadLocalizedString(425),
	LoadLocalizedString(426),
	LoadLocalizedString(427),
	LoadLocalizedString(428),
	LoadLocalizedString(429),
	LoadLocalizedString(430),
	LoadLocalizedString(431),
	LoadLocalizedString(432),
	LoadLocalizedString(433),
	LoadLocalizedString(434),
	LoadLocalizedString(435),
	LoadLocalizedString(436),
	LoadLocalizedString(437),
	LoadLocalizedString(438),
	LoadLocalizedString(439),
	LoadLocalizedString(440),
	LoadLocalizedString(441),
	LoadLocalizedString(442),
	LoadLocalizedString(443),
	LoadLocalizedString(444),
	LoadLocalizedString(445),
	LoadLocalizedString(446),
	LoadLocalizedString(447),
	LoadLocalizedString(448),
	LoadLocalizedString(449),
	LoadLocalizedString(450),
	LoadLocalizedString(451),
	LoadLocalizedString(452),
	LoadLocalizedString(453),
	LoadLocalizedString(454),
	LoadLocalizedString(455),
	LoadLocalizedString(456),
	LoadLocalizedString(457),
	LoadLocalizedString(458),
	LoadLocalizedString(459),
	LoadLocalizedString(460),
	LoadLocalizedString(461),
	LoadLocalizedString(462),
	LoadLocalizedString(463),
	LoadLocalizedString(464),
	LoadLocalizedString(465),
	LoadLocalizedString(466),
	LoadLocalizedString(467),
	LoadLocalizedString(468),
	LoadLocalizedString(469),
	LoadLocalizedString(470),
	LoadLocalizedString(471),
	LoadLocalizedString(472),
	LoadLocalizedString(473),
	LoadLocalizedString(474),
	LoadLocalizedString(547)
};

const std::string KEffectItemNegativeDescriptions[KAllEffectTypes] =
{
	LoadLocalizedString(475),
	LoadLocalizedString(476),
	LoadLocalizedString(477),
	LoadLocalizedString(478),
	LoadLocalizedString(479),
	LoadLocalizedString(480),
	LoadLocalizedString(481),
	LoadLocalizedString(482),
	LoadLocalizedString(483),
	LoadLocalizedString(484),
	LoadLocalizedString(485),
	LoadLocalizedString(486),
	LoadLocalizedString(487),
	LoadLocalizedString(488),
	LoadLocalizedString(489),
	LoadLocalizedString(490),
	LoadLocalizedString(491),
	LoadLocalizedString(492),
	LoadLocalizedString(493),
	LoadLocalizedString(494),
	LoadLocalizedString(495),
	LoadLocalizedString(496),
	LoadLocalizedString(497),
	LoadLocalizedString(498),
	LoadLocalizedString(499),
	LoadLocalizedString(500),
	LoadLocalizedString(501),
	LoadLocalizedString(502),
	LoadLocalizedString(503),
	LoadLocalizedString(504),
	LoadLocalizedString(505),
	LoadLocalizedString(506),
	LoadLocalizedString(507),
	LoadLocalizedString(508),
	LoadLocalizedString(509),
	LoadLocalizedString(510),
	LoadLocalizedString(511),
	LoadLocalizedString(512),
	LoadLocalizedString(513),
	LoadLocalizedString(514),
	LoadLocalizedString(515),
	LoadLocalizedString(516),
	LoadLocalizedString(517),
	LoadLocalizedString(518),
	LoadLocalizedString(519),
	LoadLocalizedString(520),
	LoadLocalizedString(521),
	LoadLocalizedString(522),
	LoadLocalizedString(523),
	LoadLocalizedString(524),
	LoadLocalizedString(525),
	LoadLocalizedString(526),
	LoadLocalizedString(527),
	LoadLocalizedString(528),
	LoadLocalizedString(529),
	LoadLocalizedString(530),
	LoadLocalizedString(531),
	LoadLocalizedString(532),
	LoadLocalizedString(533),
	LoadLocalizedString(534),
	LoadLocalizedString(535),
	LoadLocalizedString(536),
	LoadLocalizedString(537),
	LoadLocalizedString(538),
	LoadLocalizedString(539),
	LoadLocalizedString(540),
	LoadLocalizedString(541),
	LoadLocalizedString(542),
	LoadLocalizedString(543),
	LoadLocalizedString(544),
	LoadLocalizedString(545),
	LoadLocalizedString(546),
	LoadLocalizedString(547)
};

const std::string KEffectItemPositiveDescriptions[KAllEffectTypes] =
{
	LoadLocalizedString(403),
	LoadLocalizedString(404),
	LoadLocalizedString(405),
	LoadLocalizedString(406),
	LoadLocalizedString(407),
	LoadLocalizedString(408),
	LoadLocalizedString(409),
	LoadLocalizedString(410),
	LoadLocalizedString(411),
	LoadLocalizedString(412),
	LoadLocalizedString(413),
	LoadLocalizedString(414),
	LoadLocalizedString(415),
	LoadLocalizedString(416),
	LoadLocalizedString(417),
	LoadLocalizedString(418),
	LoadLocalizedString(419),
	LoadLocalizedString(420),
	LoadLocalizedString(421),
	LoadLocalizedString(422),
	LoadLocalizedString(423),
	LoadLocalizedString(424),
	LoadLocalizedString(425),
	LoadLocalizedString(426),
	LoadLocalizedString(427),
	LoadLocalizedString(428),
	LoadLocalizedString(429),
	LoadLocalizedString(430),
	LoadLocalizedString(431),
	LoadLocalizedString(432),
	LoadLocalizedString(433),
	LoadLocalizedString(434),
	LoadLocalizedString(435),
	LoadLocalizedString(436),
	LoadLocalizedString(437),
	LoadLocalizedString(438),
	LoadLocalizedString(439),
	LoadLocalizedString(440),
	LoadLocalizedString(441),
	LoadLocalizedString(442),
	LoadLocalizedString(443),
	LoadLocalizedString(444),
	LoadLocalizedString(445),
	LoadLocalizedString(446),
	LoadLocalizedString(447),
	LoadLocalizedString(448),
	LoadLocalizedString(449),
	LoadLocalizedString(450),
	LoadLocalizedString(451),
	LoadLocalizedString(452),
	LoadLocalizedString(453),
	LoadLocalizedString(454),
	LoadLocalizedString(455),
	LoadLocalizedString(456),
	LoadLocalizedString(457),
	LoadLocalizedString(458),
	LoadLocalizedString(459),
	LoadLocalizedString(460),
	LoadLocalizedString(461),
	LoadLocalizedString(462),
	LoadLocalizedString(463),
	LoadLocalizedString(464),
	LoadLocalizedString(465),
	LoadLocalizedString(466),
	LoadLocalizedString(467),
	LoadLocalizedString(468),
	LoadLocalizedString(469),
	LoadLocalizedString(470),
	LoadLocalizedString(471),
	LoadLocalizedString(472),
	LoadLocalizedString(473),
	LoadLocalizedString(474),
	LoadLocalizedString(547)
};

const std::string KEffectSpellNegativeDescriptions[KAllEffectTypes] =
{
	LoadLocalizedString(475), 
	LoadLocalizedString(476),
	LoadLocalizedString(477),
	LoadLocalizedString(478),
	LoadLocalizedString(479),
	LoadLocalizedString(480),
	LoadLocalizedString(481),
	LoadLocalizedString(482),
	LoadLocalizedString(483),
	LoadLocalizedString(484),
	LoadLocalizedString(485),
	LoadLocalizedString(486),
	LoadLocalizedString(487),
	LoadLocalizedString(488),
	LoadLocalizedString(489),
	LoadLocalizedString(490),
	LoadLocalizedString(491),
	LoadLocalizedString(492),
	LoadLocalizedString(493),
	LoadLocalizedString(494),
	LoadLocalizedString(495),
	LoadLocalizedString(496),
	LoadLocalizedString(497),
	LoadLocalizedString(498),
	LoadLocalizedString(499),
	LoadLocalizedString(500),
	LoadLocalizedString(501),
	LoadLocalizedString(502),
	LoadLocalizedString(503),
	LoadLocalizedString(504),
	LoadLocalizedString(505),
	LoadLocalizedString(506),
	LoadLocalizedString(507),
	LoadLocalizedString(508),
	LoadLocalizedString(509),
	LoadLocalizedString(510),
	LoadLocalizedString(511),
	LoadLocalizedString(512),
	LoadLocalizedString(513),
	LoadLocalizedString(514),
	LoadLocalizedString(515),
	LoadLocalizedString(516),
	LoadLocalizedString(517),
	LoadLocalizedString(518),
	LoadLocalizedString(519),
	LoadLocalizedString(520),
	LoadLocalizedString(521),
	LoadLocalizedString(522),
	LoadLocalizedString(523),
	LoadLocalizedString(524),
	LoadLocalizedString(525),
	LoadLocalizedString(526),
	LoadLocalizedString(527),
	LoadLocalizedString(528),
	LoadLocalizedString(529),
	LoadLocalizedString(530),
	LoadLocalizedString(531),
	LoadLocalizedString(532),
	LoadLocalizedString(533),
	LoadLocalizedString(534),
	LoadLocalizedString(535),
	LoadLocalizedString(536),
	LoadLocalizedString(537),
	LoadLocalizedString(538),
	LoadLocalizedString(539),
	LoadLocalizedString(540),
	LoadLocalizedString(541),
	LoadLocalizedString(542),
	LoadLocalizedString(543),
	LoadLocalizedString(544),
	LoadLocalizedString(545),
	LoadLocalizedString(546),
	LoadLocalizedString(547)
};

const uint32 KPositiveSuffixes( 5 );
const std::string KPositivePrefixSuffix[KEffectTypes][KPositiveSuffixes] =
{
	{ LoadLocalizedString(548),LoadLocalizedString(549),LoadLocalizedString(550),LoadLocalizedString(551),LoadLocalizedString(552) },
	{ LoadLocalizedString(553),LoadLocalizedString(554),LoadLocalizedString(555),LoadLocalizedString(556),LoadLocalizedString(557) },
	{ LoadLocalizedString(558),LoadLocalizedString(559),LoadLocalizedString(560),LoadLocalizedString(561),LoadLocalizedString(562) },
	{ LoadLocalizedString(563),LoadLocalizedString(564),LoadLocalizedString(565),LoadLocalizedString(566),LoadLocalizedString(567) },
	{ LoadLocalizedString(568),LoadLocalizedString(569),LoadLocalizedString(570),LoadLocalizedString(571),LoadLocalizedString(572) }, 
	{ LoadLocalizedString(573),LoadLocalizedString(574),LoadLocalizedString(575),LoadLocalizedString(576),LoadLocalizedString(577) },
	{ LoadLocalizedString(578),LoadLocalizedString(579),LoadLocalizedString(580),LoadLocalizedString(581),LoadLocalizedString(582) },
	{ LoadLocalizedString(583),LoadLocalizedString(584),LoadLocalizedString(585),LoadLocalizedString(586),LoadLocalizedString(587) },
	{ LoadLocalizedString(588),LoadLocalizedString(589),LoadLocalizedString(590),LoadLocalizedString(591),LoadLocalizedString(592) },
	{ LoadLocalizedString(593),LoadLocalizedString(594),LoadLocalizedString(595),LoadLocalizedString(596),LoadLocalizedString(597) },
	{ LoadLocalizedString(598),LoadLocalizedString(599),LoadLocalizedString(600),LoadLocalizedString(601),LoadLocalizedString(602) },
	{ LoadLocalizedString(603),LoadLocalizedString(604),LoadLocalizedString(605),LoadLocalizedString(606),LoadLocalizedString(607) },
	{ LoadLocalizedString(608),LoadLocalizedString(609),LoadLocalizedString(610),LoadLocalizedString(611),LoadLocalizedString(612) },
	{ LoadLocalizedString(613),LoadLocalizedString(614),LoadLocalizedString(615),LoadLocalizedString(616),LoadLocalizedString(617) },
	{ LoadLocalizedString(618),LoadLocalizedString(619),LoadLocalizedString(620),LoadLocalizedString(621),LoadLocalizedString(622) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(623),LoadLocalizedString(624),LoadLocalizedString(625),LoadLocalizedString(626),LoadLocalizedString(627) },
	{ LoadLocalizedString(628),LoadLocalizedString(629),LoadLocalizedString(630),LoadLocalizedString(631),LoadLocalizedString(632) },
	{ LoadLocalizedString(633),LoadLocalizedString(634),LoadLocalizedString(635),LoadLocalizedString(636),LoadLocalizedString(637) },
	{ LoadLocalizedString(638),LoadLocalizedString(639),LoadLocalizedString(640),LoadLocalizedString(641),LoadLocalizedString(642) },
	{ LoadLocalizedString(643),LoadLocalizedString(644),LoadLocalizedString(645),LoadLocalizedString(646),LoadLocalizedString(647) },
	{ LoadLocalizedString(648),LoadLocalizedString(649),LoadLocalizedString(650),LoadLocalizedString(651),LoadLocalizedString(652) },
	{ LoadLocalizedString(653),LoadLocalizedString(654),LoadLocalizedString(655),LoadLocalizedString(656),LoadLocalizedString(657) },
	{ LoadLocalizedString(658),LoadLocalizedString(659),LoadLocalizedString(660),LoadLocalizedString(661),LoadLocalizedString(662) },
	{ LoadLocalizedString(663),LoadLocalizedString(664),LoadLocalizedString(665),LoadLocalizedString(666),LoadLocalizedString(667) },
	{ LoadLocalizedString(668),LoadLocalizedString(669),LoadLocalizedString(670),LoadLocalizedString(671),LoadLocalizedString(672) },
	{ LoadLocalizedString(673),LoadLocalizedString(674),LoadLocalizedString(675),LoadLocalizedString(676),LoadLocalizedString(677) },
	{ LoadLocalizedString(678),LoadLocalizedString(679),LoadLocalizedString(680),LoadLocalizedString(681),LoadLocalizedString(682) },
	{ LoadLocalizedString(683),LoadLocalizedString(684),LoadLocalizedString(685),LoadLocalizedString(686),LoadLocalizedString(687) },
	{ LoadLocalizedString(688),LoadLocalizedString(689),LoadLocalizedString(690),LoadLocalizedString(691),LoadLocalizedString(692) },
	{ LoadLocalizedString(693),LoadLocalizedString(694),LoadLocalizedString(695),LoadLocalizedString(696),LoadLocalizedString(697) },
	{ LoadLocalizedString(698),LoadLocalizedString(699),LoadLocalizedString(700),LoadLocalizedString(701),LoadLocalizedString(702) },
	{ LoadLocalizedString(703),LoadLocalizedString(704),LoadLocalizedString(705),LoadLocalizedString(706),LoadLocalizedString(707) },
	{ LoadLocalizedString(708),LoadLocalizedString(709),LoadLocalizedString(710),LoadLocalizedString(711),LoadLocalizedString(712) },
	{ LoadLocalizedString(713),LoadLocalizedString(714),LoadLocalizedString(715),LoadLocalizedString(716),LoadLocalizedString(717) },
	{ LoadLocalizedString(718),LoadLocalizedString(719),LoadLocalizedString(720),LoadLocalizedString(721),LoadLocalizedString(722) },
	{ LoadLocalizedString(723),LoadLocalizedString(724),LoadLocalizedString(725),LoadLocalizedString(726),LoadLocalizedString(727) },
	{ LoadLocalizedString(728),LoadLocalizedString(729),LoadLocalizedString(730),LoadLocalizedString(731),LoadLocalizedString(732) },
	{ LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733) },
	{ LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733) },
	{ LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733) },
	{ LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733) },
	{ LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733) },
	{ LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733) },
	{ LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733),LoadLocalizedString(733) }

};


const uint32 KNegativeSuffixes( 2 );
const std::string KNegativePrefixSuffix[KEffectTypes][KNegativeSuffixes] =
{
	{ LoadLocalizedString(734),LoadLocalizedString(735) },
	{ LoadLocalizedString(736),LoadLocalizedString(737) },
	{ LoadLocalizedString(738),LoadLocalizedString(739) },
	{ LoadLocalizedString(740),LoadLocalizedString(741) },
	{ LoadLocalizedString(742),LoadLocalizedString(743) },
	{ LoadLocalizedString(744),LoadLocalizedString(745) },
	{ LoadLocalizedString(746),LoadLocalizedString(747) },
	{ LoadLocalizedString(748),LoadLocalizedString(749) },
	{ LoadLocalizedString(750),LoadLocalizedString(751) },
	{ LoadLocalizedString(752),LoadLocalizedString(753) },
	{ LoadLocalizedString(754),LoadLocalizedString(755) },
	{ LoadLocalizedString(756),LoadLocalizedString(757) },
	{ LoadLocalizedString(758),LoadLocalizedString(759) },
	{ LoadLocalizedString(760),LoadLocalizedString(761) },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ "","" },
	{ LoadLocalizedString(762),LoadLocalizedString(763) },
	{ LoadLocalizedString(764),LoadLocalizedString(765) },
	{ LoadLocalizedString(766),LoadLocalizedString(767) },
	{ LoadLocalizedString(768),LoadLocalizedString(769) },
	{ LoadLocalizedString(770),LoadLocalizedString(771) },
	{ LoadLocalizedString(772),LoadLocalizedString(773) },
	{ LoadLocalizedString(774),LoadLocalizedString(775) },
	{ LoadLocalizedString(776),LoadLocalizedString(777) },
	{ LoadLocalizedString(778),LoadLocalizedString(779) },
	{ LoadLocalizedString(780),LoadLocalizedString(781) },
	{ LoadLocalizedString(782),LoadLocalizedString(783) },
	{ LoadLocalizedString(784),LoadLocalizedString(785) },
	{ LoadLocalizedString(786),LoadLocalizedString(787) },
	{ LoadLocalizedString(788),LoadLocalizedString(789) },
	{ LoadLocalizedString(790),LoadLocalizedString(791) },
	{ LoadLocalizedString(792),LoadLocalizedString(793) },
	{ LoadLocalizedString(794),LoadLocalizedString(795) },
	{ LoadLocalizedString(796),LoadLocalizedString(797) },
	{ LoadLocalizedString(798),LoadLocalizedString(799) },
	{ LoadLocalizedString(800),LoadLocalizedString(801) },
	{ LoadLocalizedString(802),LoadLocalizedString(803) },
	{ LoadLocalizedString(804),LoadLocalizedString(804) },
	{ LoadLocalizedString(804),LoadLocalizedString(804) },
	{ LoadLocalizedString(804),LoadLocalizedString(804) },
	{ LoadLocalizedString(804),LoadLocalizedString(804) },
	{ LoadLocalizedString(804),LoadLocalizedString(804) },
	{ LoadLocalizedString(804),LoadLocalizedString(804) },
	{ LoadLocalizedString(804),LoadLocalizedString(804) }
};

const int32 KEffectMinimum[KEffectTypes] =
{
	-10,
	-10,
	-10,
	-10,
	-10,
	-10,
	-5,
	-5,
	-5,
	-10,
	-10,
	-10,
	-10,
	30,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	-50,
	-50,
	-50,
	-50,
	-50,
	-50,
	-50,
	-20,
	-20,
	-50,
	-50,
	-50,
	30,
	0,
	0,
	0,
	-30,
	-30,
	0,
	-30,
	-30,
	-30,
	-30,
	-30,
	-30,
	-30,
	-30,
	-30
};

const int32 KEffectMaximum[KEffectTypes] =
{
	30,
	30,
	30,
	30,
	30,
	30,
	10,
	10,
	10,
	30,
	30,
	30,
	30,
	-10,
	10,
	20,
	20,
	20,
	20,
	20,
	20,
	20,
	20,
	20,
	20,
	20,
	20,
	20,
	20,
	20,
	50,
	50,
	50,
	50,
	50,
	50,
	50,
	30,
	30,
	50,
	50,
	50,
	-30,
	30,
	100,
	20,
	8,	// life stolen
	10,
	10,
	10, 
	10,
	30,
	30,
	30,
	30,
	30,
	30,
	30
};

const int32		 KUnlimitedUses( -9999 );
const int32		 KDurationInstant( -900 );
const int32		 KDurationAlways( -1000 );

enum EEffectTarget
{
	KEffectUser,
	KEffectTarget,
	KEffectTargets
};

enum ELightPoolType
{
	KLightFlicker,
	KLightPulse
};

const uint32 KExperienceGates = 100;
const uint32 KExperienceGate[KExperienceGates] =
{
	0,
	847,
	1424,
	2379,
	3946,
	6498,
	10623,
	17241,
	27779,
	44432,
	70546,
	111183,
	173930,
	251775,
	263013,
	294761,
	337798,
	380835,
	423872,
	466909,
	509946,
	552984,
	596021,
	659538,
	725587,
	791636,
	857685,
	923734,
	989782,
	1055831,
	1121880,
	1208774,
	1300717,
	1392659,
	1484602,
	1576545,
	1668488,
	1760431,
	1871890,
	1999642,
	2127395,
	2255147,
	2382899,
	2510652,
	2638404,
	2766156,
	2893909,
	3027737,
	3198893,
	3370048,
	3541204,
	3712359,
	3883514,
	4054670,
	4225825,
	4396980,
	4602433,
	4834834,
	5067234,
	5299635,
	5532035,
	5764436,
	5996836,
	6229237,
	6487487,
	6752000,
	7016513,
	7281026,
	7545538,
	7810051,
	8074564,
	8339077,
	8637166,
	9050909,
	9464652,
	9878396,
	10292139,
	10705882,
	11119625,
	11533368,
	11972685,
	12412931,
	12853176,
	13293421,
	13733666,
	14210761,
	14754810,
	15298858,
	15842906,
	16386955,
	16966785,
	17573608,
	18180431,
	18787255,
	19462627,
	20181825,
	20901024,
	21693940,
	22528322,
	24137891

};

// base AC for all characters
const int32 KBaseArmorClass( 10 );

inline int32 AbilityModifier( int32 Ability )
{
	Ability -= 10;

	int32 Modifier = (int32)floor( (float32)Ability / 2 );
	return Modifier;
}

inline int32 RollDie( int32 Sides ) 
{
	return 1 + rand() % Sides;
}


inline int32 RollDice( int32 Dice, int32 Sides ) 
{
	int32 Sum( Dice );
	for ( int32 i = 0; i < Dice; i++ )
	{
		Sum += rand() % Sides;
	}
	return Sum;
}

inline int32 MinDiceRoll( int32 Dice, int32 Sides )
{
	return Dice;
}

inline int32 MaxDiceRoll( int32 Dice, int32 Sides )
{
	return Dice * Sides;
}

inline int32 ExperienceGate( int32 Level )
{
	if( Level == 0 )
	{
		return 0;
	}
	if( Level > 99 )
	{
		Level = 99;
	}
	return KExperienceGate[Level];
}

class CSFXDescription
{
public:
	CSFXDescription( ESFXType SFXType,						// type of sfx this is
					 const std::string& Path,				// path to animated mesh
					 float32 Duration,						// lifetime ( -1 to end when animation ends )
					 bool Looping,							// loop the animation? nonapplicable for a time of -1
					 float32 Speed,							// projectile speed
					 float32 TurnRate,
					 bool Exclusive,						// projectile turn rate
					 bool RespectBonuses ) :				// respect duration bonuses
					 m_Duration( Duration ),
					 m_Looping( Looping ),
					 m_Exclusive( Exclusive ),
					 m_Path( Path ),
					 m_Type( SFXType ),
					 m_Speed( Speed ),
					 m_TurnRate( TurnRate ),
					 m_RespectBonuses( RespectBonuses )
	{
	}
	CSFXDescription( CSFXDescription* pDescription ) :	// description to copy
					 m_Duration( pDescription->m_Duration ),
					 m_Looping( pDescription->m_Looping ),
					 m_Exclusive( pDescription->m_Exclusive ),
					 m_Path( pDescription->m_Path ),
					 m_Type( pDescription->m_Type ),
					 m_Speed( pDescription->m_Speed ),
					 m_TurnRate( pDescription->m_TurnRate ),
					 m_RespectBonuses( pDescription->m_RespectBonuses )
	{
	}

	ESFXType	m_Type;
	std::string	m_Path;
	float32		m_Duration;
	bool		m_Looping;
	bool		m_Exclusive;
	bool		m_RespectBonuses;
	float32		m_Speed;
	float32		m_TurnRate;
};

#endif

