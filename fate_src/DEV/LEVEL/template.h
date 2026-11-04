/*
	template.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/19/2004

*/

#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "d3dx8_shim.h"


#include <d3d8.h>
#include <d3dx8.h>
#include "maze.h"
#include "../UTILITIES/randomizer.h"
#include "../GAMECLIENT/gamerules.h"

class CMaterial;
class CDataFile;
class CModelTemplate;
class CRefManager;


enum EPropLocation
{
	KPropInRoom,
	KPropInHall,
	KPropNHallWall,
	KPropSHallWall,
	KPropEHallWall,
	KPropWHallWall,
	KPropNRoomWall,
	KPropSRoomWall,
	KPropERoomWall,
	KPropWRoomWall,
	KPropNWall,
	KPropSWall,
	KPropEWall,
	KPropWWall,
	KPropNStairsUp,
	KPropSStairsUp,
	KPropEStairsUp,
	KPropWStairsUp,
	KPropNStairsDown,
	KPropSStairsDown,
	KPropEStairsDown,
	KPropWStairsDown,
	KPropNSDoor,
	KPropEWDoor,
	KPropNTorch,
	KPropSTorch,
	KPropETorch,
	KPropWTorch,
	KPropWater,
	KPropPit,
	KPropAnywhere,
	KPropLocations
};

const std::string KPropLocationKeyword[KPropLocations] =
{
	"ROOM",
	"HALL",
	"NORTHHALLWALL",
	"SOUTHHALLWALL",
	"EASTHALLWALL",
	"WESTHALLWALL",
	"NORTHROOMWALL",
	"SOUTHROOMWALL",
	"EASTROOMWALL",
	"WESTROOMWALL",
	"NORTHWALL",
	"SOUTHWALL",
	"EASTWALL",
	"WESTWALL",
	"NORTHSTAIRSUP",
	"SOUTHSTAIRSUP",
	"EASTSTAIRSUP",
	"WESTSTAIRSUP",
	"NORTHSTAIRSDOWN",
	"SOUTHSTAIRSDOWN",
	"EASTSTAIRSDOWN",
	"WESTSTAIRSDOWN",
	"NSDOOR",
	"EWDOOR",
	"NORTHTORCH",
	"SOUTHTORCH",
	"EASTTORCH",
	"WESTTORCH",
	"WATER",
	"PIT",
	"ANYWHERE"
};


// prop templates hold placement and model information for individual props

class CPropTemplate
{

public:

	CPropTemplate( const std::string& Name,					// name of prop
				   const std::string& ModelPath,			// path to prop model path
				   const std::string& CollisionPath,		// path to prop model path
				   bool Collideable,						// collideable?
				   bool Shadow,								// casts shadow?
				   bool Rotateable,							// can be rotated about y axis?
				   uint32 MinimumBunch,						// minimum number
				   uint32 MaximumBunch,						// minimum number
				   const D3DXVECTOR3& MinimumOffset,		// minimum offset from center
				   const D3DXVECTOR3& MaximumOffset,		// minimum offset from center
				   float32 BaseRotation,					// starting rotation about y axis
				   float32 ScaleVariation ) :				// variation in base scale
												m_ModelPath( ModelPath ),					   
												m_TexturePath( "" ),					   
												m_CollisionPath( CollisionPath ),
												m_Collideable( Collideable ),
												m_MinimumBunch( MinimumBunch ),
												m_MaximumBunch( MaximumBunch ),
												m_MinimumOffset( MinimumOffset ),
												m_MaximumOffset( MaximumOffset ),
												m_Shadow( Shadow ),
												m_Name( Name ),
												m_Structural( kFalse ),
												m_RequiresFlat( kFalse ),
												m_HasLightPool( kFalse ),
												m_IsStairsUp( kFalse ),
												m_IsStairsDown( kFalse ),
												m_Rotateable( Rotateable ),
												m_BaseRotation( BaseRotation ),
												m_ScaleVariation( ScaleVariation ),
												m_Rarity( 1 ),
												m_BaseSoundRadius( 32 ),
												m_BaseSoundVolume( 255 ),
												m_ActivationRadius( 16 ),
												m_DeactivationRadius( 16 ),
												m_AmbientSoundPath( "" ),
												m_ActivationSoundPath( "" ),
												m_DeactivationSoundPath( "" ),
												m_PlacementRay( 0, 0, 0 ),
												m_PlacementRayHeight( 5 ),
												m_BufferN( kFalse ),
												m_BufferS( kFalse ),
												m_BufferE( kFalse ),
												m_BufferW( kFalse ),
												m_BufferNW( kFalse ),
												m_BufferNE( kFalse ),
												m_BufferSW( kFalse ),
												m_BufferSE( kFalse ),
												m_LightPoolPath ( "" ),
												m_LightPoolType ( KLightFlicker ),
												m_LightPoolRadius ( 0.0f )
	{
	}

