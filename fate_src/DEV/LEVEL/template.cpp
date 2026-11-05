/*
	template.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/17/2004

*/
#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/math3D.h"
#include "../UTILITIES/datafile.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "modeltemplate.h"

#include "maze.h"

#include "template.h"

CTemplate::CTemplate( LPDIRECT3DDEVICE8 pD3DDevice,	// the direct3d device - needed to create vert and index buffers
					  CRefManager& pRefManager,		// the reference manager, for tracking submaterials
					  const std::string& Path ) :	// path to template
													m_pRefManager( pRefManager ),
													m_WaterPath( "" ),
													m_WaterPathAlpha( "" ),
													m_WaterPathAVI( "" ),
													m_WaterSoundPath( "" ),
													m_HasWater( kFalse ),
													m_WaterSoundRadius( 64 ),
													m_WaterDepth( -4 ),
													m_MinimumWidth( 24 ),
													m_MaximumWidth( 48 ),
													m_MinimumHeight( 24 ),
													m_MaximumHeight( 48 ),
													m_MinimumRoomWidth( 3 ),
													m_MaximumRoomWidth( 9 ),
													m_MinimumRoomHeight( 3 ),
													m_MaximumRoomHeight( 9 ),
													m_MinimumRooms( 4 ),
													m_MaximumRooms( 12 ),
													m_MinimumPits( 0 ),
													m_MaximumPits( 12 ),
													m_CeilingTerrain( 0 ),
													m_MinimumTwistiness( 0 ),
													m_MaximumTwistiness( 1000 ),
													m_MinimumConnectedness( 0 ),
													m_MaximumConnectedness( 1000 ),
													m_MinimumSparseness( 0 ),
													m_MaximumSparseness( 1000 ),
													m_MinimumRoundedness( 0 ),
													m_MaximumRoundedness( 1000 ),
													m_DarkStart( 7 ),
													m_DarkEnd( 20 ),
													m_WaterColor( D3DCOLOR_RGBA( 64, 64, 128, 255 ) ),
													m_TileWidth( 0.0f ),
													m_TileHeight( 0.0f ),
													m_HeightDistortion( 0.0f ),
													m_XYDistortion( 0.0f )
{
						  
	ParseTemplate( pD3DDevice, Path );
} // CTemplate::CTemplate()

CTemplate::~CTemplate( void )
{
	for( uint32 i = 0; i < m_pPropTemplate.size(); i++ )
	{
		DELETE_SAFELY( m_pPropTemplate[i] );
	}
	m_pPropTemplate.clear();

	for( int32 i = 0; i < KPropLocations; i++ )
	{
		m_pPropTemplateForLocation[i].clear();
	}

	for( int32 i = 0; i < m_pTileTemplate.size(); i++ )
	{
		m_pRefManager.RemoveMaterial( m_pWallMaterial[i] );
		for( uint32 j = 0; j < m_pTileTemplate[i].size(); j++ )
		{
			for( uint32 k = 0; k < m_pTileTemplate[i][j].size(); k++ )
			{
				DELETE_SAFELY( m_pTileTemplate[i][j][k] );
			}
			m_pTileTemplate[i][j].clear();
		}
		m_pTileTemplate[i].clear();
	}
	m_pTileTemplate.clear();
	m_pWallMaterial.clear();

	for( int32 i = 0; i < m_pTileCollisionTemplate.size(); i++ )
	{
		for( uint32 j = 0; j < m_pTileCollisionTemplate[i].size(); j++ )
		{
			for( uint32 k = 0; k < m_pTileCollisionTemplate[i][j].size(); k++ )
			{
				DELETE_SAFELY( m_pTileCollisionTemplate[i][j][k] );
			}
			m_pTileCollisionTemplate[i][j].clear();
		}
		m_pTileCollisionTemplate[i].clear();
	}
	m_pTileCollisionTemplate.clear();


	for( int32 i = 0; i < m_pGroundMaterial.size(); i++ )
	{
		m_pRefManager.RemoveMaterial( m_pGroundMaterial[i] );
	}
	m_pGroundMaterial.clear();
} // CTemplate::~CTemplate()

