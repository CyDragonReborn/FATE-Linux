/*
	maze.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/17/2004

*/

#include <assert.h>
#include <time.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"

#include "../GAME/gameconstants.h"

#include "template.h"
#include "levelmeshpatch.h"
#include "maze.h"

CMaze::CMaze( CTemplate& pTemplate,	// template of available pieces to use
			  int32 Seed,			// starting seed
			  int32 Level ) :		// level depth
								m_pTemplate( pTemplate ),
								m_Twistiness( 600 ),		// value between 0-1000
								m_Connectedness( 1000 ),	// value between 0-1000
								m_Sparseness( 5 ),		// value between 0-1000
								m_MinRoomWidth( 3 ),
								m_MaxRoomWidth( 8 ),
								m_MinRoomHeight( 3 ),
								m_MaxRoomHeight( 9 ),
								m_MinPitWidth( 2 ),
								m_MaxPitWidth( 6 ),
								m_MinPitHeight( 2 ),
								m_MaxPitHeight( 6 ),
								m_Rooms( 8 ),
								m_Pits( 8 ),
								m_Seed( Seed ),
								m_CellsVisited( 0 ),
								m_MazeWidth( 32 ),
								m_MazeHeight( 32 ),
								m_CellHeightData( NULL ),
								m_RoomIndexData( NULL ),
								m_HeightData( NULL ),
								m_XOffsetData( NULL ),
								m_YOffsetData( NULL ),
								m_CellVisited( NULL ),
								m_CellType( NULL ),
								m_Roundedness ( 0 ),
								m_TerrainTypes ( 0 ),
								m_WallTypeData ( NULL ),
								m_TerrainTypeData ( NULL ),
								m_CellHasWater ( NULL ),
								m_CellIsFishable ( NULL ),
								m_CellHasCollideableProp ( NULL ),
								m_HeightDistortion ( 0.0f ),
								m_XYDistortion ( 0.0f )
{
	m_Seed *= Level * 2;

	srand( m_Seed );


	m_MazeWidth = RandomIntegerBetween( m_pTemplate.MinimumWidth(),
									    m_pTemplate.MaximumWidth() );

	m_MazeHeight = RandomIntegerBetween( m_pTemplate.MinimumHeight(),
									     m_pTemplate.MaximumHeight() );

	// make sure the width and height fit the number of tiles we have per patch
	m_MazeWidth = (int32)floor( (float32)m_MazeWidth / (float32)KPatchTiles );
	m_MazeHeight = (int32)floor( (float32)m_MazeHeight / (float32)KPatchTiles );
	m_MazeWidth *= KPatchTiles;
	m_MazeHeight *= KPatchTiles;

	m_HeightDistortion = m_pTemplate.HeightDistortion();

	m_XYDistortion = m_pTemplate.XYDistortion();

	m_Twistiness = RandomIntegerBetween( pTemplate.MinimumTwistiness(), pTemplate.MaximumTwistiness() );

	m_Connectedness = RandomIntegerBetween( pTemplate.MinimumConnectedness(), pTemplate.MaximumConnectedness() );

	m_Sparseness = RandomIntegerBetween( pTemplate.MinimumSparseness(), pTemplate.MaximumSparseness() );

	m_Roundedness = RandomIntegerBetween( pTemplate.MinimumRoundedness(), pTemplate.MaximumRoundedness() );

	m_Rooms = RandomIntegerBetween( m_pTemplate.MinimumRooms(),
									m_pTemplate.MaximumRooms() );

	m_Pits = RandomIntegerBetween( m_pTemplate.MinimumPits(),
								   m_pTemplate.MaximumPits() );

	m_MinRoomWidth = pTemplate.MinimumRoomWidth();
	m_MaxRoomWidth = pTemplate.MaximumRoomWidth();
	m_MinRoomHeight = pTemplate.MinimumRoomHeight();
	m_MaxRoomHeight = pTemplate.MaximumRoomHeight();
	if( m_MinRoomWidth < 3 )
	{
		m_MinRoomWidth = 3;
	}
	
	if( m_MaxRoomWidth > m_MazeWidth - 4 )
	{
		m_MaxRoomWidth = m_MazeWidth - 4;
	}
	if( m_MaxRoomWidth > m_MazeWidth - 4 )
	{
		m_MaxRoomWidth = m_MazeWidth - 4;
	}

	if( m_MinRoomHeight < 3 )
	{
		m_MinRoomHeight = 3;
	}
	
	if( m_MaxRoomHeight > m_MazeHeight - 4 )
	{
		m_MaxRoomHeight = m_MazeHeight - 4;
	}
	if( m_MaxRoomHeight > m_MazeHeight - 4 )
	{
		m_MaxRoomHeight = m_MazeHeight - 4;
	}

	m_TerrainTypes = (uint16)pTemplate.GroundLayers();

	m_CellHeightData = new int16[m_MazeWidth*m_MazeHeight];
	m_RoomIndexData = new int32[m_MazeWidth*m_MazeHeight];
	m_CellVisited = new bool[m_MazeWidth*m_MazeHeight];
	m_CellHasWater = new bool[m_MazeWidth*m_MazeHeight];
	m_CellIsFishable = new bool[m_MazeWidth*m_MazeHeight];
	m_CellHasCollideableProp = new bool[m_MazeWidth*m_MazeHeight];
	m_CellType = new ECellType[m_MazeWidth*m_MazeHeight];
	m_WallTypeData = new uint16[m_MazeWidth*m_MazeHeight];
	m_TerrainTypeData = new int32[m_MazeWidth*m_MazeHeight];
	m_HeightData = new float32[m_MazeWidth*m_MazeHeight];
	m_XOffsetData = new float32[m_MazeWidth*m_MazeHeight];
	m_YOffsetData = new float32[m_MazeWidth*m_MazeHeight];

	Generate();
} // CMaze::CMaze()

CMaze::~CMaze( void )
{
	for( uint32 i = 0; i < m_pProps.size(); i++ )
	{
		DELETE_SAFELY( m_pProps[i] );
	}
	m_pProps.clear();

	DELETE_ARRAY_SAFELY( m_CellHeightData );
	DELETE_ARRAY_SAFELY( m_RoomIndexData );
	DELETE_ARRAY_SAFELY( m_HeightData );
	DELETE_ARRAY_SAFELY( m_XOffsetData );
	DELETE_ARRAY_SAFELY( m_YOffsetData );
	DELETE_ARRAY_SAFELY( m_WallTypeData );
	DELETE_ARRAY_SAFELY( m_TerrainTypeData );
	DELETE_ARRAY_SAFELY( m_CellVisited );
	DELETE_ARRAY_SAFELY( m_CellType );
	DELETE_ARRAY_SAFELY( m_CellHasWater );
	DELETE_ARRAY_SAFELY( m_CellIsFishable );
	DELETE_ARRAY_SAFELY( m_CellHasCollideableProp );
} // CMaze::~CMaze()

void CMaze::DumpToFile( const std::string& Path )	// file to dump maze to
{
	FILE* pFile;
	pFile = fopen( Path.c_str(), "wt" );
	fputs( ( IntegerToString( m_Seed ) + "\n" ).c_str(), pFile );
	fputs( ( IntegerToString( m_Rooms ) + "\n" ).c_str(), pFile );
	fputs( ( IntegerToString( m_Pits ) + "\n" ).c_str(), pFile );
	if ( pFile != NULL )
	{

		for( int32 y = 0; y < m_MazeHeight; y++ )
		{
			for( int32 x = 0; x < m_MazeWidth; x++ )
			{
				if( CellHeight( x, y ) == 0 )
				{
					if( CellType( x, y ) == KCTypeRoom )
					{
						if( RoomIndexData( x, y ) < 0 )
						{
							fputs( "*", pFile );
						}
						else
						{
							fputs( IntegerToString( RoomIndexData( x, y ) ).c_str(), pFile );
						}
					}
					else
					{
						fputs( ".", pFile );
					}
				}
				else if( CellHeight( x, y ) < 0 )
				{
					fputs( "O", pFile );
				}
				else 
				{
					fputs( "X", pFile );
				}
			}
			fputs( "\n", pFile );

		}
		for( uint32 i = 0; i < Rooms(); i++ )
		{
			fputs( ( "Room " + IntegerToString( i ) + " Exits = " + IntegerToString( RoomExits( i ) ) + "\n" ).c_str(), pFile );
		}
		fclose( pFile );
	}

} // CMaze::DumpToFile()

void CMaze::FillMaze( int16 Value )	// height value to fill maze with
{
	m_CellsVisited = 0;
	// first fill in the borders of the map area
	for( int32 i = 0; i < m_MazeWidth; i++ )
	{
		for( int32 j = 0; j < m_MazeHeight; j++ )
		{

			SetFloorHeight( i, j, ( ( rand() % 1000 / 1000.0f ) - .5f ) * m_HeightDistortion );
			SetOffset( i, 
					   j, 
					   ( ( rand() % 1000 / 1000.0f ) - .5f ) * m_XYDistortion,
					   ( ( rand() % 1000 / 1000.0f ) - .5f ) * m_XYDistortion );
			SetCellHeight( i, j, Value );
			uint16 Terrain = (uint16)m_pTemplate.RandomTerrain();
			if( Terrain >= m_TerrainTypes )
			{
				Terrain = 0;
			}
			SetTerrainType( i, j, Terrain );
			SetWallType( i, j, 0 );
			UnVisitCell( i, j );
			m_CellType[ i + j * m_MazeWidth] = KCTypeSolid;
			SetHasWater( i, j, kFalse );
			SetIsFishable( i, j, kFalse );
			SetHasCollideableProp( i, j, kFalse );
			SetRoomIndexData( i, j, -1 );
		}
	}

} // CMaze::FillMaze()

void CMaze::Generate( void )
{

	int32 X( 0 );
	int32 Y( 0 );

	FillMaze( 1 );
	// first fill in the borders of the map area
	for( int32 i = 0; i < m_MazeWidth; i++ )
	{
		VisitCell( i, 0 );
		VisitCell( i, m_MazeHeight - 1 );
	}
	for( int32 i = 0; i < m_MazeHeight; i++ )
	{
		VisitCell( 0, i );
		VisitCell( m_MazeWidth - 1, i );
	}

	GenerateCorridors();

	Sparsify();

	GenerateRooms( m_Rooms );

	CountRoomExits();

	for( int32 i = 0; i < (int32)m_RoomIsolated.size(); i++ )
	{
		if( m_RoomExits[i] == 0 && !m_RoomIsolated[i] )
		{
			m_RoomIsolated[i] = kTrue;
		}
	}

	ConnectIsolatedRooms();

	ConnectDeadEnds();

	GeneratePits( m_Pits );

	VerifyWalls();

	PlugHoles();


	GenerateExits();


	RaiseRoof();

	// fill in the border terrain types of all the walls
	SetWallBorders();

	CountRoomExits();

	GenerateDoors();



	GenerateProps();

} // CMaze::Generate()

