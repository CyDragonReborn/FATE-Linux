/***************************************************************************************
 *
 * DSPFilter.h
 *
 * Copyright (C) 2003-2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 4/16/2003: Gary H:	Creation
 * 2/19/2004: Jason Glover: Updated to wtKernel_v2 and added to wtAudio.
 *
 ***************************************************************************************/
#pragma once

#include "WT/System/Object.h"
#include "WT/Audio/AudioException.h"

//Note: DSPFilter, DSPFilterFIR, and DSPFilterIIR were adapted 
//from FILTER.H found in the "C++ Algorithms for Digital Signal 
//Processing" book by Paul M. Embree and Damon Danieli

namespace WT
{
namespace Audio
{

///DSPFilter - DSP Filter
class CDSPFilter
{
public:
	// Constructor
	CDSPFilter() :
		m_coef( NULL ),
		m_hist( NULL ),
		m_length( 0 )
	{ 
	}

	// Destructor
	virtual ~CDSPFilter()
	{
		delete[] m_hist;
		delete[] m_coef;
	}

	///////////////////////////////////////////////////////////////////////
	//
	// Virtual pure functions are implemented in derived classes
	//
	///////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////
	//
	// filter( const Vector& vIn )
	//   Performs filtering on a Vector.
	//
	//  Note:
	//   Default implementation uses the virtual pure
	//   function filterElement() which derived classes
	//   must implement.
	//
	//  Returns:
	//   Filtered Vector
	//
	///////////////////////////////////////////////////////////////////////
	virtual void filter( float32 const vIn[], float32 vOut[], int32 length )
	{
		for( int i = 0; i < length; i++ )
			vOut[i] = filterElement( vIn[i] );
	}

	///////////////////////////////////////////////////////////////////////
	//
	//  filterElement( const float32 input )
	//    Performs filtering sample by sample on float32.
	//
	//  Returns:
	//   Filtered element
	//
	///////////////////////////////////////////////////////////////////////
	virtual float32 filterElement( const float32 input ) = 0;

	///////////////////////////////////////////////////////////////////////
	//
	//  reset()
	//    Reset history of filter, but keep parameters.
	//
	///////////////////////////////////////////////////////////////////////
	virtual void reset() = 0;

	// Allow fast read-only access to data
	const float32 *getCoefficients( int len ) const
	{
        assert(len == m_length); //Coefficients must have the same length

		return m_coef;
		len;
	}

	// Return the number of coefficients
	int length() const { return m_length; }

protected:
	// Pointer to coefficients of filter
	float32 *m_coef;
	// Pointer to history of filter
	float32 *m_hist;
	// Size of the filter
	int32 m_length;
};

///DSPFilterFIR - Finite Impulse Response filter
class CDSPFilterFIR : public CDSPFilter
{
public:
	// Constructor
	CDSPFilterFIR( const float32 *coef, int length )
	{
		// Validate parameters
        assert(length > 0); //There must be filter data
        assert(coef != NULL); //There must be filter coefficients

		// Allocate memory for filter coefficients and history
		m_coef = new float32[length];
		m_hist = new float32[length];
		m_length = length;

		// Copy coefficients
		float32 *pc = m_coef;
		for( int i = 0; i < length; i++ )
			*pc++ = *coef++;
		reset();
	}

