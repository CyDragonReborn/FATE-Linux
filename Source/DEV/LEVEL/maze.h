/*
	maze.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/17/2004

*/

#ifndef _MAZE_H
#define _MAZE_H

class CTemplate;
class CPropInstance;

const enum ECellType
{
	KCTypeRoom,
	KCTypeRoomWall,
	KCTypeHall,
	KCTypeSolid,
	KCTypePit,
	KCTypeNSDoor,
	KCTypeEWDoor,
	KCTypeStairwellUp,
	KCTypeStairwellDown,
	KCTypeTemporaryPatch,
	KCellTypes
};

const std::string KCellNames[KCellTypes] =
{
	"ROOM",
	"ROOMWALL",
	"HALL",
	"SOLID",
	"PIT",
	"NSDOOR",
	"EWDOOR",
	"STAIRUP",
	"STAIRDOWN"
};

const enum EWallType
{
	KWTypeOpen,
	KWTypeNWSolid,
	KWTypeSWSolid,
	KWTypeNESolid,
	KWTypeSESolid,
	KWTypeNWCorner,
	KWTypeSWCorner,
	KWTypeNECorner,
	KWTypeSECorner,
	KWTypeNWInnerCorner,
	KWTypeSWInnerCorner,
	KWTypeNEInnerCorner,
	KWTypeSEInnerCorner,
	KWTypeNWCornerJoin,
	KWTypeSWCornerJoin,
	KWTypeNECornerJoin,
	KWTypeSECornerJoin,
	KWTypeNWHorizontal,
	KWTypeSWHorizontal,
	KWTypeNEHorizontal,
	KWTypeSEHorizontal,
	KWTypeNWVertical,
	KWTypeSWVertical,
	KWTypeNEVertical,
	KWTypeSEVertical,
	KWTypeNWHorizontalCorner,
	KWTypeSWHorizontalCorner,
	KWTypeNEHorizontalCorner,
	KWTypeSEHorizontalCorner,
	KWTypeNWVerticalCorner,
	KWTypeSWVerticalCorner,
	KWTypeNEVerticalCorner,
	KWTypeSEVerticalCorner,
	KWallTypes
};

const enum ETerrainType
{
	KTTypeOpen,
	KTTypeNWDiagonal,
	KTTypeNEDiagonal,
	KTTypeNorth,
	KTTypeSWDiagonal,
	KTTypeWest,
	KTTypeNEDiagonalJoin,
	KTTypeNWDiagonalInner,
	KTTypeSEDiagonal,
	KTTypeNWDiagonalJoin,
	KTTypeEast,
	KTTypeNEDiagonalInner,
	KTTypeSouth,
	KTTypeSWDiagonalInner,
	KTTypeSEDiagonalInner,
	KTTypeEmpty,
	KTTypeInvalid,
	KTerrainTypes
};

const static std::string KWallTypeName[KWallTypes] = 
{
	"OPEN",
	"NWSOLID",
	"SWSOLID",
	"NESOLID",
	"SESOLID",
	"NWCORNER",
	"SWCORNER",
	"NECORNER",
	"SECORNER",
	"NWINNERCORNER",
	"SWINNERCORNER",
	"NEINNERCORNER",
	"SEINNERCORNER",
	"NWCORNERJOIN",
	"SWCORNERJOIN",
	"NECORNERJOIN",
	"SECORNERJOIN",
	"NWHORIZONTAL",
	"SWHORIZONTAL",
	"NEHORIZONTAL",
	"SEHORIZONTAL",
	"NWVERTICAL",
	"SWVERTICAL",
	"NEVERTICAL",
	"SEVERTICAL",
	"NWHORIZONTALCORNER",
	"SWHORIZONTALCORNER",
	"NEHORIZONTALCORNER",
	"SEHORIZONTALCORNER",
	"NWVERTICALCORNER",
	"SWVERTICALCORNER",
	"NEVERTICALCORNER",
	"SEVERTICALCORNER"
};

