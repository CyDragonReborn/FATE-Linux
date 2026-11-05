/*
	quest.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/19/2004

*/

#ifndef _QUEST_H
#define _QUEST_H

#include "d3dx8_shim.h"

#include "gamerules.h"
#include "../CHARACTER/charactertemplate.h"
#include "../CHARACTER/character.h"
#include "../CHARACTER/monster.h"
#include "../ITEM/itemtemplate.h"
#include "../LEVEL/level.h"
#include "../NAMES/namegenerator.h"
#include "questdescriptions.h"
#include "../ITEM/item.h"
#include "../UTILITIES/loadlocalizedstring.h"


// a quest can be of several types, and several different rewards
// players store quests, which are activated upon entry to their
// target level

class CQuest
{
public:

			CQuest( CNameGenerator& pNameGenerator,	// generator to make random name
					CQuestDescriptions& pQuestDescriptions,	// random quest descriptions
					CCharacterTemplate& pCharacterManifest,		// character manifest
					CItemTemplate& pItemManifest,				// item manifest
					int32 Level,			// level the quest is for
					EQuestType Type,		// type of quest
					CItem* pQuestItem,		// quest item
					CItem* pItem,			// reward item
					EDifficulty Difficulty,	// difficulty level
					std::string Giver ) :	// name of giver,
											m_Level( Level ),
											m_Type( Type ),
											m_Spawned( kFalse ),
											m_Completed( kFalse ),
											m_CompletionNotified( kFalse ),
											m_GiverName( Giver ),
											m_QuestMonsterName( "" ),
											m_QuestMonsterBaseName( "" ),
											m_QuestMinionMonsterBaseName( "" ),
											m_QuestName( "" ),
											m_QuestDescription( "" ),
											m_QuestMiniDescription( "" ),
											m_CompletionDescription( "" ),
											m_IncompleteDescription( "" ),
											m_QuestMonsterCount( 0 ),
											m_QuestMinionCount( 0 ),
											m_QuestItemCount( 0 ),
											m_QuestMonstersCompleted( 0 ),
											m_QuestMinionsCompleted( 0 ),
											m_QuestItemsCompleted( 0 ),
											m_GoldReward( 0 ),
											m_ExperienceReward( 0 ),
											m_FameReward( 0 ),
											m_pQuestItem( NULL ),
											m_pItemReward( NULL )
			{
				m_QuestDescription = pQuestDescriptions.RandomQuestDescription( Type );

				// randomize level depth a little
				m_Level += RandomIntegerBetween( 0, 2 );

				int32 AcceptableMonsterDepth( (int32)ceil( (float32)m_Level * KDifficultyAppearanceMultiplier[Difficulty] ) );

				if( AcceptableMonsterDepth > 97 )
				{
					AcceptableMonsterDepth = 97;
				}

				bool Done( kFalse );
				CCharacterDescription* pDescription;
				while( !Done )
				{
					uint32 Monster = RandomIntegerBetween( 0, pCharacterManifest.Characters() - 1 );

					pDescription = pCharacterManifest.CharacterDescription( Monster );
					if( pDescription->Alignment() == KEvil &&
						pDescription->WalkingSpeed() > 0 &&
						pDescription->MinimumDepth() <= AcceptableMonsterDepth + 2 &&
						pDescription->MaximumDepth() >= AcceptableMonsterDepth + 3 &&
						!( Type == KQuestMasterQuest && pDescription->BaseLevel() < 26 ) )
					{
						Done = kTrue;
						m_QuestMonsterBaseName = pDescription->Name();

					}
				}

				Done = kFalse;
				while( !Done )
				{
					uint32 Monster = RandomIntegerBetween( 0, pCharacterManifest.Characters() - 1 );

					CCharacterDescription* pMinionDescription = pCharacterManifest.CharacterDescription( Monster );
					if( pMinionDescription->Alignment() == KEvil &&
						pMinionDescription->WalkingSpeed() > 0 &&
						pMinionDescription->MinimumDepth() <= AcceptableMonsterDepth &&
						pMinionDescription->MaximumDepth() >= AcceptableMonsterDepth + 1 &&
						pMinionDescription->Minions() == 0 &&
						pMinionDescription->ExperienceAward() <= pDescription->ExperienceAward() )
					{
						Done = kTrue;
						m_QuestMinionMonsterBaseName = pMinionDescription->Name();
					}
				}

				if( RandomIntegerBetween( 0, 100 ) < 30 )
				{
					m_QuestMonsterName = pNameGenerator.GenerateName();
				}
				else
				{
					m_QuestMonsterName = pNameGenerator.GenerateSimpleName();
				}

				m_ExperienceReward = RandomIntegerBetween( m_Level * 100, m_Level * 500 );
				m_GoldReward = RandomIntegerBetween( m_Level * 500, m_Level * 1200 );
				m_FameReward = RandomIntegerBetween( m_Level * 50, m_Level * 100 );
				m_QuestName = LoadLocalizedString1Variable(936, m_GiverName );

				if( pItem != NULL )
				{
					m_pItemReward = new CItemSaveInstance();
					pItem->FillInstance( *m_pItemReward, 0, kFalse );
				}

				if( pQuestItem != NULL )
				{
					m_pQuestItem = new CItemSaveInstance();
					pQuestItem->FillInstance( *m_pQuestItem, 0, kFalse );
				}

				m_IncompleteDescription = LoadLocalizedString(937);

				switch( m_Type )
				{
				case KQuestMasterQuest :
					m_QuestMonsterCount = 1;
					m_ExperienceReward *= 2;
					m_GoldReward *= 2;
					m_FameReward *= 2;
					m_QuestMinionCount = RandomIntegerBetween( 4, 9 );
					m_CompletionDescription = LoadLocalizedString4Variables(938, m_QuestMonsterName, IntegerToString( m_ExperienceReward ), IntegerToString( m_FameReward ), IntegerToString( m_GoldReward ) );
					m_IncompleteDescription = LoadLocalizedString3Variables(939, m_QuestMonsterName, m_QuestMonsterBaseName, IntegerToString( m_Level ) );
					m_QuestMiniDescription = LoadLocalizedString4Variables(940, m_QuestMonsterName, m_QuestMonsterBaseName, m_QuestMinionMonsterBaseName, IntegerToString( m_Level ) );
					break;
				case KQuestKillArmy :
					m_QuestMonsterCount = 1;
					m_QuestMinionCount = RandomIntegerBetween( 3, 6 );
					m_CompletionDescription = LoadLocalizedString4Variables(941, m_QuestMonsterName, IntegerToString( m_ExperienceReward ), IntegerToString( m_FameReward ), IntegerToString( m_GoldReward ) );
					m_IncompleteDescription = LoadLocalizedString1Variable(942, m_QuestMonsterName );
					m_QuestMiniDescription = LoadLocalizedString4Variables(943, m_QuestMonsterName, m_QuestMonsterBaseName, m_QuestMinionMonsterBaseName, IntegerToString( m_Level ) );
					break;
				case KQuestArmyItem :
					m_QuestMonsterCount = 1;
					m_QuestItemCount = 1;
					m_QuestMinionCount = RandomIntegerBetween( 3, 6 );
					m_CompletionDescription = LoadLocalizedString5Variables(944, QuestItemName(), m_QuestMonsterName, IntegerToString( m_ExperienceReward ), IntegerToString( m_FameReward ), IntegerToString( m_GoldReward ) );
					m_IncompleteDescription = LoadLocalizedString2Variables(945, m_QuestMonsterName, QuestItemName() );
					m_QuestMiniDescription = LoadLocalizedString5Variables(946, m_QuestMonsterName, m_QuestMonsterBaseName, m_QuestMinionMonsterBaseName, IntegerToString( m_Level ), QuestItemName() );

					break;
				case KQuestKillMonster :
					m_QuestMonsterCount = 1;
					m_CompletionDescription = LoadLocalizedString4Variables(947, m_QuestMonsterName, IntegerToString( m_ExperienceReward ), IntegerToString( m_FameReward ), IntegerToString( m_GoldReward ) );
					m_IncompleteDescription = LoadLocalizedString1Variable(948, m_QuestMonsterName );
					m_QuestMiniDescription = LoadLocalizedString3Variables(949, m_QuestMonsterName, m_QuestMonsterBaseName, IntegerToString( m_Level ) );

					break;
				case KQuestKillMonsters :
					m_QuestMonsterCount = RandomIntegerBetween( 4, 10 );
					m_QuestMonsterName = m_QuestMonsterBaseName;
					m_CompletionDescription = LoadLocalizedString3Variables(950, IntegerToString( m_ExperienceReward ), IntegerToString( m_FameReward ), IntegerToString( m_GoldReward ) );
					m_IncompleteDescription = LoadLocalizedString1Variable(951, m_QuestMonsterBaseName );
					m_QuestMiniDescription = LoadLocalizedString3Variables(952, IntegerToString( m_QuestMonsterCount ), m_QuestMonsterName, IntegerToString( m_Level ) );
					break;
				case KQuestMonsterItem :
					m_QuestMonsterCount = 1;
					m_QuestItemCount = 1;
					m_CompletionDescription = LoadLocalizedString4Variables(953, QuestItemName(), IntegerToString( m_ExperienceReward ), IntegerToString( m_FameReward ), IntegerToString( m_GoldReward ) );
					m_QuestMiniDescription = LoadLocalizedString4Variables(954, m_QuestMonsterName, m_QuestMonsterBaseName, IntegerToString( m_Level ), QuestItemName() );
					m_IncompleteDescription = LoadLocalizedString1Variable(955, QuestItemName() );
					break;
				case KQuestFindItem :
					m_QuestItemCount = 1;
					m_CompletionDescription = LoadLocalizedString4Variables(956, QuestItemName(), IntegerToString( m_ExperienceReward ), IntegerToString( m_FameReward ), IntegerToString( m_GoldReward ) );
					m_QuestMiniDescription = LoadLocalizedString2Variables(957, QuestItemName(), IntegerToString( m_Level ) );
					m_IncompleteDescription = LoadLocalizedString1Variable(958, QuestItemName() );
					break;
				}

				if( m_pItemReward != NULL )
				{
					std::string RewardDescription = LoadLocalizedString1Variable(959, ItemRewardName() );
					while( RewardDescription.find( "\n" ) != -1 )
					{
						int32 Index = RewardDescription.find( "\n" );
						RewardDescription.replace( Index, 1, " " );
					}
					m_CompletionDescription = m_CompletionDescription + "\n\n\b" + LoadLocalizedString1Variable(960, ItemRewardName() );
					m_QuestMiniDescription = m_QuestMiniDescription + "\n" + RewardDescription;
				}

				std::string LevelText = IntegerToString( m_Level );

				//this block only makes sense in English
				if( CGame::GetLocaleString() == "en-US" || CGame::GetLocaleString() == "en-UK" )
				{
					int32 LastDigit = atoi( LevelText.substr( LevelText.length() - 1, 1 ).c_str() );
					if( m_Level >= 10 && m_Level <= 20 )
					{
						LevelText = LevelText + "th";
					}
					else if( LastDigit == 1 )
					{
						LevelText = LevelText + "st";
					}
					else if( LastDigit == 2 )
					{
						LevelText = LevelText + "nd";
					}
					else if( LastDigit == 3 )
					{
						LevelText = LevelText + "rd";
					}
					else
					{
						LevelText = LevelText + "th";
					}
				}
				while( m_QuestDescription.find( "<basemonster>" ) != -1 )
				{
					int32 Index = m_QuestDescription.find( "<basemonster>" );
					m_QuestDescription.replace( Index, 13, m_QuestMonsterBaseName );
				}
				while( m_QuestDescription.find( "<minion>" ) != -1 )
				{
					int32 Index = m_QuestDescription.find( "<minion>" );
					m_QuestDescription.replace( Index, 8, m_QuestMinionMonsterBaseName );
				}
				while( m_QuestDescription.find( "<monster>" ) != -1 )
				{
					int32 Index = m_QuestDescription.find( "<monster>" );
					m_QuestDescription.replace( Index, 9, m_QuestMonsterName );
				}
				while( m_QuestDescription.find( "<level>" ) != -1 )
				{
					int32 Index = m_QuestDescription.find( "<level>" );
					m_QuestDescription.replace( Index, 7, LevelText );
				}
				while( m_QuestDescription.find( "<levelsingular>" ) != -1 )
				{
					int32 Index = m_QuestDescription.find( "<levelsingular>" );
					m_QuestDescription.replace( Index, 15, IntegerToString( m_Level ) );
				}
				while( m_QuestDescription.find( "<monstercount>" ) != -1 )
				{
					int32 Index = m_QuestDescription.find( "<monstercount>" );
					m_QuestDescription.replace( Index, 14, IntegerToString( m_QuestMonsterCount ) );
				}
				while( m_QuestDescription.find( "<minioncount>" ) != -1 )
				{
					int32 Index = m_QuestDescription.find( "<minioncount>" );
					m_QuestDescription.replace( Index, 13, IntegerToString( m_QuestMinionCount ) );
				}

				if( m_pItemReward != NULL )
				{
					while( m_QuestDescription.find( "<rewarditem>" ) != -1 )
					{
						int32 Index = m_QuestDescription.find( "<rewarditem>" );
						m_QuestDescription.replace( Index, 12, ItemRewardName() );
					}
				}

				if( m_pQuestItem != NULL )
				{
					while( m_QuestDescription.find( "<item>" ) != -1 )
					{
						int32 Index = m_QuestDescription.find( "<item>" );
						m_QuestDescription.replace( Index, 6, QuestItemName() );
					}
				}

				if( m_pItemReward != NULL )
				{
					m_QuestDescription = m_QuestDescription + LoadLocalizedString1Variable(961, ItemRewardName() );
				}



				if( m_Type != KQuestMasterQuest )
				{
					// remove any carriage returns so this can wrap properly
					while( m_QuestDescription.find( "\n" ) != -1 )
					{
						int32 Index = m_QuestDescription.find( "\n" );
						m_QuestDescription.replace( Index, 1, " " );
					}
				}
				// remove any carriage returns so this can wrap properly
				while( m_QuestMiniDescription.find( "\n" ) != -1 )
				{
					int32 Index = m_QuestMiniDescription.find( "\n" );
					m_QuestMiniDescription.replace( Index, 1, " " );
				}
				while( m_IncompleteDescription.find( "\n" ) != -1 )
				{
					int32 Index = m_IncompleteDescription.find( "\n" );
					m_IncompleteDescription.replace( Index, 1, " " );
				}

				// give a little helper text for low-level quests to direct them to the dungeon
				if( m_Level <= 3 )
				{
					m_QuestDescription = m_QuestDescription + LoadLocalizedString(962);
				}

			};


