/*
	datapoint.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved


	Travis Baldree
	2/26/2004

	Change History :

	Travis Baldree
	4/16/2004	-	Added support for orientation
*/


#ifndef _DATAPOINT_H
#define _DATAPOINT_H


#include <d3dx8.h>

#include "../UTILITIES/utilities.h"


// a named point in space - models and scenes may have 'points' inside them
// these are usually used for object placement.
// For instance, the original MAX file for a scene might have a cluster of 'points'
// inside it with a unique name. When the scene is loaded as a scene object, those
// points can be requested, and their names compared to place other objects at those locations
// Primarily used for glow and object placement
class CDataPoint
{
public:
	CDataPoint( std::string Name,				// name of point
			const D3DXVECTOR3& Position,		// position in space
			const D3DXMATRIX& Orientation ) :	// orientation matrix
												m_Name( Name ),
												m_Position( Position ),
												m_Orientation( Orientation ),
												m_UserData ( "" )
	{
	}; // CDataPoint()
	
	const std::string&	Name( void )		{	return m_Name;		};
	
	const D3DXVECTOR3&	Position( void )	{	return m_Position;		};
	const D3DXMATRIX&	Orientation( void )	{	return m_Orientation;	};
	
	const std::string&	UserData( void )						{	return m_UserData;	};
	void				SetUserData( const std::string& Data )	{	m_UserData = Data;	};

private:

	std::string		m_UserData;

	std::string		m_Name;
	D3DXVECTOR3		m_Position;
	D3DXMATRIX		m_Orientation;
};


#endif