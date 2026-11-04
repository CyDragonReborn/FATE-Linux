#include "d3dx8_shim.h"
/*
	astar.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/8/2004

*/
#include <assert.H>
#include <math.H>
#include "d3dx8.h"

#include "../UTILITIES/macros.H"
#include "../UTILITIES/utilities.H"
#include "../UTILITIES/constants.H"

#include "astar.H"

// used for calculating local Distance between cells for heuristic
inline float32 Distance( int32 X, // x delta
					   int32 Y ) // y delta
{
	return ( ( (float32)X * (float32)X ) + ( (float32)Y * (float32)Y ) );
} // Distance()



CAStar::CAStar( float32 TopLeftX,		// top left worldspace corner of the map
			    float32 TopLeftY,		// top left worldspace corner of the map
				uint32 Width,			// width of map area, in cells
				uint32 Height,			// height of map area, in cells
				int16** pTileMap,		// array of all tile values for the map
				int16** pObjectMap,		// array of all object tile values for the map
				float32 CellWidth ) :	// width/height of cells, in worldspace units
										m_Width( Width ),
										m_Height( Height ),
										m_TopLeftX( TopLeftX ),
										m_TopLeftY( TopLeftY ),
										m_TotalSize( m_Width * m_Height ),
										m_CellWidth( CellWidth ),
										m_PathValid( kFalse ),
										m_pOpenNode( NULL ),
										m_pClosedNode( NULL ),
										m_pPathNode( NULL ),
										m_WasPerfectPath( kTrue ),
										m_DestinationUnreachable( kFalse ),
										m_pTileMap( pTileMap ),
										m_MaximumTreeDepth( KMaximumTreeDepth ),
										m_pObjectMap( pObjectMap ),
										m_pStack ( ( AStarStack * )calloc( 1 , sizeof( AStarStack ) )),
										m_pBestSolution( NULL )
{
} // CAStar::CAStar()

CAStar::~CAStar()
{
    FreeNodes();
    free( m_pStack );

	if( m_pOpenNode != NULL )
	{
		free( m_pOpenNode );
	}
	if( m_pClosedNode != NULL )
	{
		free( m_pClosedNode );
	}

	m_pTileMap = NULL;
	m_pObjectMap = NULL;
} // CAStar::~CAStar()

bool CAStar::FindPath( float32 StartX,			// starting position X in worldspace
					   float32 StartY,			// starting position Y in worldspace
					   float32 DestinationX,	// destination position X in worldspace
					   float32 DestinationY )	// destionation position X in worldspace
{
	m_WasPerfectPath = kTrue;
	StartX -= m_TopLeftX;
	StartY -= m_TopLeftY;
	DestinationX -= m_TopLeftX;
	DestinationY -= m_TopLeftY;
	// first we have to reverse the Y coordinates - they are always backwards
	StartY *= -1.0f;
	DestinationY *= -1.0f;

	if( DestinationY < 0 )
	{
		DestinationY = 0;
	}
	if( StartY < 0 )
	{
		StartY = 0;
	}
	if( DestinationX < 0 )
	{
		DestinationX = 0;
	}
	if( StartX < 0 )
	{
		StartX = 0;
	}

	m_DestinationUnreachable = kFalse;

	uint32 TileStartX( (uint32)floor( StartX / m_CellWidth ) );
	uint32 TileStartY( (uint32)floor( StartY / m_CellWidth ) );
	uint32 TileDestinationX( (uint32)floor( DestinationX / m_CellWidth ) );
	uint32 TileDestinationY( (uint32)floor( DestinationY / m_CellWidth ) );

	uint32 SecondaryDestinationX( TileDestinationX );
	uint32 SecondaryDestinationY( TileDestinationY );

	// check to see if our target destination is in walkable space
	if( !TileFree( TileDestinationX, TileDestinationY, KDirStill )  )
	{
		m_DestinationUnreachable = kTrue;
		// if not, find the X/Y of the nearest open tile to this
		FindNearestOpen( TileStartX, 
						 TileStartY, 
						 TileDestinationX, 
						 TileDestinationY,
						 SecondaryDestinationX, 
						 SecondaryDestinationY );
	}

	// check to see if our target destination is in walkable space
	if( !(  TileFree( TileStartX, TileStartY, KDirStill ) &&
			TileFree( TileStartX, TileStartY - 1, KDirStill ) &&
			TileFree( TileStartX, TileStartY + 1, KDirStill ) &&
			TileFree( TileStartX + 1, TileStartY, KDirStill ) &&
			TileFree( TileStartX - 1, TileStartY, KDirStill ) ) )
	{
		// if not, find the X/Y of the nearest open tile to this
		FindNearestWideOpen( TileDestinationX, 
							 TileDestinationY, 
							 TileStartX, 
							 TileStartY,
							 TileStartX, 
							 TileStartY );
	}


	// if the destination is clear, the start is clear, and the destination
	// and start are not identical, then we can try to path
    if ( TileFree( TileStartX, TileStartY, KDirStill ) && 
         ( TileIndex( TileStartX, TileStartY ) != TileIndex( TileDestinationX, TileDestinationY ) ) )
    {
		// this is a valid path
        m_PathValid = kTrue;
		// free nodes from previous paths
        FreeNodes();
		// and find the new one
        CreatePath( TileDestinationX, TileDestinationY, TileStartX, TileStartY );        
        return ( m_PathValid );
    }
	// otherwise, the path is a dud
    return ( m_PathValid = kFalse );
} // CAStar::FindPath()

