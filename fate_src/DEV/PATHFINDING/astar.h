/*
	astar.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/8/2004

*/


#ifndef _ASTAR_H
#define _ASTAR_H

// max number of children per node
const uint32	KMaximumChildren( 8 );
// max number of nodes in a tree
const uint32	KMaximumTreeDepth( 150 );

enum ENodeDirection
{
	KDirStill,
	KDirNorth,
	KDirSouth,
	KDirEast,
	KDirWest,
	KDirNorthEast,
	KDirNorthWest,
	KDirSouthEast,
	KDirSouthWest,
	KDirections
};

//	A Star pathfinding object
class CAStar
{
    typedef struct AStarNode
    {
        float32  F;
        float32  H;
        float32  G;
        int32  X;
        int32  Y;
        int32  NodeNum;
        AStarNode *m_pParent;
        AStarNode *m_pChild[ KMaximumChildren ];
        AStarNode *m_pNextNode;
    } AStarNode;

    typedef struct AStarStack
    {
        AStarNode  *m_pNodePtr;
        AStarStack *m_pNextStackPtr;    

    } AStarStack;

public:

    CAStar( float32 TopLeftX,		// top left worldspace corner of the map
			float32 TopLeftY,		// top left worldspace corner of the map
			uint32 Width,			// width of map area, in cells
			uint32 Height,			// height of map area, in cellsuint32 Width,		// width of map area, in cells
			int16** pTileMap,		// array of all tile values for the map
			int16** pObjectMap,		// array of all object tile values for the map
			float32 CellWidth );	// width/height of cells, in worldspace units
    ~CAStar( void );

	////////////////////////////////////////////////////////////////////////////////////////////
	//MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////

    bool FindPath( float32 StartX,			// starting position x in worldspace
				   float32 StartY,			// starting position y in worldspace
				   float32 DestinationX,	// destination position x in worldspace
				   float32 DestinationY );	// destionation position x in worldspace

    void	AdvanceNextNode( void );	
	
	void	SetMaximumTreeDepth( int32 Depth )		{	m_MaximumTreeDepth = Depth;			};

	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	bool	WasPerfectPath( void )					{	return m_WasPerfectPath;			};

    bool	ReachedGoal( void );
    float32	GetNodeX( void );	// worldspace x-position of current node
    float32	GetNodeY( void );	// worldspace y-position of current node

    float32	GetNextNodeX( void );	// worldspace x-position of next node
    float32	GetNextNodeY( void );	// worldspace y-position of next node

	bool	PathValid( void )		{	return m_PathValid;		};

private:
		void	CreatePath( int32 CellStartX,			// starting position x in tile space
							int32 CellStartY,			// starting position y in tile space
							int32 CellDestinationX,		// destination position x in tile space
							int32 CellDestinationY,		// destination position x in tile space
							bool Reverse = kTrue );		// reverse nodes when complete?

    bool		TileFree( uint32 X,						// X Tile Position
						  uint32 Y,						// Y TilePosition
						  ENodeDirection Direction );	// direction to check
    int32		TileIndex( uint32 x , 
						   uint32 y );

    void		FreeNodes( void );
    
	AStarNode*	ReturnBestNode( void );
    
	void		GenerateSuccessors( AStarNode *pBestNode,		// node to start from 
									uint32 TileDestinationX,	// destination tile x
									uint32 TileDestinationY );	// destination tile y
    void		GenerateSuccessor( AStarNode *pBestNode,		// node to generate successor for
									uint32 X,					// tile location x
									uint32 Y,					// tile location y
									uint32 TileDestinationX,	// tile destination x
									uint32 TileDestinationY );	// tile destination y
    
	AStarNode*	OpenNode( uint32 TileIndex );		// index of tile
    AStarNode*	ClosedNode( uint32 TileIndex );		// index of tile 
    
	void		Insert( AStarNode *pSuccessor );	// node to insert
    
	void		PropagateDown( AStarNode *pOld );	// node to propagate

    void		Push( AStarNode *pNode );			// node to push onto the stack

    AStarNode*	Pop();
	AStarNode*	ReverseNodes(AStarNode* node);

	void		FindNearestOpen( uint32 TileStartX,				// starting position X in tile space
								 uint32 TileStartY,				// starting position Y in tile space
								 uint32 TileDestinationX,		// destination position X in tile space
								 uint32 TileDestinationY,		// destionation position X in tile space
								 uint32& NearestX,				// nearest position X in tile space to be filled
								 uint32& NearestY );			// nearest position X in tile space to be filled

	void		FindNearestWideOpen( uint32 TileStartX,				// starting position X in tile space
								 uint32 TileStartY,				// starting position Y in tile space
								 uint32 TileDestinationX,		// destination position X in tile space
								 uint32 TileDestinationY,		// destionation position X in tile space
								 uint32& NearestX,				// nearest position X in tile space to be filled
								 uint32& NearestY );			// nearest position X in tile space to be filled

	AStarNode  *m_pBestSolution;

    AStarNode  *m_pOpenNode;
    AStarNode  *m_pClosedNode;
    AStarNode  *m_pPathNode;
    AStarStack *m_pStack;
    
    bool		m_PathValid;
    
    uint32		m_Height;
    uint32		m_Width;
	float32		m_CellWidth;
    int32		m_TotalSize;

    int16		**m_pTileMap;
    int16		**m_pObjectMap;

	float32		m_TopLeftX;
	float32		m_TopLeftY;

	bool		m_DestinationUnreachable;

	bool		m_WasPerfectPath;

	int32		m_MaximumTreeDepth;
};


#endif