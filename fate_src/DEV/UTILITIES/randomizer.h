/*
	randomizer.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	11/26/2004

*/


#ifndef _RANDOMIZER_H
#define _RANDOMIZER_H

#include "constants.h"
#include <vector>

using namespace std;


#pragma once

class CRandomizer
{
public:
    
    CRandomizer( void );
	void Clear( void ); //start over

    void AddChoice( int32 Index, int32 Odds );
    
	int32 GetRandom( void );
    
	int32 GetOdds( int32 Choice ); //in case you want to know what the odds were on something

private:

    void ComputeOdds( void );

	std::vector<int32> m_Odds;  //save original
    std::vector<float32> m_ComputedOdds; 

    bool m_NeedsComputation;	//true if we've never computed or a new number was added

};

#endif