void CAStar::FindNearestOpen( uint32 TileStartX,			// starting position X in tile space
							  uint32 TileStartY,			// starting position Y in tile space
						      uint32 TileDestinationX,		// destination position X in tile space
						      uint32 TileDestinationY,		// destionation position X in tile space
							  uint32& NearestX,				// nearest position X in tile space to be filled
							  uint32& NearestY )			// nearest position X in tile space to be filled
{

	float32 TargetWorldspaceX = (float32)TileDestinationX * m_CellWidth;
	float32 TargetWorldspaceY = (float32)TileDestinationY * -m_CellWidth;
	TargetWorldspaceX += m_TopLeftX;
	TargetWorldspaceY += m_TopLeftY;

	D3DXVECTOR3 Delta( 0, 0, 0 );

	float32 BestDistance = 9999;
	for( int32 X = (int32)TileDestinationX - 16; X <= (int32)TileDestinationX + 16; X++ )
	{
		for( int32 Y = (int32)TileDestinationY - 16; Y <= (int32)TileDestinationY + 16; Y++ )
		{
			if( X >= 0 && Y >= 0 )
			{

				if( TileFree( (uint32)X, (uint32)Y, KDirStill ) )
				{
					float32 WorldspaceX = (float32)X * m_CellWidth;
					float32 WorldspaceY = (float32)Y * -m_CellWidth;
					WorldspaceX += m_TopLeftX;
					WorldspaceY += m_TopLeftY;
					Delta.x = WorldspaceX - TargetWorldspaceX;
					Delta.z = WorldspaceY - TargetWorldspaceY;

					float32 Distance = D3DXVec3Length( &Delta );
					if( Distance < BestDistance )
					{
						BestDistance = Distance;
						NearestX = X;
						NearestY = Y;
					}
				}
			}
		}
	}

	if( BestDistance == 9999 )
	{
		NearestX = TileStartX;
		NearestY = TileStartY;
	}

} // CAStar::FindNearestOpen()

void CAStar::FindNearestWideOpen( uint32 TileStartX,			// starting position X in tile space
							  uint32 TileStartY,			// starting position Y in tile space
						      uint32 TileDestinationX,		// destination position X in tile space
						      uint32 TileDestinationY,		// destionation position X in tile space
							  uint32& NearestX,				// nearest position X in tile space to be filled
							  uint32& NearestY )			// nearest position X in tile space to be filled
{

	float32 TargetWorldspaceX = (float32)TileDestinationX * m_CellWidth;
	float32 TargetWorldspaceY = (float32)TileDestinationY * -m_CellWidth;
	TargetWorldspaceX += m_TopLeftX;
	TargetWorldspaceY += m_TopLeftY;
	D3DXVECTOR3 Delta( 0, 0, 0 );

	float32 BestDistance = 9999;
	for( int32 X = (int32)TileDestinationX - 16; X <= (int32)TileDestinationX + 16; X++ )
	{
		for( int32 Y = (int32)TileDestinationY - 16; Y <= (int32)TileDestinationY + 16; Y++ )
		{
			if( X >= 0 && Y >= 0 )
			{

				if( TileFree( (uint32)X, (uint32)Y, KDirStill ) &&
					TileFree( (uint32)X, (uint32)Y - 1, KDirStill ) &&
					TileFree( (uint32)X, (uint32)Y + 1, KDirStill ) &&
					TileFree( (uint32)X - 1, (uint32)Y, KDirStill ) &&
					TileFree( (uint32)X + 1, (uint32)Y, KDirStill ) )
				{
					float32 WorldspaceX = (float32)X * m_CellWidth;
					float32 WorldspaceY = (float32)Y * -m_CellWidth;
					WorldspaceX += m_TopLeftX;
					WorldspaceY += m_TopLeftY;
					Delta.x = WorldspaceX - TargetWorldspaceX;
					Delta.z = WorldspaceY - TargetWorldspaceY;

					float32 Distance = D3DXVec3Length( &Delta );
					if( Distance < BestDistance )
					{
						BestDistance = Distance;
						NearestX = X;
						NearestY = Y;
					}
				}
			}
		}
	}

	if( BestDistance == 9999 )
	{
		NearestX = TileStartX;
		NearestY = TileStartY;
	}

} // CAStar::FindNearestWideOpen()