// the maze generates a random 2d maze based upon several input parameters
// and a random number seed. The level object uses the maze to determine where
// a level's walls and media should be placed.  The maze is NOT responsible
// for actual visual media, only the underlying layout of a level
class CMaze
{

public:

			CMaze( CTemplate& pTemplate,	// template of available pieces to use
				   int32 Seed,				// starting seed
				   int32 Level );			// level depth
			~CMaze( void );
	
	void	Generate( void );

	void	DumpToFile( const std::string& Path );	// file to dump maze to

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	int32	Seed( void )		{	return m_Seed;			};

	uint32	Width( void )		{	return m_MazeWidth;		};
	uint32	Height( void )		{	return m_MazeHeight;	};

	CMaze::EWallType	WallTypeNW( int32 X,		// x location of cell to retrieve
								    int32 Y );		// y location of cell to retrieve

	CMaze::EWallType	WallTypeNE( int32 X,		// x location of cell to retrieve
								    int32 Y );		// y location of cell to retrieve

	CMaze::EWallType	WallTypeSW( int32 X,		// x location of cell to retrieve
								    int32 Y );		// y location of cell to retrieve

	CMaze::EWallType	WallTypeSE( int32 X,		// x location of cell to retrieve
								    int32 Y );		// y location of cell to retrieve

	CMaze::ETerrainType	TerrainTypeNW( int32 X,		// x location of cell to retrieve
									   int32 Y );		// y location of cell to retrieve

	CMaze::ETerrainType	TerrainTypeNE( int32 X,		// x location of cell to retrieve
								       int32 Y );		// y location of cell to retrieve

	CMaze::ETerrainType	TerrainTypeSW( int32 X,		// x location of cell to retrieve
								       int32 Y );		// y location of cell to retrieve

	CMaze::ETerrainType	TerrainTypeSE( int32 X,		// x location of cell to retrieve
								       int32 Y );		// y location of cell to retrieve

	float32				FloorHeight( int32 X,		// x cell to check
									 int32 Y );		// y cell to check

	float32				XOffset( int32 X,		// x cell to check
								 int32 Y );		// y cell to check

	float32				YOffset( int32 X,		// x cell to check
								 int32 Y );		// y cell to check

	bool		CellOpen( int32 X,		// x coordinate of cell to check
						  int32 Y );	// y coordinate of cell to check

	bool		CellOpenIgnoreDoors( int32 X,		// x coordinate of cell to check
									 int32 Y );	// y coordinate of cell to check

	int16		CellHeight( int32 X,	// x location of cell to retrieve
						    int32 Y );	// y location of cell to retrieve

	bool		HasWater( int32 X,		// x location of cell to retrieve
						  int32 Y );	// y location of cell to retrieve

	bool		IsFishable( int32 X,	// x location of cell to retrieve
							int32 Y );	// y location of cell to retrieve

	bool		HasCollideableProp( int32 X,	// x location of cell to retrieve
									int32 Y );	// y location of cell to retrieve

	uint16		WallType( int32 X,		// x location of cell to retrieve
						  int32 Y );	// y location of cell to retrieve

	int32		TerrainType( int32 X,	// x location of cell to retrieve
						     int32 Y );	// y location of cell to retrieve

	int32		RoomExits( int32 RoomIndex )		{	return m_RoomExits[RoomIndex];		};

	int32		RoomWidth( int32 RoomIndex )		{	return m_RoomWidth[RoomIndex];		};

	int32		RoomHeight( int32 RoomIndex )		{	return m_RoomHeight[RoomIndex];		};

	uint32		Rooms( void )						{	return m_RoomX.size();				};

	bool		IsCliff( int32 X,	// x location of cell to retrieve
						 int32 Y );	// y location of cell to retrieve

	bool		IsFlat( int32 X,	// x location of cell to retrieve
						int32 Y );	// y location of cell to retrieve

	bool		IsDiagonal( int32 X,	// x location of cell to retrieve
							int32 Y );	// y location of cell to retrieve

	bool		IsBorder( int32 X,		// x location of cell to retrieve
						  int32 Y );	// y location of cell to retrieve

