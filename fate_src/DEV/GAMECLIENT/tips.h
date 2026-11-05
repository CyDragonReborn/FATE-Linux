/*
	questdescriptions.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	8/12/2004

*/

#ifndef _TIPS_H
#define _TIPS_H

#include "../UTILITIES/constants.h"

enum EContextTip
{
	KTipInventory,
	KTipPetInventory,
	KTipIdentify,
	KTipInventoryFull,
	KTipHealthLow,
	KTipShopping,
	KTipLevelUp,
	KTipLevelUp2,
	KTipSpell,
	KTipWriteSpell,
	KTipGamble,
	KTipWelcome,
	KTipFish,
	KTipGem,
	KTipStats,
	KTipPetFlee,
	KTipStamina,
	KTipPetFull,
	KTipStash,
	KContextTips
};

const std::string KContextTipName[KContextTips] =
{
	"INVENTORY",
	"PETINVENTORY",
	"IDENTIFY",
	"INVENTORYFULL",
	"HEALTHLOW",
	"SHOPPING",
	"LEVELUP",
	"LEVELUP2",
	"SPELL",
	"WRITESPELL",
	"GAMBLE",
	"WELCOME",
	"FISH",
	"GEM",
	"STATS",
	"PETFLEE",
	"STAMINA",
	"PETFULL",
	"STASH"
};

// loads a list of all cars and tracks required by the game
class CTips
{
public:
			CTips( const std::string& Path );	// path to the directory containing data files
			~CTips( void );

	void	Load(  const std::string& Path );	// path to the directory containing data files

	void	ClearState( void );

	void	SetTipSeen( EContextTip Tip,	// tip to set
						bool State )		// state of tip
						{	m_TipSeen[Tip] = State;		};

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	bool				TipSeen( EContextTip Tip )		{	return m_TipSeen[Tip];				};

	uint32				Tips( void )					{	return m_GeneralTips.size();		};

	const std::string&	RandomTip( void );

	const std::string&	ContextTip( EContextTip Tip );	// type of tip

private:

	std::string			m_ContextTips[KContextTips];

	bool				m_TipSeen[KContextTips];

	std::vector< std::string >		m_GeneralTips;
};


#endif