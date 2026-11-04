/*
	inventory.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _INVENTORY_H
#define _INVENTORY_H

#include "fmod.h"

#include "../UTILITIES/constants.h"
#include "../GAMECLIENT/gamerules.h"
#include "../ITEM/item.h"
#include "character.h"
#include "../UTILITIES/llist.h"
class CItem;
class CLevel;
class CCharacter;

const uint32 KInventorySlotsWide( 10 );
const uint32 KInventorySlotsHigh( 4 );

const uint32 KMerchantInventorySlotsWide( 10 );
const uint32 KMerchantInventorySlotsHigh( 11 );

const uint32 KIconSlotSize( 48 );

const enum EItemSlot
{
	KSlotHead,
	KSlotChest,
	KSlotLeftHand,
	KSlotRightHand,
	KSlotLeftArm,
	KSlotGloves,
	KSlotNeck,
	KSlotFeet,
	KSlotLeftFinger,
	KSlotRightFinger,
	KSlotBelt,
	KEquipmentSlots,
	KSlotQuick1,
	KSlotQuick2,
	KSlotQuick3,
	KSlotQuick4,
	KSlotQuick5,
	KQuickSlots
};

const EAttachmentPoint KItemAttachments[ KEquipmentSlots ] =
{
	KAttachHead,
	KAttachChest,
	KAttachLeftHand,
	KAttachRightHand,
	KAttachLeftArm,
	KAttachRightArm,
	KAttachNone,
	KAttachNone,
	KAttachNone,
	KAttachNone,
	KAttachNone
};


class CItemRef
{
public:
	CItemRef( CItem* pItem,		// item the reference should hold
			 uint32 Slot ) :	// slot to place in
			 m_pItem( pItem ),
			 m_Equipped( kFalse ),
			 m_SlotIndex( Slot ),
			 m_LastSlotIndex( Slot )
	{
	}

	~CItemRef( void )
	{
		DELETE_SAFELY( m_pItem );
	}

	// clears the reference, so that the item will not be deleted
	void RemoveItem( void )
	{
		m_pItem = NULL;
	}

	CItem*			m_pItem;

	int32			m_SlotIndex;
	int32			m_LastSlotIndex;
	
	bool			m_Equipped;

};

// inventory contains item references, and manages equipping/unequipping
// and trading between slots
class CInventory 
{
public:

				CInventory( CCharacter& pCharacter,		// parent character
							uint32 MaxSlots );			// maximum number of storage slots
	
				~CInventory( void );

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	void		Resize( uint32 MaxSlots );		// slots to resize to

	void		AddGold( uint32 Gold );			// gold to add

	void		RemoveGold( uint32 Gold )		{	m_Gold -= Gold; if( m_Gold < 0 ) m_Gold = 0; };

	bool		IsSlotFree( uint32 Slot );		// slot to check

	void		EquipItem( CLevel& pLevel,		// level to open items into
						   uint32 Slot );		// slot to check

	// GET only returns a pointer
	CItem*		GetItemFromSlot( uint32 Slot,			// slot to retrieve item from
								 uint32& ActualSlot );	// slot the retrieved item was actually in

	CItem*		GetItemFromSlot( uint32 Slot,			// slot to retrieve item from
							     uint32 Width,			// width to check within
							     uint32 Height,			// height to check within
								 uint32& ActualSlot );	// slot the retrieved item was actually in

	CItem*		GetItemFromSlot( uint32 Slot );		// slot to retrieve item from

	CItemRef*	GetItemRefFromSlot( uint32 Slot );	// slot to retrieve item ref from

	CItem*		GetItem( CItem* pItem );		// slot to retrieve item from

	uint32		ItemsInSlot( uint32 SlotIndex,	// top left slot
							 uint32 Width,		// width of item
							 uint32 Height );	// height of item
	// REMOVE physically removes item from inventory
	CItem*		RemoveItemFromSlot( uint32 Slot,			// slot to retrieve item from
									uint32& ActualSlot );	// slot the retrieved item was actually in

	CItem*		RemoveItemFromSlot( uint32 Slot,			// slot to retrieve item from
									uint32 Width,			// width to check within
								    uint32 Height,			// height to check within
									uint32& ActualSlot );	// slot the retrieved item was actually in

	CItem*		RemoveItemFromSlot( uint32 Slot );			// slot to retrieve item from

	CItem*		RemoveItem( CItem* pItem );		// slot to retrieve item from

	void		DropAll( CLevel& pLevel );		// level to open items into

	void		DropGold( CLevel& pLevel );		// level to open items into

	void		DropAllButEquipment( CLevel& pLevel );		// level to open items into

	bool		AddItem( CItem* pItem,		// item to add to the inventory
						 CLevel& pLevel );	// level to open items into

	bool		AddItem( CItem* pItem,	// item to add to the inventory
						 uint32 Slot,	// slot to place in
						 CLevel* pLevel );	// level to open items into

	void		EquipBest( CLevel& pLevel,						// level to open items into
						   bool EquipUnIdentified = kTrue );	//	equip unidentified objects?

	void		EquipEmpty( CLevel& pLevel,						// level to open items into
						    bool EquipUnIdentified = kTrue );	//	equip unidentified objects?

	void		EquipBestWeapon( CLevel& pLevel,						// level to open items into
								 bool EquipUnIdentified = kTrue );	//	equip unidentified objects?

	void		EquipBestDualWieldWeapon( CLevel& pLevel,						// level to open items into
										  bool EquipUnIdentified = kTrue );	//	equip unidentified objects?

	void		EquipBestHelmet( CLevel& pLevel,						// level to open items into
								 bool EquipUnIdentified = kTrue );	//	equip unidentified objects?

	void		EquipBestBelt( CLevel& pLevel,						// level to open items into
							   bool EquipUnIdentified = kTrue );	//	equip unidentified objects?

	void		EquipBestBoots( CLevel& pLevel,						// level to open items into
								bool EquipUnIdentified = kTrue );	//	equip unidentified objects?

	void		EquipBestGloves( CLevel& pLevel,						// level to open items into
								 bool EquipUnIdentified = kTrue );	//	equip unidentified objects?

	void		EquipBestShirt( CLevel& pLevel,						// level to open items into
								bool EquipUnIdentified = kTrue );	//	equip unidentified objects?

	void		EquipBestShield( CLevel& pLevel,						// level to open items into
								 bool EquipUnIdentified = kTrue );	//	equip unidentified objects?

	void		EquipItem( CLevel& pLevel,						// level to open items into
						   CItemRef* pItem,						// item to equip
						   EItemSlot Slot = KEquipmentSlots );	// slot to equip in( default is EquipmentSlots, which will autodetect )

	void		UnEquipItem( CLevel& pLevel,	// level to open items into
							 CItemRef* pItem );	// item to unequip

	void		UnEquipItem( CLevel& pLevel,	// level to open items into
							 EItemSlot Slot );	// slot to unequip item from

	void		SetCanEquip( EItemSlot Slot,		// slot to set state for
							 bool State );			// state to set


	void		SetSupportsItemType( EItemType Type,		// type of item
									 bool State );			// state to set

	void		SetSlotEquipped( EItemSlot Slot );	// slot to equip in( default is EquipmentSlots, which will autodetect )

	void		VerifyEquipment( CLevel& pLevel );		// level to open items into

	void		SetAllowQuickSlots( bool State )		{	m_AllowQuickSlots = State;		};

	void		CalculateEffectValues( void );

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////
	CItem*		FindItem( CItemSaveInstance& pInstance );	// item to check for 
	bool		HasItems( CItemSaveInstance& pInstance,	// item to check for
						  uint32 Count );				// number of copies

	bool		HasItems( std::string Name,				// item to check for
						  uint32 Count );				// number of copies

	bool		HasItems( EItemType Type,				// item type to check for
						  uint32 Count );				// number of copies

	bool		SlotEquipped( EItemSlot Slot );		// slot to check

	CLList<CItemRef>*		ItemList( void )	{	return m_pItemList;		};

	bool		CanEquip( EItemSlot Slot );			// slot to check

	bool		SupportsItemType( EItemType Type );	// type to check

	bool		FitsSlot( uint32 Slot,		// slot to check
						  CItem* pItem );	// item to check for fit

	int32		FindFreeSlot( uint32 Width,		// width of item
							  uint32 Height );	// height of item

	int32		FindFreeQuickSlot( void );

	uint32		Gold( void )			{	return m_Gold;		};

	EAttachmentPoint	EquipmentAttachment( EItemSlot Slot )	{	return KItemAttachments[Slot];	};

	int32		ArmorBonus( void );

	float32		EffectValue( EEffectType Type );				// type of effect

	float32		CalculateEffectValue( EEffectType Type );		// type of effect

	bool		RequirementsMet( CItem* pItem );				// item to check


protected:



	CLList<CItemRef>*	m_pItemList;	// list of pointers to all contained item refs

	uint32				m_MaximumSlots;

	uint32				m_Gold;			// gold amount in inventory

	bool				m_CanEquip[KEquipmentSlots];

	bool				m_SupportsItemType[KItemTypes];

	CItemRef*			m_pEquippedItems[KEquipmentSlots];

	float32				m_EffectValue[KAllEffectTypes];

	CCharacter&			m_pCharacter;

	uint32				m_SlotsHigh;

	bool				m_AllowQuickSlots;

	void	SyncGems(std::string newGem);
	void	SyncCaughtFish(std::string newFish);
};

#endif