/*
	gamestates.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	8/4/2004

*/


#ifndef _GAMESTATES_H
#define _GAMESTATES_H


const enum EMenu
{
	KMainMenu,
	KNewGameMenu,
	KNewPetMenu,
	KContinueGameMenu,
	KLoadingMenu,
	KOptionsMenu,
	KIntroMenu,
	KIntroQuestMenu,
	KDifficultyMenu,
	KRetireMenu,
	KAncestorMenu,
	KAncestorPetMenu,
	KCreditsMenu,
	KAncestorDifficultyMenu,
	KLogoMenu,
	KNoMenu,
	KMenus
};

const enum EGameState
{
	KGameStateNone,
	KGameStateIntroMovie,
	KGameStateDownloadMenu,
	KGameStateLoadMenu,
	KGameStateLoadPauseMenu,
	KGameStateMenu,
	KGameStateLoadLevel,
	KGameStateLoadGame,
	KGameStateInGame,
	KGameStateQuitToMenu,
	KGameStatePaused,
	KGameStateQuit,
	KGameStates
};
#endif