	// Implementation of abstract base class function (see Filter)
	virtual void filter( float32 const vIn[], float32 vOut[], int32 length )
	{
		int lenIn = length;

		// Set up for coefficients
		float32 *startCoef = m_coef;
		int lenCoef2 = ( m_length + 1 ) / 2;

		// Set up input data pointers
		const float32 *endIn = &vIn[lenIn - 1];
		const float32 *ptrIn = &vIn[lenCoef2 - 1];

		// Initial value of accumulation length for startup
		int lenAcc = lenCoef2;

		for(int i = 0 ; i < lenIn; i++ )
		{
			// Set up pointers for accumulation
			const float32 *ptrData = ptrIn;
			const float32 *ptrCoef = startCoef;

			// Do accumulation and write result
			float32 acc = *ptrCoef++ * *ptrData--;
			for( int j = 1; j < lenAcc; j++ )
				acc += *ptrCoef++ * *ptrData--;
			vOut[i] = acc;

			// Check for end case
			if( ptrIn == endIn )
			{
				// One shorter each time
				lenAcc--;
				// Next coefficient each time
				startCoef++;
			}
			else 
			{
				// Check for startup
				if( lenAcc < m_length )
				{
					// Add to input pointer
					lenAcc++;
				}
				ptrIn++;
			}
		}
	}

	// Implementation of abstract base class function (see Filter)
	virtual float32 filterElement( const float32 input )
	{
		// Start at beginning of history
		float32 *ptrHist = m_hist;
		float32 *ptrHist1 = m_hist;

		// point to last coefficient
		float32 *ptrCoef = m_coef + m_length - 1;

		// Form output accumulation
		float32 output = *ptrHist++ * *ptrCoef--;
		for( int i = 2; i < m_length; i++ )
		{
			// Update history array
			*ptrHist1++ = *ptrHist;
			output += *ptrHist++ * *ptrCoef--;
		}

		// Input tap
		output += input * *ptrCoef;
		// Last history
		*ptrHist1 = input;

		return output;
	}

	// Implementation of abstract base class function (see Filter)
	virtual void reset()
	{
		float32 *ph = m_hist;
		for( int i = 0; i < m_length; i++ )
			*ph++ = 0.0f;
	}

	float32 getHistory( int i )
	{
        assert(i >= 0);
        assert(i < m_length);
        if (i < 0 || i >= m_length)
            return 0.0f;

		return m_hist[i];
	}

private:
	// No default constructor
	CDSPFilterFIR();
};

///DSPFilterIIR - Infinite Impulse Response filter
class CDSPFilterIIR : public CDSPFilter
{
public:
    CDSPFilterIIR()
    {
        m_length = -1;
    }

	void SetCoefficients( const float32 *coef, int length, int sections )
	{
		// Validate parameters
        assert(length > 0); //There must be filter data
        assert(coef != NULL); //There must be filter coefficients

		assert( ( sections * 4 ) + 1 == length ); //Coefficients must match sections

        if (m_length != sections)
        {
            delete[] m_hist;
            delete[] m_coef;

		    // Allocate memory for filter coefficients and history
		    m_coef = new float32[length];
		    m_hist = new float32[2 * sections];
            memset(m_hist, 0, 2 * sections * sizeof float32);

		    // Length of IIR filter is number of sections
		    m_length = sections;
        }

		// Copy coefficients
		float32 *pc = m_coef;
		for( int i = 0; i < length; i++ )
			*pc++ = *coef++;
	}

	// Implementation of abstract base class function (see Filter)
	virtual float32 filterElement( const float32 input )
	{
		// Coefficient pointer
		float32 *ptrCoef = m_coef;

        // History pointer
		float32 *ptrHist = m_hist;

		// Form output accumulation
		float32 output = input * ( *ptrCoef++ );
		for( int i = 0; i < m_length; i++, ptrHist += 2, ptrCoef += 4)
		{
			// Save history values
			float32 hist1 = ptrHist[0];
			float32 hist2 = ptrHist[1];
			ptrHist[1] = hist1;

			// Poles
			output -= hist1 * ptrCoef[0] + hist2 * ptrCoef[1];
			ptrHist[0] = output;

			// Zeros
            output += hist1 * ptrCoef[2] + hist2 * ptrCoef[3];
		}
		return output;
	}

	// Implementation of abstract base class function (see Filter)
	virtual void reset()
	{
		float32 *ph = m_hist;
		for( int i = 0; i < 2 * m_length; i++ )
			*ph++ = 0.0f;
	}
};

}	// namespace Audio
}	// namespace WT