void CMaze::GenerateProps( void )
{
	for( int32 X = 1; X < m_MazeWidth - 1; X++ )
	{
		for( int32 Y = 1; Y < m_MazeHeight - 1; Y++ )
		{
			ECellType Location = CellType( X, Y );
			if( CellOpenIgnoreDoors( X, Y ) || HasWater( X, Y ) ||
				Location == KCTypePit )
			{

				bool ValidProp( kTrue );
				EPropLocation PropLocation( KPropInRoom );
				EPropLocation TorchLocation( KPropAnywhere );
				
				if( HasWater( X, Y ) &&
					CellHeight( X, Y ) < 0 )
				{
					if( !IsCliff( X, Y ) )
					{
						PropLocation = KPropWater;
						if( RandomBetween( 0, 10 ) > 5  )
						{
							ValidProp = kFalse;
						}
					}
					else
					{
						ValidProp = kFalse;
					}
				}
				else 
				{
					switch( Location )
					{
					case KCTypePit :
						if( !IsCliff( X, Y ) &&
							CellHeight( X, Y ) < 0 )
						{
							PropLocation = KPropPit;
							if( RandomBetween( 0, 10 ) > 5  )
							{
								ValidProp = kFalse;
							}
						}
						else
						{
							ValidProp = kFalse;
						}
						break;
					case KCTypeNSDoor :
						PropLocation = KPropNSDoor;
						break;
					case KCTypeEWDoor :
						PropLocation = KPropEWDoor;
						break;
					case KCTypeRoom :
						PropLocation = KPropInRoom;
						if( CellType( X, Y + 1 ) == KCTypeRoomWall )
						{
							PropLocation = KPropSWall;
							if( RandomIntegerBetween( 0, 100 ) < 50 )
							{
								TorchLocation = KPropSTorch;
							}
						}
						if( CellType( X + 1, Y ) == KCTypeRoomWall )
						{
							PropLocation = KPropEWall;
							if( RandomIntegerBetween( 0, 100 ) < 50 )
							{
								TorchLocation = KPropETorch;
							}
						}
						if( CellType( X - 1, Y ) == KCTypeRoomWall )
						{
							PropLocation = KPropWWall;
							if( RandomIntegerBetween( 0, 100 ) < 50 )
							{
								TorchLocation = KPropWTorch;
							}
						}
						if( CellType( X, Y - 1 ) == KCTypeRoomWall )
						{
							PropLocation = KPropNWall;
							if( RandomIntegerBetween( 0, 100 ) < 50 )
							{
								TorchLocation = KPropNTorch;
							}
						}
						if( RandomBetween( 0, 100 ) <= 70 )
						{
							ValidProp = kFalse;
						}
						if( CellType( X - 1, Y ) == KCTypeEWDoor ||
							CellType( X + 1, Y ) == KCTypeEWDoor ||
							CellType( X , Y - 1 ) == KCTypeNSDoor ||
							CellType( X , Y + 1 ) == KCTypeNSDoor )
						{
							ValidProp = kFalse;
						}
						if( CellType( X - 1, Y ) == KCTypeStairwellUp ||
							CellType( X + 1, Y ) == KCTypeStairwellUp ||
							CellType( X , Y - 1 ) == KCTypeStairwellUp ||
							CellType( X , Y + 1 ) == KCTypeStairwellUp )
						{
							ValidProp = kFalse;
						}
						if( CellType( X - 1, Y ) == KCTypeStairwellDown ||
							CellType( X + 1, Y ) == KCTypeStairwellDown ||
							CellType( X , Y - 1 ) == KCTypeStairwellDown ||
							CellType( X , Y + 1 ) == KCTypeStairwellDown )
						{
							ValidProp = kFalse;
						}
						if( IsCliff( X, Y ) && PropLocation == KPropInRoom )
						{
							ValidProp = kFalse;
						}
						break; 
					case KCTypeHall :
						PropLocation = KPropInHall;

						if( CellType( X, Y + 1 ) == KCTypeSolid )
						{
							PropLocation = KPropSHallWall;
							if( RandomIntegerBetween( 0, 100 ) < 50 )
							{
								TorchLocation = KPropSTorch;
							}
						}
						if( CellType( X + 1, Y ) == KCTypeSolid )
						{
							PropLocation = KPropEHallWall;
							if( RandomIntegerBetween( 0, 100 ) < 50 )
							{
								TorchLocation = KPropETorch;
							}
						}
						if( CellType( X, Y - 1 ) == KCTypeSolid )
						{
							PropLocation = KPropNHallWall;
							if( RandomIntegerBetween( 0, 100 ) < 50 )
							{
								TorchLocation = KPropNTorch;
							}
						}
						if( CellType( X - 1, Y ) == KCTypeSolid )
						{
							PropLocation = KPropWHallWall;
							if( RandomIntegerBetween( 0, 100 ) < 50 )
							{
								TorchLocation = KPropWTorch;
							}
						}
						if( RandomBetween( 0, 100 ) <= 50 )
						{
							ValidProp = kFalse;
						}
						if( CellType( X - 1, Y ) == KCTypeEWDoor ||
							CellType( X + 1, Y ) == KCTypeEWDoor ||
							CellType( X , Y - 1 ) == KCTypeNSDoor ||
							CellType( X , Y + 1 ) == KCTypeNSDoor )
						{
							ValidProp = kFalse;
						}
						if( CellType( X - 1, Y ) == KCTypeStairwellUp ||
							CellType( X + 1, Y ) == KCTypeStairwellUp ||
							CellType( X , Y - 1 ) == KCTypeStairwellUp ||
							CellType( X , Y + 1 ) == KCTypeStairwellUp )
						{
							ValidProp = kFalse;
						}
						if( CellType( X - 1, Y ) == KCTypeStairwellDown ||
							CellType( X + 1, Y ) == KCTypeStairwellDown ||
							CellType( X , Y - 1 ) == KCTypeStairwellDown ||
							CellType( X , Y + 1 ) == KCTypeStairwellDown )
						{
							ValidProp = kFalse;
						}
						break;		
					}
				}
				if( ValidProp )
				{
					CPropTemplate* pRandomProp = m_pTemplate.RandomProp( PropLocation );
					if( pRandomProp != NULL )
					{

						if( pRandomProp->RequiresFlat() )
						{

							if( ( PropLocation == KPropSWall ||
								  PropLocation == KPropSHallWall ) &&
								  !IsFlat( X, Y + 1 ) )
							{
								ValidProp = kFalse;
							}
							if( ( PropLocation ==KPropNWall ||
								  PropLocation == KPropNHallWall ) &&
								  !IsFlat( X, Y - 1 ) )
							{
								ValidProp = kFalse;
							}
							if( ( PropLocation == KPropWWall ||
								  PropLocation == KPropWHallWall ) &&
								  !IsFlat( X - 1, Y )  )
							{
								ValidProp = kFalse;
							}
							if( ( PropLocation == KPropEWall ||
								  PropLocation == KPropEHallWall ) &&
								  !IsFlat( X + 1, Y ) )
							{
								ValidProp = kFalse;
							}
						}

						if( pRandomProp->BufferN() && HasCollideableProp( X, Y - 1 ) ||
							pRandomProp->BufferS() && HasCollideableProp( X, Y + 1 ) ||
							pRandomProp->BufferE() && HasCollideableProp( X + 1, Y ) ||
							pRandomProp->BufferW() && HasCollideableProp( X - 1, Y ) ||
							pRandomProp->BufferNW() && HasCollideableProp( X - 1, Y - 1 ) ||
							pRandomProp->BufferNE() && HasCollideableProp( X + 1, Y - 1 ) ||
							pRandomProp->BufferSW() && HasCollideableProp( X - 1, Y + 1 ) ||
							pRandomProp->BufferSE() && HasCollideableProp( X + 1, Y + 1 ) )
						{
							ValidProp = kFalse;
						}
						if( pRandomProp->Collideable() )
						{
							if( IsDiagonal( X, Y ) ||
								IsDiagonal( X, Y - 1 ) ||
								IsDiagonal( X, Y + 1 ) ||
								IsDiagonal( X - 1, Y ) ||
								IsDiagonal( X + 1, Y ) )
							{
								ValidProp = kFalse;
							}

							if( HasCollideableProp( X - 1, Y - 1 ) ||
								HasCollideableProp( X + 1, Y - 1 ) ||
								HasCollideableProp( X - 1, Y + 1 ) ||
								HasCollideableProp( X + 1, Y + 1 ) )
							{
								ValidProp = kFalse;
							}

							if( CellType( X - 1, Y ) == KCTypeEWDoor ||
								CellType( X + 1, Y ) == KCTypeEWDoor ||
								CellType( X , Y - 1 ) == KCTypeNSDoor ||
								CellType( X , Y + 1 ) == KCTypeNSDoor )
							{
								ValidProp = kFalse;
							}

							if( PropLocation == KPropInRoom &&
								!CellOpen( X - 1, Y ) && !CellOpen( X + 1, Y ) )
							{
								ValidProp = kFalse;
							}
							if( PropLocation == KPropInRoom &&
								!CellOpen( X, Y - 1 ) && !CellOpen( X, Y + 1 ) )
							{
								ValidProp = kFalse;
							}
							if( PropLocation == KPropInRoom &&
								!CellOpen( X - 1, Y - 1 ) && !CellOpen( X + 1, Y + 1 ) )
							{
								ValidProp = kFalse;
							}
							if( PropLocation == KPropInRoom &&
								!CellOpen( X + 1, Y - 1 ) && !CellOpen( X - 1, Y + 1 ) )
							{
								ValidProp = kFalse;
							}
						}
						// last check to make sure collideable objects aren't in front of a door!
						if( ValidProp )
						{
							if( PropLocation == KPropWater )
							{
								if( ( CellHeight( X - 1, Y ) == 0 &&
									!HasCollideableProp( X - 1, Y ) ) ||
									( CellHeight( X + 1, Y ) == 0 &&
									!HasCollideableProp( X + 1, Y ) ) ||
									( CellHeight( X, Y - 1 ) == 0 &&
									!HasCollideableProp( X, Y - 1 ) ) ||
									( CellHeight( X, Y + 1 ) == 0 &&
									!HasCollideableProp( X, Y + 1 ) ) )
								{
									if( !IsFishable( X - 1, Y ) &&
										!IsFishable( X + 1, Y ) &&
										!IsFishable( X, Y - 1 ) &&
										!IsFishable( X, Y + 1 ) &&
										!IsFishable( X - 1, Y - 1 ) &&
										!IsFishable( X + 1, Y - 1 ) &&
										!IsFishable( X - 1, Y - 1 ) &&
										!IsFishable( X + 1, Y + 1 ) )
									{
										if( RandomIntegerBetween( 0, 1000 ) < 200 )
										{
											SetIsFishable( X, Y, kTrue );
										}
									}
								}
							}
							if( pRandomProp->Collideable() )
							{
								SetHasCollideableProp( X, Y, kTrue );
							}

							if( pRandomProp->BufferN() )
							{
								SetHasCollideableProp( X, Y - 1, kTrue );
							}
							if( pRandomProp->BufferS() )
							{
								SetHasCollideableProp( X, Y + 1, kTrue );
							}
							if( pRandomProp->BufferE() )
							{
								SetHasCollideableProp( X + 1, Y, kTrue );
							}
							if( pRandomProp->BufferW() )
							{
								SetHasCollideableProp( X - 1, Y, kTrue );
							}
							if( pRandomProp->BufferNW() )
							{
								SetHasCollideableProp( X - 1, Y - 1, kTrue );
							}
							if( pRandomProp->BufferNE() )
							{
								SetHasCollideableProp( X + 1, Y - 1, kTrue );
							}
							if( pRandomProp->BufferSW() )
							{
								SetHasCollideableProp( X - 1, Y + 1, kTrue );
							}
							if( pRandomProp->BufferSE() )
							{
								SetHasCollideableProp( X + 1, Y + 1, kTrue );
							}

							int32 Instances = (int32)floor( RandomBetween( (float32)pRandomProp->MinimumBunch(),
																				(float32)pRandomProp->MaximumBunch() ) + .5f );
							for( int32 i = 0; i < Instances; i++ )
							{
								D3DXVECTOR3	Offset( pRandomProp->MinimumOffset() );
								Offset.x = RandomBetween( pRandomProp->MinimumOffset().x, pRandomProp->MaximumOffset().x );
								Offset.y = RandomBetween( pRandomProp->MinimumOffset().y, pRandomProp->MaximumOffset().y );
								Offset.z = RandomBetween( pRandomProp->MinimumOffset().z, pRandomProp->MaximumOffset().z );
								m_pProps.push_back( new CPropInstance( *pRandomProp,
																		X,
																		Y,
																		Offset ) );
							}
						}
					}
				}


				// if we didn't place ANYTHING, and this spot is OK for a torch -
				// maybe we'll place one. Torches have more likely placement
				// than anything else
				if( !ValidProp &&
					TorchLocation != KPropAnywhere )
				{
					if( RandomIntegerBetween( 0, 100 ) < 50 )
					{
						CPropTemplate* pRandomProp = m_pTemplate.RandomProp( TorchLocation );
						if( pRandomProp != NULL )
						{
							int32 Instances = (int32)floor( RandomBetween( (float32)pRandomProp->MinimumBunch(),
																				(float32)pRandomProp->MaximumBunch() ) + .5f );
							for( int32 i = 0; i < Instances; i++ )
							{
								D3DXVECTOR3	Offset( pRandomProp->MinimumOffset() );
								Offset.x = RandomBetween( pRandomProp->MinimumOffset().x, pRandomProp->MaximumOffset().x );
								Offset.y = RandomBetween( pRandomProp->MinimumOffset().y, pRandomProp->MaximumOffset().y );
								Offset.z = RandomBetween( pRandomProp->MinimumOffset().z, pRandomProp->MaximumOffset().z );
								m_pProps.push_back( new CPropInstance( *pRandomProp,
																		X,
																		Y,
																		Offset ) );
							}
						}
					}
				}
			}
		}
	}

	// find fishing holes
	for( int32 X = 1; X < m_MazeWidth - 1; X++ )
	{
		for( int32 Y = 1; Y < m_MazeHeight - 1; Y++ )
		{
			if( HasWater( X, Y ) && !IsCliff( X, Y ) )
			{
				if( ( CellHeight( X - 1, Y ) == 0 &&
					  !HasCollideableProp( X - 1, Y ) ) ||
					( CellHeight( X + 1, Y ) == 0 &&
					  !HasCollideableProp( X + 1, Y ) ) ||
					( CellHeight( X, Y - 1 ) == 0 &&
					  !HasCollideableProp( X, Y - 1 ) ) ||
					( CellHeight( X, Y + 1 ) == 0 &&
					  !HasCollideableProp( X, Y + 1 ) ) )
				{
					if( !IsFishable( X - 1, Y ) &&
						!IsFishable( X + 1, Y ) &&
						!IsFishable( X, Y - 1 ) &&
						!IsFishable( X, Y + 1 ) &&
						!IsFishable( X - 1, Y - 1 ) &&
						!IsFishable( X + 1, Y - 1 ) &&
						!IsFishable( X - 1, Y - 1 ) &&
						!IsFishable( X + 1, Y + 1 ) )
					{
						if( RandomIntegerBetween( 0, 1000 ) < 75 )
						{
							SetIsFishable( X, Y, kTrue );
						}
					}
				}
			}
		}
	}
} // CMaze::GenerateProps()