bool CAStar::ReachedGoal( void )
{
    if ( !m_PathValid ) 
	{
		return kTrue;
	}
    if ( m_pPathNode->m_pParent != NULL ) 
	{
		return kFalse;
	}

    return kTrue;
} // CAStar::ReachedGoal()

int32 CAStar::TileIndex( uint32 X,	// tile X position
					     uint32 Y )	// tile Y position
{
    return ( X + Y * m_Width );
} // CAStar::TileIndex()

bool CAStar::TileFree( uint32 X,						// tile X position
					   uint32 Y,						// tile Y position
					   ENodeDirection Direction )	// direction to check
{
	// make sure we're checking in bounds
	if( X < 0 || Y < 0 || X >= m_Width || Y >= m_Height )
	{
		return kFalse;
	}
	if ( m_pTileMap[X][Y] != 1 && 
		 m_pObjectMap[X][Y] <= 0 ) 
	{
		return kTrue;
	}
    return kFalse;
} // CAStar::TileFree()

// clean up all the active nodes
void CAStar::FreeNodes( void )
{
    AStarNode *pNode = NULL;
	AStarNode *pOldNode = NULL;

    if ( m_pOpenNode != NULL )
    {
        pNode = m_pOpenNode->m_pNextNode;

        while ( pNode != NULL )
        {
            pOldNode = pNode;
            pNode = pNode->m_pNextNode;
            free( pOldNode );
        }
    }

    if ( m_pClosedNode != NULL )
    {
        pNode = m_pClosedNode->m_pNextNode;

        while( pNode != NULL )
        {
            pOldNode = pNode;
            pNode = pNode->m_pNextNode;
            free( pOldNode );
        }
    }
} // CAStar::FreeNodes()

void CAStar::AdvanceNextNode( void )
{
    m_pPathNode = m_pPathNode->m_pParent;
} // CAStar::AdvanceNextNode()

float32 CAStar::GetNodeX( void )
{
    return (float32)m_pPathNode->X * m_CellWidth + m_CellWidth * .5f + m_TopLeftX;
} // CAStar::GetNodeX()

float32 CAStar::GetNodeY( void )
{
    return (float32)m_pPathNode->Y * -m_CellWidth - m_CellWidth * .5f + m_TopLeftY;
} // CAStar::GetNodeY()

float32 CAStar::GetNextNodeX( void )
{
    return (float32)m_pPathNode->m_pParent->X * m_CellWidth + m_CellWidth * .5f + m_TopLeftX;
} // CAStar::GetNextNodeX()

float32 CAStar::GetNextNodeY( void )
{
    return (float32)m_pPathNode->m_pParent->Y * -m_CellWidth - m_CellWidth * .5f + m_TopLeftY;
} // CAStar::GetNextNodeY()