			CQuest( CQuest& pQuest ) :
					m_Level( pQuest.m_Level ),
					m_Type( pQuest.m_Type ),
					m_Spawned( pQuest.m_Spawned ),
					m_Completed( pQuest.m_Completed ),
					m_CompletionNotified( pQuest.m_CompletionNotified ),
					m_GiverName( pQuest.m_GiverName ),
					m_QuestMonsterName( pQuest.m_QuestMonsterName ),
					m_QuestMonsterBaseName( pQuest.m_QuestMonsterBaseName ),
					m_QuestMinionMonsterBaseName( pQuest.m_QuestMinionMonsterBaseName ),
					m_QuestDescription( pQuest.m_QuestDescription ),
					m_QuestMiniDescription( pQuest.m_QuestMiniDescription ),
					m_QuestName( pQuest.m_QuestName ),
					m_CompletionDescription( pQuest.m_CompletionDescription ),
					m_IncompleteDescription( pQuest.m_IncompleteDescription ),
					m_QuestMonsterCount( pQuest.m_QuestMonsterCount ),
					m_QuestMinionCount( pQuest.m_QuestMinionCount ),
					m_QuestItemCount( pQuest.m_QuestItemCount ),
					m_QuestMonstersCompleted( pQuest.m_QuestMonstersCompleted ),
					m_QuestMinionsCompleted( pQuest.m_QuestMinionsCompleted ),
					m_QuestItemsCompleted( pQuest.m_QuestItemsCompleted ),
					m_GoldReward( pQuest.m_GoldReward ),
					m_ExperienceReward( pQuest.m_ExperienceReward ),
					m_FameReward( pQuest.m_FameReward ),
					m_pQuestItem( NULL ),
					m_pItemReward( NULL )
			{
				if( pQuest.m_pItemReward != NULL )
				{
					m_pItemReward = new CItemSaveInstance( *pQuest.m_pItemReward );
				}
				if( pQuest.m_pQuestItem != NULL )
				{
					m_pQuestItem = new CItemSaveInstance( *pQuest.m_pQuestItem );
				}
			};