void CMaze::GenerateExits( void )
{
	bool GeneratedExitUp( kFalse );
	bool GeneratedExitDown( kFalse );
	bool FoundRoomWall( kFalse );
	int32 Tries( 0 );
	while( !GeneratedExitUp || !GeneratedExitDown )
	{
		for( int32 X = 1; X < m_MazeWidth - 1; X++ )
		{
			for( int32 Y = 1; Y < m_MazeHeight - 1; Y++ )
			{
				ECellType Location = CellType( X, Y );
				if( Location == KCTypeRoomWall )
				{
					int32 Chance = RandomIntegerBetween( 0, 1000 );
					if( X > 1 && Y > 1 && 
						X < m_MazeWidth - 2 && Y < m_MazeHeight - 2 )
					{
						FoundRoomWall = kTrue;
						if( Chance < 10 )
						{
							if( !GeneratedExitUp )
							{
								bool Valid( kFalse );
								EPropLocation PropLocation( KPropNStairsUp );
								if( CellOpen( X, Y + 1 ) &&
									!CellOpen( X - 1, Y ) &&
									!CellOpen( X + 1, Y ) &&
									!CellOpen( X, Y - 1 ) )
								{
									PropLocation = KPropNStairsUp;
									Valid = kTrue;
								}
								else if( CellOpen( X, Y - 1 ) &&
										!CellOpen( X - 1, Y ) &&
										!CellOpen( X + 1, Y ) &&
										!CellOpen( X, Y + 1 ) )
								{
									PropLocation = KPropSStairsUp;
									Valid = kTrue;
								}
								else if( CellOpen( X + 1, Y ) &&
										!CellOpen( X - 1, Y ) &&
										!CellOpen( X, Y - 1 ) &&
										!CellOpen( X, Y + 1 ) )
								{
									PropLocation = KPropWStairsUp;
									Valid = kTrue;
								}
								else if( CellOpen( X - 1, Y ) &&
										!CellOpen( X + 1, Y ) &&
										!CellOpen( X, Y - 1 ) &&
										!CellOpen( X, Y + 1 ) )
								{
									PropLocation = KPropEStairsUp;
									Valid = kTrue;
								}
								if( Valid )
								{
									SetCellHeightProtected( X, Y, 0 );
									SetCellType( X, Y, KCTypeStairwellUp );
									SetFloorHeight( X, Y, 0 );
									SetFloorHeight( X - 1, Y, 0 );
									SetFloorHeight( X + 1, Y, 0 );
									SetFloorHeight( X, Y - 1, 0 );
									SetFloorHeight( X, Y + 1, 0 );
									SetOffset( X, Y, 0, 0 );
									SetOffset( X - 1, Y, 0, 0 );
									SetOffset( X + 1, Y, 0, 0 );
									SetOffset( X, Y - 1, 0, 0 );
									SetOffset( X, Y + 1, 0, 0 );

									CPropTemplate* pRandomProp = m_pTemplate.RandomProp( PropLocation );
									if( pRandomProp != NULL )
									{
										GeneratedExitUp = kTrue;
										m_pProps.push_back( new CPropInstance( *pRandomProp,
																				X,
																				Y,
																				D3DXVECTOR3( 0, 0, 0 ) ) );
									}
								}
							}
						}
						else if( Chance < 20 )
						{
							if( !GeneratedExitDown )
							{
								bool Valid( kFalse );
								EPropLocation PropLocation = KPropNStairsDown;	
								

								if( X > 2 && Y > 2 && 
									X < m_MazeWidth - 2 && Y < m_MazeHeight - 2 &&
									CellType( X - 1, Y - 1 ) != KCTypeStairwellUp &&
									CellType( X + 1, Y - 1 ) != KCTypeStairwellUp &&
									CellType( X - 1, Y + 1 ) != KCTypeStairwellUp &&
									CellType( X + 1, Y + 1 ) != KCTypeStairwellUp &&
									CellType( X, Y - 1 ) != KCTypeStairwellUp &&
									CellType( X, Y + 1 ) != KCTypeStairwellUp &&
									CellType( X - 1, Y ) != KCTypeStairwellUp &&
									CellType( X + 1, Y ) != KCTypeStairwellUp )
								{
									if( CellOpen( X, Y + 1 ) &&
										//!CellOpen( X - 1, Y ) &&
										//!CellOpen( X + 1, Y ) &&
										!CellOpen( X, Y - 1 ) )
									{
										PropLocation = KPropNStairsDown;
										Valid = kTrue;
									}
									else if( CellOpen( X, Y - 1 ) &&
										//!CellOpen( X - 1, Y ) &&
										//!CellOpen( X + 1, Y ) &&
										!CellOpen( X, Y + 1 ) )
									{
										PropLocation = KPropSStairsDown;
										Valid = kTrue;
									}
									else if( CellOpen( X + 1, Y ) &&
											!CellOpen( X - 1, Y ) )//&&
											//!CellOpen( X, Y - 1 ) &&
											//!CellOpen( X, Y + 1 ) )
									{
										PropLocation = KPropWStairsDown;
										Valid = kTrue;
									}
									else if( CellOpen( X - 1, Y ) &&
											!CellOpen( X + 1, Y ) )//&&
											//!CellOpen( X, Y - 1 ) &&
											//y!CellOpen( X, Y + 1 ) )
									{
										PropLocation = KPropEStairsDown;
										Valid = kTrue;
									}
								}


								if( Valid )
								{
									uint16 NewWallType( 0 );
									if( m_pTemplate.TileTypes() > 1 )
									{
										NewWallType = 1;
									}
									int32 TerrainType( m_pTemplate.TileGroundLayer( 1 ) );


									SetTerrainType( X, Y, TerrainType );
									SetFloorHeight( X, Y, 0 );
									SetFloorHeight( X - 1, Y, 0 );
									SetFloorHeight( X + 1, Y, 0 );
									SetFloorHeight( X, Y - 1, 0 );
									SetFloorHeight( X, Y + 1, 0 );
									SetFloorHeight( X, Y, 0 );
									SetFloorHeight( X - 1, Y - 1, 0 );
									SetFloorHeight( X + 1, Y - 1, 0 );
									SetFloorHeight( X - 1, Y + 1, 0 );
									SetFloorHeight( X + 1, Y + 1, 0 );
									SetFloorHeight( X + 2, Y, 0 );
									SetFloorHeight( X + 2, Y - 1, 0 );
									SetFloorHeight( X + 2, Y + 1, 0 );
									SetFloorHeight( X - 1, Y + 2, 0 );
									SetFloorHeight( X, Y + 2, 0 );
									SetFloorHeight( X + 1, Y + 2, 0 );
									SetFloorHeight( X + 2, Y + 2, 0 );

									SetWallType( X, Y, NewWallType );
									SetWallType( X - 1, Y, NewWallType );
									SetWallType( X + 1, Y, NewWallType );
									SetWallType( X, Y - 1, NewWallType );
									SetWallType( X, Y + 1, NewWallType );
									SetWallType( X, Y, NewWallType );
									SetWallType( X - 1, Y - 1, NewWallType );
									SetWallType( X + 1, Y - 1, NewWallType );
									SetWallType( X - 1, Y + 1, NewWallType );
									SetWallType( X + 1, Y + 1, NewWallType );
									
									SetOffset( X, Y, 0, 0 );
									SetOffset( X - 1, Y, 0, 0 );
									SetOffset( X + 1, Y, 0, 0 );
									SetOffset( X, Y - 1, 0, 0 );
									SetOffset( X, Y + 1, 0, 0 );
									SetOffset( X, Y, 0, 0 );
									SetOffset( X - 1, Y - 1, 0, 0 );
									SetOffset( X + 1, Y - 1, 0, 0 );
									SetOffset( X - 1, Y + 1, 0, 0 );
									SetOffset( X + 1, Y + 1, 0, 0 );
									for( int32 x = X - 2; x <= X + 2; x++ )
									{
										for( int32 y = Y - 2; y <= Y + 2; y++ )
										{
											SetCellHeightProtected( x, y, 0 );
											if( CellType( x, y ) != KCTypeStairwellDown &&
												CellType( x, y ) != KCTypeStairwellUp )
											{
												SetCellType( x, y, KCTypeRoom );
											}
										}
									}
									SetCellType( X, Y, KCTypeStairwellDown );
									SetCellHeightProtected( X, Y, -1 );

									SetHasWater( X - 1, Y, kFalse );
									SetHasWater( X + 1, Y, kFalse );
									SetHasWater( X, Y - 1, kFalse );
									SetHasWater( X, Y + 1, kFalse );
									SetHasWater( X - 1, Y - 1, kFalse );
									SetHasWater( X + 1, Y + 1, kFalse );
									SetHasWater( X - 1, Y + 1, kFalse );
									SetHasWater( X + 1, Y - 1, kFalse );
/*									SetCellType( X - 1, Y, KCTypeRoom );
									SetCellType( X + 1, Y, KCTypeRoom );
									SetCellType( X, Y - 1, KCTypeRoom );
									SetCellType( X, Y + 1, KCTypeRoom );
									SetCellType( X - 2, Y, KCTypeRoom );
									SetCellType( X + 2, Y, KCTypeRoom );
									SetCellType( X, Y - 2, KCTypeRoom );
									SetCellType( X, Y + 2, KCTypeRoom );
									SetCellType( X - 1, Y - 1, KCTypeRoom );
									SetCellType( X + 1, Y + 1, KCTypeRoom );
									SetCellType( X - 1, Y + 1, KCTypeRoom );
									SetCellType( X + 1, Y - 1, KCTypeRoom );*/
		

									CPropTemplate* pRandomProp = m_pTemplate.RandomProp( PropLocation );
									if( pRandomProp != NULL )
									{
										GeneratedExitDown = kTrue;
										m_pProps.push_back( new CPropInstance( *pRandomProp,
																				X,
																				Y,
																				D3DXVECTOR3( 0, 0, 0 ) ) );
									}
								}
							}
						}
					}
				}
			}
		}
		Tries++;
		if( !FoundRoomWall )
		{
			break;
		}
		if( Tries > 10000 )
		{
			FoundRoomWall = kFalse;
			break;
		}
	}

	bool LastDitchEffort( kFalse );
	int32 RoomWalls( 0 );
	if( !FoundRoomWall )
	{
		while( !GeneratedExitUp || !GeneratedExitDown )
		{
			RoomWalls = 0;
			for( int32 X = 1; X < m_MazeWidth - 1; X++ )
			{
				for( int32 Y = 1; Y < m_MazeHeight - 1; Y++ )
				{
					ECellType Location = CellType( X, Y );
					if( Location == KCTypeRoomWall ||
						LastDitchEffort )
					{
						int32 Chance = RandomIntegerBetween( 0, 1000 );
						if( X > 1 && Y > 1 && 
							X < m_MazeWidth - 2 && Y < m_MazeHeight - 2 )
						{
							RoomWalls++;
							if( Chance < 10 )
							{
								if( !GeneratedExitUp )
								{
									bool Valid( kFalse );
									EPropLocation PropLocation( KPropNStairsUp );
									if( CellOpen( X, Y + 1 ) &&
										!CellOpen( X - 1, Y ) &&
										!CellOpen( X + 1, Y ) &&
										!CellOpen( X, Y - 1 ) )
									{
										PropLocation = KPropNStairsUp;
										Valid = kTrue;
									}
									else if( CellOpen( X, Y - 1 ) &&
											!CellOpen( X - 1, Y ) &&
											!CellOpen( X + 1, Y ) &&
											!CellOpen( X, Y + 1 ) )
									{
										PropLocation = KPropSStairsUp;
										Valid = kTrue;
									}
									else if( CellOpen( X + 1, Y ) &&
											!CellOpen( X - 1, Y ) &&
											!CellOpen( X, Y - 1 ) &&
											!CellOpen( X, Y + 1 ) )
									{
										PropLocation = KPropWStairsUp;
										Valid = kTrue;
									}
									else /*if( CellOpen( X - 1, Y ) &&
											!CellOpen( X + 1, Y ) &&
											!CellOpen( X, Y - 1 ) &&
											!CellOpen( X, Y + 1 ) )*/
									{
										PropLocation = KPropEStairsUp;
										Valid = kTrue;
									}
									if( Valid )
									{
										SetCellHeightProtected( X, Y, 0 );
										SetCellType( X, Y, KCTypeStairwellUp );
										SetFloorHeight( X, Y, 0 );
										SetFloorHeight( X - 1, Y, 0 );
										SetFloorHeight( X + 1, Y, 0 );
										SetFloorHeight( X, Y - 1, 0 );
										SetFloorHeight( X, Y + 1, 0 );

										CPropTemplate* pRandomProp = m_pTemplate.RandomProp( PropLocation );
										if( pRandomProp != NULL )
										{
											GeneratedExitUp = kTrue;
											m_pProps.push_back( new CPropInstance( *pRandomProp,
																					X,
																					Y,
																					D3DXVECTOR3( 0, 0, 0 ) ) );
										}
									}
								}
							}
							else if( Chance < 20 )
							{
								if( !GeneratedExitDown )
								{
									bool Valid( kFalse );
									EPropLocation PropLocation = KPropNStairsDown;	
									if( X > 2 && Y > 2 && 
										X < m_MazeWidth - 2 && Y < m_MazeHeight - 2 &&
										CellType( X - 1, Y - 1 ) != KCTypeStairwellUp &&
										CellType( X + 1, Y - 1 ) != KCTypeStairwellUp &&
										CellType( X - 1, Y + 1 ) != KCTypeStairwellUp &&
										CellType( X + 1, Y + 1 ) != KCTypeStairwellUp &&
										CellType( X, Y - 1 ) != KCTypeStairwellUp &&
										CellType( X, Y + 1 ) != KCTypeStairwellUp &&
										CellType( X - 1, Y ) != KCTypeStairwellUp &&
										CellType( X + 1, Y ) != KCTypeStairwellUp )
									{
										if( CellOpen( X, Y + 1 ) &&
											//!CellOpen( X - 1, Y ) &&
											//!CellOpen( X + 1, Y ) &&
											!CellOpen( X, Y - 1 ) )
										{
											PropLocation = KPropNStairsDown;
											Valid = kTrue;
										}
										else if( CellOpen( X, Y - 1 ) &&
											//!CellOpen( X - 1, Y ) &&
											//!CellOpen( X + 1, Y ) &&
											!CellOpen( X, Y + 1 ) )
										{
											PropLocation = KPropSStairsDown;
											Valid = kTrue;
										}
										else if( CellOpen( X + 1, Y ) &&
												//!CellOpen( X - 1, Y ) &&
												//!CellOpen( X, Y - 1 ) &&
												!CellOpen( X, Y + 1 ) )
										{
											PropLocation = KPropWStairsDown;
											Valid = kTrue;
										}
										else /*if( CellOpen( X - 1, Y ) &&
												//!CellOpen( X + 1, Y ) &&
												//!CellOpen( X, Y - 1 ) &&
												!CellOpen( X, Y + 1 ) )*/
										{
											PropLocation = KPropEStairsDown;
											Valid = kTrue;
										}
									}
									if( Valid )
									{
										SetFloorHeight( X, Y, 0 );
										SetFloorHeight( X - 1, Y, 0 );
										SetFloorHeight( X + 1, Y, 0 );
										SetFloorHeight( X, Y - 1, 0 );
										SetFloorHeight( X, Y + 1, 0 );
										SetFloorHeight( X, Y, 0 );
										SetFloorHeight( X - 1, Y - 1, 0 );
										SetFloorHeight( X + 1, Y - 1, 0 );
										SetFloorHeight( X - 1, Y + 1, 0 );
										SetFloorHeight( X + 1, Y + 1, 0 );
										for( int32 x = X - 2; x <= X + 2; x++ )
										{
											for( int32 y = Y - 2; y <= Y + 2; y++ )
											{
												SetCellHeightProtected( x, y, 0 );
												if( CellType( x, y ) != KCTypeStairwellDown &&
													CellType( x, y ) != KCTypeStairwellUp )
												{
													SetCellType( x, y, KCTypeRoom );
												}
											}
										}
										SetCellType( X, Y, KCTypeStairwellDown );
										SetCellHeightProtected( X, Y, -1 );
										SetHasWater( X - 1, Y, kFalse );
										SetHasWater( X + 1, Y, kFalse );
										SetHasWater( X, Y - 1, kFalse );
										SetHasWater( X, Y + 1, kFalse );
										SetHasWater( X - 1, Y - 1, kFalse );
										SetHasWater( X + 1, Y + 1, kFalse );
										SetHasWater( X - 1, Y + 1, kFalse );
										SetHasWater( X + 1, Y - 1, kFalse );
										/*SetCellType( X - 1, Y, KCTypeRoom );
										SetCellType( X + 1, Y, KCTypeRoom );
										SetCellType( X, Y - 1, KCTypeRoom );
										SetCellType( X, Y + 1, KCTypeRoom );
										SetCellType( X - 2, Y, KCTypeRoom );
										SetCellType( X + 2, Y, KCTypeRoom );
										SetCellType( X, Y - 2, KCTypeRoom );
										SetCellType( X, Y + 2, KCTypeRoom );
										SetCellType( X - 1, Y - 1, KCTypeRoom );
										SetCellType( X + 1, Y + 1, KCTypeRoom );
										SetCellType( X - 1, Y + 1, KCTypeRoom );
										SetCellType( X + 1, Y - 1, KCTypeRoom );*/

										CPropTemplate* pRandomProp = m_pTemplate.RandomProp( PropLocation );
										if( pRandomProp != NULL )
										{
											GeneratedExitDown = kTrue;
											m_pProps.push_back( new CPropInstance( *pRandomProp,
																					X,
																					Y,
																					D3DXVECTOR3( 0, 0, 0 ) ) );
										}
									}
								}
							}
						}
					}
				}
			}
			if( RoomWalls <= 1 )
			{
				LastDitchEffort = kTrue;
			}

		}
	}

} // CMaze::GenerateExits()


void CMaze::SetWallBorders( void )
{

	//for( int32 i = m_pTemplate.TileTypes() - 1; i >= 0; i-- )
	for( int32 i = 0; i < (int32)m_pTemplate.TileTypes(); i++ )
	{
		for( int32 X = 0; X < m_MazeWidth; X++ )
		{
			for( int32 Y = 0; Y < m_MazeHeight; Y++ )
			{
				if( IsCliff( X, Y ) &&
					WallType( X, Y ) == i )
				{
					int32 CurrentHeight( CellHeight( X, Y ) );
					if( CurrentHeight <= 0 )
					{
						SetWallBorder( X, Y, (uint16)m_pTemplate.TileGroundLayer( i ) );
					}
					else
					{
					//	SetTerrainType( X, Y, (uint16)m_pTemplate.TileGroundLayer( i ) );
					}
				}
			}
		}
	}
} // CMaze::SetWallBorders()

void CMaze::SetWallBorder( int32 X,			// x cell
						   int32 Y,			// y cell
						   uint16 TerrainType )	// terrain type
{
	int32 CurrentHeight( CellHeight( X, Y ) );

	SetTerrainType( X, Y, TerrainType );
	SetTerrainType( X - 1, Y, TerrainType );
	SetTerrainType( X + 1, Y, TerrainType );
	SetTerrainType( X, Y - 1, TerrainType );
	SetTerrainType( X, Y + 1, TerrainType );
	/*			
	if( X > 0 )
	{
		if( CellHeight( X - 1, Y ) <= CurrentHeight )
		{
			SetTerrainType( X - 1, Y, TerrainType );
		}
		if( Y > 0 )
		{
			if( CellHeight( X - 1, Y - 1 ) <= CurrentHeight )
			{
				SetTerrainType( X - 1, Y - 1, TerrainType );
			}
		}
		if( Y < m_MazeHeight - 1 )
		{
			if( CellHeight( X - 1, Y + 1 ) <= CurrentHeight )
			{
				SetTerrainType( X - 1, Y + 1, TerrainType );
			}
		}
	}
	if( X < m_MazeWidth - 1 )
	{
		if( CellHeight( X + 1, Y ) <= CurrentHeight )
		{
			SetTerrainType( X + 1, Y, TerrainType );
		}
		if( Y > 0 )
		{
			if( CellHeight( X + 1, Y - 1 ) <= CurrentHeight )
			{
				SetTerrainType( X + 1, Y - 1, TerrainType );
			}
		}
		if( Y < m_MazeHeight - 1 )
		{
			if( CellHeight( X + 1, Y + 1 ) <= CurrentHeight )
			{
				SetTerrainType( X + 1, Y + 1, TerrainType );
			}
		}
	}
	if( Y > 0 )
	{
		if( CellHeight( X, Y - 1 ) <= CurrentHeight )
		{
			SetTerrainType( X, Y - 1, TerrainType );
		}
	}
	if( Y < m_MazeHeight - 1 )
	{
		if( CellHeight( X, Y + 1 ) <= CurrentHeight )
		{
			SetTerrainType( X, Y + 1, TerrainType );
		}
	}*/
} // CMaze::SetWallBorder()

void CMaze::GenerateCorridors( void )
{
	int32 X( 0 );
	int32 Y( 0 );
	// choose a random location to start from
	SelectRandomUnvisitedCell( X, Y );
	
	// mark it as visited
	VisitCell( X, Y );
	
	// open up the cell
	SetCellHeight( X, Y, 0 );
	SetCellType( X, Y, KCTypeHall );

	// visit the cells in the four cardinal directions
	VisitCell( X, Y - 1 );
	VisitCell( X, Y + 1 );
	VisitCell( X - 1, Y );
	VisitCell( X + 1, Y );

	int32 Direction = rand() % 4;
	if( Direction > 3)
	{
		Direction = 3;
	}

	// while there are still unvisited cells,
	// generate a load of corridors
	int32 OptOutCount( 0 );
	while( (int32)m_CellsVisited < m_MazeWidth * m_MazeHeight - 4 &&
		OptOutCount < 10000 )
	{
		if( (int32)m_CellsVisited > m_MazeWidth * m_MazeHeight - 20 )
		{
			OptOutCount++;
		}
		int32 CellX( 0 );
		int32 CellY( 0 );
		// we change direction based upon twistiness 
		if( rand() % 1000 < m_Twistiness )
		{
			Direction = rand() % 4;
			if( Direction > 3 )
			{
				Direction = 3;
			}
		}
		bool Done( kFalse );
		bool DirectionTried[KDirections];
		for( int32 i = 0; i < KDirections; i++ )
		{
			DirectionTried[i] = kFalse;
		}
		
		while( !Done )
		{
			// if we've already tried all directions,
			// just pick a spot in an existing corridor
			// to start from again.
			if( DirectionTried[KNorth] && 
				DirectionTried[KSouth] &&
				DirectionTried[KEast] &&
				DirectionTried[KWest] )
			{
				SelectRandomCell( CellX, CellY, 0 );
				break;
			}

			// otherwise, start plowing out the corridor
			CellX = X;
			CellY = Y;
			if( DirectionTried[Direction] )
			{
				// pick a new direction
				Direction = rand() % 4;
				if( Direction > 3)
				{
					Direction=3;
				}
			}
			else
			{
				DirectionTried[Direction] = kTrue;
				// advance in the chosen direction
				switch( Direction )
				{
				case KNorth:
					CellY--;
					break;
				case KEast:
					CellX++;
					break;
				case KSouth:
					CellY++;
					break;
				case KWest:
					CellX--;
					break;
				}
			}

			Done = kTrue;

			// make sure the cell is in the bounds of the maze
			if( !ValidCell( CellX, CellY ) )
			{
				Done = kFalse;
			}
			else
			{
				// if there's already corridor there, we're Done
				if( CellHeight( CellX, CellY ) == 0 )
				{
					Done = kFalse;
				}
				if( Done && !( CellX-1 == X && CellY == Y ) &&
					CellHeight( CellX-1, CellY ) == 0 )
				{
					Done = kFalse;
				}
				if( Done && !( CellX+1 == X && CellY == Y ) &&
					CellHeight( CellX+1, CellY ) == 0 )
				{
					Done = kFalse;
				}
				if( Done && !( CellX == X && CellY-1 == Y ) &&
					CellHeight( CellX, CellY-1 ) == 0 )
				{
					Done = kFalse;
				}
				if( Done && !( CellX == X && CellY+1 == Y ) &&
					CellHeight( CellX, CellY+1 ) == 0 )
				{
					Done = kFalse;
				}	
			}
		}

		// this was a successful 'dig' - advance our current location
		X = CellX;
		Y = CellY;

		// open the cell
		SetCellHeight( CellX, CellY, 0 );
		SetCellType( CellX, CellY, KCTypeHall );

		// visit the surrounding cells
		VisitCell( CellX, CellY );
		VisitCell( CellX, CellY - 1 );
		VisitCell( CellX, CellY + 1 );
		VisitCell( CellX - 1, CellY );
		VisitCell( CellX + 1, CellY );
		VisitCell( CellX - 1, CellY - 1 );
		VisitCell( CellX - 1, CellY + 1 );
		VisitCell( CellX + 1, CellY - 1 );
		VisitCell( CellX + 1, CellY + 1 );
	}

} // CMaze::GenerateCorridors()