	bool		RequiresFloorTile( int32 X,		// x location of cell to retrieve
								   int32 Y );	// y location of cell to retrieve

	bool		CellPassable( int32 X,		// x location of cell to check
							  int32 Y );	// y location of cell to check

	uint32			Props( void )			{	return m_pProps.size();		};

	CPropInstance*	Prop( int32 Index )	{	return m_pProps[Index];		};

	ECellType	CellType( int32 X,			// x location of cell to check
						  int32 Y );		// y location of cell to check

	uint32				PositionalAudioItems( void )			{	return m_PositionalAudioRadius.size();		};

	const std::string&	PositionalAudioPath( int32 Index )		{	return m_PositionalAudioPath[Index];		};

	float32				PositionalAudioRadius( int32 Index )	{	return m_PositionalAudioRadius[Index];		};

	const D3DXVECTOR3&	PositionalAudioPosition( int32 Index )	{	return m_PositionalAudioPosition[Index];	};

	int32		RoomIndexData( int32 X,		// x location of cell to retrieve
							   int32 Y );	// y location of cell to retrieve

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////


private:
	void		GenerateProps( void );

	void		VerifyWalls( void );

	void		GenerateExits( void );

	void		SetWallBorders( void );

	void		SetWallBorder( int32 X,			// x cell
							   int32 Y,			// y cell
							   uint16 TerrainType );// terrain type
	void		FillMaze( int16 Value );	// height value to fill maze with

	void		GenerateCorridors( void );

	void		ConnectDeadEnds( void );

	void		ConnectIsolatedRooms( void );

	void		Sparsify( void );

	void		PlugHoles( void );

	void		RaiseRoof( void );

	void		Roundify( void );

	void		GenerateDoors( void );

	void		GenerateRooms( int32 Rooms );	// rooms to generate

	void		GenerateRoom( int32 BestX,		// top left x
							  int32 BestY,		// top left y
							  int32 Width,		// room width
							  int32 Height );	// room height

	void		GenerateOvalRoom( int32 BestX,		// top left x
								  int32 BestY,		// top left y
								  int32 Width,		// room width
								  int32 Height );	// room height

	void		GenerateRingRoom( int32 BestX,		// top left x
								  int32 BestY,		// top left y
								  int32 Width,		// room width
								  int32 Height );	// room height

	void		GeneratePits( int32 Pits );	// pits to generate

	void		GeneratePit( int32 BestX,		// top left x
							 int32 BestY,		// top left y
							 int32 Width,		// room width
							 int32 Height );	// room height

	void		UnVisitCell( int32 X,	// x location of cell to mark as visited
							 int32 Y );// y location of cell to mark as visited

	void		SetRoomIndexData( int32 X,			// x location of cell to mark as visited
								  int32 Y,			// y location of cell to mark as visited
								  int32 Value ); // value to set

	void		SetCellType( int32 X,			// x location of cell to mark as visited
							 int32 Y,			// y location of cell to mark as visited
							 ECellType Value ); // value to set

	bool		CellVisited( int32 X,		// x location of cell to check
						     int32 Y );	// y location of cell to check

	bool		SelectRandomUnvisitedCell( int32& X,	// x coordinate to fill 
										   int32& Y );	// y coordinate to fill

	bool		SelectRandomCell( int32& X,			// x coordinate to fill 
								  int32& Y,			// y coordinate to fill
								  int16 Value );		// value requirement for cell 

	bool		SelectRandomCell( int32& X,			// x coordinate to fill 
								  int32& Y,			// y coordinate to fill
								  ECellType Value );		// value requirement for cell 

	int32		CalculateScoreForRoom( int32 X,		// x top left corner of room
									   int32 Y,		// y top left corner of room
									   int32 Width,	// width of room
									   int32 Height );	// height of room

	int32		CalculateScoreForPit( int32 X,			// x top left corner of room
									  int32 Y,			// y top left corner of room
									  int32 Width,		// width of room
									  int32 Height );	// height of room

	void		CountRoomExits( void );

	int32		CountExits( int32 X,		// x cell to check
							int32 Y );		// y cell to check