	~CPropTemplate( void )
	{
	}

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void	SetRarity( int32 Rarity )			{	m_Rarity = Rarity;			};

	void	SetStructural( bool State )			{	m_Structural = State;		};

	void	SetRequiresFlat( bool State )		{	m_RequiresFlat = State;		};

	void	SetLightPool( const std::string& Path,	// path to light texture
						  float32 Radius,			// radius of light pool
						  ELightPoolType Type )		// flicker or pulse?
	{
		m_HasLightPool = kTrue;
		m_LightPoolRadius = Radius;
		m_LightPoolPath = Path;
		m_LightPoolType = Type;
	}

	void	SetTexturePath( const std::string& Path )			{	m_TexturePath = Path;			};

	void	SetAmbientSoundPath( const std::string& Path )		{	m_AmbientSoundPath = Path;			};
	void	SetActivationSoundPath( const std::string& Path )	{	m_ActivationSoundPath = Path;		};
	void	SetDeactivationSoundPath( const std::string& Path )	{	m_DeactivationSoundPath = Path;		};

	void	SetActivationRadius( float32 Radius )				{	m_ActivationRadius = Radius;		};
	void	SetDeactivationRadius( float32 Radius )				{	m_DeactivationRadius = Radius;		};

	void	SetBaseSoundRadius( float32 Radius )				{	m_BaseSoundRadius = Radius;			};
	
	void	SetBaseSoundVolume( uint32 Volume )					{	m_BaseSoundVolume = Volume;			};

	void	SetIsStairsUp( bool State )							{	m_IsStairsUp = State;				};
	void	SetIsStairsDown( bool State )						{	m_IsStairsDown = State;				};

	void	SetPlacementRay( const D3DXVECTOR3& Ray )			{	m_PlacementRay = Ray;				};

	void	SetPlacementRayHeight( float32 Height )				{	m_PlacementRayHeight = Height;		};

	void	SetBufferN( bool State )							{	m_BufferN = State;					};
	void	SetBufferS( bool State )							{	m_BufferS = State;					};
	void	SetBufferE( bool State )							{	m_BufferE = State;					};
	void	SetBufferW( bool State )							{	m_BufferW = State;					};
	void	SetBufferNW( bool State )							{	m_BufferNW = State;					};
	void	SetBufferNE( bool State )							{	m_BufferNE = State;					};
	void	SetBufferSW( bool State )							{	m_BufferSW = State;					};
	void	SetBufferSE( bool State )							{	m_BufferSE = State;					};

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	bool	BufferN( void )										{	return m_BufferN;					};
	bool	BufferS( void )										{	return m_BufferS;					};
	bool	BufferE( void )										{	return m_BufferE;					};
	bool	BufferW( void )										{	return m_BufferW;					};
	bool	BufferNW( void )									{	return m_BufferNW;					};
	bool	BufferNE( void )									{	return m_BufferNE;					};
	bool	BufferSW( void )									{	return m_BufferSW;					};
	bool	BufferSE( void )									{	return m_BufferSE;					};

	const std::string&	Name( void )			{	return m_Name;				};

	int32				Rarity( void )			{	return m_Rarity;			};

	bool				Structural( void )		{	return m_Structural;		};

	bool				RequiresFlat( void )	{	return m_RequiresFlat;		};

	bool				HasPlacementRay( void )	{	return D3DXVec3Length( &m_PlacementRay ) != 0;	};

	const D3DXVECTOR3&	PlacementRay( void )	{	return m_PlacementRay;		};