void CMaze::Sparsify( void )
{
	int32 OpenCells( 0 );
	for( int32 X = 0; X < m_MazeWidth; X++ )
	{
		for( int32 Y = 0; Y < m_MazeHeight; Y++ )
		{
			if( CellHeight( X, Y ) == 0 )
			{
				OpenCells ++;
			}
		}
	}
	// sparsify the map
	for( int32 i = 0; i < m_Sparseness; i++ )
	{
		for( int32 X = 0; X < m_MazeWidth; X++ )
		{
			for( int32 Y = 0; Y < m_MazeHeight; Y++ )
			{
				
				int32 Exits( CountExits( X, Y ) );

				
				// if this square has only one exit, fill it back in.
				if( Exits == 1 &&
					CellType( X, Y ) != KCTypeRoom &&
					OpenCells > 100 )
				{
					OpenCells--;
					SetCellHeight( X, Y, 1 );
					SetCellType( X, Y, KCTypeSolid );
				}
			}
		}
	}

} // CMaze::Sparsify()

void CMaze::PlugHoles( void )
{
	for( int32 X = 0; X < m_MazeWidth; X++ )
	{
		for( int32 Y = 0; Y < m_MazeHeight; Y++ )
		{
			
			int32 Exits( CountExits( X, Y ) );

			if( CellHeight( X, Y ) == 0 &&
				CellHeight( X - 1, Y ) < 0 &&
				CellHeight( X + 1, Y ) < 0 &&
				CellHeight( X , Y - 1 ) < 0 &&
				CellHeight( X , Y + 1 ) < 0 )
			{
				SetCellHeight( X, Y, -1 );
				SetCellType( X, Y, KCTypePit );
				SetHasWater( X, Y, m_pTemplate.HasWater() );
				SetHasWater( X - 1, Y, m_pTemplate.HasWater() );
				SetHasWater( X + 1, Y, m_pTemplate.HasWater() );
				SetHasWater( X, Y - 1, m_pTemplate.HasWater() );
				SetHasWater( X, Y + 1, m_pTemplate.HasWater() );
				SetHasWater( X - 1, Y - 1, m_pTemplate.HasWater() );
				SetHasWater( X + 1, Y - 1, m_pTemplate.HasWater() );
				SetHasWater( X - 1, Y + 1, m_pTemplate.HasWater() );
				SetHasWater( X + 1, Y + 1, m_pTemplate.HasWater() );
			}
				
			// if this square has only one exit, fill it back in.
			if( Exits == 0 &&
				CellHeight( X, Y ) == 0 )
			{
				SetCellHeight( X, Y, 1 );
				SetCellType( X, Y, KCTypeSolid );
			}
			else if( Exits == 1 &&
					 CellHeight( X, Y ) == 0 &&
					 CellHeight( X - 1, Y ) > 0 &&
					 CellHeight( X + 1, Y ) > 0 &&
					 CellHeight( X , Y - 1 ) > 0 &&
					 CellHeight( X , Y + 1 ) > 0 )
			{
				SetCellHeight( X, Y, 1 );
				SetCellType( X, Y, KCTypeSolid );
			}

		}
	}

} // CMaze::PlughHoles()

void CMaze::RaiseRoof( void )
{
	for( int32 X = 0; X < m_MazeWidth; X++ )
	{
		for( int32 Y = 0; Y < m_MazeHeight; Y++ )
		{
			if( CellHeight( X, Y ) == 1 &&
				CellHeight( X - 1, Y ) >= 1 &&
				CellHeight( X + 1, Y ) >= 1 &&
				CellHeight( X , Y - 1 ) >= 1 &&
				CellHeight( X , Y + 1 ) >= 1 &&
				CellHeight( X - 1, Y - 1 ) >= 1 &&
				CellHeight( X + 1, Y - 1 ) >= 1 &&
				CellHeight( X - 1, Y + 1 ) >= 1 &&
				CellHeight( X + 1, Y + 1 ) >= 1 )
			{
				SetCellHeight( X, Y, 2 );
			}		
			else if( CellHeight( X, Y ) == -1 &&
					CellHeight( X - 1, Y ) <= -1 &&
					CellHeight( X + 1, Y ) <= -1 &&
					CellHeight( X , Y - 1 ) <= -1 &&
					CellHeight( X , Y + 1 ) <= -1 &&
					CellHeight( X - 1, Y - 1 ) <= -1 &&
					CellHeight( X + 1, Y - 1 ) <= -1 &&
					CellHeight( X - 1, Y + 1 ) <= -1 &&
					CellHeight( X + 1, Y + 1 ) <= -1 )
			{
				SetCellHeight( X, Y, -2 );
			}		
		}
	}

} // CMaze::RaiseRoof()

void CMaze::Roundify( void )
{
	// round off edges of the map
	for( int32 X = 0; X < m_MazeWidth; X++ )
	{
		for( int32 Y = 0; Y < m_MazeHeight; Y++ )
		{
			int16 Height;
			if( IsCorner( X, Y, Height ) &&
				rand() % 1000 < m_Roundedness )
			{
				SetCellHeight( X, Y, Height );
				SetCellType( X, Y, KCTypeSolid );
			}
		}
	}

} // CMaze::Roundify()

// finds any dead end corridors, and continues them on until they intersect
// with an existing corridor
void CMaze::ConnectDeadEnds( void )
{
	int32 Direction = rand() % 4;
	if( Direction > 3)
	{
		Direction = 3;
	}

	// connect dead ends
	for( int32 X = 1; X < m_MazeWidth - 1; X++ )
	{
		for( int32 Y = 1; Y < m_MazeHeight - 1; Y++ )
		{
			int32 Exits( CountExits( X, Y ) );
			
			if( Exits == 1 && 
				CellType( X, Y ) == KCTypeHall &&
				rand() % 1000 < m_Connectedness )
			{
				int32 CellX( X );
				int32 CellY( Y );
				bool Connected( kFalse );
				while( !Connected )
				{

					if( rand()%1000 < m_Twistiness )
					{
						// pick a new direction
						Direction = rand() % 4;
						if( Direction > 3)
						{
							Direction=3;
						}
					}
					bool Done( kFalse );
					while( !Done )
					{		
						int32 LastX( CellX );
						int32 LastY( CellY );
						// advance in the selected direction
						switch( Direction )
						{
						case KNorth:
							CellY--;
							break;
						case KEast:
							CellX++;
							break;
						case KSouth:
							CellY++;
							break;
						case KWest:
							CellX--;
							break;
						}
						
						Done = kTrue;

						// since this doesn't care about previously visited cells,
						// we need to keep our directions inbounds of the maze
						if ( CellX < 1 || 
							 CellY < 1 || 
							 CellX >= m_MazeWidth - 1 || 
							 CellY >= m_MazeHeight - 1 )
						{
							// pick a new direction
							Direction = rand() % 4;
							if( Direction > 3)
							{
								Direction=3;
							}

							CellX = LastX;
							CellY = LastY;
							Done = kFalse;						
						}
						
					}
					
					if( CellHeight( CellX, CellY ) == 0 )
					{
						Connected = kTrue;
					}
					// open the cell
					if( CellHeight( CellX, CellY ) != 0 )
					{
						SetCellHeight( CellX, CellY, 0 );
						SetCellType( CellX, CellY, KCTypeHall );
					}
				}

			}
		}
	}
} // CMaze::ConnectDeadEnds()

// count the exits for each room
void CMaze::CountRoomExits( void )
{

	for( int32 i = 0; i < (int32)Rooms(); i++ )
	{
		int32 Exits( 0 );
		for( int32 Y = 0; Y < m_MazeHeight; Y++ )
		{
			for( int32 X = 0; X < m_MazeWidth; X++ )
			{
				if( CellHeight( X, Y ) == 0 &&
					RoomIndexData( X, Y ) == i )
				{
					if( CellHeight( X, Y - 1 ) == 0 &&
						( CellType( X, Y - 1 ) != KCTypeRoom ||
						  RoomIndexData( X, Y - 1 ) != i ) )
					{
						Exits++;
					}
					if( CellHeight( X, Y + 1 ) == 0 &&
						( CellType( X, Y + 1 ) != KCTypeRoom ||
						  RoomIndexData( X, Y + 1 ) != i ) )
					{
						Exits++;
					}
					if( CellHeight( X - 1, Y ) == 0 &&
						( CellType( X - 1, Y ) != KCTypeRoom ||
						  RoomIndexData( X - 1, Y ) != i ) )
					{
						Exits++;
					}
					if( CellHeight( X + 1, Y ) == 0 &&
						( CellType( X + 1, Y ) != KCTypeRoom ||
						  RoomIndexData( X + 1, Y ) != i ) )
					{
						Exits++;
					}
				}
			}
		}
		m_RoomExits[i] = Exits;
	}
} // CMaze::CountRoomExits()

// finds any isolated rooms and connect them to an existing corridor
void CMaze::ConnectIsolatedRooms( void )
{
	int32 Direction = rand() % 4;
	if( Direction > 3)
	{
		Direction = 3;
	}

	// connect dead ends
	for( uint32 i = 0; i < m_RoomIsolated.size(); i++ )
	{
		if( m_RoomIsolated[i] )
		{
			// we have a percent chance of just making
			// a straight shot to a room we know is good
			int32 ConnectingRoom( -1 );
			for( uint32 j = 0; j < m_RoomIsolated.size(); j++ )
			{
				if( j != i &&
					!m_RoomIsolated[j] )
				{
					if( RandomIntegerBetween( 0, 1000 ) < 100 )
					{
						ConnectingRoom = j;
					}
				}
			}
			// otherwise, we weave around until we find it

			if( ConnectingRoom != -1 )
			{
				int32 CellX( m_RoomX[i] );
				int32 CellY( m_RoomY[i] );
				bool Connected( kFalse );
				while( !Connected )
				{
					// x axis first
					if( CellX < m_RoomX[ConnectingRoom] )
					{
						CellX++;
					}
					else if( CellX > m_RoomX[ConnectingRoom] )
					{
						CellX--;
					}
					else
					{
						// y axis second
						if( CellY < m_RoomY[ConnectingRoom] )
						{
							CellY++;
						}
						else if( CellY > m_RoomY[ConnectingRoom] )
						{
							CellY--;
						}
					}

					if( CellHeight( CellX, CellY ) == 0 &&
						RoomIndexData( CellX, CellY ) != i &&
						( CellType( CellX, CellY ) == KCTypeHall ||
						  RoomIndexData( CellX, CellY ) >= 0 ) )
					{
						Connected = kTrue;
						if( RoomIndexData( CellX, CellY ) != -1 &&
							m_RoomIsolated[RoomIndexData( CellX, CellY )] )
						{
							Connected = kFalse;
						}
						
					}
					if( CellX == m_RoomX[ConnectingRoom] &&
							 CellY == m_RoomY[ConnectingRoom] )
					{
						Connected = kTrue;
					}
					if( Connected )
					{
						m_RoomIsolated[i] = kFalse;
					}
					// open the cell

					if( CellHeight( CellX, CellY ) != 0 &&
						( CellType( CellX, CellY ) == KCTypeSolid ||
						  CellType( CellX, CellY ) == KCTypeRoomWall ) )
					{
						SetCellType( CellX, CellY, KCTypeHall );
					}

					// open the cell
					SetCellHeight( CellX, CellY, 0 );

				}
			}
			else
			{
				int32 CellX( m_RoomX[i] );
				int32 CellY( m_RoomY[i] );
				int32 TargetX;
				int32 TargetY;
				// select a hallway to connect to
				SelectRandomCell( TargetX, TargetY, KCTypeHall );
				bool Connected( kFalse );
				while( !Connected )
				{
					// x axis first
					if( CellX < TargetX )
					{
						CellX++;
					}
					else if( CellX > TargetX )
					{
						CellX--;
					}
					else
					{
						// y axis second
						if( CellY < TargetY )
						{
							CellY++;
						}
						else if( CellY > TargetY )
						{
							CellY--;
						}
					}

					if( CellHeight( CellX, CellY ) == 0 &&
						RoomIndexData( CellX, CellY ) != i &&
						( CellType( CellX, CellY ) == KCTypeHall ||
						  RoomIndexData( CellX, CellY ) >= 0 ) )
					{
						Connected = kTrue;
						if( RoomIndexData( CellX, CellY ) != -1 &&
							m_RoomIsolated[RoomIndexData( CellX, CellY )] )
						{
							Connected = kFalse;
						}
						
					}
					if( CellX == TargetX &&
							 CellY == TargetY )
					{
						Connected = kTrue;
					}
					if( Connected )
					{
						m_RoomIsolated[i] = kFalse;
					}

					if( CellHeight( CellX, CellY ) != 0 &&
						( CellType( CellX, CellY ) == KCTypeSolid ||
						  CellType( CellX, CellY ) == KCTypeRoomWall ) )
					{
						SetCellType( CellX, CellY, KCTypeHall );
					}

					// open the cell
					SetCellHeight( CellX, CellY, 0 );
				}

			}

		}
	}

} // CMaze::ConnectIsolatedRooms()

void CMaze::VerifyWalls( void )
{
	for( int32 X = 0; X < m_MazeWidth; X++ )
	{
		for( int32 Y = 0; Y < m_MazeHeight; Y++ )
		{
			if( CellHeight( X, Y ) == 0 &&
				CellType( X, Y ) == KCTypeRoomWall )
			{
				SetCellType( X, Y, KCTypeRoom );
			}
		}
	}
} // CMaze::VerifyWalls

void CMaze::GenerateDoors( void )
{
	for( int32 X = 0; X < m_MazeWidth; X++ )
	{
		for( int32 Y = 0; Y < m_MazeHeight; Y++ )
		{
			if( CellHeight( X, Y ) == 0 )
			{
				if( CellHeight( X - 1, Y ) == 0 &&
					CellHeight( X + 1, Y ) == 0 &&
					CellHeight( X, Y - 1 ) > 0 &&
					CellHeight( X, Y + 1 ) > 0 )
				{
					if( CellType( X - 1, Y ) == KCTypeRoom ||
						CellType( X + 1, Y ) == KCTypeRoom )
					{
						if( CellType( X - 1, Y ) != KCTypeEWDoor &&
							CellType( X + 1, Y ) != KCTypeEWDoor )
						{
							SetCellType( X, Y, KCTypeEWDoor );
						}
					}
				}
				else if( CellHeight( X, Y - 1 ) == 0 &&
						 CellHeight( X, Y + 1 ) == 0 &&
						 CellHeight( X - 1, Y ) > 0 &&
						 CellHeight( X + 1, Y ) > 0 )
				{
					if( CellType( X , Y - 1 ) == KCTypeRoom ||
						CellType( X , Y + 1) == KCTypeRoom )
					{
						if( CellType( X , Y - 1 ) != KCTypeNSDoor &&
							CellType( X , Y + 1) != KCTypeNSDoor )
						{
							SetCellType( X, Y, KCTypeNSDoor );
						}
					}
				}

			}
		}
	}

} // CMaze::GenerateDoors( )