void CAStar::CreatePath( int32 TileStartX,			// starting position x in tile space
						 int32 TileStartY,			// starting position y in tile space
						 int32 TileDestinationX,	// destination position x in tile space
						 int32 TileDestinationY,	// destination position x in tile space
						 bool Reverse )				// reverse nodes when complete?
{
	m_pBestSolution = NULL;
    AStarNode *pNode = NULL;
	AStarNode *pBestNode = NULL;
    int32 DestinationIndex;

    m_PathValid = kTrue;

	// find the index of our destination tile
    DestinationIndex = TileIndex( TileStartX, TileStartY );

	if( m_pOpenNode != NULL )
	{
		free( m_pOpenNode );
	}
	if( m_pClosedNode != NULL )
	{
		free( m_pClosedNode );
	}

    m_pOpenNode = ( AStarNode * )calloc( 1 , sizeof( AStarNode ) );
    m_pClosedNode = ( AStarNode * )calloc( 1 , sizeof( AStarNode ) );

    pNode = ( AStarNode * )calloc( 1 , sizeof( AStarNode ) );

    pNode->G = 0;
    pNode->H = Distance( ( TileDestinationX - TileStartX ), ( TileDestinationY - TileStartY ) );
    pNode->F = pNode->G + pNode->H;
    pNode->NodeNum = TileIndex( TileDestinationX, TileDestinationY );
    pNode->X = TileDestinationX;
    pNode->Y = TileDestinationY;

    m_pOpenNode->m_pNextNode = pNode;
	//CheckNode = pBestNode;
	int32 Steps=0;
    for ( ;; )
    {
        pBestNode = ReturnBestNode();

		// do an early opt-out if our solution is unreachable,
		// and we get REALLY close
		if( m_DestinationUnreachable && 
			m_pBestSolution != NULL )
		{
			if( m_pBestSolution->H < 2 )
			{
				m_WasPerfectPath = kFalse;
				pBestNode = m_pBestSolution;
				break;
			}
	
		}
        if ( pBestNode == NULL || Steps > m_MaximumTreeDepth ) 
		{
			if( m_pBestSolution != NULL  )
			{
				m_WasPerfectPath = kFalse;
				pBestNode = m_pBestSolution;
				break;
			}
			m_PathValid = kFalse;
			break;
		}

        if ( pBestNode->NodeNum == DestinationIndex )
		{
            break;
		}

		GenerateSuccessors( pBestNode, TileStartX, TileStartY );
		Steps++;
    }

	if( Reverse )
	{
		pBestNode = ReverseNodes( pBestNode );
	}

    m_pPathNode = pBestNode;
} // CAStar::CreatePath()

CAStar::AStarNode* CAStar::ReverseNodes( AStarNode* pNode )	// node to start from
{
	if( pNode == NULL )
	{
		return NULL;
	}
	AStarNode* pPreviousNode = NULL;
	AStarNode* pCurrentNode = NULL;
	AStarNode* pNextNode = NULL;

	pCurrentNode = pNode;
	while( pCurrentNode->m_pParent != NULL )
	{
		pNextNode = pCurrentNode->m_pParent;
		pCurrentNode->m_pParent = pPreviousNode;
		pPreviousNode = pCurrentNode;
		pCurrentNode = pNextNode;
	}
	pCurrentNode->m_pParent = pPreviousNode;

	pNode = pCurrentNode;

	return pNode;
} // CAStar::ReverseNodes()

CAStar::AStarNode* CAStar::ReturnBestNode( void )
{
    AStarNode  *pTempNode;

    if ( m_pOpenNode->m_pNextNode == NULL )
    {
        m_PathValid = kFalse;
        pTempNode = NULL;
        return pTempNode;
    }

    pTempNode = m_pOpenNode->m_pNextNode;
    m_pOpenNode->m_pNextNode = pTempNode->m_pNextNode;
    pTempNode->m_pNextNode = m_pClosedNode->m_pNextNode;
    m_pClosedNode->m_pNextNode = pTempNode;

   return( pTempNode );
} // CAStar::ReturnBestNode()