	float32				PlacementRayHeight( void )	{	return m_PlacementRayHeight;	};

	bool				HasLightPool( void )	{	return m_HasLightPool;		};

	float32				LightPoolRadius( void )	{	return m_LightPoolRadius;	};

	ELightPoolType		LightPoolType( void )	{	return m_LightPoolType;		};

	const std::string&	LightPoolPath( void )	{	return m_LightPoolPath;		};

	const std::string&	ModelPath( void )		{	return m_ModelPath;			};
	const std::string&	CollisionPath( void )	{	return m_CollisionPath;		};
	const std::string&	TexturePath( void )		{	return m_TexturePath;		};

	bool				Rotateable( void )		{	return m_Rotateable;		};

	bool				Shadow( void )			{	return m_Shadow;		};

	bool				Collideable( void )		{	return m_Collideable;		};

	float32				BaseRotation( void )	{	return m_BaseRotation;		};

	float32				ScaleVariation( void )	{	return m_ScaleVariation;	};

	uint32				MinimumBunch( void )	{	return m_MinimumBunch;		};
	uint32				MaximumBunch( void )	{	return m_MaximumBunch;		};


	const D3DXVECTOR3&	MinimumOffset( void )	{	return m_MinimumOffset;		};
	const D3DXVECTOR3&	MaximumOffset( void )	{	return m_MaximumOffset;		};

	float32				ActivationRadius( void )	{	return m_ActivationRadius;	};
	float32				DeactivationRadius( void )	{	return m_DeactivationRadius;	};

	const std::string&	AmbientSoundPath( void )		{	return m_AmbientSoundPath;				};
	const std::string&	ActivationSoundPath( void )		{	return m_ActivationSoundPath;			};
	const std::string&	DeactivationSoundPath( void )	{	return m_DeactivationSoundPath;			};


	float32				BaseSoundRadius( void )		{	return m_BaseSoundRadius;	};
	uint32				BaseSoundVolume( void )		{	return m_BaseSoundVolume;	};

	bool				IsStairsUp( void )			{	return m_IsStairsUp;		};
	bool				IsStairsDown( void )		{	return m_IsStairsDown;		};
private:

	int32				m_Rarity;

	std::string			m_ModelPath;
	std::string			m_CollisionPath;
	std::string			m_TexturePath;

	std::string			m_Name;

	bool				m_Structural;
	bool				m_RequiresFlat;

	bool				m_Rotateable;

	bool				m_Collideable;

	bool				m_Shadow;

	float32				m_BaseRotation;
	float32				m_ScaleVariation;

	uint32				m_MinimumBunch;
	uint32				m_MaximumBunch;

	D3DXVECTOR3			m_MinimumOffset;
	D3DXVECTOR3			m_MaximumOffset;

	std::string			m_LightPoolPath;
	ELightPoolType		m_LightPoolType;	
	float32				m_LightPoolRadius;
	bool				m_HasLightPool;

	float32				m_ActivationRadius;
	float32				m_DeactivationRadius;

	std::string			m_ActivationSoundPath;
	std::string			m_DeactivationSoundPath;
	std::string			m_AmbientSoundPath;

	float32				m_BaseSoundRadius;
	uint32				m_BaseSoundVolume;

	bool				m_IsStairsUp;
	bool				m_IsStairsDown;

	D3DXVECTOR3			m_PlacementRay;
	float32				m_PlacementRayHeight;

	bool				m_BufferN;
	bool				m_BufferS;
	bool				m_BufferE;
	bool				m_BufferW;
	bool				m_BufferNW;
	bool				m_BufferNE;
	bool				m_BufferSW;
	bool				m_BufferSE;
};

class CPropInstance
{
public:
	CPropInstance( CPropTemplate& pTemplate,		// prop this is an instance of
				   uint32 CellX,					// maze cell location
				   uint32 CellY,					// maze cell location
				   const D3DXVECTOR3& Offset ) :	// offset from center
													m_pTemplate( pTemplate ),
													m_CellX( CellX ),
													m_CellY( CellY ),
													m_Offset( Offset )