// generate rooms
void CMaze::GenerateRooms( int32 Rooms )	// rooms to generate
{
	for( int32 i = 0 ; i < Rooms; i++ )
	{
		int32 BestX( 1 );
		int32 BestY( 1 );
		int32 BestScore = 9999999;
		// select random room dimensions within bounds

		int32 Width = ( rand()% ( m_MaxRoomWidth - m_MinRoomWidth ) ) + 
					   m_MinRoomWidth ;
		int32 Height = ( rand()% ( m_MaxRoomHeight - m_MinRoomHeight ) ) + 
						m_MinRoomHeight;
		int32 RandomRoom( RandomIntegerBetween( 0, 1000 ) );
		int32 RoomType( 0 );
		if( RandomRoom < 500 )
		{
			RoomType = 0;
		}
		else if( RandomRoom < 750 )
		{
			RoomType = 1;
		}
		else
		{
			RoomType = 2;
			if( Width < 5 )
			{
				Width = 5;
			}
			if( Height < 5 )
			{
				Height = 5;
			}
		}
		
		// go through the map checking all possible locations for the room
		for( int32 X = 1; X < ( (int32)m_MazeWidth - 1 - Width ); X++ )
		{
			for( int32 Y = 1; Y < ( (int32)m_MazeHeight - 1 - Height ); Y++ ) 
			{
				int32 Score( CalculateScoreForRoom( X, Y, Width, Height ) );

				// if it's a smaller score, then it's a better room - 
				// we take its coordinates and keep them
				if( Score < BestScore && Score != -999 )
				{
					BestX = X;
					BestY = Y;
					BestScore = Score;
				}
			}
		}
		if( RoomType == 0 )
		{
			GenerateRoom( BestX, BestY, Width, Height );
		}
		else if( RoomType == 1 )
		{
			GenerateOvalRoom( BestX, BestY, Width, Height );
		}
		else
		{
			GenerateRingRoom( BestX, BestY, Width, Height );
		}
	}
} // CMaze::GenerateRooms()

void CMaze::GenerateRoom( int32 BestX,		// top left x
						  int32 BestY,		// top left y
						  int32 Width,		// room width
						  int32 Height )	// room height
{
	bool Isolated( kTrue );
	int32 RoomCenterX = BestX + Width / 2;
	int32 RoomCenterY = BestY + Height / 2;

	bool Crenellated( RandomIntegerBetween( 0, 100 ) < 70 );

	int32 NewWallType( 0 );//rand() % m_pTemplate.TileTypes() );

		if( NewWallType >= (int32)m_pTemplate.TileTypes() )
		{
			NewWallType = 0;
		}

		for( int32 RoomX = BestX - 1; RoomX < BestX + Width + 2; RoomX++ )
		{
			for( int32 RoomY = BestY - 1; RoomY < BestY + Height + 2; RoomY++ )
			{
				if( RoomX >= BestX && RoomX < BestX + Width &&
					RoomY >= BestY && RoomY < BestY + Height )
				{

					if( CellType( RoomX, RoomY ) == KCTypeHall )
					{
						Isolated = kFalse;
					}
				}
				if( CellType( RoomX, RoomY ) == KCTypeRoom )
				{
					NewWallType = WallType( RoomX, RoomY );
					break;
				}
			}
		}
		for( int32 RoomX = BestX - 1; RoomX < BestX + Width + 2; RoomX++ )
		{
			for( int32 RoomY = BestY - 1; RoomY < BestY + Height + 2; RoomY++ )
			{
				if( CellType( RoomX, RoomY ) != KCTypeRoom )
				{
					SetWallType( RoomX, RoomY, (uint16)NewWallType );
				}
			}
		}

		for( int32 RoomX = BestX - 1; RoomX < BestX + Width + 2; RoomX++ )
		{
			for( int32 RoomY = BestY - 1; RoomY < BestY + Height + 2; RoomY++ )
			{
				if( RoomX == BestX - 1 || RoomX == BestX + Width + 1 ||
					RoomY == BestY - 1 || RoomY == BestY + Height + 1 )
				{
					if( CellType( RoomX, RoomY ) != KCTypeRoom &&
						CellHeight( RoomX, RoomY ) > 0 )
					{
						SetCellType( RoomX, RoomY, KCTypeRoomWall );
					}

				}
			}
		}

		// now we have the best coordinates
		// so we carve out the space
		for( int32 RoomX = BestX; RoomX < BestX + Width; RoomX++ )
		{
			for( int32 RoomY = BestY; RoomY < BestY + Height; RoomY++ )
			{

				SetCellHeight( RoomX, RoomY, 0 );
				SetCellType( RoomX, RoomY, KCTypeRoom );

				if( Crenellated )
				{
					if( RoomY == BestY || RoomY == BestY + Height - 1 ||
						RoomX == BestX || RoomX == BestX + Width - 1 )
					{
						if( CellType( RoomX - 1, RoomY ) != KCTypeHall &&
							CellType( RoomX + 1, RoomY ) != KCTypeHall &&						
							CellType( RoomX, RoomY - 1 ) != KCTypeHall &&						
							CellType( RoomX, RoomY + 1 ) != KCTypeHall &&
							( RoomIndexData( RoomX, RoomY ) < 0 || RoomIndexData( RoomX, RoomY ) == m_RoomIsolated.size() ) )
						{
							if( ( RoomX % 2 == 0 &&
								  RoomY % 2 != 0 ) ||
								 ( RoomX % 2 != 0 &&
								  RoomY % 2 == 0 ) )
							{
								SetCellHeight( RoomX, RoomY, 1 );
								SetCellType( RoomX, RoomY, KCTypeRoomWall );
							}
						}
					}
				}
				SetRoomIndexData( RoomX, RoomY, m_RoomIsolated.size() );
			}
		}

		// make sure we have no nasty diagonal corners
		if( Crenellated )
		{
			if( CellHeight( BestX, BestY + Height - 1 ) == 0 &&
				CellHeight( BestX, BestY + Height - 2 ) != 0 &&
				CellHeight( BestX + 1, BestY + Height - 1 ) != 0 )
			{
				SetCellHeight( BestX, BestY + Height - 2, 0 );
				SetCellType( BestX, BestY + Height - 2, KCTypeRoom );
			}
			if( CellHeight( BestX + Width - 1, BestY + Height - 1 ) == 0 &&
				CellHeight( BestX + Width - 1, BestY + Height - 2 ) != 0 &&
				CellHeight( BestX + Width - 2, BestY + Height - 1 ) != 0 )
			{
				SetCellHeight( BestX + Width - 1, BestY + Height - 2, 0 );
				SetCellType( BestX + Width - 1, BestY + Height - 2, KCTypeRoom );
			}
			if( CellHeight( BestX, BestY ) == 0 &&
				CellHeight( BestX, BestY + 1) != 0 &&
				CellHeight( BestX + 1, BestY ) != 0 )
			{
				SetCellHeight( BestX, BestY + 1, 0 );
				SetCellType( BestX, BestY + 1, KCTypeRoom );
			}
			if( CellHeight( BestX + Width - 1, BestY ) == 0 &&
				CellHeight( BestX + Width - 1, BestY + 1 ) != 0 &&
				CellHeight( BestX + Width - 2, BestY ) != 0 )
			{
				SetCellHeight( BestX + Width - 1, BestY + 1, 0 );
				SetCellType( BestX + Width - 1, BestY + 1, KCTypeRoom );
			}
		}
	m_RoomIsolated.push_back( Isolated );
	m_RoomX.push_back( RoomCenterX );
	m_RoomY.push_back( RoomCenterY );
	m_RoomWidth.push_back( Width );
	m_RoomHeight.push_back( Height );
	m_RoomExits.push_back( 0 );
} // CMaze::GenerateRoom()

void CMaze::GenerateOvalRoom( int32 BestX,		// top left x
							  int32 BestY,		// top left y
							  int32 Width,		// room width
							  int32 Height )	// room height
{
	bool Isolated( kTrue );
	int32 RoomCenterX = BestX + Width / 2;
	int32 RoomCenterY = BestY + Height / 2;

	int32 NewWallType( 0 );//rand() % m_pTemplate.TileTypes() );

		if( NewWallType >= (int32)m_pTemplate.TileTypes() )
		{
			NewWallType = 0;
		}


		float32 WidthRadius = ( Width - 2.0f ) * ( Width - 2.0f );
		float32 HeightRadius = ( Height - 2.5f ) * ( Height - 2.5f );
		float32 CenterX = (float32)( BestX + BestX + Width - 1 ) / 2.0f;
		float32 CenterY = (float32)( BestY + BestY + Height - 1 ) / 2.0f;

		for( int32 RoomX = BestX - 1; RoomX < BestX + Width + 2; RoomX++ )
		{
			for( int32 RoomY = BestY - 1; RoomY < BestY + Height + 2; RoomY++ )
			{
				 float32 Dist = ( ( ( (float32)RoomX - CenterX) * ( (float32)RoomX - CenterX ) * 100.0f ) / 
						  WidthRadius + 
						  ( ( (float32)RoomY - CenterY ) * ( (float32)RoomY - CenterY ) * 100.0f )
							/ HeightRadius );
				 if ( Dist < 50 )
				 {
					if( CellType( RoomX, RoomY ) == KCTypeHall )
					{
						Isolated = kFalse;
					}
				 }

				 if( CellType( RoomX, RoomY ) == KCTypeRoom )
				{
					NewWallType = WallType( RoomX, RoomY );
					break;
				}
			}
		}
		for( int32 RoomX = BestX - 1; RoomX < BestX + Width + 2; RoomX++ )
		{
			for( int32 RoomY = BestY - 1; RoomY < BestY + Height + 2; RoomY++ )
			{
				 float32 Dist = ( ( ( (float32)RoomX - CenterX) * ( (float32)RoomX - CenterX ) * 100.0f ) / 
						  WidthRadius + 
						  ( ( (float32)RoomY - CenterY ) * ( (float32)RoomY - CenterY ) * 100.0f )
							/ HeightRadius );
				 if ( Dist < 50 )
				 {
					if( CellType( RoomX, RoomY ) != KCTypeRoom )
					{
						SetWallType( RoomX, RoomY, (uint16)NewWallType );
					}
				 }
			}
		}

		for( int32 RoomX = BestX - 1; RoomX < BestX + Width + 2; RoomX++ )
		{
			for( int32 RoomY = BestY - 1; RoomY < BestY + Height + 2; RoomY++ )
			{
				 float32 Dist = ( ( ( (float32)RoomX - CenterX) * ( (float32)RoomX - CenterX ) * 100.0f ) / 
						  WidthRadius + 
						  ( ( (float32)RoomY - CenterY ) * ( (float32)RoomY - CenterY ) * 100.0f )
							/ HeightRadius );
				 if ( Dist >= 50 )
				 {
						if( CellType( RoomX, RoomY ) != KCTypeRoom &&
							CellHeight( RoomX, RoomY ) > 0 )
						{
							SetCellType( RoomX, RoomY, KCTypeRoomWall );
						}
				 }
			}
		}

		// now we have the best coordinates
		// so we carve out the space
		for( int32 RoomX = BestX; RoomX < BestX + Width; RoomX++ )
		{
			for( int32 RoomY = BestY; RoomY < BestY + Height; RoomY++ )
			{
				 float32 Dist = ( ( ( (float32)RoomX - CenterX) * ( (float32)RoomX - CenterX ) * 100.0f ) / 
						  WidthRadius + 
						  ( ( (float32)RoomY - CenterY ) * ( (float32)RoomY - CenterY ) * 100.0f )
							/ HeightRadius );
				 if ( Dist < 50 )
				 {
					SetRoomIndexData( RoomX, RoomY, m_RoomIsolated.size() );
					SetCellHeight( RoomX, RoomY, 0 );
					SetCellType( RoomX, RoomY, KCTypeRoom );
				 }
			}
		}
	m_RoomIsolated.push_back( Isolated );
	m_RoomX.push_back( RoomCenterX );
	m_RoomY.push_back( RoomCenterY );
	m_RoomWidth.push_back( Width );
	m_RoomHeight.push_back( Height );
	m_RoomExits.push_back( 0 );
} // CMaze::GenerateOvalRoom()

void CMaze::GenerateRingRoom( int32 BestX,		// top left x
							  int32 BestY,		// top left y
							  int32 Width,		// room width
							  int32 Height )	// room height
{
	bool Isolated( kTrue );
	int32 RoomCenterX = BestX + Width / 2;
	int32 RoomCenterY = BestY + Height / 2;
	int32 NewWallType( 0 );//rand() % m_pTemplate.TileTypes() );

		if( NewWallType >= (int32)m_pTemplate.TileTypes() )
		{
			NewWallType = 0;
		}


		float32 WidthRadius = ( Width - 2.0f ) * ( Width - 2.0f );
		float32 HeightRadius = ( Height - 2.5f ) * ( Height - 2.5f );
		float32 CenterX = (float32)( BestX + BestX + Width - 1 ) / 2.0f;
		float32 CenterY = (float32)( BestY + BestY + Height - 1 ) / 2.0f;

		for( int32 RoomX = BestX - 1; RoomX < BestX + Width + 2; RoomX++ )
		{
			for( int32 RoomY = BestY - 1; RoomY < BestY + Height + 2; RoomY++ )
			{
				 float32 Dist = ( ( ( (float32)RoomX - CenterX) * ( (float32)RoomX - CenterX ) * 100.0f ) / 
						  WidthRadius + 
						  ( ( (float32)RoomY - CenterY ) * ( (float32)RoomY - CenterY ) * 100.0f )
							/ HeightRadius );
				 if ( Dist < 50 )
				 {
					if( CellType( RoomX, RoomY ) == KCTypeHall )
					{
						Isolated = kFalse;
					}
				 }

				 if( CellType( RoomX, RoomY ) == KCTypeRoom )
				{
					NewWallType = WallType( RoomX, RoomY );
					break;
				}
			}
		}
		for( int32 RoomX = BestX - 1; RoomX < BestX + Width + 2; RoomX++ )
		{
			for( int32 RoomY = BestY - 1; RoomY < BestY + Height + 2; RoomY++ )
			{
				 float32 Dist = ( ( ( (float32)RoomX - CenterX) * ( (float32)RoomX - CenterX ) * 100.0f ) / 
						  WidthRadius + 
						  ( ( (float32)RoomY - CenterY ) * ( (float32)RoomY - CenterY ) * 100.0f )
							/ HeightRadius );
				 if ( Dist < 50 )
				 {
					if( CellType( RoomX, RoomY ) != KCTypeRoom )
					{
						SetWallType( RoomX, RoomY, (uint16)NewWallType );
					}
				 }
			}
		}

		for( int32 RoomX = BestX - 1; RoomX < BestX + Width + 2; RoomX++ )
		{
			for( int32 RoomY = BestY - 1; RoomY < BestY + Height + 2; RoomY++ )
			{
				 float32 Dist = ( ( ( (float32)RoomX - CenterX) * ( (float32)RoomX - CenterX ) * 100.0f ) / 
						  WidthRadius + 
						  ( ( (float32)RoomY - CenterY ) * ( (float32)RoomY - CenterY ) * 100.0f )
							/ HeightRadius );
				 if ( Dist >= 50 )
				 {
						if( CellType( RoomX, RoomY ) != KCTypeRoom &&
							CellHeight( RoomX, RoomY ) > 0 )
						{
							SetCellType( RoomX, RoomY, KCTypeRoomWall );
						}
				 }
			}
		}

		// now we have the best coordinates
		// so we carve out the space
		for( int32 RoomX = BestX; RoomX < BestX + Width; RoomX++ )
		{
			for( int32 RoomY = BestY; RoomY < BestY + Height; RoomY++ )
			{
				 float32 Dist = ( ( ( (float32)RoomX - CenterX) * ( (float32)RoomX - CenterX ) * 100.0f ) / 
						  WidthRadius + 
						  ( ( (float32)RoomY - CenterY ) * ( (float32)RoomY - CenterY ) * 100.0f )
							/ HeightRadius );
				 if ( Dist < 10 )
				 {
					SetCellHeight( RoomX, RoomY, 1 );
					SetCellType( RoomX, RoomY, KCTypeSolid );
				 }
				 else if ( Dist < 50 )
				 {
					RoomCenterX = RoomX;
					RoomCenterY = RoomY;
					SetRoomIndexData( RoomX, RoomY, m_RoomIsolated.size() );
					SetCellHeight( RoomX, RoomY, 0 );
					SetCellType( RoomX, RoomY, KCTypeRoom );
				 }
			}
		}
	m_RoomIsolated.push_back( Isolated );
	m_RoomX.push_back( RoomCenterX );
	m_RoomY.push_back( RoomCenterY );
	m_RoomWidth.push_back( Width );
	m_RoomHeight.push_back( Height );
	m_RoomExits.push_back( 0 );

} // CMaze::GenerateRingRoom()