void CAStar::GenerateSuccessors( AStarNode *pBestNode,		// node to start from 
								 uint32 TileDestinationX,	// destination tile x
								 uint32 TileDestinationY )	// destination tile y
{
    int32 X( 0 );
	int32 Y( 0 );

// Upper - Left  
    if ( ( TileFree( pBestNode->X, pBestNode->Y - 1, KDirNorth ) &&
		   TileFree( pBestNode->X - 1, pBestNode->Y, KDirWest ) ) &&
		 TileFree( X = pBestNode->X - 1, Y = pBestNode->Y - 1, KDirNorthWest ) )
	{
        GenerateSuccessor( pBestNode, X, Y, TileDestinationX, TileDestinationY );
	}

// Upper  
    if ( ( TileFree( pBestNode->X - 1, pBestNode->Y - 1, KDirNorth ) ||
		   TileFree( pBestNode->X + 1, pBestNode->Y - 1, KDirNorth ) ) &&
		TileFree( X = pBestNode->X, Y = pBestNode->Y - 1, KDirNorth ) )
	{
        GenerateSuccessor( pBestNode, X, Y, TileDestinationX, TileDestinationY );
	}

// Upper - Right  
    if ( ( TileFree( pBestNode->X, pBestNode->Y - 1, KDirNorth ) &&
		   TileFree( pBestNode->X + 1, pBestNode->Y, KDirEast ) ) &&
		 TileFree( X = pBestNode->X + 1, Y = pBestNode->Y - 1, KDirNorthEast ) )
	{
        GenerateSuccessor( pBestNode, X, Y, TileDestinationX, TileDestinationY );
	}

//  Right
    if ( ( TileFree( pBestNode->X + 1, pBestNode->Y - 1, KDirNorth ) ||
		   TileFree( pBestNode->X + 1, pBestNode->Y + 1, KDirNorth ) ) &&
		   TileFree( X = pBestNode->X + 1, Y = pBestNode->Y, KDirEast ) )
	{
        GenerateSuccessor( pBestNode, X, Y, TileDestinationX, TileDestinationY );
	}

// Lower - Right  
    if ( ( TileFree( pBestNode->X, pBestNode->Y + 1, KDirSouth ) &&
		   TileFree( pBestNode->X + 1, pBestNode->Y, KDirEast ) ) &&
		 TileFree( X = pBestNode->X + 1, Y = pBestNode->Y + 1, KDirSouthEast ) )
	{
        GenerateSuccessor( pBestNode, X, Y, TileDestinationX, TileDestinationY );
	}

// Lower  
    if ( ( TileFree( pBestNode->X - 1, pBestNode->Y + 1, KDirNorth ) ||
		   TileFree( pBestNode->X + 1, pBestNode->Y + 1, KDirNorth ) ) &&
		   TileFree( X = pBestNode->X, Y = pBestNode->Y + 1, KDirSouth ) )
	{
        GenerateSuccessor( pBestNode, X, Y, TileDestinationX, TileDestinationY );
	}

// Lower - Left  
    if ( ( TileFree( pBestNode->X, pBestNode->Y + 1, KDirSouth) &&
		   TileFree( pBestNode->X - 1, pBestNode->Y, KDirWest) ) &&
		 TileFree( X = pBestNode->X - 1, Y = pBestNode->Y + 1, KDirSouthWest ) )
	{
        GenerateSuccessor( pBestNode, X, Y, TileDestinationX, TileDestinationY );
	}

// Left  
    if ( ( TileFree( pBestNode->X - 1, pBestNode->Y - 1, KDirNorth ) ||
		   TileFree( pBestNode->X - 1, pBestNode->Y + 1, KDirNorth ) ) &&
		   TileFree( X = pBestNode->X - 1, Y = pBestNode->Y, KDirWest) )
	{
        GenerateSuccessor( pBestNode, X, Y, TileDestinationX, TileDestinationY );
	}
} // CAStar::GenerateSuccessors()

void CAStar::GenerateSuccessor( AStarNode *pBestNode,		// node to generate successor for
							    uint32 X,					// tile location x
								uint32 Y,					// tile location y
								uint32 TileDestinationX,	// tile destination x
								uint32 TileDestinationY )	// tile destination y
{
    AStarNode* pOld = NULL;
	AStarNode* pSuccessor = NULL;
    
    float32 G = pBestNode->G + 1;
    int32 StartIndex = TileIndex( X, Y );

    if ( ( pOld = OpenNode( StartIndex ) ) != NULL ) 
    {
        for( uint32 c = 0; c  <  KMaximumChildren; c ++  )
        {
            if( pBestNode->m_pChild[c] == NULL )            
            {
                break;
            }
        }
        pBestNode->m_pChild[c] = pOld;

        if ( G  <  pOld->G )
        {
            pOld->m_pParent = pBestNode;
            pOld->G = G;
            pOld->F = G + pOld->H;
        }
    }
    else if ( ( pOld = ClosedNode( StartIndex ) ) != NULL ) 
    {
        for( uint32 c = 0; c <  KMaximumChildren; c ++  )
        {
            if ( pBestNode->m_pChild[c] == NULL )
                break;
        }
        
        pBestNode->m_pChild[c] = pOld;

        if ( G  <  pOld->G )
        {
            pOld->m_pParent = pBestNode;
            pOld->G = G;
            pOld->F = G + pOld->H;
            PropagateDown( pOld );  
        }
    }
    else
    {
        pSuccessor  =  ( AStarNode* )calloc( 1 , sizeof( AStarNode ) );

        pSuccessor->m_pParent = pBestNode;
        pSuccessor->G = G;
        pSuccessor->H = Distance( ( X - TileDestinationX ), ( Y - TileDestinationY ) );
        pSuccessor->F = G + pSuccessor->H;
        pSuccessor->X = X;
        pSuccessor->Y = Y;
        pSuccessor->NodeNum = StartIndex;
        Insert( pSuccessor );    
        for( uint32 c = 0; c  <  KMaximumChildren; c ++  )
        {
            if ( pBestNode->m_pChild[c] == NULL )
                break;
        }
		if( m_pBestSolution == NULL ||
			m_pBestSolution->H > pSuccessor->H )
		{
			m_pBestSolution = pSuccessor;
		}
        pBestNode->m_pChild[c] = pSuccessor;
    }
} // CAStar::GenerateSuccessor()