	{
		m_Angle = m_pTemplate.BaseRotation();

		if( m_pTemplate.Rotateable() )
		{
			m_Angle +=  RandomBetween( 0, 360 );
		}

		m_Scale = RandomBetween( 0, m_pTemplate.ScaleVariation() );
		m_Scale += 1.0f;

	}

	~CPropInstance( void )
	{
	};

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	const D3DXVECTOR3&	Offset( void )		{	return m_Offset;		};

	float32				Angle( void )		{	return m_Angle;			};
	float32				Scale( void )		{	return m_Scale;			};

	uint32				CellX( void )		{	return m_CellX;			};
	uint32				CellY( void )		{	return m_CellY;			};

	CPropTemplate&		Template( void )	{	return m_pTemplate;		};

private :

	CPropTemplate&	m_pTemplate;
	D3DXVECTOR3		m_Offset;

	float32			m_Angle;
	float32			m_Scale;
	uint32			m_CellX;
	uint32			m_CellY;
};

// a template contains paths to tiles, and their classifications
// for use in constructing a visual representation of a maze

class CTemplate
{

public:

	CTemplate( LPDIRECT3DDEVICE8 pD3DDevice,	// the direct3d device - needed to create vert and index buffers
			   CRefManager& pRefManager,		// the reference manager, for tracking submaterials
			   const std::string& Path );		// path to template
	~CTemplate( void );
	
////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	uint32	Tiles( uint32 WallIndex,		// index of wall set
				   EWallType TileType )		{	return m_pTileTemplate[WallIndex][TileType].size();	};


	float32				DarkStart( void )		{	return m_DarkStart;		};

	float32				DarkEnd( void )			{	return m_DarkEnd;		};

	float32				TileWidth( void )		{	return m_TileWidth;		};

	float32				TileHeight( void )		{	return m_TileHeight;	};

	float32				HeightDistortion( void )	{	return m_HeightDistortion;		};

	float32				XYDistortion( void )		{	return m_XYDistortion;			};

	uint32				RandomTerrain( void )		{	return m_TerrainRandomizer.GetRandom();	};

	uint32				CeilingTerrain( void )		{	return m_CeilingTerrain;		};

	CModelTemplate*		TileTemplate( uint32 WallIndex,		// index of wall set
									  EWallType TileType,	// type of tile
									  uint32 Index );		// index within this tile type

	CModelTemplate*		TileCollisionTemplate( uint32 WallIndex,	// index of wall set
											   EWallType TileType,	// type of tile
											   uint32 Index );		// index within this tile type

	uint32				TileTemplates( uint32 WallIndex,		// index of wall set
									  EWallType TileType );		// type of tile

	CMaterial*			WallMaterial( uint32 WallIndex )		{	return m_pWallMaterial[WallIndex];		};

	uint32				TileGroundLayer( uint32 WallIndex )		{	return m_TileGroundLayer[WallIndex];	};

	CMaterial*			GroundMaterial( uint32 Layer )			{	return m_pGroundMaterial[Layer];		};

	bool				GroundMaterialAvailable( uint32 Layer )	{	return m_GroundMaterialAvailable[Layer];};

	uint32				GroundLayers( void )					{	return m_pGroundMaterial.size();		};

	uint32				Props( void )							{	return m_pPropTemplate.size();			};

	CPropTemplate*		PropTemplate( uint32 Index )			{	return m_pPropTemplate[Index];			};

	CPropTemplate*		PropTemplate( std::string Name );

	uint32				TileTypes( void )						{	return m_pTileTemplate.size();				};

	bool				HasWater( void )						{	return m_HasWater;						};

	float32				WaterDepth( void )						{	return m_WaterDepth;					};

	float32				WaterSoundRadius( void )				{	return m_WaterSoundRadius;				};

	D3DCOLOR			WaterColor( void )						{	return m_WaterColor;		};

	const std::string&	WaterPath( void )						{	return m_WaterPath;						};
	const std::string&	WaterPathAlpha( void )					{	return m_WaterPathAlpha;						};

	const std::string&	WaterPathAVI( void )					{	return m_WaterPathAVI;						};

	const std::string&	WaterSoundPath( void )					{	return m_WaterSoundPath;				};

	CPropTemplate*		RandomProp( EPropLocation Location );	// location the prop must support

	uint32				MinimumWidth( void )	{	return m_MinimumWidth;		};

	uint32				MaximumWidth( void )	{	return m_MaximumWidth;		};

	uint32				MinimumHeight( void )	{	return m_MinimumHeight;		};

	uint32				MaximumHeight( void )	{	return m_MaximumHeight;		};

	uint32				MinimumRoomWidth( void )	{	return m_MinimumRoomWidth;		};

	uint32				MaximumRoomWidth( void )	{	return m_MaximumRoomWidth;		};

	uint32				MinimumRoomHeight( void )	{	return m_MinimumRoomHeight;		};

	uint32				MaximumRoomHeight( void )	{	return m_MaximumRoomHeight;		};

	uint32				MinimumPits( void )		{	return m_MinimumPits;		};

	uint32				MaximumPits( void )		{	return m_MaximumPits;		};

	uint32				MinimumRooms( void )	{	return m_MinimumRooms;		};

	uint32				MaximumRooms( void )	{	return m_MaximumRooms;		};

	uint32				MinimumTwistiness( void )		{	return m_MinimumTwistiness;		};

	uint32				MaximumTwistiness( void )		{	return m_MaximumTwistiness;		};

	uint32				MinimumConnectedness( void )	{	return m_MinimumConnectedness;		};

	uint32				MaximumConnectedness( void )	{	return m_MaximumConnectedness;		};

	uint32				MinimumSparseness( void )		{	return m_MinimumSparseness;		};

	uint32				MaximumSparseness( void )		{	return m_MaximumSparseness;		};

	uint32				MinimumRoundedness( void )		{	return m_MinimumRoundedness;		};

	uint32				MaximumRoundedness( void )		{	return m_MaximumRoundedness;		};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