int32 CMaze::CountExits( int32 X,		// x cell to check
						  int32 Y )	// y cell to check
{
	int32 Exits( 0 );
	// count the number of adjoining open squares to this one
	if( CellHeight( X, Y )  == 0 )
	{
		if( CellHeight( X, Y - 1 )  == 0 )
		{
			Exits++;
		}
		if( CellHeight( X, Y + 1 )  == 0 )
		{
			Exits++;
		}
		if( CellHeight( X - 1, Y )  == 0 )
		{
			Exits++;
		}
		if( CellHeight( X + 1, Y )  == 0 )
		{
			Exits++;
		}

		if( Exits == 0 )
		{
			if( CellHeight( X - 1, Y - 1 )  == 0 )
			{
				Exits = 1;
			}
			if( CellHeight( X + 1, Y - 1 )  == 0 )
			{
				Exits = 1;
			}
			if( CellHeight( X - 1, Y + 1 )  == 0 )
			{
				Exits = 1;
			}
			if( CellHeight( X + 1, Y + 1 )  == 0 )
			{
				Exits = 1;
			}
		}

	}

	return Exits;
} // CMaze::CountExits()

bool CMaze::IsCorner( int32 X,		// x cell to check
					  int32 Y,		// y cell to check
					  int16& FilledHeight )	// height of corner walls to fill
{
	int16 Height = CellHeight( X, Y );

	if( CellHeight( X - 1, Y ) > Height &&
		CellHeight( X - 1, Y + 1 ) > Height &&
		CellHeight( X, Y + 1 ) > Height &&
		CellHeight( X + 1, Y ) <= Height &&
		CellHeight( X + 1, Y - 1 ) <= Height &&
		CellHeight( X, Y - 1 ) <= Height )
	{
		FilledHeight = CellHeight( X - 1, Y );
		return kTrue;
	}

	if( CellHeight( X + 1, Y ) > Height &&
		CellHeight( X + 1, Y + 1 ) > Height &&
		CellHeight( X, Y + 1 ) > Height &&
		CellHeight( X - 1, Y ) <= Height &&
		CellHeight( X - 1, Y - 1 ) <= Height &&
		CellHeight( X, Y - 1 ) <= Height )
	{
		FilledHeight = CellHeight( X + 1, Y );
		return kTrue;
	}

	if( CellHeight( X - 1, Y ) > Height &&
		CellHeight( X - 1, Y - 1 ) > Height &&
		CellHeight( X, Y - 1 ) > Height &&
		CellHeight( X + 1, Y ) <= Height &&
		CellHeight( X + 1, Y + 1 ) <= Height &&
		CellHeight( X, Y + 1 ) <= Height )
	{
		FilledHeight = CellHeight( X - 1, Y );
		return kTrue;
	}

	if( CellHeight( X + 1, Y ) > Height &&
		CellHeight( X + 1, Y - 1 ) > Height &&
		CellHeight( X, Y - 1 ) > Height &&
		CellHeight( X - 1, Y ) <= Height &&
		CellHeight( X - 1, Y + 1 ) <= Height &&
		CellHeight( X, Y + 1 ) <= Height )
	{
		FilledHeight = CellHeight( X + 1, Y );
		return kTrue;
	}

	return kFalse;
} // CMaze::IsCorner()

int32 CMaze::CalculateScoreForRoom( int32 X,		// x top left corner of room
									int32 Y,		// y top left corner of room
									int32 Width,	// Width of room
									int32 Height )// Height of room
{
	int32 Score( 0 );
	bool TouchesHall( kFalse );
	bool TouchesRoom( kFalse );
	for( int32 i = X; i < X + Width; i++ )
	{
		for( int32 j = Y; j < Y + Height; j++ )
		{
			if( !TouchesHall && CellType( i, j ) == KCTypeHall )
			{
				TouchesHall = kTrue;
			}
			// we get a 3 score for every open cell
			if( CellHeight( i, j ) == 0 )
			{
				Score += 100;
			}
			// adjoining open cells in the cardinal directions get a 1 score bonus
			if( CellHeight( i, j - 1 ) == 0 )
			{
				Score += 100;
			}
			if( CellHeight( i, j + 1 ) == 0 )
			{
				Score += 100;
			}
			if( CellHeight( i - 1, j ) == 0 )
			{
				Score += 100;
			}
			if( CellHeight( i + 1, j ) == 0 )
			{
				Score += 100;
			}
			// filled cells give a 100 bonus
			if( CellHeight( i, j ) == 1 )
			{
				Score -= 500;
			}
			if( CellType( i, j ) == KCTypeRoom )
			{
				Score += 200;
				TouchesRoom = kTrue;
			}
			if( CellType( i, j ) == KCTypeRoomWall )
			{
				Score += 200;
				TouchesRoom = kTrue;
			}
		}
	}
	if( !TouchesHall && !TouchesRoom )
	{
		Score = -999;
	}

	return Score;
} // CMaze::CalculateScoreForRoom()

// generate pits
void CMaze::GeneratePits( int32 Pits )	// rooms to generate
{
	for( int32 i = 0 ; i < Pits; i++ )
	{
		int32 BestX( 1 );
		int32 BestY( 1 );
		int32 BestScore = 9999999;
		// select random room dimensions within bounds

		int32 Width = ( rand()% ( m_MaxPitWidth - m_MinPitWidth ) ) + 
					   m_MinPitWidth ;
		int32 Height = ( rand()% ( m_MaxPitHeight - m_MinPitHeight ) ) + 
						m_MinPitHeight;
		
		// go through the map checking all possible locations for the room
		for( int32 X = 1; X < ( m_MazeWidth - 1 - Width ); X++ )
		{
			for( int32 Y = 1; Y < ( m_MazeHeight - 1 - Height ); Y++ ) 
			{
				int32 Score( CalculateScoreForPit( X, Y, Width, Height ) );

				// if it's a smaller score, then it's a better room - 
				// we take its coordinates and keep them
				if( Score < BestScore )
				{
					BestX = X;
					BestY = Y;
					BestScore = Score;
				}
			}
		}

		GeneratePit( BestX, BestY, Width, Height );
	}
} // CMaze::GeneratePits()

void CMaze::GeneratePit( int32 BestX,		// top left x
						 int32 BestY,		// top left y
						 int32 Width,		// room width
						 int32 Height )		// room height
{
		bool Crenellated( RandomIntegerBetween( 0, 100 ) < 50 );
		bool WaterFilled( kFalse );
		if( m_pTemplate.HasWater() )
		{
			WaterFilled = kTrue;
		}
		int32 NewWallType( 0 );
		if( m_pTemplate.TileTypes() > 1 )
		{
			NewWallType = 1;
		}
		int32 TerrainType( m_pTemplate.TileGroundLayer( 1 ) );
		// now we have the best coordinates
		// so we carve out the space

		for( int32 RoomX = BestX + 1 ; RoomX < BestX + Width - 1; RoomX++ )
		{
			for( int32 RoomY = BestY + 1; RoomY < BestY + Height - 1; RoomY++ )
			{
				if( CellType( RoomX, RoomY ) != KCTypeRoomWall )
				{
					SetCellHeight( RoomX, RoomY, -1 );
					SetCellType( RoomX, RoomY, KCTypePit );

					if( Crenellated )
					{
						if( RoomY == BestY + 1 || RoomY == BestY + Height - 2 )
						{
							if( RoomX % 2 == 0 )
							{
								SetCellHeight( RoomX, RoomY, 0 );
								SetCellType( RoomX, RoomY, KCTypeRoom );
							}
						}
						else if( RoomX == BestX + 1 || RoomX == BestX + Width - 2 )
						{
							if( RoomY % 2 == 0 )
							{
								SetCellHeight( RoomX, RoomY, 0 );
								SetCellType( RoomX, RoomY, KCTypeRoom );
//								SetHasWater( RoomX, RoomY, kFalse );
							}
						}
					}
				}

			}
		}
		for( int32 RoomX = BestX; RoomX < BestX + Width; RoomX++ )
		{
			for( int32 RoomY = BestY; RoomY < BestY + Height; RoomY++ )
			{

				if( CellHeight( RoomX, RoomY ) > 0 )
				{
					SetCellHeight( RoomX, RoomY, 0 );
					SetCellType( RoomX, RoomY, KCTypeRoom );
				}
				SetWallType( RoomX, RoomY, (uint16)NewWallType );
				//SetWallBorder( RoomX, RoomY, (uint16)NewWallType );
			}
		}
		for( int32 RoomX = BestX - 1; RoomX < BestX + Width; RoomX++ )
		{
			for( int32 RoomY = BestY - 1; RoomY < BestY + Height; RoomY++ )
			{

				if( CellHeight( RoomX, RoomY ) < 0 ||
					( CellHeight( RoomX, RoomY ) <= 0 &&
					  IsCliff( RoomX, RoomY ) ) )
				{
					SetHasWater( RoomX, RoomY, WaterFilled );
				}
			}
		}
		for( int32 RoomX = BestX + 1; RoomX < BestX + Width; RoomX++ )
		{
			for( int32 RoomY = BestY + 1; RoomY < BestY + Height; RoomY++ )
			{
				SetTerrainType( RoomX, RoomY, TerrainType );
			}
		}

		for( int32 RoomX = BestX; RoomX <= BestX + Width; RoomX++ )
		{
			for( int32 RoomY = BestY; RoomY <= BestY + Height; RoomY++ )
			{
				//if( CellHeight( RoomX, RoomY ) < 0 ||
				//	( CellHeight( RoomX, RoomY ) <= 0 &&
				//	  IsCliff( RoomX, RoomY ) ) )
				{
					SetFloorHeight( RoomX, RoomY, 0 );
				}
			}
		}

		// add a positional audio element, if applicable
		if( WaterFilled && m_pTemplate.WaterSoundPath().length() > 0 )
		{
			m_PositionalAudioPath.push_back( m_pTemplate.WaterSoundPath() );
			m_PositionalAudioRadius.push_back( m_pTemplate.WaterSoundRadius() );
			m_PositionalAudioPosition.push_back( D3DXVECTOR3( RandomBetween( (float32)BestX, (float32)BestX + (float32)Width ),
															  0,
															  RandomBetween( (float32)BestY, (float32)BestY + (float32)Height ) ) );
		}
} // CMaze::GeneratePit()


int32 CMaze::CalculateScoreForPit( int32 X,		// x top left corner of room
								   int32 Y,		// y top left corner of room
								   int32 Width,	// Width of room
								   int32 Height )// Height of room
{
	int32 Score( 0 );
	
	for( int32 i = X; i < X + Width; i++ )
	{
		for( int32 j = Y; j < Y + Height; j++ )
		{
			// we get a 3 score for every open cell
			if( CellHeight( i, j ) == 0 )
			{
				Score -= 3;
			}
			// adjoining open cells in the cardinal directions get a 1 score bonus
			if( CellHeight( i, j - 1 ) == 0 )
			{
				Score -= 1;
			}
			if( CellHeight( i, j + 1 ) == 0 )
			{
				Score -= 1;
			}
			if( CellHeight( i - 1, j ) == 0 )
			{
				Score -= 1;
			}
			if( CellHeight( i + 1, j ) == 0 )
			{
				Score -= 1;
			}
			// filled cells give a 100 bonus
			if( CellHeight( i, j ) > 0 )
			{
				Score += 100;
			}
			// filled cells give a 100 bonus
			if( CellHeight( i, j ) < 0 )
			{
				Score += 300;
			}

			if( CellType( i, j ) == KCTypeRoom )
			{
				Score -= 200;
			}
			if( CellType( i, j ) == KCTypeRoomWall )
			{
				Score += 100;
			}
		}
	}
	return Score;
} // CMaze::CalculateScoreForPit()

bool CMaze::SelectRandomUnvisitedCell( int32& X,	// x coordinate to fill 
									   int32& Y )	// y coordinate to fill
{
	int32 RandomX( 0 );
	int32 RandomY( 0 );
	bool Done( kFalse );

	// if there are no remaining unvisited cells, return kFalse
	if( (int32)m_CellsVisited >= m_MazeWidth * m_MazeHeight )
	{
		return kFalse;
	}


	while( !Done )
	{
		RandomX = ( rand() % ( m_MazeWidth - 2 ) ) + 1;
		RandomY = ( rand() % ( m_MazeHeight - 2 ) ) + 1;

		// if this cell has not been visited yet,
		// and is not adjoining any open cells, return it
		if( !CellVisited( RandomX, RandomY ) && 
			!( CellHeight( RandomX - 1, RandomY ) == 0 ||
			   CellHeight( RandomX + 1, RandomY ) == 0 ||
			   CellHeight( RandomX, RandomY - 1 ) == 0 ||
			   CellHeight( RandomX, RandomY + 1) == 0 ) )
		{
			Done = kTrue;
		}
	}
	
	if( Done )
	{
		X = RandomX;
		Y = RandomY;
		return kTrue;
	}
	return kFalse;
} // CMaze::SelectRandomUnvisitedCell()


// todo: requires a check to make sure that cells of type actually exist
bool CMaze::SelectRandomCell( int32& X,		// x coordinate to fill 
							  int32& Y,		// y coordinate to fill
							  int16 Value )		// value requirement for cell 
{
	int32 RandomX( 0 );
	int32 RandomY( 0 );
	bool Done( kFalse );
	bool HasCell( kFalse );
	for( int32 x = 1; x < m_MazeWidth - 1; x++ )
	{
		for( int32 y = 1; y < m_MazeHeight - 1; y++ )
		{
			if( CellHeight( x, y ) == Value )
			{
				HasCell = kTrue;
				break;
			}
		}
		if( HasCell )
		{
			break;
		}
	}
	// ok, it doesn't exist. Just pick anything.
	if( !HasCell )
	{
		RandomX = ( rand() % ( m_MazeWidth - 2 ) ) + 1;
		RandomY = ( rand() % ( m_MazeHeight - 2 ) ) + 1;
		Done = kTrue;
	}

	while( !Done )
	{
		RandomX = ( rand() % ( m_MazeWidth - 2 ) ) + 1;
		RandomY = ( rand() % ( m_MazeHeight - 2 ) ) + 1;

		// if this cell has not been visited yet,
		// and is not adjoining any open cells, return it
		if( CellHeight( RandomX, RandomY ) == Value )
		{
			Done = kTrue;
		}
	}
	
	if( Done )
	{
		X = RandomX;
		Y = RandomY;
		return kTrue;
	}
	return kFalse;
} // CMaze::SelectRandomOpenCell()

// todo: requires a check to make sure that cells of type actually exist
bool CMaze::SelectRandomCell( int32& X,		// x coordinate to fill 
							  int32& Y,		// y coordinate to fill
							  ECellType Value )	// value to set
{
	int32 RandomX( 0 );
	int32 RandomY( 0 );
	bool Done( kFalse );
	bool HasCell( kFalse );
	for( int32 x = 1; x < m_MazeWidth - 1; x++ )
	{
		for( int32 y = 1; y < m_MazeHeight - 1; y++ )
		{
			if( CellType( x, y ) == Value )
			{
				HasCell = kTrue;
				break;
			}
		}
		if( HasCell )
		{
			break;
		}
	}
	// ok, it doesn't exist. Just pick anything.
	if( !HasCell )
	{
		RandomX = ( rand() % ( m_MazeWidth - 2 ) ) + 1;
		RandomY = ( rand() % ( m_MazeHeight - 2 ) ) + 1;
		Done = kTrue;
	}
	while( !Done )
	{
		RandomX = ( rand() % ( m_MazeWidth - 2 ) ) + 1;
		RandomY = ( rand() % ( m_MazeHeight - 2 ) ) + 1;

		// if this cell has not been visited yet,
		// and is not adjoining any open cells, return it
		if( CellType( RandomX, RandomY ) == Value )
		{
			Done = kTrue;
		}
	}
	
	if( Done )
	{
		X = RandomX;
		Y = RandomY;
		return kTrue;
	}
	return kFalse;
} // CMaze::SelectRandomOpenCell()