			CQuest( void ) :
				m_pQuestItem( NULL ),
				m_pItemReward( NULL )
			{
			}

			~CQuest( void )
			{
				DELETE_SAFELY( m_pItemReward );
				DELETE_SAFELY( m_pQuestItem );
			};

	void Save( FILE* pFile )	// open file to save to
	{
		fwrite( &m_Level, sizeof( int32 ), 1, pFile );

		fwrite( &m_Type, sizeof( EQuestType ), 1, pFile );

		fwrite( &m_Spawned, sizeof( bool ), 1, pFile );

		fwrite( &m_Completed, sizeof( bool ), 1, pFile );

		fwrite( &m_CompletionNotified, sizeof( bool ), 1, pFile );

		fwrite( &m_QuestMonsterCount, sizeof( int32 ), 1, pFile );
		fwrite( &m_QuestMonstersCompleted, sizeof( int32 ), 1, pFile );
		fwrite( &m_QuestMinionCount, sizeof( int32 ), 1, pFile );
		fwrite( &m_QuestMinionsCompleted, sizeof( int32 ), 1, pFile );

		fwrite( &m_QuestItemCount, sizeof( int32 ), 1, pFile );
		fwrite( &m_QuestItemsCompleted, sizeof( int32 ), 1, pFile );

		fwrite( &m_GoldReward, sizeof( uint32 ), 1, pFile );

		fwrite( &m_ExperienceReward, sizeof( uint32 ), 1, pFile );

		fwrite( &m_FameReward, sizeof( uint32 ), 1, pFile );

		uint16 Size = m_QuestName.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_QuestName.c_str(), sizeof( char ) * Size, 1, pFile );