	bool		IsCorner( int32 X,			// x cell to check
						  int32 Y,			// y cell to check
						  int16& Height );	// height of corner walls to fill

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void		SetCellHeight( int32 X,					// x location of cell
							   int32 Y,					// y location of cell
							   int16 Value );			// value to fill cell with

	void		SetCellHeightProtected( int32 X,					// x location of cell
										int32 Y,					// y location of cell
										int16 Value );			// value to fill cell with

	void		SetHasWater( int32 X,					// x location of cell
							 int32 Y,					// y location of cell
							 bool Value );				// value to fill cell with

	void		SetIsFishable( int32 X,					// x location of cell
							   int32 Y,					// y location of cell
							   bool Value );			// value to fill cell with

	void		SetHasCollideableProp( int32 X,					// x location of cell
										int32 Y,					// y location of cell
										bool Value );			// value to fill cell with

	void		SetWallType( int32 X,					// x location of cell
							 int32 Y,					// y location of cell
							 uint16 Value );			// value to fill cell with

	void		SetTerrainType( int32 X,				// x location of cell
							    int32 Y,				// y location of cell
							    int32 Value );			// value to fill cell with

	void		SetFloorHeight( int32 X,				// x location of cell
							    int32 Y,				// y location of cell
							    float32 Value );		// value to fill cell with

	void		SetOffset( int32 X,				// x location of cell
					       int32 Y,				// y location of cell
						   float32 XOffset,		// x turbulence
						   float32 YOffset );	// y turbulence

	void		VisitCell( int32 X,	// x location of cell to mark as visited
						   int32 Y );	// y location of cell to mark as visited

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	bool		ValidCell( int32 X,		// x coordinate of cell to check
						   int32 Y );	// y coordinate of cell to check		 


	int32			m_MazeWidth;
	int32			m_MazeHeight;

	int32			m_Seed;

	int32			m_Sparseness;		// how dense the maze will be in terms of rooms and corridors
	int32			m_Roundedness;		// how likely corners in rooms and halls will be shaved off
	int32			m_Connectedness;	// how likely rooms are to be intersected
	int32			m_Twistiness;		// how kinked corridors may be

	int32			m_Rooms;			// how many rooms in the maze
	int32			m_MinRoomWidth;		// minimum cell width of a room
	int32			m_MaxRoomWidth;		// maximum cell width of a room
	int32			m_MinRoomHeight;	// minimum cell height of a room
	int32			m_MaxRoomHeight;	// maximum cell height of a room

	int32			m_Pits;				// how many pits in the maze
	int32			m_MinPitWidth;		// minimum cell width of a pit
	int32			m_MaxPitWidth;		// maximum cell width of a pit
	int32			m_MinPitHeight;		// minimum cell height of a pit
	int32			m_MaxPitHeight;		// maximum cell height of a pit

	uint16			m_TerrainTypes;

	int16*			m_CellHeightData;
	uint16*			m_WallTypeData;
	int32*			m_TerrainTypeData;
	float32*		m_HeightData;
	float32*		m_XOffsetData;
	float32*		m_YOffsetData;
	bool*			m_CellHasWater;
	bool*			m_CellIsFishable;
	bool*			m_CellHasCollideableProp;

	int32*			m_RoomIndexData;

	float32			m_HeightDistortion;
	float32			m_XYDistortion;

	bool*			m_CellVisited;
	ECellType*		m_CellType;

	int32			m_CellsVisited;

	CTemplate&		m_pTemplate;

	std::vector< CPropInstance* >		m_pProps;

	std::vector< std::string >			m_PositionalAudioPath;
	std::vector< D3DXVECTOR3 >			m_PositionalAudioPosition;
	std::vector< float32 >				m_PositionalAudioRadius;

	std::vector< bool >					m_RoomIsolated;
	std::vector< int32 >				m_RoomX;
	std::vector< int32 >				m_RoomY;
	std::vector< int32 >				m_RoomWidth;
	std::vector< int32 >				m_RoomHeight;
	std::vector< int32 >				m_RoomExits;
};

#endif