void CMaze::SetRoomIndexData( int32 X,	// x coordinate of cell to visit 
							  int32 Y,	// y coordinate of cell to visit
							  int32 Value )	// value to set
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < m_MazeWidth );
	assert( Y < m_MazeHeight );

	if( X >= 0 &&
		Y >= 0 &&
		X < (int32)m_MazeWidth &&
		Y < (int32)m_MazeHeight )
	{
		m_RoomIndexData[X + Y * m_MazeWidth] = Value;
	}
} // CMaze::SetRoomIndexData()

int32 CMaze::RoomIndexData( int32 X,	// x coordinate of cell to check
							int32 Y )	// y coordinate of cell to check
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < m_MazeWidth );
	assert( Y < m_MazeHeight );

	return m_RoomIndexData[X + Y * m_MazeWidth];
} // CMaze::RoomIndexData()

void CMaze::SetCellType( int32 X,	// x coordinate of cell to visit 
						 int32 Y,	// y coordinate of cell to visit
						 ECellType Value )	// value to set
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < m_MazeWidth );
	assert( Y < m_MazeHeight );


	if( X >= 0 &&
		Y >= 0 &&
		X < (int32)m_MazeWidth &&
		Y < (int32)m_MazeHeight )
	{
		// we NEVER overwrite stairwell specifications
		if( m_CellType[X + Y * m_MazeWidth] == KCTypeStairwellUp ||
			m_CellType[X + Y * m_MazeWidth] == KCTypeStairwellDown )
		{
			return;
		}
		m_CellType[X + Y * m_MazeWidth] = Value;
	}
} // CMaze::SetCellType()

void CMaze::VisitCell( int32 X,	// x coordinate of cell to visit 
					   int32 Y )	// y coordinate of cell to visit
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < m_MazeWidth );
	assert( Y < m_MazeHeight );

	if( !CellVisited( X, Y ) )
	{
		m_CellVisited[X + Y * m_MazeWidth] = kTrue;
		m_CellsVisited ++;
	}
} // CMaze::VisitCell()

void CMaze::UnVisitCell( int32 X,	// x coordinate of cell to visit 
					     int32 Y )	// y coordinate of cell to visit
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < m_MazeWidth );
	assert( Y < m_MazeHeight );

	m_CellVisited[X + Y * m_MazeWidth] = kFalse;
} // CMaze::VisitCell()

bool CMaze::CellVisited( int32 X,	// x coordinate of cell to check
						 int32 Y )	// y coordinate of cell to check
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < m_MazeWidth );
	assert( Y < m_MazeHeight );

	return m_CellVisited[X + Y * m_MazeWidth];
} // CMaze::CellVisited()

bool CMaze::CellOpen( int32 X,		// x coordinate of cell to check
					  int32 Y )	// y coordinate of cell to check
{
	if( X < 0 )
	{
		X = 0;
	}
	if( X >= m_MazeWidth )
	{
		X = m_MazeWidth - 1;
	}
	if( Y < 0 )
	{
		Y = 0;
	}
	if( Y >= m_MazeHeight )
	{
		Y = m_MazeHeight - 1;
	}
	if( CellType( X, Y ) == KCTypeSolid ||
		CellType( X, Y ) == KCTypeRoomWall ||
		CellType( X, Y ) == KCTypePit ||
		CellType( X, Y ) == KCTypeEWDoor ||
		CellType( X, Y ) == KCTypeNSDoor ||
		CellType( X, Y ) == KCTypeStairwellUp ||
		CellType( X, Y ) == KCTypeStairwellDown )
	{
		return kFalse;
	}
	return kTrue;
} // CMaze::CellOpen()

bool CMaze::CellOpenIgnoreDoors( int32 X,		// x coordinate of cell to check
								 int32 Y )	// y coordinate of cell to check
{
	if( X < 0 )
	{
		X = 0;
	}
	if( X >= m_MazeWidth )
	{
		X = m_MazeWidth - 1;
	}
	if( Y < 0 )
	{
		Y = 0;
	}
	if( Y >= m_MazeHeight )
	{
		Y = m_MazeHeight - 1;
	}
	if( CellType( X, Y ) == KCTypeSolid ||
		CellType( X, Y ) == KCTypeRoomWall ||
		CellType( X, Y ) == KCTypePit ||
		CellType( X, Y ) == KCTypeStairwellUp ||
		CellType( X, Y ) == KCTypeStairwellDown )
	{
		return kFalse;
	}
	return kTrue;
} // CMaze::CellOpen()

ECellType CMaze::CellType( int32 X,	// x coordinate of cell to check
						   int32 Y )	// y coordinate of cell to check
{
	if( X < 0 )
	{
		X = 0;
	}
	if( X >= m_MazeWidth )
	{
		X = m_MazeWidth - 1;
	}
	if( Y < 0 )
	{
		Y = 0;
	}
	if( Y >= m_MazeHeight )
	{
		Y = m_MazeHeight - 1;
	}

	return m_CellType[X + Y * m_MazeWidth];
} // CMaze::CellType()

bool CMaze::CellPassable( int32 X,			// x location of cell to retrieve
						  int32 Y )			// y location of cell to retrieve
{		
	if( X < 0 ||
		Y < 0 ||
		X >= m_MazeWidth ||
		Y >= m_MazeHeight )
	{
		return kFalse;
	}

	return CellHeight( X, Y) == 0;
} // CMaze::CellPassable()

int16 CMaze::CellHeight( int32 X,			// x location of cell to retrieve
						 int32 Y )			// y location of cell to retrieve
{		
	if( X < 0 ||
		Y < 0 ||
		X >= m_MazeWidth ||
		Y >= m_MazeHeight )
	{
		return 1;
	}

	return m_CellHeightData[X + Y * m_MazeWidth];
} // CMaze::CellHeight()


bool CMaze::HasWater( int32 X,			// x location of cell to retrieve
					  int32 Y )			// y location of cell to retrieve
{		
	if( X < 0 ||
		Y < 0 ||
		X >= m_MazeWidth ||
		Y >= m_MazeHeight )
	{
		return kFalse;
	}

	return m_CellHasWater[X + Y * m_MazeWidth];
} // CMaze::HasWater()

bool CMaze::IsFishable( int32 X,			// x location of cell to retrieve
						int32 Y )			// y location of cell to retrieve
{		
	if( X < 0 ||
		Y < 0 ||
		X >= m_MazeWidth ||
		Y >= m_MazeHeight )
	{
		return kFalse;
	}

	return m_CellIsFishable[X + Y * m_MazeWidth];
} // CMaze::IsFishable()

bool CMaze::HasCollideableProp( int32 X,			// x location of cell to retrieve
								int32 Y )			// y location of cell to retrieve
{		
	if( X < 0 ||
		Y < 0 ||
		X >= m_MazeWidth ||
		Y >= m_MazeHeight )
	{
		return kFalse;
	}

	return m_CellHasCollideableProp[X + Y * m_MazeWidth];
} // CMaze::HasCollideableProp()

uint16 CMaze::WallType( int32 X,			// x location of cell to retrieve
						int32 Y )			// y location of cell to retrieve
{		
	if( X < 0 ||
		Y < 0 ||
		X >= m_MazeWidth ||
		Y >= m_MazeHeight )
	{
		return 1;
	}

	return m_WallTypeData[X + Y * m_MazeWidth];
} // CMaze::WallType()

int32 CMaze::TerrainType( int32 X,			// x location of cell to retrieve
						   int32 Y )		// y location of cell to retrieve
{		
	if( X < 0 ||
		Y < 0 ||
		X >= m_MazeWidth ||
		Y >= m_MazeHeight )
	{
		return 1;
	}

	return m_TerrainTypeData[X + Y * m_MazeWidth];
} // CMaze::TerrainType()

float32 CMaze::FloorHeight( int32 X,		// x location of cell to retrieve
						    int32 Y )		// y location of cell to retrieve
{		
	if( X < 0 ||
		Y < 0 ||
		X >= m_MazeWidth ||
		Y >= m_MazeHeight )
	{
		return 0;
	}

	return m_HeightData[X + Y * m_MazeWidth];
} // CMaze::FloorHeight()

float32 CMaze::XOffset( int32 X,		// x location of cell to retrieve
						int32 Y )		// y location of cell to retrieve
{		
	if( X < 0 ||
		Y < 0 ||
		X >= m_MazeWidth ||
		Y >= m_MazeHeight )
	{
		return 0;
	}

	return m_XOffsetData[X + Y * m_MazeWidth];
} // CMaze::XOffset()

float32 CMaze::YOffset( int32 X,		// x location of cell to retrieve
						int32 Y )		// y location of cell to retrieve
{		
	if( X < 0 ||
		Y < 0 ||
		X >= m_MazeWidth ||
		Y >= m_MazeHeight )
	{
		return 0;
	}

	return m_YOffsetData[X + Y * m_MazeWidth];
} // CMaze::YOffset()

void CMaze::SetHasWater( int32 X,			// x location of cell to retrieve
						 int32 Y,			// y location of cell to retrieve
						 bool Value )		// value to set
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < (int32)m_MazeWidth );
	assert( Y < (int32)m_MazeHeight );

	if( X >= 0 &&
		Y >= 0 &&
		X < (int32)m_MazeWidth &&
		Y < (int32)m_MazeHeight )
	{
		m_CellHasWater[X + Y * m_MazeWidth] = Value;
	}
} // CMaze::SetHasWater()

void CMaze::SetIsFishable( int32 X,			// x location of cell to retrieve
						   int32 Y,			// y location of cell to retrieve
						   bool Value )		// value to set
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < (int32)m_MazeWidth );
	assert( Y < (int32)m_MazeHeight );

	if( X >= 0 &&
		Y >= 0 &&
		X < (int32)m_MazeWidth &&
		Y < (int32)m_MazeHeight )
	{
		m_CellIsFishable[X + Y * m_MazeWidth] = Value;
	}
} // CMaze::SetIsFishable()

void CMaze::SetHasCollideableProp( int32 X,			// x location of cell to retrieve
								   int32 Y,			// y location of cell to retrieve
								   bool Value )		// value to set
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < (int32)m_MazeWidth );
	assert( Y < (int32)m_MazeHeight );

	if( X >= 0 &&
		Y >= 0 &&
		X < (int32)m_MazeWidth &&
		Y < (int32)m_MazeHeight )
	{
		m_CellHasCollideableProp[X + Y * m_MazeWidth] = Value;
	}
} // CMaze::SetHasCollideableProp()

void CMaze::SetCellHeight( int32 X,			// x location of cell to retrieve
						   int32 Y,			// y location of cell to retrieve
						   int16 Value )	// value to set
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < (int32)m_MazeWidth );
	assert( Y < (int32)m_MazeHeight );

	if( X >= 0 &&
		Y >= 0 &&
		X < (int32)m_MazeWidth &&
		Y < (int32)m_MazeHeight )
	{
		m_CellHeightData[X + Y * m_MazeWidth] = Value;
	}
} // CMaze::SetCellHeight()

void CMaze::SetCellHeightProtected( int32 X,			// x location of cell to retrieve
									int32 Y,			// y location of cell to retrieve
									int16 Value )	// value to set
{
	if( X > 1 && Y > 1 && X < m_MazeWidth - 1 && Y < m_MazeHeight - 1 )
	{
		m_CellHeightData[X + Y * m_MazeWidth] = Value;
	}
} // CMaze::SetCellHeightProtected()

void CMaze::SetWallType( int32 X,			// x location of cell to retrieve
						 int32 Y,			// y location of cell to retrieve
						 uint16 Value )		// value to set
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < (int32)m_MazeWidth );
	assert( Y < (int32)m_MazeHeight );

	if( X >= 0 &&
		Y >= 0 &&
		X < (int32)m_MazeWidth &&
		Y < (int32)m_MazeHeight )
	{
		m_WallTypeData[X + Y * m_MazeWidth] = Value;
	}
} // CMaze::SetWallType()

void CMaze::SetTerrainType( int32 X,			// x location of cell to retrieve
						    int32 Y,			// y location of cell to retrieve
						    int32 Value )		// value to set
{
	if( X >= 0 &&
		Y >= 0 &&
		X < (int32)m_MazeWidth &&
		Y < (int32)m_MazeHeight )
	{
		m_TerrainTypeData[X + Y * m_MazeWidth] = Value;
	}
} // CMaze::SetTerrainTypeHeight()

void CMaze::SetFloorHeight( int32 X,			// x location of cell to retrieve
						    int32 Y,			// y location of cell to retrieve
						    float32 Value )		// value to set
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < (int32)m_MazeWidth );
	assert( Y < (int32)m_MazeHeight );

	if( X >= 0 &&
		Y >= 0 &&
		X < (int32)m_MazeWidth &&
		Y < (int32)m_MazeHeight )
	{
		m_HeightData[X + Y * m_MazeWidth] = Value;
	}
} // CMaze::SetFloorHeight()

void CMaze::SetOffset( int32 X,			// x location of cell to retrieve
					   int32 Y,			// y location of cell to retrieve
					   float32 XOffset,		// x turbulence
					   float32 YOffset )	// y turbulence
{
	assert( X >= 0 );
	assert( Y >= 0 );
	assert( X < (int32)m_MazeWidth );
	assert( Y < (int32)m_MazeHeight );

	if( X >= 0 &&
		Y >= 0 &&
		X < (int32)m_MazeWidth &&
		Y < (int32)m_MazeHeight )
	{
		m_XOffsetData[X + Y * m_MazeWidth] = XOffset;
		m_YOffsetData[X + Y * m_MazeWidth] = YOffset;
	}
} // CMaze::SetOffset()

bool CMaze::IsCliff( int32 X,		// x location of cell to retrieve
					 int32 Y )		// y location of cell to retrieve
{
	EWallType Type;
	Type = WallTypeNW( X, Y );
	if( Type != KWTypeOpen &&
		Type != KWTypeNWSolid )
	{
		return kTrue;
	}
	Type = WallTypeNE( X, Y );
	if( Type != KWTypeOpen &&
		Type != KWTypeNESolid )
	{
		return kTrue;
	}
	Type = WallTypeSW( X, Y );
	if( Type != KWTypeOpen &&
		Type != KWTypeSWSolid )
	{
		return kTrue;
	}
	Type = WallTypeSE( X, Y );
	if( Type != KWTypeOpen &&
		Type != KWTypeSESolid )
	{
		return kTrue;
	}

	return kFalse;
} // CMaze::IsCliff()


bool CMaze::IsFlat( int32 X,		// x location of cell to retrieve
					int32 Y )		// y location of cell to retrieve
{
	EWallType Type;
	Type = WallTypeNW( X, Y );
	if( Type == KWTypeNWCorner ||
		Type == KWTypeNWCornerJoin ||
		Type == KWTypeNWInnerCorner )
	{
		return kFalse;
	}
	Type = WallTypeNE( X, Y );
	if( Type == KWTypeNECorner ||
		Type == KWTypeNECornerJoin ||
		Type == KWTypeNEInnerCorner )
	{
		return kFalse;
	}
	Type = WallTypeSW( X, Y );
	if( Type == KWTypeSWCorner ||
		Type == KWTypeSWCornerJoin ||
		Type == KWTypeSWInnerCorner )
	{
		return kFalse;
	}
	Type = WallTypeSE( X, Y );
	if( Type == KWTypeSECorner ||
		Type == KWTypeSECornerJoin ||
		Type == KWTypeSEInnerCorner )
	{
		return kFalse;
	}

	return kTrue;
} // CMaze::IsFlat()

bool CMaze::IsDiagonal( int32 X,		// x location of cell to retrieve
						int32 Y )		// y location of cell to retrieve
{
	EWallType Type;
	Type = WallTypeNW( X, Y );
	if( Type == KWTypeNWCornerJoin )
	{
		return kTrue;
	}
	Type = WallTypeNE( X, Y );
	if( Type == KWTypeNECornerJoin )
	{
		return kTrue;
	}
	Type = WallTypeSW( X, Y );
	if( Type == KWTypeSWCornerJoin )
	{
		return kTrue;
	}
	Type = WallTypeSE( X, Y );
	if( Type == KWTypeSECornerJoin )
	{
		return kTrue;
	}

	return kFalse;
} // CMaze::IsDiagonal()