CAStar::AStarNode* CAStar::OpenNode( uint32 TileIndex )	// tile index to look for
{
    AStarNode  *pTempNode;

    pTempNode = m_pOpenNode->m_pNextNode;
    while ( pTempNode != NULL )
    {
        if ( pTempNode->NodeNum == TileIndex )
        {
            return ( pTempNode );
        }
        else
        {
            pTempNode = pTempNode->m_pNextNode;
        }
    }

    return NULL;
} // CAStar::OpenNode()

CAStar::AStarNode* CAStar::ClosedNode( uint32 TileIndex )	// tile index to look for
{
    AStarNode  *pTempNode = NULL;

    pTempNode = m_pClosedNode->m_pNextNode;

    while ( pTempNode != NULL )
    {
        if ( pTempNode->NodeNum == TileIndex )
        {
            return( pTempNode );
        }
        else
        {
            pTempNode = pTempNode->m_pNextNode;
        }
    }

    return NULL;
} // CAStar::ClosedNode()

void CAStar::Insert( AStarNode *pSuccessor ) // pNode to insert
{
    AStarNode *pTempNode1 = NULL;
	AStarNode *pTempNode2 = NULL;
    
    if ( m_pOpenNode->m_pNextNode == NULL )
    {
        m_pOpenNode->m_pNextNode = pSuccessor;
        return;
    }

     //  insert into m_pOpenNode successor wrt F
    float32 F = pSuccessor->F;
    pTempNode1 = m_pOpenNode;
    pTempNode2 = m_pOpenNode->m_pNextNode;

    while ( ( pTempNode2 != NULL ) && ( pTempNode2->F  <  F ) )
    {
        pTempNode1 = pTempNode2;
        pTempNode2 = pTempNode2->m_pNextNode;
    }

    pSuccessor->m_pNextNode = pTempNode2;
    pTempNode1->m_pNextNode = pSuccessor;
} // CAStar::Insert()

void CAStar::PropagateDown( AStarNode *pOldNode )	// pNode to propagate down
{
    float32 G ( pOldNode->G );
    AStarNode *pChild = NULL;
	AStarNode *pParent = NULL;

    for ( uint32 c = 0; c < KMaximumChildren; c ++  )
    {
        if ( ( pChild = pOldNode->m_pChild[c] ) == NULL )
		{
            break;
		}
        
        if ( G + 1  <  pChild->G )
        {
            pChild->G = G + 1;
            pChild->F = pChild->G + pChild->H;
            pChild->m_pParent = pOldNode;

            Push( pChild );         
        }     
    }

    while ( m_pStack->m_pNextStackPtr != NULL )
    {
        pParent = Pop();
        for ( uint32 c = 0; c < KMaximumChildren; c ++ )
        {
            if ( ( pChild = pParent->m_pChild[c] ) == NULL )
                break;

            if ( pParent->G + 1  <  pChild->G )
            {              
                pChild->G = pParent->G + 1;
                pChild->F = pChild->G + pChild->H;
                pChild->m_pParent = pParent;
                Push( pChild );
            }
        }
    }
} // CAStar::PropagateDown()

void CAStar::Push( AStarNode *pNode )	// pNode to push onto the stack
{
	AStarStack* pTempStack= ( AStarStack * )calloc( 1 , sizeof( AStarStack ) );

	pTempStack->m_pNodePtr = pNode;
    pTempStack->m_pNextStackPtr = m_pStack->m_pNextStackPtr;
    m_pStack->m_pNextStackPtr = pTempStack;
} // CAStar::Push()

CAStar::AStarNode* CAStar::Pop( void )
{
    AStarNode  *pTemp;
    AStarStack  *pTempStack;

    pTempStack = m_pStack->m_pNextStackPtr;
    pTemp = pTempStack->m_pNodePtr;

    m_pStack->m_pNextStackPtr = pTempStack->m_pNextStackPtr;
    free( pTempStack );

   return( pTemp );
} // CAStar::Pop()