		Size = m_QuestMonsterName.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_QuestMonsterName.c_str(), sizeof( char ) * Size, 1, pFile );

		Size = m_QuestMonsterBaseName.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_QuestMonsterBaseName.c_str(), sizeof( char ) * Size, 1, pFile );

		Size = m_QuestMinionMonsterBaseName.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_QuestMinionMonsterBaseName.c_str(), sizeof( char ) * Size, 1, pFile );

		Size = m_GiverName.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_GiverName.c_str(), sizeof( char ) * Size, 1, pFile );


		Size = m_QuestDescription.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_QuestDescription.c_str(), sizeof( char ) * Size, 1, pFile );

		Size = m_QuestMiniDescription.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_QuestMiniDescription.c_str(), sizeof( char ) * Size, 1, pFile );

		Size = m_CompletionDescription.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_CompletionDescription.c_str(), sizeof( char ) * Size, 1, pFile );

		Size = m_IncompleteDescription.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_IncompleteDescription.c_str(), sizeof( char ) * Size, 1, pFile );

		bool QuestItem( kFalse );
		if( m_pQuestItem != NULL )
		{
			QuestItem = kTrue;
		}

		fwrite( &QuestItem, sizeof( bool ), 1, pFile );

		if( QuestItem )
		{
			m_pQuestItem->Save( pFile );
		}

		QuestItem = kFalse;
		if( m_pItemReward != NULL )
		{
			QuestItem = kTrue;
		}

		fwrite( &QuestItem, sizeof( bool ), 1, pFile );

		if( QuestItem )
		{
			m_pItemReward->Save( pFile );
		}
	}

	void Load( FILE* pFile )	// item manifest
	{
		fread( &m_Level, sizeof( int32 ), 1, pFile );

		fread( &m_Type, sizeof( EQuestType ), 1, pFile );

		fread( &m_Spawned, sizeof( bool ), 1, pFile );

		fread( &m_Completed, sizeof( bool ), 1, pFile );

		fread( &m_CompletionNotified, sizeof( bool ), 1, pFile );

		fread( &m_QuestMonsterCount, sizeof( int32 ), 1, pFile );
		fread( &m_QuestMonstersCompleted, sizeof( int32 ), 1, pFile );
		fread( &m_QuestMinionCount, sizeof( int32 ), 1, pFile );
		fread( &m_QuestMinionsCompleted, sizeof( int32 ), 1, pFile );

		fread( &m_QuestItemCount, sizeof( int32 ), 1, pFile );
		fread( &m_QuestItemsCompleted, sizeof( int32 ), 1, pFile );

		fread( &m_GoldReward, sizeof( uint32 ), 1, pFile );

		fread( &m_ExperienceReward, sizeof( uint32 ), 1, pFile );

		fread( &m_FameReward, sizeof( uint32 ), 1, pFile );

		uint16 Size;
		fread( &Size, sizeof( uint16 ), 1, pFile );
		if( Size > 0 )
		{
			m_QuestName = ReadString( pFile, Size );
		}

		fread( &Size, sizeof( uint16 ), 1, pFile );
		if( Size > 0 )
		{
			m_QuestMonsterName = ReadString( pFile, Size );
		}

		fread( &Size, sizeof( uint16 ), 1, pFile );
		if( Size > 0 )
		{
			m_QuestMonsterBaseName = ReadString( pFile, Size );
		}

		fread( &Size, sizeof( uint16 ), 1, pFile );
		if( Size > 0 )
		{
			m_QuestMinionMonsterBaseName = ReadString( pFile, Size );
		}

		fread( &Size, sizeof( uint16 ), 1, pFile );
		if( Size > 0 )
		{
			m_GiverName = ReadString( pFile, Size );
		}

		fread( &Size, sizeof( uint16 ), 1, pFile );
		if( Size > 0 )
		{
			m_QuestDescription = ReadString( pFile, Size );
		}

		fread( &Size, sizeof( uint16 ), 1, pFile );
		if( Size > 0 )
		{
			m_QuestMiniDescription = ReadString( pFile, Size );
		}

		fread( &Size, sizeof( uint16 ), 1, pFile );
		if( Size > 0 )
		{
			m_CompletionDescription = ReadString( pFile, Size );
		}

		fread( &Size, sizeof( uint16 ), 1, pFile );
		if( Size > 0 )
		{
			m_IncompleteDescription = ReadString( pFile, Size );
		}

		bool QuestItem( kFalse );

		DELETE_SAFELY( m_pItemReward );
		m_pItemReward = NULL;
		DELETE_SAFELY( m_pQuestItem );
		m_pQuestItem = NULL;

		fread( &QuestItem, sizeof( bool ), 1, pFile );

		if( QuestItem )
		{
			m_pQuestItem = new CItemSaveInstance();
			m_pQuestItem->Load( pFile );
		}

		fread( &QuestItem, sizeof( bool ), 1, pFile );

		if( QuestItem )
		{
			m_pItemReward = new CItemSaveInstance();
			m_pItemReward->Load( pFile );

		}

	}