void CTemplate::ParseTemplate( LPDIRECT3DDEVICE8 pD3DDevice,	// the direct3d device - needed to create vert and index buffers
							   const std::string& Path )		// path to template
{
	std::string BasePath = GetLocalPath();
	CDataFile* pDataFile = new CDataFile( Path );

	// parse tile widths
	CDataGroup* pGroup = pDataFile->GetDataGroup( "TILEWIDTH" );

	if( pGroup != NULL )
	{
		m_TileWidth = (float32)atof( pGroup->Item( 0 ).c_str() );
	}

	// parse tile heights
	pGroup = pDataFile->GetDataGroup( "TILEHEIGHT" );
	if( pGroup != NULL )
	{
		m_TileHeight = (float32)atof( pGroup->Item( 0 ).c_str() );
	}

	// parse maze parameters
	pGroup = pDataFile->GetDataGroup( "MINIMUMWIDTH" );
	if( pGroup != NULL )
	{
		m_MinimumWidth = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MAXIMUMWIDTH" );
	if( pGroup != NULL )
	{
		m_MaximumWidth = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MINIMUMHEIGHT" );
	if( pGroup != NULL )
	{
		m_MinimumHeight = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MAXIMUMHEIGHT" );
	if( pGroup != NULL )
	{
		m_MaximumHeight = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}

	pGroup = pDataFile->GetDataGroup( "MINIMUMROOMWIDTH" );
	if( pGroup != NULL )
	{
		m_MinimumRoomWidth = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MAXIMUMROOMWIDTH" );
	if( pGroup != NULL )
	{
		m_MaximumRoomWidth = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MINIMUMROOMHEIGHT" );
	if( pGroup != NULL )
	{
		m_MinimumRoomHeight = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MAXIMUMROOMHEIGHT" );
	if( pGroup != NULL )
	{
		m_MaximumRoomHeight = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	
	
	pGroup = pDataFile->GetDataGroup( "MINIMUMROOMS" );
	if( pGroup != NULL )
	{
		m_MinimumRooms = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MAXIMUMROOMS" );
	if( pGroup != NULL )
	{
		m_MaximumRooms = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MINIMUMPITS" );
	if( pGroup != NULL )
	{
		m_MinimumPits = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MAXIMUMPITS" );
	if( pGroup != NULL )
	{
		m_MaximumPits = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}

	// parse maze parameters
	pGroup = pDataFile->GetDataGroup( "MINIMUMTWISTINESS" );
	if( pGroup != NULL )
	{
		m_MinimumTwistiness = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MAXIMUMTWISTINESS" );
	if( pGroup != NULL )
	{
		m_MaximumTwistiness = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}

	// parse maze parameters
	pGroup = pDataFile->GetDataGroup( "MINIMUMCONNECTEDNESS" );
	if( pGroup != NULL )
	{
		m_MinimumConnectedness = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MAXIMUMCONNECTEDNESS" );
	if( pGroup != NULL )
	{
		m_MaximumConnectedness = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}

	// parse maze parameters
	pGroup = pDataFile->GetDataGroup( "MINIMUMSPARSENESS" );
	if( pGroup != NULL )
	{
		m_MinimumSparseness = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MAXIMUMSPARSENESS" );
	if( pGroup != NULL )
	{
		m_MaximumSparseness = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}

	// parse maze parameters
	pGroup = pDataFile->GetDataGroup( "MINIMUMROUNDEDNESS" );
	if( pGroup != NULL )
	{
		m_MinimumRoundedness = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "MAXIMUMROUNDEDNESS" );
	if( pGroup != NULL )
	{
		m_MaximumRoundedness = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}

	// parse water depth
	pGroup = pDataFile->GetDataGroup( "WATER_DEPTH" );

	if( pGroup != NULL )
	{
		m_WaterDepth = (float32)atof( pGroup->Item( 0 ).c_str() );
	}

	// parse water depth
	pGroup = pDataFile->GetDataGroup( "WATER_COLOR" );

	if( pGroup != NULL )
	{
		uint32 R = (uint32)atoi( pGroup->Item( 0 ).c_str() );
		uint32 G = (uint32)atoi( pGroup->Item( 1 ).c_str() );
		uint32 B = (uint32)atoi( pGroup->Item( 2 ).c_str() );
		m_WaterColor = D3DCOLOR_RGBA( R, G, B, 255 );
	}

	pGroup = pDataFile->GetDataGroup( "WATER_SOUND_RADIUS" );

	if( pGroup != NULL )
	{
		m_WaterSoundRadius = (float32)atof( pGroup->Item( 0 ).c_str() );
	}

	// parse water texture
	pGroup = pDataFile->GetDataGroup( "WATER" );

	if( pGroup != NULL )
	{
		m_WaterPath = pGroup->Item( 0 );
		m_WaterPathAlpha = pGroup->Item( 1 );
		m_WaterPathAVI = pGroup->Item( 2 );
		m_HasWater = kTrue;

	}

	// parse maze parameters
	pGroup = pDataFile->GetDataGroup( "DARK_START" );
	if( pGroup != NULL )
	{
		m_DarkStart = (float32)atoi( pGroup->Item( 0 ).c_str() );
	}
	pGroup = pDataFile->GetDataGroup( "DARK_END" );
	if( pGroup != NULL )
	{
		m_DarkEnd = (float32)atoi( pGroup->Item( 0 ).c_str() );
	}

	// parse water texture
	pGroup = pDataFile->GetDataGroup( "CEILING_TILE" );

	if( pGroup != NULL )
	{
		m_CeilingTerrain = (uint32)atoi( pGroup->Item( 0 ).c_str() );
	}

	if( m_HasWater )
	{
		// parse water sound
		pGroup = pDataFile->GetDataGroup( "WATER_SOUND" );

		if( pGroup != NULL )
		{
			m_WaterSoundPath = pGroup->Item( 0 );
		}
	}

	// parse height distortion max
	pGroup = pDataFile->GetDataGroup( "HEIGHT_DISTORTION" );

	if( pGroup != NULL )
	{
		m_HeightDistortion = (float32)atof( pGroup->Item( 0 ).c_str() );
	}

	// parse height distortion max
	pGroup = pDataFile->GetDataGroup( "XY_DISTORTION" );

	if( pGroup != NULL )
	{
		m_XYDistortion = (float32)atof( pGroup->Item( 0 ).c_str() );
	}

	uint32 WallSets = pDataFile->DataGroups( "WALL" );
	m_pTileTemplate.resize( WallSets );
	m_pTileCollisionTemplate.resize( WallSets );
	for( uint32 w = 0; w < WallSets; w++ )
	{
		m_pTileTemplate[w].resize( KWallTypes );
		m_pTileCollisionTemplate[w].resize( KWallTypes );

		// parse tile widths
		CDataGroup* pMasterGroup = pDataFile->GetDataGroup( "WALL", w );

		if( pMasterGroup != NULL )
		{
			CDataGroup* pLayerGroup = pMasterGroup->GetSubDataGroup( "TILELEVEL" );
			if( pLayerGroup != NULL )
			{
				m_TileGroundLayer.push_back( (uint32)atoi( pLayerGroup->Item( 0 ).c_str() ) );
			}
			pLayerGroup = pMasterGroup->GetSubDataGroup( "TEXTURE" );
			if( pLayerGroup != NULL )
			{
				m_pWallMaterial.push_back( m_pRefManager.AddMaterial( pD3DDevice, pLayerGroup->Item( 0 ) ) );
			}

			for( uint32 i = 0; i < KWallTypes; i++ )
			{
				for( uint32 j = 0; j < pMasterGroup->SubDataGroups( KWallTypeName[i] ); j++ )
				{
					CDataGroup* pGroup = pMasterGroup->GetSubDataGroup( KWallTypeName[i], j );

					std::string Path = pGroup->Item( 0 );

					Path = AssembleAbsolutePath( BasePath,
												Path );

					CModelTemplate* pTemplate = new CModelTemplate( m_pRefManager );
					pTemplate->Load( pD3DDevice, Path, "", kTrue, kTrue );
					m_pTileTemplate[w][i].push_back( pTemplate );

					if( pGroup->Items() > 1 )
					{
						Path = pGroup->Item( 1 );

						Path = AssembleAbsolutePath( BasePath,
													Path );
					}

					pTemplate = new CModelTemplate( m_pRefManager );
					pTemplate->Load( pD3DDevice, Path, "", kTrue, kTrue );
					m_pTileCollisionTemplate[w][i].push_back( pTemplate );
					
				}
			}
		}
	}

	// parse ground layers
	CDataGroup* pMasterGroup = pDataFile->GetDataGroup( "GROUND" );

	uint32 Layers = pMasterGroup->SubDataGroups( "TILES" );
	m_pGroundMaterial.resize( Layers );
	m_GroundMaterialAvailable.resize( Layers );
	
	for( uint32 j = 0; j < Layers; j++ )
	{
		CDataGroup* pGroup = pMasterGroup->GetSubDataGroup( "TILES", j );

		uint32		Index = (uint32)atoi( pGroup->Item( 0 ).c_str() );
		bool		Available = ( atoi( pGroup->Item( 1 ).c_str() ) == 1 );
		uint32		Rarity = 1000 - (uint32)atoi( pGroup->Item( 2 ).c_str() );
		std::string Path( pGroup->Item( 3 ) );
		std::string AlphaPath( "" );
		if( pGroup->Items() > 3 )
		{
			AlphaPath = pGroup->Item( 4 );
		}
		if( Available )
		{
			m_TerrainRandomizer.AddChoice( j, Rarity );
		}

		m_GroundMaterialAvailable[Index] = Available;
		m_pGroundMaterial[Index] = m_pRefManager.AddMaterialAlpha( pD3DDevice, Path, AlphaPath );
		D3DMATERIAL8& Material = m_pGroundMaterial[Index]->D3DMaterial();

		Material.Diffuse.r	= 1.0f;					
		Material.Diffuse.g	= 1.0f;
		Material.Diffuse.b	= 1.0f;
		Material.Diffuse.a	= 1.0f;

		Material.Specular.r	= 0.0F;
		Material.Specular.g	= 0.0F;
		Material.Specular.b	= 0.0F;

		Material.Power		= 0.0f;

		Material.Ambient.r	= 0.15f;						
		Material.Ambient.g	= 0.15f;
		Material.Ambient.b	= 0.15f;
		Material.Ambient.a	= 1.0f;

		Material.Emissive.r = 0.0f;
		Material.Emissive.g = 0.0f;
		Material.Emissive.b = 0.0f;
		Material.Emissive.a = 0.0f;

	}


	uint32 Props = pDataFile->DataGroups( "PROP" );

	for( uint32 p = 0; p < Props; p++ )
	{
		std::string		Name( "" );
		std::string		ModelPath( "" );
		std::string		CollisionPath( "" );
		bool			Collideable( kTrue );
		bool			Shadow( kTrue );
		bool			Rotateable( kTrue );
		float32			BaseRotation( 0 );
		float32			ScaleVariation( 0 );
		uint32			MinimumBunch( 1 );
		uint32			MaximumBunch( 1 );
		D3DXVECTOR3		MinimumOffset( 0, 0, 0 );
		D3DXVECTOR3		MaximumOffset( 0, 0, 0 );

		EPropLocation	PropLocation( KPropAnywhere );
		
		// parse prop templates
		CDataGroup* pMasterGroup = pDataFile->GetDataGroup( "PROP", p );

		CDataGroup* pPathGroup = pMasterGroup->GetSubDataGroup( "MODEL" );
		if( pPathGroup != NULL )
		{
			ModelPath =  pPathGroup->Item( 0 );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "NAME" );
		if( pPathGroup != NULL )
		{
			Name =  pPathGroup->Item( 0 );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "COLLISION" );
		if( pPathGroup != NULL )
		{
			CollisionPath =  pPathGroup->Item( 0 );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "ROTATEABLE" );
		if( pPathGroup != NULL )
		{
			Rotateable = atoi( pPathGroup->Item( 0 ).c_str() ) == 1;
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "COLLIDEABLE" );
		if( pPathGroup != NULL )
		{
			Collideable = atoi( pPathGroup->Item( 0 ).c_str() ) == 1;
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "SHADOW" );
		if( pPathGroup != NULL )
		{
			Shadow = atoi( pPathGroup->Item( 0 ).c_str() ) == 1;
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "BASE_ROTATION" );
		if( pPathGroup != NULL )
		{
			BaseRotation = (float32)atof( pPathGroup->Item( 0 ).c_str() );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "SCALE_VARIATION" );
		if( pPathGroup != NULL )
		{
			ScaleVariation = (float32)atof( pPathGroup->Item( 0 ).c_str() );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "MINIMUM_BUNCH" );
		if( pPathGroup != NULL )
		{
			MinimumBunch = (uint32)atoi( pPathGroup->Item( 0 ).c_str() );
			if( MinimumBunch < 1 )
			{
				MinimumBunch = 1;
			}
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "MAXIMUM_BUNCH" );
		if( pPathGroup != NULL )
		{
			MaximumBunch = (uint32)atoi( pPathGroup->Item( 0 ).c_str() );
			if( MaximumBunch > 10 )
			{
				MaximumBunch = 10;
			}
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "MINIMUM_OFFSET" );
		if( pPathGroup != NULL )
		{
			MinimumOffset.x = (float32)atof( pPathGroup->Item( 0 ).c_str() );
			MinimumOffset.y = (float32)atof( pPathGroup->Item( 1 ).c_str() );
			MinimumOffset.z = (float32)atof( pPathGroup->Item( 2 ).c_str() );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "MAXIMUM_OFFSET" );
		if( pPathGroup != NULL )
		{
			MaximumOffset.x = (float32)atof( pPathGroup->Item( 0 ).c_str() );
			MaximumOffset.y = (float32)atof( pPathGroup->Item( 1 ).c_str() );
			MaximumOffset.z = (float32)atof( pPathGroup->Item( 2 ).c_str() );
		}

		CPropTemplate* pProp = new CPropTemplate( Name,
												  ModelPath,
												  CollisionPath,
												  Collideable,
												  Shadow,
												  Rotateable,
												  MinimumBunch,
												  MaximumBunch,
												  MinimumOffset,
												  MaximumOffset,
												  BaseRotation,
												  ScaleVariation );
		m_pPropTemplate.push_back( pProp );

		pPathGroup = pMasterGroup->GetSubDataGroup( "TEXTUREPATH" );
		if( pPathGroup != NULL )
		{
			pProp->SetTexturePath( pPathGroup->Item( 0 ) );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "RARITY" );
		if( pPathGroup != NULL )
		{
			int32 Rarity = atoi( pPathGroup->Item( 0 ).c_str() );
			Rarity = 1000 - Rarity;
			if( Rarity < 0 )
			{
				Rarity = 0;
			}
			pProp->SetRarity( Rarity );
		}
		else
		{
			pProp->SetRarity( 1000 );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "STRUCTURAL" );
		if( pPathGroup != NULL )
		{
			pProp->SetStructural( atoi( pPathGroup->Item( 0 ).c_str() ) == 1 );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "REQUIRES_FLAT" );
		if( pPathGroup != NULL )
		{
			pProp->SetRequiresFlat( atoi( pPathGroup->Item( 0 ).c_str() ) == 1 );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "STAIRS_UP" );
		if( pPathGroup != NULL )
		{
			pProp->SetIsStairsUp( atoi( pPathGroup->Item( 0 ).c_str() ) == 1 );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "STAIRS_DOWN" );
		if( pPathGroup != NULL )
		{
			pProp->SetIsStairsDown( atoi( pPathGroup->Item( 0 ).c_str() ) == 1 );
		}

		uint32 Locations = pMasterGroup->SubDataGroups( "LOCATION" );
		if( Locations > 0 )
		{
			for( uint32 m = 0; m < Locations; m++ )
			{
				pPathGroup = pMasterGroup->GetSubDataGroup( "LOCATION", m );
	
				std::string Keyword( StringUpper( pPathGroup->Item( 0 ) ) );
				if( Keyword == "ANYWHERE" )
				{
					for( uint32 l = 0; l < KPropNStairsUp; l++ )
					{
						m_PropRandomizer[l].AddChoice( m_pPropTemplateForLocation[l].size(), pProp->Rarity() );
						m_pPropTemplateForLocation[l].push_back( pProp );
					} 
				}
				else
				{
					EPropLocation Location( KPropAnywhere );
					for( uint32 l = 0; l < KPropLocations; l++ )
					{
						if( Keyword == KPropLocationKeyword[l] )
						{
							PropLocation = (EPropLocation)l;
							m_PropRandomizer[l].AddChoice( m_pPropTemplateForLocation[l].size(), pProp->Rarity() );
							m_pPropTemplateForLocation[l].push_back( pProp );
							break;
						}
					} 
				}
			}
		}
		else
		{
			for( uint32 l = 0; l < KPropNStairsUp; l++ )
			{
				m_PropRandomizer[l].AddChoice( m_pPropTemplateForLocation[l].size(), pProp->Rarity() );
				m_pPropTemplateForLocation[l].push_back( pProp );
			} 
		}

		// non-standard effects that may be applied
		pPathGroup = pMasterGroup->GetSubDataGroup( "LIGHT" );
		if( pPathGroup != NULL )
		{
			std::string LightPath =  pPathGroup->Item( 0 );
			float32 Radius = (float32)atof( pPathGroup->Item( 1 ).c_str() );
			ELightPoolType LightType( KLightFlicker );
			if( StringUpper( pPathGroup->Item( 2 ) ) == "PULSE" )
			{
				LightType = KLightPulse;
			}
			pProp->SetLightPool( LightPath, Radius, LightType );
			
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "BUFFER_N" );
		if( pPathGroup != NULL )
		{
			pProp->SetBufferN( atoi( pPathGroup->Item( 0 ).c_str() ) == 1 );
		}
		pPathGroup = pMasterGroup->GetSubDataGroup( "BUFFER_S" );
		if( pPathGroup != NULL )
		{
			pProp->SetBufferS( atoi( pPathGroup->Item( 0 ).c_str() ) == 1 );
		}
		pPathGroup = pMasterGroup->GetSubDataGroup( "BUFFER_E" );
		if( pPathGroup != NULL )
		{
			pProp->SetBufferE( atoi( pPathGroup->Item( 0 ).c_str() ) == 1 );
		}
		pPathGroup = pMasterGroup->GetSubDataGroup( "BUFFER_W" );
		if( pPathGroup != NULL )
		{
			pProp->SetBufferW( atoi( pPathGroup->Item( 0 ).c_str() ) == 1 );
		} 
		pPathGroup = pMasterGroup->GetSubDataGroup( "BUFFER_NW" );
		if( pPathGroup != NULL )
		{
			pProp->SetBufferNW( atoi( pPathGroup->Item( 0 ).c_str() ) == 1 );
		}
		pPathGroup = pMasterGroup->GetSubDataGroup( "BUFFER_NE" );
		if( pPathGroup != NULL )
		{
			pProp->SetBufferNE( atoi( pPathGroup->Item( 0 ).c_str() ) == 1 );
		}
		pPathGroup = pMasterGroup->GetSubDataGroup( "BUFFER_SW" );
		if( pPathGroup != NULL )
		{
			pProp->SetBufferSW( atoi( pPathGroup->Item( 0 ).c_str() ) == 1 );
		}
		pPathGroup = pMasterGroup->GetSubDataGroup( "BUFFER_SE" );
		if( pPathGroup != NULL )
		{
			pProp->SetBufferSE( atoi( pPathGroup->Item( 0 ).c_str() ) == 1 );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "PLACEMENT_RAY" );
		if( pPathGroup != NULL )
		{
			float32 x = (float32)atof( pPathGroup->Item( 0 ).c_str() );
			float32 y = (float32)atof( pPathGroup->Item( 1 ).c_str() );
			float32 z = (float32)atof( pPathGroup->Item( 2 ).c_str() );
			pProp->SetPlacementRay( D3DXVECTOR3( x, y, z ) );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "RAY_HEIGHT" );
		if( pPathGroup != NULL )
		{
			float32 y = (float32)atof( pPathGroup->Item( 0 ).c_str() );
			pProp->SetPlacementRayHeight( y );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "ACTIVATE_RADIUS" );
		if( pPathGroup != NULL )
		{
			float32 Radius = (float32)atof( pPathGroup->Item( 0 ).c_str() );
			pProp->SetActivationRadius( Radius );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "DEACTIVATE_RADIUS" );
		if( pPathGroup != NULL )
		{
			float32 Radius = (float32)atof( pPathGroup->Item( 0 ).c_str() );
			pProp->SetDeactivationRadius( Radius );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "ACTIVATE_SOUND" );
		if( pPathGroup != NULL )
		{
			pProp->SetActivationSoundPath( pPathGroup->Item( 0 ) );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "DEACTIVATE_SOUND" );
		if( pPathGroup != NULL )
		{
			pProp->SetDeactivationSoundPath( pPathGroup->Item( 0 ) );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "AMBIENT_SOUND" );
		if( pPathGroup != NULL )
		{
			pProp->SetAmbientSoundPath( pPathGroup->Item( 0 ) );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "BASE_SOUND_RADIUS" );
		if( pPathGroup != NULL )
		{
			float32 Radius = (float32)atof( pPathGroup->Item( 0 ).c_str() );
			pProp->SetBaseSoundRadius( Radius );
		}

		pPathGroup = pMasterGroup->GetSubDataGroup( "BASE_SOUND_VOLUME" );
		if( pPathGroup != NULL )
		{
			uint32 Volume = (uint32)atoi( pPathGroup->Item( 0 ).c_str() );
			pProp->SetBaseSoundVolume( Volume );
		}

	}

	DELETE_SAFELY( pDataFile );

} // CTemplate::ParseTemplate()


CModelTemplate* CTemplate::TileTemplate( uint32 WallIndex,		// index of wall set
										 EWallType TileType,	// type of tile
									 	 uint32 Index )		// index within this tile type list
{
	assert( WallIndex < m_pTileTemplate.size() );
	assert( Index < m_pTileTemplate[WallIndex][TileType].size() );

	return m_pTileTemplate[WallIndex][TileType][Index];
} // CTemplate::TileTemplate()

CModelTemplate* CTemplate::TileCollisionTemplate( uint32 WallIndex,		// index of wall set
												  EWallType TileType,	// type of tile
									 			  uint32 Index )		// index within this tile type list
{
	assert( WallIndex < m_pTileCollisionTemplate.size() );
	assert( Index < m_pTileCollisionTemplate[WallIndex][TileType].size() );

	return m_pTileCollisionTemplate[WallIndex][TileType][Index];
} // CTemplate::TileCollisionTemplate()

uint32  CTemplate::TileTemplates( uint32 WallIndex,		// index of wall set
								  EWallType TileType )	// type of tile
{
	if( WallIndex >= m_pTileTemplate.size() )
	{
		return 0;
	}
	if( TileType >= (int32)m_pTileTemplate[WallIndex].size() )
	{
		return 0;
	}

	return m_pTileTemplate[WallIndex][TileType].size();
} // CTemplate::TileTemplates()

CPropTemplate* CTemplate::RandomProp( EPropLocation Location )	// location the prop must support
{
	if( m_pPropTemplateForLocation[Location].size() == 0 )
	{
		return NULL;
	}
	uint32 Prop = m_PropRandomizer[Location].GetRandom();
	return m_pPropTemplateForLocation[Location][Prop];
/*

	uint32 Prop = rand() % m_pPropTemplateForLocation[Location].size();
	if( Prop >= m_pPropTemplateForLocation[Location].size() )
	{
		Prop = 0;
	}
	return m_pPropTemplateForLocation[Location][Prop];

	return NULL;*/
} // CTemplate::RandomProp()

CPropTemplate* CTemplate::PropTemplate( std::string Name )	// name of character to retriev
{	
	Name = StringUpper( Name );
	for( uint32 i = 0; i < m_pPropTemplate.size(); i++ )
	{
		if( StringUpper( m_pPropTemplate[i]->Name() ) == Name )
		{
			return m_pPropTemplate[i];
		}
	}
	return NULL;
} // CTemplate::PropDescription()
