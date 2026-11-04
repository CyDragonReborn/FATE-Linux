/*
	inventory.cpp

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

#include "../ITEM/item.h"

#include "../LEVEL/level.h"

#include "character.h"
#include "inventory.h"
#include "../MEDIA/material.h"

#include "../UTILITIES/StatsAndAchievements.h"

CInventory::CInventory( CCharacter& pCharacter,		// parent character
					    uint32 MaxSlots ) : // maximum number of storage slots
												m_pCharacter( pCharacter ),
												m_MaximumSlots( KQuickSlots + MaxSlots ),
												m_Gold( 0 ),
												m_AllowQuickSlots( kFalse ),
												m_pItemList ( NULL ),
												m_SlotsHigh ( 0 )

{
	m_SlotsHigh = MaxSlots / KInventorySlotsWide;
	for( uint32 i = 0; i < KEquipmentSlots; i++ )
	{
		m_pEquippedItems[i] = NULL;
		SetCanEquip( (EItemSlot)i, kFalse );
	}
	for( i = 0; i < KItemTypes; i++ )
	{
		SetSupportsItemType( (EItemType)i, kFalse );
	}
	m_pItemList = new CLList< CItemRef >();
	CalculateEffectValues();
} // CInventory::CInventory()

CInventory::~CInventory( void )
{
	DELETE_SAFELY( m_pItemList );
} // CInventory::~CInventory()

void CInventory::Resize( uint32 MaxSlots ) // slots to resize to
{
	m_MaximumSlots = KQuickSlots + MaxSlots;
	m_SlotsHigh = MaxSlots / KInventorySlotsWide;
} // CInventory::Resize( )

bool CInventory::AddItem( CItem* pItem,		// item to add to the inventory
						  CLevel& pLevel )	// level to open items into
{
	if( pItem == NULL )
	{
		return kFalse;
	}

	if( pItem->Type() == KItemGold )
	{
		m_pCharacter.NotifyOfItemReceipt( pItem, &pLevel );
		AddGold( pItem->Value() );
		return kTrue;
	}
	int32 Slot( -1 );
	if( pItem->Type() == KItemPotion &&
		m_AllowQuickSlots )
	{
		Slot = FindFreeQuickSlot();
	}
	if( Slot == -1 )
	{
		Slot = FindFreeSlot( pItem->SlotsWide(),
							 pItem->SlotsTall() );
	}
	if( Slot == -1 )
	{
		// ok, no free slots were found. We're desperate! Try to equip it!
		for( uint32 i = 0; i < KEquipmentSlots; i++ )
		{
			if( FitsSlot( i, pItem ) &&
				GetItemFromSlot( i ) == NULL )
			{
				if( !( i == KSlotRightHand && GetItemFromSlot( KSlotLeftHand ) != NULL &&
					   KItemTwoHanded[GetItemFromSlot( KSlotLeftHand )->Type()] ) &&
					!( i == KSlotLeftArm && GetItemFromSlot( KSlotLeftHand ) != NULL ) &&
					!( i == KSlotLeftArm && GetItemFromSlot( KSlotRightHand ) != NULL &&
					   KItemTwoHanded[GetItemFromSlot( KSlotRightHand )->Type()] ) &&
					!( i == KSlotLeftHand && GetItemFromSlot( KSlotLeftArm ) != NULL ) &&
					!( i == KSlotLeftHand && GetItemFromSlot( KSlotRightHand ) != NULL &&
					   KItemTwoHanded[GetItemFromSlot( KSlotRightHand )->Type()] ) )
				{
					Slot = i;
					break;
				}
			}
		}
	}

	//Collect different gems for achievement
	if(m_pCharacter.IsPlayer() && pItem->Type() == KItemGem )
	{
		SyncGems(pItem->IconMaterial()->GetPath());
	}
	//Collect different fish for achievement
	if(m_pCharacter.IsPlayer() && pItem->Type() == KItemPetFood )
	{
		SyncCaughtFish(pItem->IconMaterial()->GetPath());
	}

	if( Slot != -1 )
	{
		m_pCharacter.NotifyOfItemReceipt( pItem, &pLevel );
		CItemRef* pRef = new CItemRef( pItem,
									  Slot );
		m_pItemList->Queue( pRef );
		if( Slot < KEquipmentSlots )
		{
			EquipItem( pLevel, pRef, (EItemSlot)Slot );
		}
		CalculateEffectValues();

		return kTrue;
	}
	return kFalse;
} // CInventory::AddItem()

bool CInventory::AddItem( CItem* pItem,		// item to add to the inventory
						  uint32 Slot,		// slot to place in
						  CLevel* pLevel )	// level to open items into
{
	if( pItem == NULL ||
		ItemsInSlot( Slot, pItem->SlotsWide(), pItem->SlotsTall() ) > 0 )
	{
		return kFalse;
	}

	m_pCharacter.NotifyOfItemReceipt( pItem, pLevel );

	if( pItem->Type() == KItemGold )
	{
		AddGold( pItem->Value() );
		return kTrue;
	}

	//Collect different gems for achievement
	if(m_pCharacter.IsPlayer() && pItem->Type() == KItemGem )
	{
		SyncGems(pItem->IconMaterial()->GetPath());
	}
	//Collect different fish for achievement
	if(m_pCharacter.IsPlayer() && pItem->Type() == KItemPetFood )
	{
		SyncCaughtFish(pItem->IconMaterial()->GetPath());
	}

	CItemRef* pRef = new CItemRef( pItem,
								   Slot );
	m_pItemList->Queue( pRef );
	CalculateEffectValues();
	return kTrue;
} // CInventory::AddItem()

uint32 CInventory::ItemsInSlot( uint32 SlotIndex,	// top left slot
								uint32 Width,		// width of item
								uint32 Height )		// height of item
{
	uint32 Items( 0 );
	if( SlotIndex >= KQuickSlots )
	{
		uint32 Y = (uint32)floor( (float32)( SlotIndex - KQuickSlots ) / (float32)KInventorySlotsWide );
		uint32 X = ( SlotIndex - KQuickSlots ) - Y * KInventorySlotsWide;
		uint32 YMax = Y + Height - 1;
		uint32 XMax = X + Width - 1;

		CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
		while( Root != NULL )
		{
			if( Root->Data()->m_SlotIndex >= KQuickSlots )
			{
				uint32 SubY = (uint32)floor( (float32)( Root->Data()->m_SlotIndex - KQuickSlots ) / (float32)KInventorySlotsWide );
				uint32 SubX = ( Root->Data()->m_SlotIndex - KQuickSlots ) - SubY * KInventorySlotsWide;
				uint32 SubYMax = SubY + Root->Data()->m_pItem->SlotsTall() - 1;
				uint32 SubXMax = SubX + Root->Data()->m_pItem->SlotsWide() - 1;

				if( SubY <= YMax && SubYMax >= Y &&
					SubX <= XMax && SubXMax >= X )
				{
					Items++;
				}
			}

			Root = Root->Next();
		}
	}
	else
	{
		if( GetItemFromSlot( SlotIndex ) != NULL )
		{
			Items = 1;
		}
	}
	return Items;
} // CInventory::ItemsInSlot()

int32 CInventory::FindFreeSlot( uint32 Width,	// width of item
								uint32 Height )	// height of item
{
	// start in the slots that are not equipment slots
	int32 Slot( KQuickSlots );
	int32 SlotsTried( KQuickSlots );

	// sort horizontally for merchants
	if( m_pCharacter.IsMerchant() )
	{
		while( SlotsTried < (int32)m_MaximumSlots )
		{
			uint32 Y = (uint32)floor( (float32)( Slot - KQuickSlots ) / (float32)KInventorySlotsWide );
			uint32 X = ( Slot - KQuickSlots ) - Y * KInventorySlotsWide;
			if( ItemsInSlot( Slot, Width, Height ) == 0 &&
				X + Width <= KInventorySlotsWide &&
				Y + Height <= m_SlotsHigh )
			{
				break;
			}
			if( SlotsTried < (int32)m_MaximumSlots )
			{
				Slot ++;
			}
			SlotsTried++;
		}
	}
	else // vertically for players
	{
		while( SlotsTried < (int32)m_MaximumSlots )
		{
			uint32 Y = (uint32)floor( (float32)( Slot - KQuickSlots ) / (float32)KInventorySlotsWide );
			uint32 X = ( Slot - KQuickSlots ) - Y * KInventorySlotsWide;
			if( ItemsInSlot( Slot, Width, Height ) == 0 &&
				X + Width <= KInventorySlotsWide &&
				Y + Height <= m_SlotsHigh )
			{
				break;
			}
			if( SlotsTried < (int32)m_MaximumSlots )
			{
				Slot += KInventorySlotsWide;
				if( Slot >= (int32)m_MaximumSlots )
				{
					Slot -= ( KInventorySlotsWide * m_SlotsHigh );
					Slot++;
				}
			}
			SlotsTried++;
		}
	}
	if( SlotsTried >= (int32)m_MaximumSlots )
	{
		Slot = -1;
	}
	return Slot;
} // CInventory::FindFreeSlot()

int32 CInventory::FindFreeQuickSlot( void )
{
	// quickslots not allowed for merchants
	if( m_pCharacter.IsMerchant() )
	{
		return -1;
	}
	// start in the slots that are not equipment slots
	int32 Slot( KEquipmentSlots );

	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while( Root != NULL )
	{
		if( Root->Data()->m_SlotIndex == Slot )
		{
			Slot++;
			Root = m_pItemList->GetFirst();			
			continue;
		}

		Root = Root->Next();
	}
	if( Slot >= (int32)KQuickSlots )
	{
		Slot = -1;
	}
	return Slot;
} // CInventory::FindFreeQuickSlot()

bool CInventory::RequirementsMet( CItem* pItem )	// item to check
{
	if( !SupportsItemType( pItem->Type() ) )
	{
		return kFalse;
	}
	for( uint32 i = 0; i < pItem->Requirements(); i++ )
	{
		if( !m_pCharacter.RequirementMet( pItem->RequirementStat( i ),
										  pItem->RequirementValue( i ),
										  kTrue ) )
		{
			return kFalse;
		}

	}
	return kTrue;
} // CInventory::RequirementsMet()

bool CInventory::FitsSlot( uint32 Slot,		// slot to check
						   CItem* pItem )	// item to check for fit
{
	// general inventory slots always fit
	if( Slot >= KEquipmentSlots &&
		Slot < m_MaximumSlots )
	{
		if( Slot < KQuickSlots &&
			( ( pItem->Type() != KItemPotion && pItem->Type() != KItemScroll && pItem->Type() != KItemPetFood )
			  || !m_AllowQuickSlots ) )
		{
			return kFalse;
		}
		if( Slot >= KQuickSlots )
		{
			uint32 Y = (uint32)floor( (float32)( Slot - KQuickSlots ) / (float32)KInventorySlotsWide );
			uint32 X = ( Slot - KQuickSlots ) - Y * KInventorySlotsWide;
			if( X + pItem->SlotsWide() > KInventorySlotsWide ||
				Y + pItem->SlotsTall() > m_SlotsHigh )
			{
				return kFalse;
			}
		}
		if( ItemsInSlot( Slot, pItem->SlotsWide(), pItem->SlotsTall() ) > 1 )
		{
			return kFalse;
		}
		return kTrue;
	}

	if( !SupportsItemType( pItem->Type() ) )
	{
		return kFalse;
	}

	switch( Slot )
	{
	case KSlotChest :
		if( pItem->Type() == KItemShirt && RequirementsMet( pItem ) )
		{
			return kTrue;
		}
		break;
	case KSlotGloves :
		if( pItem->Type() == KItemGloves && RequirementsMet( pItem ) )
		{
			return kTrue;
		}
		break;
	case KSlotFeet :
		if( pItem->Type() == KItemBoots && RequirementsMet( pItem ) )
		{
			return kTrue;
		}
		break;
	case KSlotHead :
		if( pItem->Type() == KItemHelmet && RequirementsMet( pItem ) )
		{
			return kTrue;
		}
		break;
	case KSlotNeck :
		if( pItem->Type() == KItemNecklace && RequirementsMet( pItem ) )
		{
			return kTrue;
		}
		break;
	case KSlotLeftFinger :
	case KSlotRightFinger :
		if( pItem->Type() == KItemRing && RequirementsMet( pItem ) )
		{
			return kTrue;
		}
		break;
	case KSlotBelt :
		if( pItem->Type() == KItemBelt && RequirementsMet( pItem ) )
		{
			return kTrue;
		}
		break;
	case KSlotLeftArm :
		if( pItem->Type() == KItemShield && RequirementsMet( pItem ) )
		{
			return kTrue;
		}
		break;
	case KSlotRightHand :
		if( pItem->Category() == KCategoryWeapon && RequirementsMet( pItem ) )
		{
			return kTrue;
		}
		break;
	case KSlotLeftHand :
		if( pItem->Category() == KCategoryWeapon && RequirementsMet( pItem ) )
		{
			return kTrue;
		}
		break;
	}
	return kFalse;
} // CInventory::FitsSlot()


CItem* CInventory::RemoveItemFromSlot( uint32 Slot )			// slot to retrieve item from
{									   
	uint32 ActualSlot( 0 );
	return RemoveItemFromSlot( Slot, ActualSlot );
} // CInventory::RemoveItemFromSlot()

CItem* CInventory::RemoveItemFromSlot( uint32 SlotIndex,			// slot to retrieve item from
									   uint32 Width,		// width to check within
									   uint32 Height,		// height to check within
									   uint32& ActualSlot )	// slot the retrieved item was actually in
{
	if( SlotIndex < KQuickSlots )
	{
		return RemoveItemFromSlot( SlotIndex, ActualSlot );
	}
	for( uint32 x = 0; x < Width; x++ )
	{
		for( uint32 y = 0; y < Height; y++ )
		{
			if( x < KInventorySlotsWide &&
				y < m_SlotsHigh )
			{
				uint32 Slot = SlotIndex + x + y * KInventorySlotsWide;
				if( Slot < m_MaximumSlots )
				{
					CItem* pItem = RemoveItemFromSlot( Slot, ActualSlot );
					if( pItem != NULL )
					{
						return pItem;
					}
				}
			}
		}
	}
	return NULL;
} // CInventory::RemoveItemFromSlot()

CItem* CInventory::RemoveItemFromSlot( uint32 Slot,				// slot to retrieve item from
									   uint32& ActualSlot )	// slot the retrieved item was actually in
{
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	CLLNode<CItemRef>* Next;
	while( Root != NULL )
	{
		Next = Root->Next();
		if( Root->Data()->m_SlotIndex >= KQuickSlots )
		{
			for( int32 x = 0; x < Root->Data()->m_pItem->SlotsWide(); x++ )
			{
				for( int32 y = 0; y < Root->Data()->m_pItem->SlotsTall(); y++ )
				{
					int32 SlotIndex = Root->Data()->m_SlotIndex + x + y * KInventorySlotsWide;
					if( SlotIndex == Slot )
					{
						ActualSlot = Root->Data()->m_SlotIndex;
						CItem* pReturnedItem = Root->Data()->m_pItem;
						if( Root->Data()->m_SlotIndex < KEquipmentSlots )
						{
							m_pEquippedItems[Root->Data()->m_SlotIndex] = NULL;
							Root->Data()->m_Equipped = kFalse;
							Root->Data()->m_pItem->SetInWorld( kFalse );
						}
						if( m_pCharacter.ActiveWeapon() == Root->Data()->m_pItem )
						{
							m_pCharacter.ClearCurrentAttack();
						}
						Root->Data()->RemoveItem();
						m_pItemList->Remove( Root );
						CalculateEffectValues();
						return pReturnedItem;
					}
				}
			}
		}
		else
		{
			if( Root->Data()->m_SlotIndex == Slot )
			{
				ActualSlot = Root->Data()->m_SlotIndex;
				CItem* pReturnedItem = Root->Data()->m_pItem;
				if( Root->Data()->m_SlotIndex < KEquipmentSlots )
				{
					m_pEquippedItems[Root->Data()->m_SlotIndex] = NULL;
					Root->Data()->m_Equipped = kFalse;
					Root->Data()->m_pItem->SetInWorld( kFalse );
				}
				if( m_pCharacter.ActiveWeapon() == Root->Data()->m_pItem )
				{
					m_pCharacter.ClearCurrentAttack();
				}
				Root->Data()->RemoveItem();
				m_pItemList->Remove( Root );
				CalculateEffectValues();
				return pReturnedItem;
			}
		}

		Root = Next;
	}
	return NULL;
} // CInventory::RemoveItemFromSlot()

CItem* CInventory::RemoveItem( CItem* pItem )	// slot to retrieve item from
{
	uint32 ActualSlot( 0 );
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	CLLNode<CItemRef>* Next;
	while( Root != NULL )
	{
		Next = Root->Next();
		if( Root->Data()->m_pItem == pItem )
		{
			return RemoveItemFromSlot( Root->Data()->m_SlotIndex, ActualSlot );
		}

		Root = Next;
	}
	return NULL;
} // CInventory::RemoveItem()

CItemRef* CInventory::GetItemRefFromSlot( uint32 Slot )			// slot to retrieve item from
{
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while( Root != NULL )
	{
		if( Root->Data()->m_SlotIndex == Slot )
		{
			return Root->Data();
		}
		Root = Root->Next();
	}
	return NULL;
} // CInventory::GetItemFromSlot();

CItem* CInventory::GetItemFromSlot( uint32 Slot )			// slot to retrieve item from
{
	uint32 ActualSlot( 0 );
	return GetItemFromSlot( Slot, ActualSlot );
} // CInventory::GetItemFromSlot();

CItem* CInventory::GetItemFromSlot( uint32 SlotIndex,		// slot to retrieve item from
								    uint32 Width,			// width to check within
									uint32 Height,			// height to check within
									uint32& ActualSlot )	// slot the retrieved item was actually in
{
	if( SlotIndex < KQuickSlots )
	{
		return GetItemFromSlot( SlotIndex, ActualSlot );
	}
	for( uint32 x = 0; x < Width; x++ )
	{
		for( uint32 y = 0; y < Height; y++ )
		{
			if( x < KInventorySlotsWide &&
				y < m_SlotsHigh )
			{
				uint32 Slot = SlotIndex + x + y * KInventorySlotsWide;
				if( Slot < m_MaximumSlots )
				{
					CItem* pItem = GetItemFromSlot( Slot, ActualSlot );
					if( pItem != NULL )
					{
						return pItem;
					}
				}
			}
		}
	}
	return NULL;
} // CInventory::GetItemFromSlot()

CItem* CInventory::GetItemFromSlot( uint32 Slot,			// slot to retrieve item from
									uint32& ActualSlot )	// slot the retrieved item was actually in
{

	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while( Root != NULL )
	{
		if( Root->Data()->m_SlotIndex >= KQuickSlots )
		{
			for( int32 x = 0; x < Root->Data()->m_pItem->SlotsWide(); x++ )
			{
				for( int32 y = 0; y < Root->Data()->m_pItem->SlotsTall(); y++ )
				{
					int32 SlotIndex = Root->Data()->m_SlotIndex + x + y * KInventorySlotsWide;
					if( SlotIndex == Slot )
					{
						ActualSlot = Root->Data()->m_SlotIndex;
						return Root->Data()->m_pItem;
					}
				}
			}
		}
		else
		{
			if( Root->Data()->m_SlotIndex == Slot )
			{
				ActualSlot = Root->Data()->m_SlotIndex;
				return Root->Data()->m_pItem;
			}
		}
		Root = Root->Next();
	}
	return NULL;
} // CInventory::GetItemFromSlot()


void CInventory::EquipItem( CLevel& pLevel,	// level to open items into
						    uint32 Slot )	// slot to retrieve item from
{

	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while( Root != NULL )
	{
		if( Root->Data()->m_SlotIndex == Slot )
		{
			if( Slot < KEquipmentSlots )
			{
				EquipItem( pLevel, Root->Data(), (EItemSlot)Slot );
			}
			else
			{
				EquipItem( pLevel, Root->Data() );
			}
			CalculateEffectValues();
			VerifyEquipment( pLevel );
			return;
		}

		Root = Root->Next();
	}
} // CInventory::GetItemFromSlot()

CItem* CInventory::GetItem( CItem* pItem )	// item to retrieve
{
	uint32 ActualSlot( 0 );
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while( Root != NULL )
	{
		if( Root->Data()->m_pItem == pItem )
		{
			return GetItemFromSlot( Root->Data()->m_SlotIndex, ActualSlot );
		}
		Root = Root->Next();
	}
	return NULL;
} // CInventory::GetItem()

void CInventory::DropAll( CLevel& pLevel )						// level to open items into
{
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	CLLNode<CItemRef>* Next; 
	while( Root != NULL )
	{
		Next = Root->Next();
		CItem* pItem = RemoveItem( Root->Data()->m_pItem );
		if( m_pCharacter.ActiveWeapon() == pItem )
		{
			m_pCharacter.ClearCurrentAttack();
		}
		pLevel.AddItem( pItem, m_pCharacter.Position() );
		pItem->Drop( pLevel );
		Root = Next;
	}

	m_pItemList->Clear();

	CalculateEffectValues();

	if( Gold() > 0 )
	{
		pLevel.AddGold( Gold(), m_pCharacter.Position() );
	}
	m_Gold = 0;
} // CInventory::DropAll()

void CInventory::DropGold( CLevel& pLevel )						// level to open items into
{
	if( Gold() > 0 )
	{
		pLevel.AddGold( Gold(), m_pCharacter.Position() );
	}
	m_Gold = 0;
} // CInventory::DropGold()

void CInventory::DropAllButEquipment( CLevel& pLevel )						// level to open items into
{
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	CLLNode<CItemRef>* Next; 
	while( Root != NULL )
	{
		Next = Root->Next();
		if( Root->Data()->m_pItem->Effects( KActivationPassive ) > 0 ||
			Root->Data()->m_pItem->IsUnique() ||
			Root->Data()->m_pItem->DamageBonuses() > 0 ||
			( Root->Data()->m_pItem->Category() != KCategoryWeapon &&
			  Root->Data()->m_pItem->Type() != KItemShield ) )
		{
			CItem* pItem = RemoveItem( Root->Data()->m_pItem );
			pLevel.AddItem( pItem, m_pCharacter.Position() );
			pItem->Drop( pLevel );
		}
		Root = Next;
	}


	CalculateEffectValues();

	if( Gold() > 0 )
	{
		pLevel.AddGold( Gold(), m_pCharacter.Position() );
	}
	m_Gold = 0;
} // CInventory::DropAllButEquipment()

void CInventory::AddGold( uint32 Gold )	// gold to add
{
	m_Gold += Gold;
	m_pCharacter.IncrementStat( KJournalStatGoldGathered, Gold );
} // CInventory::AddGold()

void CInventory::EquipBest( CLevel& pLevel,				// level to open items into
						    bool EquipUnIdentified )	//	equip unidentified objects?
{
	EquipBestWeapon( pLevel, EquipUnIdentified );
	EquipBestHelmet( pLevel, EquipUnIdentified );
	EquipBestShield( pLevel, EquipUnIdentified );
	EquipBestBelt( pLevel, EquipUnIdentified );
	EquipBestGloves( pLevel, EquipUnIdentified );
	EquipBestBoots( pLevel, EquipUnIdentified );
	EquipBestShirt( pLevel, EquipUnIdentified );
} // CInventory::EquipBest()

void CInventory::EquipEmpty( CLevel& pLevel,			// level to open items into
							 bool EquipUnIdentified )	//	equip unidentified objects?
{
	if( !SlotEquipped( KSlotRightHand ) &&
		!SlotEquipped( KSlotLeftHand ) )
	{
		EquipBestWeapon( pLevel, EquipUnIdentified );
	}
	if( !SlotEquipped( KSlotHead ) )
	{
		EquipBestHelmet( pLevel, EquipUnIdentified );
	}
	if( !SlotEquipped( KSlotLeftHand ) &&
		!SlotEquipped( KSlotLeftArm ) )
	{
		EquipBestShield( pLevel, EquipUnIdentified );
	}
	if( !SlotEquipped( KSlotBelt ) )
	{
		EquipBestBelt( pLevel, EquipUnIdentified );
	}
	if( !SlotEquipped( KSlotGloves ) )
	{
		EquipBestGloves( pLevel, EquipUnIdentified );
	}
	if( !SlotEquipped( KSlotFeet ) )
	{
		EquipBestBoots( pLevel, EquipUnIdentified );
	}
	if( !SlotEquipped( KSlotChest ) )
	{
		EquipBestShirt( pLevel, EquipUnIdentified );
	}
} // CInventory::EquipEmpty()

void CInventory::EquipBestWeapon( CLevel& pLevel,			// level to open items into
								  bool EquipUnIdentified )	//	equip unidentified objects?
{
	CItemRef* pBestWeapon = NULL;
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while( Root != NULL )
	{
		if( Root->Data()->m_pItem->Category() == KCategoryWeapon )
		{
			if( ( pBestWeapon == NULL || 
				  Root->Data()->m_pItem->MaximumDamage() > 
				   pBestWeapon->m_pItem->MaximumDamage() ) &&
				   !Root->Data()->m_Equipped &&
				  !Root->Data()->m_pItem->IsCursed() &&
				  RequirementsMet( Root->Data()->m_pItem ) &&
				  ( EquipUnIdentified || 
				    Root->Data()->m_pItem->Identified() ||
					Root->Data()->m_pItem->Effects( KActivationPassive ) == 0 ) &&
				  SupportsItemType( Root->Data()->m_pItem->Type() ) )
			{
				pBestWeapon = Root->Data();
			}
		}
		Root = Root->Next();
	}

	EquipItem( pLevel, pBestWeapon );
} // CInventory::EquipBestWeapon()

void CInventory::EquipBestDualWieldWeapon( CLevel& pLevel,			// level to open items into
										   bool EquipUnIdentified )	//	equip unidentified objects?
{
	CItemRef* pBestWeapon = NULL;
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while( Root != NULL )
	{
		if( Root->Data()->m_pItem->Category() == KCategoryWeapon )
		{
			if( ( pBestWeapon == NULL || 
				  Root->Data()->m_pItem->MaximumDamage() > 
				   pBestWeapon->m_pItem->MaximumDamage() ) &&
				   !Root->Data()->m_Equipped &&
				  !Root->Data()->m_pItem->IsCursed() &&
				  !KItemTwoHanded[Root->Data()->m_pItem->Type()] &&
				  RequirementsMet( Root->Data()->m_pItem ) &&
				  ( EquipUnIdentified || Root->Data()->m_pItem->Identified() ||
					Root->Data()->m_pItem->Effects( KActivationPassive ) == 0  ) &&
				  SupportsItemType( Root->Data()->m_pItem->Type() ) )
			{
				pBestWeapon = Root->Data();
			}
		}
		Root = Root->Next();
	}

	if( GetItemFromSlot( KSlotRightHand ) != NULL &&
		KItemTwoHanded[GetItemFromSlot( KSlotRightHand )->Type()] )
	{
		pBestWeapon = NULL;
	}
	if( pBestWeapon != NULL &&
		GetItemFromSlot( KSlotLeftArm ) == NULL )
	{
		EquipItem( pLevel, pBestWeapon, KSlotLeftHand );
	}
} // CInventory::EquipBestDualWieldWeapon()

void CInventory::EquipBestHelmet( CLevel& pLevel,			// level to open items into
								  bool EquipUnIdentified )	//	equip unidentified objects?
{
	CItemRef* pBestHelm = NULL;
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while(Root != NULL)
	{
		if( Root->Data()->m_pItem->Type() == KItemHelmet )
		{
			if( ( pBestHelm == NULL || 
				  Root->Data()->m_pItem->ArmorBonus() > 
				  pBestHelm->m_pItem->ArmorBonus() ) &&
				  !Root->Data()->m_Equipped &&
				  !Root->Data()->m_pItem->IsCursed() &&
				  RequirementsMet( Root->Data()->m_pItem ) &&
				  ( EquipUnIdentified || Root->Data()->m_pItem->Identified() ||
					Root->Data()->m_pItem->Effects( KActivationPassive ) == 0 ) &&
				  SupportsItemType( Root->Data()->m_pItem->Type() ) )
			{
				pBestHelm = Root->Data();
			}
		}
		Root = Root->Next();
	}

	EquipItem( pLevel, pBestHelm );
} // CInventory::EquipBestHelmet()

void CInventory::EquipBestBelt( CLevel& pLevel,				// level to open items into
								bool EquipUnIdentified )	//	equip unidentified objects?
{
	CItemRef* pBestBelt = NULL;
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while(Root != NULL)
	{
		if( Root->Data()->m_pItem->Type() == KItemBelt )
		{
			if( ( pBestBelt == NULL || 
				  Root->Data()->m_pItem->ArmorBonus() > 
				  pBestBelt->m_pItem->ArmorBonus() ) &&
				  !Root->Data()->m_Equipped &&
				  !Root->Data()->m_pItem->IsCursed() &&
				  RequirementsMet( Root->Data()->m_pItem ) &&
				  ( EquipUnIdentified || Root->Data()->m_pItem->Identified() ||
					Root->Data()->m_pItem->Effects( KActivationPassive ) == 0 ) &&
				  SupportsItemType( Root->Data()->m_pItem->Type() ) )
			{
				pBestBelt = Root->Data();
			}
		}
		Root = Root->Next();
	}

	EquipItem( pLevel, pBestBelt );
} // CInventory::EquipBestBelt()

void CInventory::EquipBestGloves( CLevel& pLevel,			// level to open items into
								  bool EquipUnIdentified )	//	equip unidentified objects?
{
	CItemRef* pBestGloves = NULL;
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while(Root != NULL)
	{
		if( Root->Data()->m_pItem->Type() == KItemGloves )
		{
			if( ( pBestGloves == NULL || 
				  Root->Data()->m_pItem->ArmorBonus() > 
				  pBestGloves->m_pItem->ArmorBonus() ) &&
				  !Root->Data()->m_Equipped &&
				  !Root->Data()->m_pItem->IsCursed() &&
				  RequirementsMet( Root->Data()->m_pItem ) &&
				  ( EquipUnIdentified || Root->Data()->m_pItem->Identified() ||
					Root->Data()->m_pItem->Effects( KActivationPassive ) == 0 ) &&
				  SupportsItemType( Root->Data()->m_pItem->Type() ) )
			{
				pBestGloves = Root->Data();
			}
		}
		Root = Root->Next();
	}

	EquipItem( pLevel, pBestGloves );
} // CInventory::EquipBestGloves()

void CInventory::EquipBestBoots( CLevel& pLevel,			// level to open items into
								 bool EquipUnIdentified )	//	equip unidentified objects?
{
	CItemRef* pBestBoots = NULL;
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while(Root != NULL)
	{
		if( Root->Data()->m_pItem->Type() == KItemBoots )
		{
			if( ( pBestBoots == NULL || 
				  Root->Data()->m_pItem->ArmorBonus() > 
				  pBestBoots->m_pItem->ArmorBonus() ) &&
				  !Root->Data()->m_Equipped &&
				  !Root->Data()->m_pItem->IsCursed() &&
				  RequirementsMet( Root->Data()->m_pItem ) &&
				  ( EquipUnIdentified || Root->Data()->m_pItem->Identified() ||
					Root->Data()->m_pItem->Effects( KActivationPassive ) == 0 ) &&
				  SupportsItemType( Root->Data()->m_pItem->Type() ) )
			{
				pBestBoots = Root->Data();
			}
		}
		Root = Root->Next();
	}

	EquipItem( pLevel, pBestBoots );
} // CInventory::EquipBestBoots()

void CInventory::EquipBestShirt( CLevel& pLevel,			// level to open items into
								 bool EquipUnIdentified )	//	equip unidentified objects?
{
	CItemRef* pBestShirt = NULL;
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while(Root != NULL)
	{
		if( Root->Data()->m_pItem->Type() == KItemShirt )
		{
			if( ( pBestShirt == NULL || 
				  Root->Data()->m_pItem->ArmorBonus() > 
				  pBestShirt->m_pItem->ArmorBonus() ) &&
				  !Root->Data()->m_Equipped &&
				  !Root->Data()->m_pItem->IsCursed() &&
				  RequirementsMet( Root->Data()->m_pItem ) &&
				  ( EquipUnIdentified || Root->Data()->m_pItem->Identified() ||
					Root->Data()->m_pItem->Effects( KActivationPassive ) == 0 ) &&
				  SupportsItemType( Root->Data()->m_pItem->Type() ) )
			{
				pBestShirt = Root->Data();
			}
		}
		Root = Root->Next();
	}

	EquipItem( pLevel, pBestShirt );
} // CInventory::EquipBestShirt()

void CInventory::EquipBestShield( CLevel& pLevel,			// level to open items into
								  bool EquipUnIdentified )	//	equip unidentified objects?
{
	// don't bother if a two-handed weapon is equipped
	if( GetItemFromSlot( KSlotRightHand ) != NULL &&
		KItemTwoHanded[GetItemFromSlot( KSlotRightHand )->Type()] )
	{
		return;
	}
	if( GetItemFromSlot( KSlotLeftHand ) != NULL &&
		KItemTwoHanded[GetItemFromSlot( KSlotLeftHand )->Type()] )
	{
		return;
	}

	CItemRef* pBestShield = NULL;
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while(Root != NULL)
	{
		if( Root->Data()->m_pItem->Type() == KItemShield )
		{

			if( ( pBestShield == NULL || 
				  Root->Data()->m_pItem->ArmorBonus() > 
				  pBestShield->m_pItem->ArmorBonus() ) &&
				  !Root->Data()->m_Equipped &&
				  !Root->Data()->m_pItem->IsCursed() &&
				  RequirementsMet( Root->Data()->m_pItem ) &&
				  ( EquipUnIdentified || Root->Data()->m_pItem->Identified() ||
					Root->Data()->m_pItem->Effects( KActivationPassive ) == 0 ) &&
				  SupportsItemType( Root->Data()->m_pItem->Type() ) )
			{
				pBestShield = Root->Data();
			}
		}
		Root = Root->Next();
	}

	EquipItem( pLevel, pBestShield );
} // CInventory::EquipBestShield()

void CInventory::VerifyEquipment( CLevel& pLevel )		// level to open items into
{
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	CLLNode<CItemRef>* Next;
	while(Root != NULL)
	{
		Next = Root->Next();
		if( Root->Data()->m_SlotIndex < KEquipmentSlots && 
			!RequirementsMet( Root->Data()->m_pItem ) )
		{
			UnEquipItem( pLevel, (EItemSlot)Root->Data()->m_SlotIndex );
		}
		Root = Next;
	}

} // CInventory::VerifyEquipment()

void CInventory::SetSlotEquipped( EItemSlot Slot ) // slot to equip in( default is EquipmentSlots, which will autodetect )
{
	if( GetItemFromSlot( Slot ) != NULL )
	{
		CItemRef* pItem = GetItemRefFromSlot( Slot );
		pItem->m_Equipped = kTrue;
		m_pEquippedItems[Slot] = pItem;
	}
} // CInventory::SetSlotEquipped()

void CInventory::EquipItem( CLevel& pLevel,		// level to open items into
						    CItemRef* pItem,	// item to equip
						    EItemSlot Slot )	// slot to equip in( default is EquipmentSlots, which will autodetect )
{
	if( pItem == NULL )
	{
		return;
	}
	if( !RequirementsMet( pItem->m_pItem ) )
	{
		return;
	}
	if( !SupportsItemType( pItem->m_pItem->Type() ) )
	{
		return;
	}
	uint32 ActualSlot( 0 );

	bool AlreadyInSlot( kFalse );
	if( pItem->m_SlotIndex == Slot )
	{
		AlreadyInSlot = kTrue;
	}
	switch( pItem->m_pItem->Category() )
	{
	case KCategoryWeapon :
		if( KItemTwoHanded[pItem->m_pItem->Type()] )
		{
			if( KItemRightHanded[pItem->m_pItem->Type()] )
			{
				if( CanEquip( KSlotRightHand ) )
				{
					// free up our current slot
					pItem->m_SlotIndex = -1;
					if( !AlreadyInSlot || Slot != KSlotRightHand )
					{
						UnEquipItem( pLevel, KSlotRightHand );
					}
					UnEquipItem( pLevel, KSlotLeftHand );
					UnEquipItem( pLevel, KSlotLeftArm );
					pItem->m_SlotIndex = KSlotRightHand;
					pItem->m_LastSlotIndex = pItem->m_SlotIndex;
					pItem->m_Equipped = kTrue;
					m_pEquippedItems[KSlotRightHand] = pItem;
				}
			}
			else
			{
				if( CanEquip( KSlotLeftHand ) )
				{
					// free up our current slot
					pItem->m_SlotIndex = -1;
					if( !AlreadyInSlot || Slot != KSlotLeftHand )
					{
						UnEquipItem( pLevel, KSlotLeftHand );
					}
					UnEquipItem( pLevel, KSlotRightHand );
					UnEquipItem( pLevel, KSlotLeftArm );
					pItem->m_SlotIndex = KSlotLeftHand;
					pItem->m_LastSlotIndex = pItem->m_SlotIndex;
					pItem->m_Equipped = kTrue;
					m_pEquippedItems[KSlotLeftHand] = pItem;
				}
			}
		}
		else
		{
			if( Slot == KSlotLeftHand &&
				CanEquip( KSlotLeftHand ) )
			{
				// free up our current slot
				pItem->m_SlotIndex = -1;
				if( GetItemFromSlot( KSlotRightHand, ActualSlot ) != NULL &&
					KItemTwoHanded[GetItemFromSlot( KSlotRightHand, ActualSlot )->Type()] )
				{
					UnEquipItem( pLevel, KSlotRightHand );
				}

				if( GetItemFromSlot( KSlotLeftArm, ActualSlot ) != NULL  )
				{
					UnEquipItem( pLevel, KSlotLeftArm );
				}
				if( !AlreadyInSlot )
				{
					UnEquipItem( pLevel, KSlotLeftHand );
				}
				pItem->m_SlotIndex = KSlotLeftHand;
				pItem->m_LastSlotIndex = pItem->m_SlotIndex;
				pItem->m_Equipped = kTrue;
				m_pEquippedItems[KSlotLeftHand] = pItem;
			}
			else if( CanEquip( KSlotRightHand ) )
			{
				// free up our current slot
				pItem->m_SlotIndex = -1;
				if( GetItemFromSlot( KSlotLeftHand, ActualSlot ) != NULL &&
					KItemTwoHanded[GetItemFromSlot( KSlotLeftHand, ActualSlot )->Type()] )
				{
					UnEquipItem( pLevel, KSlotLeftHand );
				}
				if( !AlreadyInSlot )
				{
					UnEquipItem( pLevel, KSlotRightHand );
				}
				pItem->m_SlotIndex = KSlotRightHand;
				pItem->m_LastSlotIndex = pItem->m_SlotIndex;
				pItem->m_Equipped = kTrue;
				m_pEquippedItems[KSlotRightHand] = pItem;
			}
		}
		break;
	case KCategoryJewelry :
		switch( pItem->m_pItem->Type() )
		{
		case KItemNecklace :
			if( CanEquip( KSlotNeck ) )
			{
				// free up our current slot
				pItem->m_SlotIndex = -1;
				if( !AlreadyInSlot )
				{
					UnEquipItem( pLevel, KSlotNeck );
				}
				pItem->m_SlotIndex = KSlotNeck;
				pItem->m_LastSlotIndex = pItem->m_SlotIndex;
				pItem->m_Equipped = kTrue;
				m_pEquippedItems[KSlotNeck] = pItem;
			}
			break;
		case KItemRing :
			if( Slot == KSlotRightFinger )
			{
				if( CanEquip( KSlotRightFinger ) )
				{
					// free up our current slot
					pItem->m_SlotIndex = -1;
					if( !AlreadyInSlot )
					{
						UnEquipItem( pLevel, KSlotRightFinger );
					}
					pItem->m_SlotIndex = KSlotRightFinger;
					pItem->m_LastSlotIndex = pItem->m_SlotIndex;
					pItem->m_Equipped = kTrue;
					m_pEquippedItems[KSlotRightFinger] = pItem;
				}
			}
			else 
			{
				// if no slot was specified...
				if( Slot == KEquipmentSlots )
				{
					if( CanEquip( KSlotLeftFinger ) &&
						( GetItemFromSlot( KSlotLeftFinger, ActualSlot ) == NULL ||
						  !CanEquip( KSlotRightFinger ) ) )
					{
						// free up our current slot
						pItem->m_SlotIndex = -1;
						if( !AlreadyInSlot )
						{
							UnEquipItem( pLevel, KSlotLeftFinger );
						}
						pItem->m_SlotIndex = KSlotLeftFinger;
						pItem->m_LastSlotIndex = pItem->m_SlotIndex;
						pItem->m_Equipped = kTrue;
						m_pEquippedItems[KSlotLeftFinger] = pItem;
					}
					else if( CanEquip( KSlotRightFinger ) )
					{
						// free up our current slot
						pItem->m_SlotIndex = -1;
						if( !AlreadyInSlot )
						{
							UnEquipItem( pLevel, KSlotRightFinger );
						}
						pItem->m_SlotIndex = KSlotRightFinger;
						pItem->m_LastSlotIndex = pItem->m_SlotIndex;
						pItem->m_Equipped = kTrue;
						m_pEquippedItems[KSlotRightFinger] = pItem;
					}
				}
				else
				{
					if( CanEquip( KSlotLeftFinger ) )
					{
						// free up our current slot
						pItem->m_SlotIndex = -1;
						if( !AlreadyInSlot )
						{
							UnEquipItem( pLevel, KSlotLeftFinger );
						}
						pItem->m_SlotIndex = KSlotLeftFinger;
						pItem->m_LastSlotIndex = pItem->m_SlotIndex;
						pItem->m_Equipped = kTrue;
						m_pEquippedItems[KSlotLeftFinger] = pItem;
					}
				}
			}
			break;
		}
		break;
	case KCategoryArmor :
		switch( pItem->m_pItem->Type() )
		{
		case KItemBelt :
			if( CanEquip( KSlotBelt ) )
			{
				// free up our current slot
				pItem->m_SlotIndex = -1;
				if( !AlreadyInSlot )
				{
					UnEquipItem( pLevel, KSlotBelt );
				}
				pItem->m_SlotIndex = KSlotBelt;
				pItem->m_LastSlotIndex = pItem->m_SlotIndex;
				pItem->m_Equipped = kTrue;
				m_pEquippedItems[KSlotBelt] = pItem;
			}
			break;
		case KItemGloves :
			if( CanEquip( KSlotGloves ) )
			{
				// free up our current slot
				pItem->m_SlotIndex = -1;
				if( !AlreadyInSlot )
				{
					UnEquipItem( pLevel, KSlotGloves );
				}
				pItem->m_SlotIndex = KSlotGloves;
				pItem->m_LastSlotIndex = pItem->m_SlotIndex;
				pItem->m_Equipped = kTrue;
				m_pEquippedItems[KSlotGloves] = pItem;
			}
			break;
		case KItemBoots :
			if( CanEquip( KSlotFeet ) )
			{
				// free up our current slot
				pItem->m_SlotIndex = -1;
				if( !AlreadyInSlot )
				{
					UnEquipItem( pLevel, KSlotFeet );
				}
				pItem->m_SlotIndex = KSlotFeet;
				pItem->m_LastSlotIndex = pItem->m_SlotIndex;
				pItem->m_Equipped = kTrue;
				m_pEquippedItems[KSlotFeet] = pItem;
			}
			break;
		case KItemShirt :
			if( CanEquip( KSlotChest ) )
			{
				// free up our current slot
				pItem->m_SlotIndex = -1;
				if( !AlreadyInSlot )
				{
					UnEquipItem( pLevel, KSlotChest );
				}
				pItem->m_SlotIndex = KSlotChest;
				pItem->m_LastSlotIndex = pItem->m_SlotIndex;
				pItem->m_Equipped = kTrue;
				m_pEquippedItems[KSlotChest] = pItem;
			}
			break;
		case KItemHelmet :
			if( CanEquip( KSlotHead ) )
			{
				// free up our current slot
				pItem->m_SlotIndex = -1;
				if( !AlreadyInSlot )
				{
					UnEquipItem( pLevel, KSlotHead );
				}
				pItem->m_SlotIndex = KSlotHead;
				pItem->m_LastSlotIndex = pItem->m_SlotIndex;
				pItem->m_Equipped = kTrue;
				m_pEquippedItems[KSlotHead] = pItem;
			}
			break;
		case KItemShield :
			if( CanEquip( KSlotLeftArm ) )
			{
				if( GetItemFromSlot( KSlotRightHand, ActualSlot ) != NULL &&
					KItemTwoHanded[GetItemFromSlot( KSlotRightHand, ActualSlot )->Type()] )
				{
					UnEquipItem( pLevel, KSlotRightHand );
				}

				// free up our current slot
				pItem->m_SlotIndex = -1;
				// can't have anything in left hand when equipping a shield
				UnEquipItem( pLevel, KSlotLeftHand );
				if( !AlreadyInSlot )
				{
					UnEquipItem( pLevel, KSlotLeftArm );
				}
				pItem->m_SlotIndex = KSlotLeftArm;
				pItem->m_LastSlotIndex = pItem->m_SlotIndex;
				pItem->m_Equipped = kTrue;
				m_pEquippedItems[KSlotLeftArm] = pItem;
			}
			break;
		}
		break;
	}
	CalculateEffectValues();
	VerifyEquipment( pLevel );
} // CInventory::EquipItem()

void CInventory::UnEquipItem( CLevel& pLevel,	// level to open items into
							  CItemRef* pItem )	// item to unequip
{

	if( pItem == NULL || !pItem->m_Equipped )
	{
		CalculateEffectValues();
		VerifyEquipment( pLevel );
		return;
	}

	if( pItem->m_SlotIndex < KEquipmentSlots )
	{
		UnEquipItem( pLevel, (EItemSlot)pItem->m_SlotIndex );
	}

	CalculateEffectValues();

	VerifyEquipment( pLevel );
} // CInventory::UnEquipItem(

void CInventory::UnEquipItem( CLevel& pLevel,	// level to open items into
							  EItemSlot Slot )	// slot to unequip item from
{
	uint32 ActualSlot( 0 );
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	CLLNode<CItemRef>* Next;
	while( Root != NULL )
	{
		Next = Root->Next();
		if( Root->Data()->m_SlotIndex == Slot )
		{
			int32 ItemSlot = FindFreeSlot( Root->Data()->m_pItem->SlotsWide(),
									   Root->Data()->m_pItem->SlotsTall() );
			if( ItemSlot == -1 )
			{
				CItem* pItem = RemoveItemFromSlot( Slot, ActualSlot );
				pLevel.AddItem( pItem, m_pCharacter.Position() );
				pItem->Drop( pLevel );
				CalculateEffectValues();
				return;
			}
			else
			{
				m_pEquippedItems[Root->Data()->m_SlotIndex] = NULL;
				Root->Data()->m_SlotIndex = FindFreeSlot( Root->Data()->m_pItem->SlotsWide(),
														  Root->Data()->m_pItem->SlotsTall() );
				Root->Data()->m_LastSlotIndex = Root->Data()->m_SlotIndex;
				Root->Data()->m_Equipped = kFalse;
				Root->Data()->m_pItem->SetInWorld( kFalse );
				CalculateEffectValues();
			}
			return;
		}
		Root = Next;
	}
	CalculateEffectValues();

	VerifyEquipment( pLevel );
} // CInventory::UnEquipItem()

void CInventory::SetCanEquip( EItemSlot Slot,		// slot to set state for
							  bool State )			// state to set
{
	m_CanEquip[Slot] = State;
} // CInventory::SetCanEquip()

bool CInventory::CanEquip( EItemSlot Slot )		// slot to check
{
	return m_CanEquip[Slot];
} // CInventory::SetCanEquip()

void CInventory::SetSupportsItemType( EItemType Type,		// type to support
									  bool State )			// state to set
{
	m_SupportsItemType[Type] = State;
} // CInventory::SetSupportsItemType()

bool CInventory::SupportsItemType( EItemType Type )		// type to support
{
	return m_SupportsItemType[Type];
} // CInventory::SupportsItemType()

bool CInventory::SlotEquipped( EItemSlot Slot )		// slot to check
{
	if( m_pEquippedItems[Slot] != NULL )
	{
		return kTrue;
	}
	return kFalse;
} // CInventory::SlotEquipped()

int32 CInventory::ArmorBonus( void )
{
	int32 Bonus( 0 );
	for( uint32 i = 0; i < KEquipmentSlots; i++ )
	{
		if( m_pEquippedItems[i] != NULL &&
			m_pEquippedItems[i]->m_pItem->Category() == KCategoryArmor )
		{
			Bonus += m_pEquippedItems[i]->m_pItem->ArmorBonus();
		}
	}
	return Bonus;
} // CInventory::ArmorBonus()

float32 CInventory::CalculateEffectValue( EEffectType Type )				// type of effect
{
	float32 Value( 0 );
	for( uint32 i = 0; i < KEquipmentSlots; i++ )
	{
		if( m_pEquippedItems[i] != NULL )
		{
			Value += m_pEquippedItems[i]->m_pItem->EffectValue( KActivationPassive, Type );
		}
	}
	return Value;
} // CInventory::CalculateEffectValue()

float32 CInventory::EffectValue( EEffectType Type )				// type of effect
{
	return m_EffectValue[Type];
} // CInventory::EffectValue()

void CInventory::CalculateEffectValues( void )
{
	for( uint32 i = 0; i < KAllEffectTypes; i++ )
	{
		m_EffectValue[i] = CalculateEffectValue( (EEffectType)i );
	}
} // CInventory::CalculateEffectValues()

bool CInventory::HasItems( CItemSaveInstance& pInstance,	// item to check for
						   uint32 Count )					// number of copies
{
	uint32 Counter( 0 );
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while(Root != NULL)
	{
		if( Root->Data()->m_pItem->IsUnique() == pInstance.m_Unique &&
			Root->Data()->m_pItem->Sockets() == pInstance.m_Sockets &&
			Root->Data()->m_pItem->Name() == pInstance.m_Name &&
			Root->Data()->m_pItem->BaseModifiedName() == pInstance.m_BaseModifiedName )
		{
			Counter++;
		}
		Root = Root->Next();
	}

	if( Counter >= Count )
	{
		return kTrue;
	}

	return kFalse;
} // CInventory::HasItems()

bool CInventory::HasItems( std::string Name,			// item to check for
						   uint32 Count )					// number of copies
{
	Name = StringUpper( Name );
	uint32 Counter( 0 );
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while(Root != NULL)
	{
		if( StringUpper( Root->Data()->m_pItem->Name() ) ==
			Name )
		{
			Counter++;
		}
		Root = Root->Next();
	}

	if( Counter >= Count )
	{
		return kTrue;
	}

	return kFalse;
} // CInventory::HasItems()

bool CInventory::HasItems( EItemType Type,					// item type to check for
						   uint32 Count )					// number of copies
{
	uint32 Counter( 0 );
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while(Root != NULL)
	{
		if( Root->Data()->m_pItem->Type() == Type )
		{
			Counter++;
		}
		Root = Root->Next();
	}

	if( Counter >= Count )
	{
		return kTrue;
	}

	return kFalse;
} // CInventory::HasItems()

CItem* CInventory::FindItem( CItemSaveInstance& pInstance )	// item to check for
{
	CLLNode<CItemRef>* Root = m_pItemList->GetFirst();
	while(Root != NULL)
	{
		if( Root->Data()->m_pItem->IsUnique() == pInstance.m_Unique &&
			Root->Data()->m_pItem->Sockets() == pInstance.m_Sockets &&
			Root->Data()->m_pItem->Name() == pInstance.m_Name &&
			Root->Data()->m_pItem->BaseModifiedName() == pInstance.m_BaseModifiedName )
		{
			return Root->Data()->m_pItem;
		}
		Root = Root->Next();
	}


	return NULL;
} // CInventory::FindItem()

void CInventory::SyncGems(std::string newGem)
{
	if(!CStatsAndAchievements::GetInstance()->Enabled()) return;

	std::string input = m_pCharacter.GetGemsCollected();
	std::vector<std::string> vect;
	std::istringstream ss(input);
	std::string token;

	while(std::getline(ss, token, ',')) {
		vect.push_back(token);
	}
	
	if (!(std::find(vect.begin(), vect.end(), newGem) != vect.end()))
	{
		vect.push_back(newGem);
	}
	
	string result;
	for(vector<std::string>::iterator it = vect.begin(); it != vect.end();++it)
	{
	    if(it != vect.begin()) {
			result += ",";
		}
		result += *it;
	}
	m_pCharacter.SetGemsCollected(result); //for persisting
	m_pCharacter.SetGemsCollectedNumber(vect.size()); //avoid recalculating and string parsing
	CStatsAndAchievements::GetInstance()->CheckStats(); //for evaluating achievement
}

void CInventory::SyncCaughtFish(std::string newFish)
{
	if(!CStatsAndAchievements::GetInstance()->Enabled()) return;
	
	std::string input = m_pCharacter.GetFishCaught();
	std::vector<std::string> vect;
	std::istringstream ss(input);
	std::string token;

	while(std::getline(ss, token, ',')) {
		vect.push_back(token);
	}
	
	if (!(std::find(vect.begin(), vect.end(), newFish) != vect.end()))
	{
		vect.push_back(newFish);
	}
	
	string result;
	for(vector<std::string>::iterator it = vect.begin(); it != vect.end();++it)
	{
	    if(it != vect.begin()) {
			result += ",";
		}
		result += *it;
	}
	m_pCharacter.SetFishCaught(result); //for persisting
	m_pCharacter.SetFishCaughtNumber(vect.size()); //avoid recalculating and string parsing
	CStatsAndAchievements::GetInstance()->CheckStats(); //for evaluating achievement
}