////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	bool				CompletionNotified( void )	{	return m_CompletionNotified;		};

	uint32				GoldReward( void )		{	return m_GoldReward;		};

	uint32				ExperienceReward( void ){	return m_ExperienceReward;	};

	uint32				FameReward( void )		{	return m_FameReward;		};

	const std::string&	CompletionDescription( void )		{	return m_CompletionDescription;			};

	const std::string&	IncompleteDescription( void )		{	return m_IncompleteDescription;			};

	const std::string&	GiverName( void )		{	return m_GiverName;			};

	CItemSaveInstance*	ItemReward( void )		{	return m_pItemReward;		};

	CItemSaveInstance*	QuestItem( void )		{	return m_pQuestItem;		};

	uint32				MonstersRemaining( void )	{	return m_QuestMonsterCount -
															   m_QuestMonstersCompleted;	};

	uint32				MinionsRemaining( void )	{	return m_QuestMinionCount -
															   m_QuestMinionsCompleted;		};


	std::string			ItemRewardName( void )		
						{	
							if( m_pItemReward != NULL )
							{
								return m_pItemReward->m_Name;
							}
							return "";
						};

	std::string			QuestItemName( void )		
						{	
							if( m_pQuestItem != NULL )
							{
								return m_pQuestItem->m_Name;
							}
							return "";
						};

	int32				Level( void )			{	return m_Level;				};
	
	EQuestType			Type( void )			{	return m_Type;				};

	const std::string&	QuestMonsterName( void ){	return m_QuestMonsterName;	};

	const std::string&	QuestMinionMonsterName( void ){	return m_QuestMinionMonsterBaseName;	};

	bool				Completed( void )		{	return m_Completed;			};

	bool				Spawned( void )			{	return m_Spawned;			};

	const std::string&	Name( void )			{	return m_QuestName;			};

	const std::string&	Description( void )		{	return m_QuestDescription;	};

	const std::string&	MiniDescription( void )	{	return m_QuestMiniDescription;	};

	uint32				QuestItems( void )		{	return m_QuestItemCount;	};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void				SetSpawned( bool State )	{	m_Spawned = State;		};

	void				SetCompletionNotified( bool State )	{	m_CompletionNotified = State;	};

	void				IncreaseMonstersCompleted( void )	
						{
							if( !m_Completed )
							{
								m_QuestMonstersCompleted++;
								if( m_QuestMonstersCompleted >= 
									m_QuestMonsterCount &&
									m_QuestMinionsCompleted >= 
									m_QuestMinionCount &&
									m_QuestItemsCompleted >=
									m_QuestItemCount )
								{
									m_Completed = kTrue;
								}
								if( m_QuestMonstersCompleted > m_QuestMonsterCount )
								{
									m_QuestMonstersCompleted = m_QuestMonsterCount;
								}
							}
						}

	void				IncreaseMinionsCompleted( void )	
						{
							if( !m_Completed )
							{
								m_QuestMinionsCompleted++;
								if( m_QuestMonstersCompleted >= 
									m_QuestMonsterCount &&
									m_QuestMinionsCompleted >= 
									m_QuestMinionCount &&
									m_QuestItemsCompleted >=
									m_QuestItemCount )
								{
									m_Completed = kTrue;
								}
								if( m_QuestMinionsCompleted > m_QuestMinionCount )
								{
									m_QuestMinionsCompleted = m_QuestMinionCount;
								}
							}
						}

						void NotifyOfItemReceipt( CItem* pItem )
						{
							if( !m_Completed )
							{
								if( m_Type == KQuestFindItem ||
									m_Type == KQuestMonsterItem ||
									m_Type == KQuestArmyItem )
								{
									if( m_pQuestItem != NULL )
									{
										if( m_pQuestItem->m_Unique == pItem->IsUnique() &&
											m_pQuestItem->m_Name == pItem->Name() &&
											m_pQuestItem->m_Sockets == pItem->Sockets() &&
											m_pQuestItem->m_BaseModifiedName == pItem->BaseModifiedName() )
										{
											m_QuestItemsCompleted++;
											
											if( m_QuestMonstersCompleted >= 
												m_QuestMonsterCount &&
												m_QuestMinionsCompleted >=
												m_QuestMinionCount && 
												m_QuestItemsCompleted >=
												m_QuestItemCount )
											{
												m_Completed = kTrue;
											}
										}
									}
								}
							}
						}

	bool	ItemMatches( CItem* pItem )
	{
		if( pItem != NULL &&
			m_pQuestItem != NULL )
		{
			if( pItem->Sockets() == m_pQuestItem->m_Sockets &&
				pItem->IsUnique() == m_pQuestItem->m_Unique &&
				pItem->Effects( KActivationPassive ) == m_pQuestItem->m_pEffects[KActivationPassive].size() &&
				pItem->Name() == m_pQuestItem->m_Name )
			{
				return kTrue;
			}
				
		}
		return kFalse;
	}

	void	Spawn( CLevel& pLevel )	// level to spawn quest to
	{
		SetSpawned( kTrue );

		CCharacterDescription* pDescription;
		CItemDescription* pItemDescription;
		CCharacter* pMonster;
		CCharacter* pMinion;
		CItem* pItem;
		uint32 i( 0 );
		D3DXVECTOR3 Position;
		int32 MonsterLevel = Level();

		MonsterLevel = (int32)ceil( (float32)MonsterLevel * KDifficultyMultiplier[pLevel.Difficulty()] );
		MonsterLevel += (int32)KDifficultyLevelBase[pLevel.Difficulty()];
		if( MonsterLevel < 1 )
		{
			MonsterLevel = 1;
		}

		// we make a 'safe zone' around the stairwell so that we don't attack players immediately
		D3DXVECTOR3 StairsPosition = pLevel.PositionNearStairsUp();
		StairsPosition.y = 0;
		float32 SafetyRadius = 42;

		Position = pLevel.RandomOpenPosition();
		int32 Tries( 0 );
		while( D3DXVec3Length( &( Position - StairsPosition ) ) < SafetyRadius &&
			Tries < 100 )
		{
			Position = pLevel.RandomOpenPosition();
			Tries++;
		}

		switch( m_Type )
		{
		case KQuestMasterQuest :
			pDescription = pLevel.CharacterManifest()->CharacterDescription( m_QuestMonsterBaseName );

			if( pDescription != NULL )
			{
				pMonster = pLevel.AddMonster( *pDescription, Position );
				pLevel.OutfitCharacter( *pDescription,
										*pMonster,
										kFalse );
				pMonster->MakeUnique( m_QuestMonsterName,
									  MonsterLevel + 10, 
									  kTrue );
			}

			pDescription = pLevel.CharacterManifest()->CharacterDescription( m_QuestMinionMonsterBaseName );

			if( pDescription != NULL )
			{
				for( int32 i = 0; i < m_QuestMinionCount; i++ )
				{
					pMinion = pLevel.AddMonster( *pDescription, pLevel.RandomOpenPosition( Position, 20 ) );
					pLevel.OutfitCharacter( *pDescription,
											*pMinion,
											kFalse );
					pMonster->AddPet( pMinion );
					pMinion->MakeUnique( pLevel.NameGenerator(),
										 MonsterLevel,
										 kFalse );
				}
			}
			m_QuestMinionCount = 0;	// don't actually have to kill minions for the master quest
			break;
		case KQuestKillArmy :
			pDescription = pLevel.CharacterManifest()->CharacterDescription( m_QuestMonsterBaseName );

			if( pDescription != NULL )
			{
				pMonster = pLevel.AddMonster( *pDescription, Position );
				pLevel.OutfitCharacter( *pDescription,
										*pMonster,
										kFalse );
				pMonster->MakeUnique( m_QuestMonsterName,
									  MonsterLevel + 2,
									  kFalse );
			}

			pDescription = pLevel.CharacterManifest()->CharacterDescription( m_QuestMinionMonsterBaseName );

			if( pDescription != NULL )
			{
				for( int32 i = 0; i < m_QuestMinionCount; i++ )
				{
					pMinion = pLevel.AddMonster( *pDescription, pLevel.RandomOpenPosition( Position, 20 ) );
					pLevel.OutfitCharacter( *pDescription,
											*pMinion,
											kFalse );
					pMonster->AddPet( pMinion );
					pMinion->TweakToLevel( MonsterLevel + 2 );
				}
			}

			break;
		case KQuestArmyItem :
			pDescription = pLevel.CharacterManifest()->CharacterDescription( m_QuestMonsterBaseName );

			if( pDescription != NULL )
			{
				pMonster = pLevel.AddMonster( *pDescription, Position );
				pLevel.OutfitCharacter( *pDescription,
										*pMonster,
										kFalse );
				pMonster->MakeUnique( m_QuestMonsterName,
									  MonsterLevel + 2,
									  kFalse );

				pItemDescription = pLevel.ItemManifest()->ItemDescription( m_pQuestItem->m_BaseName );
				if( pItemDescription != NULL )
				{
					pItem = pLevel.AddItem( *pItemDescription, pLevel.RandomOpenPosition() );
					pItem->ApplyInstance( pLevel.D3DDevice(),
										  *m_pQuestItem,
										  *pLevel.ItemManifest() );
					pLevel.RemoveItem( pItem );
					pMonster->GiveItem( pLevel, pItem );
				}

			}

			pDescription = pLevel.CharacterManifest()->CharacterDescription( m_QuestMinionMonsterBaseName );

			if( pDescription != NULL )
			{
				for( int32 i = 0; i < m_QuestMinionCount; i++ )
				{
					pMinion = pLevel.AddMonster( *pDescription, pLevel.RandomOpenPosition( Position, 20 ) );
					pLevel.OutfitCharacter( *pDescription,
											*pMinion,
											kFalse );
					pMinion->TweakToLevel( MonsterLevel + 2 );
					pMonster->AddPet( pMinion );
				}
			}

			break;
		case KQuestKillMonster :
			pDescription = pLevel.CharacterManifest()->CharacterDescription( m_QuestMonsterBaseName );

			if( pDescription != NULL )
			{
				pMonster = pLevel.AddMonster( *pDescription, Position );
				pLevel.OutfitCharacter( *pDescription,
										*pMonster,
										kFalse );
				pMonster->MakeUnique( m_QuestMonsterName,
									  MonsterLevel + 2,
									  kFalse );
			}
			break;
		case KQuestKillMonsters :
			pDescription = pLevel.CharacterManifest()->CharacterDescription( m_QuestMonsterBaseName );

			if( pDescription != NULL )
			{
				for( int32 i = 0; i < m_QuestMonsterCount; i++ )
				{
					pMonster = pLevel.AddMonster( *pDescription, pLevel.RandomOpenPosition() );
					pLevel.OutfitCharacter( *pDescription,
											*pMonster,
											kFalse );
				}
			}
			break;
		case KQuestMonsterItem :
			pDescription = pLevel.CharacterManifest()->CharacterDescription( m_QuestMonsterBaseName );

			if( pDescription != NULL )
			{
				pMonster = pLevel.AddMonster( *pDescription, Position );
				pLevel.OutfitCharacter( *pDescription,
										*pMonster,
										kFalse );
				pMonster->MakeUnique( m_QuestMonsterName,
									  MonsterLevel + 2,
									  kFalse );

				pItemDescription = pLevel.ItemManifest()->ItemDescription( m_pQuestItem->m_BaseName );
				if( pItemDescription != NULL )
				{
					pItem = pLevel.AddItem( *pItemDescription, pLevel.RandomOpenPosition() );
					pItem->ApplyInstance( pLevel.D3DDevice(),
										  *m_pQuestItem,
										  *pLevel.ItemManifest() );
					pLevel.RemoveItem( pItem );
					pMonster->GiveItem( pLevel, pItem );
				}
			}
			break;
		case KQuestFindItem :

			pItemDescription = pLevel.ItemManifest()->ItemDescription( m_pQuestItem->m_BaseName );
			if( pItemDescription != NULL )
			{
				pItem = pLevel.AddItem( *pItemDescription, Position );
				pItem->ApplyInstance( pLevel.D3DDevice(),
									  *m_pQuestItem,
									  *pLevel.ItemManifest() );
				if( RandomIntegerBetween( 0, 50 ) )
				{
					pLevel.RemoveItem( pItem );
					pLevel.AddItemToRandomContainer( pItem );
				}
			}
			break;
		}
	}


protected:

	int32		m_Level;

	EQuestType	m_Type;

	std::string	m_GiverName;

	std::string	m_QuestMonsterName;

	std::string m_QuestMonsterBaseName;

	std::string m_QuestMinionMonsterBaseName;

	std::string	m_QuestName;

	uint32		m_QuestMonsterCount;

	uint32		m_QuestMinionCount;

	uint32		m_QuestItemCount;

	uint32		m_QuestMonstersCompleted;

	uint32		m_QuestMinionsCompleted;

	uint32		m_QuestItemsCompleted;

	uint32		m_GoldReward;

	uint32		m_ExperienceReward;

	uint32		m_FameReward;

	std::string	m_QuestDescription;

	std::string	m_QuestMiniDescription;

	std::string	m_CompletionDescription;

	std::string	m_IncompleteDescription;

	bool		m_Spawned;

	bool		m_Completed;

	bool		m_CompletionNotified;

	CItemSaveInstance*		m_pItemReward;
	CItemSaveInstance*		m_pQuestItem;
};

#endif