bool CMaze::IsBorder( int32 X,		// x location of cell to retrieve
					 int32 Y )		// y location of cell to retrieve
{
	int16 Height( CellHeight( X, Y ) );

	int16 HeightN( CellHeight( X, Y - 1 ) );
	int16 HeightW( CellHeight( X - 1, Y ) );
	int16 HeightS( CellHeight( X, Y + 1 ) );
	int16 HeightE( CellHeight( X + 1, Y ) );

	if( HeightN < Height ||
		HeightW < Height ||
		HeightE < Height ||
		HeightS < Height )
	{
		return kTrue;
	}
	return kFalse;
} // CMaze::IsBorder()

bool CMaze::RequiresFloorTile( int32 X,		// x location of cell to retrieve
							   int32 Y )		// y location of cell to retrieve
{
	int16 Height( CellHeight( X, Y ) );

	int16 HeightN( CellHeight( X, Y - 1 ) );
	int16 HeightW( CellHeight( X - 1, Y ) );
	int16 HeightNW( CellHeight( X - 1, Y - 1 ) );
	int16 HeightE( CellHeight( X + 1, Y ) );
	int16 HeightNE( CellHeight( X + 1, Y - 1 ) );

	int16 HeightS( CellHeight( X, Y + 1 ) );
	int16 HeightSW( CellHeight( X - 1, Y + 1 ) );
	int16 HeightSE( CellHeight( X + 1, Y + 1 ) );

	if( HeightN != Height ||
		HeightW != Height ||
		HeightE != Height ||
		HeightW != Height ||
		HeightNW != Height ||
		HeightNE != Height ||
		HeightSW != Height ||
		HeightSE != Height )
	{
		return kFalse;
	}
	return kTrue;
} // CMaze::RequiresFloorTile()

// contains the rules for determining what visual type a cell is
EWallType CMaze::WallTypeNW( int32 X,		// x location of cell to retrieve
						     int32 Y )		// y location of cell to retrieve
{		
	int16 Height( CellHeight( X, Y ) );

	int16 HeightN( CellHeight( X, Y - 1 ) );
	int16 HeightW( CellHeight( X - 1, Y ) );

	int16 HeightNW( CellHeight( X - 1, Y - 1 ) );

	EWallType Type( KWTypeOpen );
	/*if( Height < Height )
	{
		Type = KWTypeOpen;
	}
	else*/
	{
		if( HeightN < Height &&
			HeightW < Height &&
			HeightNW == Height )
		{
			Type = KWTypeNWCornerJoin;
		}
		else if( HeightN < Height &&
				 HeightW < Height &&
				 HeightNW < Height )
		{
			Type = KWTypeNWCorner;
		}
		else if( HeightN == Height &&
				 HeightW == Height &&
				 HeightNW < Height )
		{
			Type = KWTypeNWInnerCorner;
		}
		else if( HeightN == Height &&
				 HeightW == Height &&
				 HeightNW == Height )
		{
			Type = KWTypeNWSolid;
		}
		else if( HeightN < Height &&
				 HeightW == Height &&
				 HeightNW == Height)
		{
			Type = KWTypeNWHorizontal;
		}
		else if( HeightN < Height &&
				 HeightW == Height )
		{
			Type = KWTypeNWHorizontal;
		}
		else if( HeightN == Height &&
				 HeightW < Height &&
				 HeightNW == Height )
		{
			Type = KWTypeNWVertical;
		}
		else if( HeightN == Height &&
				 HeightW < Height )
		{
			Type = KWTypeNWVertical;
		}
	}
	return Type;

} // CMaze::WallTypeNW()


// contains the rules for determining what visual type a cell is
EWallType CMaze::WallTypeNE( int32 X,		// x location of cell to retrieve
						     int32 Y )		// y location of cell to retrieve
{		
	int16 Height( CellHeight( X, Y ) );

	int16 HeightN( CellHeight( X, Y - 1 ) );
	int16 HeightE( CellHeight( X + 1, Y ) );

	int16 HeightNE( CellHeight( X + 1, Y - 1 ) );

	EWallType Type( KWTypeOpen );
/*	if( Height < Height )
	{
		Type = KWTypeOpen;
	}
	else*/
	{
		if( HeightN < Height &&
			HeightE < Height &&
			HeightNE == Height )
		{
			Type = KWTypeNECornerJoin;
		}
		else if( HeightN < Height &&
				 HeightE < Height &&
				 HeightNE < Height )
		{
			Type = KWTypeNECorner;
		}
		else if( HeightN == Height &&
				 HeightE == Height &&
				 HeightNE < Height )
		{
			Type = KWTypeNEInnerCorner;
		}
		else if( HeightN == Height &&
				 HeightE == Height &&
				 HeightNE == Height )
		{
			Type = KWTypeNESolid;
		}
		else if( HeightN < Height &&
				 HeightE == Height &&
				 HeightNE == Height)
		{
			Type = KWTypeNEHorizontal;
		}
		else if( HeightN < Height &&
				 HeightE == Height )
		{
			Type = KWTypeNEHorizontal;
		}
		else if( HeightN == Height &&
				 HeightE < Height &&
				 HeightNE == Height )
		{
			Type = KWTypeNEVertical;
		}
		else if( HeightN == Height &&
				 HeightE < Height )
		{
			Type = KWTypeNEVertical;
		}
	}
	return Type;

} // CMaze::WallTypeNE()

// contains the rules for determining what visual type a cell is
EWallType CMaze::WallTypeSW( int32 X,		// x location of cell to retrieve
						     int32 Y )		// y location of cell to retrieve
{		
	int16 Height( CellHeight( X, Y ) );

	int16 HeightS( CellHeight( X, Y + 1 ) );
	int16 HeightW( CellHeight( X - 1, Y ) );

	int16 HeightSW( CellHeight( X - 1, Y + 1 ) );

	EWallType Type( KWTypeOpen );
	/*if( Height < Height )
	{
		Type = KWTypeOpen;
	}
	else*/
	{
		if( HeightS < Height &&
			HeightW < Height &&
			HeightSW == Height )
		{
			Type = KWTypeSWCornerJoin;
		}
		else if( HeightS < Height &&
				 HeightW < Height &&
				 HeightSW < Height )
		{
			Type = KWTypeSWCorner;
		}
		else if( HeightS == Height &&
				 HeightW == Height &&
				 HeightSW < Height )
		{
			Type = KWTypeSWInnerCorner;
		}
		else if( HeightS == Height &&
				 HeightW == Height &&
				 HeightSW == Height )
		{
			Type = KWTypeSWSolid;
		}
		else if( HeightS < Height &&
				 HeightW == Height &&
				 HeightSW == Height)
		{
			Type = KWTypeSWHorizontal;
		}
		else if( HeightS < Height &&
				 HeightW == Height )
		{
			Type = KWTypeSWHorizontal;
		}
		else if( HeightS == Height &&
				 HeightW < Height &&
				 HeightSW == Height )
		{
			Type = KWTypeSWVertical;
		}
		else if( HeightS == Height &&
				 HeightW < Height )
		{
			Type = KWTypeSWVertical;
		}
	}
	return Type;

} // CMaze::WallTypeSW()


// contains the rules for determining what visual type a cell is
EWallType CMaze::WallTypeSE( int32 X,		// x location of cell to retrieve
						     int32 Y )		// y location of cell to retrieve
{		
	int16 Height( CellHeight( X, Y ) );

	int16 HeightS( CellHeight( X, Y + 1 ) );
	int16 HeightE( CellHeight( X + 1, Y ) );

	int16 HeightSE( CellHeight( X + 1, Y + 1 ) );

	EWallType Type( KWTypeOpen );
	/*if( Height < Height )
	{
		Type = KWTypeOpen;
	}
	else*/
	{
		if( HeightS < Height &&
			HeightE < Height &&
			HeightSE == Height )
		{
			Type = KWTypeSECornerJoin;
		}
		else if( HeightS < Height &&
				 HeightE < Height &&
				 HeightSE < Height )
		{
			Type = KWTypeSECorner;
		}
		else if( HeightS == Height &&
				 HeightE == Height &&
				 HeightSE < Height )
		{
			Type = KWTypeSEInnerCorner;
		}
		else if( HeightS == Height &&
				 HeightE == Height &&
				 HeightSE == Height )
		{
			Type = KWTypeSESolid;
		}
		else if( HeightS < Height &&
				 HeightE == Height &&
				 HeightSE == Height)
		{
			Type = KWTypeSEHorizontal;
		}
		else if( HeightS < Height &&
				 HeightE == Height )
		{
			Type = KWTypeSEHorizontal;
		}
		else if( HeightS == Height &&
				 HeightE < Height &&
				 HeightSE == Height )
		{
			Type = KWTypeSEVertical;
		}
		else if( HeightS == Height &&
				 HeightE < Height )
		{
			Type = KWTypeSEVertical;
		}
	}
	return Type;

} // CMaze::WallTypeSE()

// contains the rules for determining what visual type a scrap of terrain is
ETerrainType CMaze::TerrainTypeNW( int32 X,		// x location of cell to retrieve
								   int32 Y )		// y location of cell to retrieve
{		
	int32 Type( TerrainType( X, Y ) );

	int32 TypeN( TerrainType( X, Y - 1 ) );
	int32 TypeW( TerrainType( X - 1, Y ) );

	int32 TypeNW( TerrainType( X - 1, Y - 1 ) );

	ETerrainType TerrainType( KTTypeEmpty );

	if( TypeN == Type &&
		TypeW == Type &&
		TypeNW == Type )
	{
		TerrainType = KTTypeOpen;
	}
	else if( TypeN == Type &&
			 TypeW == Type &&
			 TypeNW != Type )
	{
		TerrainType = KTTypeNWDiagonalInner;
	}
	else if( TypeN != Type &&
			 TypeW != Type &&
			 TypeNW != Type )
	{
		TerrainType = KTTypeNWDiagonal;
	}
	else if( TypeN == Type &&
			 TypeW != Type &&
			 TypeNW != Type )
	{
		TerrainType = KTTypeWest;
	}
	else if( TypeN != Type &&
			 TypeW == Type &&
			 TypeNW != Type )
	{
		TerrainType = KTTypeNorth;
	}
	else if( TypeN != Type &&
			 TypeW != Type &&
			 TypeNW == Type )
	{
		TerrainType = KTTypeNWDiagonalJoin;
	}
	else if( TypeN == Type &&
			 TypeW != Type &&
			 TypeNW == Type )
	{
		TerrainType = KTTypeSWDiagonalInner;
	}
	else if( TypeN != Type &&
			 TypeW == Type &&
			 TypeNW == Type )
	{
		TerrainType = KTTypeNEDiagonalInner;
	}

	return TerrainType;

} // CMaze::TerrainTypeNW()

// contains the rules for determining what visual type a scrap of terrain is
ETerrainType CMaze::TerrainTypeNE( int32 X,		// x location of cell to retrieve
								   int32 Y )		// y location of cell to retrieve
{		
	int32 Type( TerrainType( X, Y ) );

	int32 TypeN( TerrainType( X, Y - 1 ) );
	int32 TypeE( TerrainType( X + 1, Y ) );

	int32 TypeNE( TerrainType( X + 1, Y - 1 ) );

	ETerrainType TerrainType( KTTypeEmpty );

	if( TypeN == Type &&
		TypeE == Type &&
		TypeNE == Type )
	{
		TerrainType = KTTypeOpen;
	}
	else if( TypeN == Type &&
			 TypeE == Type &&
			 TypeNE != Type )
	{
		TerrainType = KTTypeNEDiagonalInner;
	}
	else if( TypeN != Type &&
			 TypeE != Type &&
			 TypeNE != Type )
	{
		TerrainType = KTTypeNEDiagonal;
	}
	else if( TypeN == Type &&
			 TypeE != Type &&
			 TypeNE != Type )
	{
		TerrainType = KTTypeEast;
	}
	else if( TypeN != Type &&
			 TypeE == Type &&
			 TypeNE != Type )
	{
		TerrainType = KTTypeNorth;
	}
	else if( TypeN != Type &&
			 TypeE != Type &&
			 TypeNE == Type )
	{
		TerrainType = KTTypeNEDiagonalJoin;
	}
	else if( TypeN == Type &&
			 TypeE != Type &&
			 TypeNE == Type )
	{
		TerrainType = KTTypeSEDiagonalInner;
	}
	else if( TypeN != Type &&
			 TypeE == Type &&
			 TypeNE == Type )
	{
		TerrainType = KTTypeNWDiagonalInner;
	}

	return TerrainType;

} // CMaze::TerrainTypeNE()

// contains the rules for determining what visual type a scrap of terrain is
ETerrainType CMaze::TerrainTypeSW( int32 X,		// x location of cell to retrieve
								   int32 Y )		// y location of cell to retrieve
{		
	int32 Type( TerrainType( X, Y ) );

	int32 TypeS( TerrainType( X, Y + 1 ) );
	int32 TypeW( TerrainType( X - 1, Y ) );

	int32 TypeSW( TerrainType( X - 1, Y + 1 ) );

	ETerrainType TerrainType( KTTypeEmpty );

	if( TypeS == Type &&
		TypeW == Type &&
		TypeSW == Type )
	{
		TerrainType = KTTypeOpen;
	}
	else if( TypeS == Type &&
			 TypeW == Type &&
			 TypeSW != Type )
	{
		TerrainType = KTTypeSWDiagonalInner;
	}
	else if( TypeS != Type &&
			 TypeW != Type &&
			 TypeSW != Type )
	{
		TerrainType = KTTypeSWDiagonal;
	}
	else if( TypeS == Type &&
			 TypeW != Type &&
			 TypeSW != Type )
	{
		TerrainType = KTTypeWest;
	}
	else if( TypeS != Type &&
			 TypeW == Type &&
			 TypeSW != Type )
	{
		TerrainType = KTTypeSouth;
	}
	else if( TypeS != Type &&
			 TypeW != Type &&
			 TypeSW == Type )
	{
		TerrainType = KTTypeNEDiagonalJoin;
	}
	else if( TypeS == Type &&
			 TypeW != Type &&
			 TypeSW == Type )
	{
		TerrainType = KTTypeNWDiagonalInner;
	}
	else if( TypeS != Type &&
			 TypeW == Type &&
			 TypeSW == Type )
	{
		TerrainType = KTTypeSEDiagonalInner;
	}

	return TerrainType;

} // CMaze::TerrainTypeSW()

// contains the rules for determining what visual type a scrap of terrain is
ETerrainType CMaze::TerrainTypeSE( int32 X,		// x location of cell to retrieve
								   int32 Y )		// y location of cell to retrieve
{		
	int32 Type( TerrainType( X, Y ) );

	int32 TypeS( TerrainType( X, Y + 1 ) );
	int32 TypeE( TerrainType( X + 1, Y ) );

	int32 TypeSE( TerrainType( X + 1, Y + 1 ) );

	ETerrainType TerrainType( KTTypeEmpty );

	if( TypeS == Type &&
		TypeE == Type &&
		TypeSE == Type )
	{
		TerrainType = KTTypeOpen;
	}
	else if( TypeS == Type &&
			 TypeE == Type &&
			 TypeSE != Type )
	{
		TerrainType = KTTypeSEDiagonalInner;
	}
	else if( TypeS != Type &&
			 TypeE != Type &&
			 TypeSE != Type )
	{
		TerrainType = KTTypeSEDiagonal;
	}
	else if( TypeS == Type &&
			 TypeE != Type &&
			 TypeSE != Type )
	{
		TerrainType = KTTypeEast;
	}
	else if( TypeS != Type &&
			 TypeE == Type &&
			 TypeSE != Type )
	{
		TerrainType = KTTypeSouth;
	}
	else if( TypeS != Type &&
			 TypeE != Type &&
			 TypeSE == Type )
	{
		TerrainType = KTTypeNWDiagonalJoin;
	}
	else if( TypeS == Type &&
			 TypeE != Type &&
			 TypeSE == Type )
	{
		TerrainType = KTTypeNEDiagonalInner;
	}
	else if( TypeS != Type &&
			 TypeE == Type &&
			 TypeSE == Type )
	{
		TerrainType = KTTypeSWDiagonalInner;
	}

	return TerrainType;

} // CMaze::TerrainTypeSE()

bool CMaze::ValidCell( int32 X,		// x coordinate of cell to check
					   int32 Y )	// y coordinate of cell to check
{
	if( X > 0 &&
	    Y > 0 &&
		X < m_MazeWidth - 1 &&
		Y < m_MazeHeight - 1 )
	{
		return kTrue;
	}

	return kFalse;
} // CMaze::ValidCell()