private:

	CRefManager&	m_pRefManager;

	void	ParseTemplate( LPDIRECT3DDEVICE8 pD3DDevice,	// the direct3d device - needed to create vert and index buffers
						   const std::string& Path );		// path to template

	std::vector< std::vector< std::vector< CModelTemplate* > > >		m_pTileTemplate;
	std::vector< std::vector< std::vector< CModelTemplate* > > >		m_pTileCollisionTemplate;
	std::vector< CMaterial* >											m_pWallMaterial;
	std::vector< uint32 >								m_TileGroundLayer;

	float32						m_TileWidth;		// width/height of all tiles in the template
	float32						m_TileHeight;		// height(tallness) of all tiles in the template

	std::vector< CMaterial* >	m_pGroundMaterial;	// materials for each of the ground layers

	std::vector< bool >			m_GroundMaterialAvailable;	// available for random placement

	std::vector< CPropTemplate* >	m_pPropTemplate;

	std::vector< CPropTemplate* >	m_pPropTemplateForLocation[KPropLocations];
	CRandomizer					m_PropRandomizer[KPropLocations];

	CRandomizer					m_TerrainRandomizer;

	uint32						m_CeilingTerrain;

	std::string					m_WaterPath;
	std::string					m_WaterPathAlpha;
	std::string					m_WaterPathAVI;
	std::string					m_WaterSoundPath;
	bool						m_HasWater;
	float32						m_WaterDepth;
	float32						m_WaterSoundRadius;

	float32						m_HeightDistortion;
	float32						m_XYDistortion;

	uint32						m_MinimumWidth;
	uint32						m_MaximumWidth;

	uint32						m_MinimumHeight;
	uint32						m_MaximumHeight;

	uint32						m_MinimumRoomWidth;
	uint32						m_MaximumRoomWidth;

	uint32						m_MinimumRoomHeight;
	uint32						m_MaximumRoomHeight;

	uint32						m_MinimumPits;
	uint32						m_MaximumPits;

	uint32						m_MinimumRooms;
	uint32						m_MaximumRooms;

	uint32						m_MinimumTwistiness;
	uint32						m_MaximumTwistiness;

	uint32						m_MinimumConnectedness;
	uint32						m_MaximumConnectedness;

	uint32						m_MinimumSparseness;
	uint32						m_MaximumSparseness;

	uint32						m_MinimumRoundedness;
	uint32						m_MaximumRoundedness;

	float32						m_DarkStart;
	float32						m_DarkEnd;

	D3DCOLOR					m_WaterColor;
};

#endif