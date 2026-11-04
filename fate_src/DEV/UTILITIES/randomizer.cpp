
#include "constants.h"
#include "randomizer.h"

CRandomizer::CRandomizer( void ) :
									m_NeedsComputation ( kTrue )
{
} // CRandomizer::CRandomizer()

int32 CRandomizer::GetOdds( int32 Choice )	// choice to get odds for
{
    return m_Odds[Choice];
} // CRandomizer::GetOdds()

void CRandomizer::Clear( void )
{
    m_Odds.clear();
    m_ComputedOdds.clear();
    m_NeedsComputation = kTrue;
} // CRandomizer::Clear()

void CRandomizer::AddChoice( int32 Index,	// index
							 int32 Odds )	// odds
{
    if ( Index + 1 > (int32)m_Odds.size() )
    {    
		m_Odds.resize( Index + 1 ); //make sure it's big enough
		m_ComputedOdds.resize( Index + 1 ); //make sure it's big enough
    }
    m_Odds[Index] = Odds;
    m_NeedsComputation = kTrue;
} // CRandomizer::AddChoice()


void CRandomizer::ComputeOdds( void )
{
    //cycle through and figure out the percentages

    int32 Total( 0 );    
    for ( int32 i=0; i < (int32)m_Odds.size(); i++ )
    {
		Total += m_Odds[i];
    }



    for ( i = 0; i < (int32)m_Odds.size(); i++ )
    {
		m_ComputedOdds[i] = float32( m_Odds[i] ) / float32( Total );    
	}


    m_NeedsComputation = kFalse; //all done, ready to compute crap

} // CRandomizer::ComputeOdds()

int32 CRandomizer::GetRandom( void )
{

    if ( m_NeedsComputation ) 
	{
		ComputeOdds();
	}
    
    float32 Random( (float32)rand() / (float32)RAND_MAX );

    //scroll through and figure out which thing we chose
      
    float32 Temp( 0 );
    
    for ( int32 i=0; i < (int32)m_ComputedOdds.size(); i++ )
    {
		Temp += m_ComputedOdds[i];
		if ( Temp > Random )
		{
			 //this must be it.
			return i;
		}
    }
    return 0;
} // CRandomizer::GetRandom()


