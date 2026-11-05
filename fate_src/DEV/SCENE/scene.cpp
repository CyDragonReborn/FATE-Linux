/*
	scene.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	Change History :

	Travis Baldree
	4/14/2004	-	double setvertexshader removed in render method
					changed return type for lights/points/paths from pointers to refs

	Travis Baldree
	4/16/2004	-	Added support for point orientations

	Travis Baldree
	5/1/2004	-	Added lightmapping support and totally changed the format.

	Travis Baldree
	5/3/2004	-	Can now do a multiply layer. Fixed some lightmapping issues with
					multiple lightmaps. 
					Shifters now work on the second layer too.
					Materials can now be marked as visible

	Travis Baldree
	5/7/2004	-	Reflective surfaces no longer return a need to render when they
					are facing away from the supplied frustum even if they are in view

	Travis Baldree
	5/17/2004	-	textures with embedded alpha are now recognized and loaded properly
					into their appropriate material bank

	Travis Baldree
	5/27/2004	-	now must specify whether you allow automatic dynamic reflections at load time
					this is useful for games, like racing games, where you are generating one
					reflection for the entire scene, and don't need an individual reflection per material.
					That way the rendertotextures don't get erroneously created for all reflective
					materials. This has a corresponding change in the refmanager.

	6/29/2004	-	Added support for glow pass, to be used in conjunction with the BlurLayer object

	8/24/2004	-	Added support for post-rendered reflections (fully additive reflections)

	11/15/2004	-	Material IDS were not being added to the collideable face list. Oops!
*/

#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/math3D.h"
#include "../UTILITIES/vertexdefinitions.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/capabilities.h"

#include "sceneoctree.h"
#include "scenequadtree.h"
#include "scene.h"
#include "subscene.h"
#include "subscenesegment.h"
#include "../MODEL/facelist.h"

#include "../MEDIA/refmanager.h"

#include "../MEDIA/material.h"


// Descending z sorting function
struct ZSort
{
     bool operator()(CSubSceneSegment*& Start, CSubSceneSegment*& End)
     {
          return Start->ZDistance() > End->ZDistance();
     }
};



CScene::CScene( CRefManager& pRefManager,	// pointer to the reference manager, for tracking submaterials
				CSettings& pSettings ) :	// pointer to the settings manager, for keeping track of card/machine capabilities
											m_pRefManager( pRefManager ),
											m_pSettings( pSettings ),
											m_HasReflection( kFalse ),
											m_HasCollision( kFalse ),
											m_RenderAlpha( kTrue ),
											m_RenderOpaque( kTrue ),
											m_HasAlpha ( kFalse ),
											m_IsTextured ( kFalse ),
											m_TotalSubMeshes( 0 ),
											m_TotalFaces( 0 ),
											m_TotalMaterials( 0 ),
											m_AmbientR( 0 ),
											m_AmbientG( 0 ),
											m_AmbientB( 0 ),
											m_FogColor( D3DCOLOR_ARGB( 0, 100, 100, 100 ) ),
											m_AmbientColor( D3DCOLOR_ARGB( 0, 0, 0, 0 ) ),
											m_pFaceList( NULL ),
											m_pOctree( NULL ),
											m_pQuadtree( NULL ),
											m_pCollisionOctree( NULL ),
											m_pCollisionQuadtree( NULL ),
											m_CurrentVertexShader( D3DFVF_PLAINVERTEXSCENE ),
											m_LocalMinBounds ( 0, 0, 0 ),
											m_LocalMaxBounds ( 0, 0, 0 )
{
	ZeroMemory ( &m_ProjectorBias, sizeof ( m_ProjectorBias ));
	// set up a projector bias for use in the default reflection projections
	float32	Offset	= .5f + ( .5f / 256 );

	m_ProjectorBias._11 = 0.5f; 
	m_ProjectorBias._12 = 0.0f; 
	m_ProjectorBias._13 = 0.0f; 
	m_ProjectorBias._14 = 0;

	m_ProjectorBias._21 = 0.0f; 
	m_ProjectorBias._22 = -0.5f; 
	m_ProjectorBias._23 = 0.0f; 
	m_ProjectorBias._24 = 0;

	m_ProjectorBias._31 = 0.0f; 
	m_ProjectorBias._32 = 0.0f; 
	m_ProjectorBias._33 = 1.0f; 
	m_ProjectorBias._34 = 0;

	m_ProjectorBias._41 = Offset; 
	m_ProjectorBias._42 = Offset; 
	m_ProjectorBias._43 = 0.0f; 
	m_ProjectorBias._44 = 1.0f;
		
} // CScene::CScene()


CScene::~CScene()
{


	DELETE_SAFELY( m_pOctree );
	DELETE_SAFELY( m_pQuadtree );
	DELETE_SAFELY( m_pCollisionOctree );
	DELETE_SAFELY( m_pCollisionQuadtree );
	
	DELETE_SAFELY( m_pFaceList );

	// release all scene-level material references
	for( uint32 i = 0; i < m_pMaterialReferences.size(); i++ )
	{
		m_pRefManager.RemoveMaterial( m_pMaterialReferences[i] );
		m_pMaterialReferences[i] = NULL;
	}

	for( int32 i = 0; i < m_ModelList.size(); i++ )
	{
		DELETE_SAFELY( m_ModelList[i] );
	}
	m_ModelList.clear();

	for( int32 i =0; i < m_Lights.size(); i++ )
	{
		DELETE_SAFELY( m_Lights[i] );
	}
	m_Lights.clear();

	for( int32 i =0; i < m_Paths.size(); i++ )
	{
		DELETE_SAFELY( m_Paths[i] );
	}
	m_Paths.clear();

	for( int32 i =0; i < m_Points.size(); i++ )
	{
		DELETE_SAFELY( m_Points[i] );
	}
	m_Points.clear();


} // CScene::~CScene()

CMaterial* CScene::GetMaterial( uint32 Index )
{
	assert( Index < m_pMaterialReferences.size() );

	return m_pMaterialReferences[Index];
} // CScene::GetMaterial()

// see if any reflective materials exist in this scene
bool CScene::HasReflection( void )
{
	if( m_pMaterialSegments[KMaterialReflect].size() > 0)
	{
		return kTrue;
	}
	if( m_pMaterialSegments[KMaterialReflectPost].size() > 0)
	{
		return kTrue;
	}
	return kFalse;
} // CScene::HasReflection()

void CScene::Load( LPDIRECT3DDEVICE8 pD3DDevice,		// the direct3d device - needed to create vert and index buffers
				   std::string FileName,				// path to the mesh file
				   std::string TexturePath,				// path to the textures (if "" is supplied, will use the mesh path)
				   bool AllowCollision,					// is this mesh collideable?
				   bool AllowRealtimeReflection,		// allow automatic real-time reflection? ( mirrors )
				   ESortType SortType )					// type of sorting to use (quadtree is default)
						   
{
	uint32 ReflectionSize = m_pSettings.Capabilities().GetCapabilityValue( KValueMaxRenderToTextureSize );

	m_TotalFaces = 0;

	m_HasCollision = AllowCollision;

	FILE*	File	= NULL;

	File = fopen( FileName.c_str(), "rb" );

	if ( File == NULL )
	{
		return;
	}

	// If no path for textures was provided, construct one
	// from the file path by removing the filename and using its path
	if ( TexturePath.length() == 0 )
	{
		int32	LastSlash	= FileName.rfind( "/" );
		int32	LastBackSlash	= FileName.rfind( "\\" );
		if( LastSlash > LastBackSlash )
		{
			TexturePath = FileName.substr( 0, LastSlash + 1 );
		}
		else
		{
			TexturePath = FileName.substr( 0, LastBackSlash + 1 );
		}
	}

	bool	BoundsSet		( kFalse );
	int32	MeshVertices	( 0 );



	std::vector <uint32>	MaterialCounts			( KMaterialTypes );

	for( uint32 i = 0; i < MaterialCounts.size(); i++ )
	{
		MaterialCounts[i] = 0;
	}

	float32 FogR = ReadFloat( File );
	float32 FogG = ReadFloat( File );
	float32 FogB = ReadFloat( File );

	m_FogColor = D3DCOLOR_ARGB( 0,
							    (int32) ( FogR * 255 ),
							    (int32) ( FogG * 255 ),
							    (int32) ( FogB * 255 ) );

	m_AmbientR = ReadFloat( File );
	m_AmbientG = ReadFloat( File );
	m_AmbientB = ReadFloat( File );

	m_AmbientColor = D3DCOLOR_ARGB( 0,
									(int32) ( m_AmbientR * 255 ),
								    (int32) ( m_AmbientG * 255 ),
								    (int32) ( m_AmbientB * 255 ) );

	// first block of the model file - basic info about the mesh
	float32	SceneScale			( ReadFloat( File ) );				// scale of the entire model
	uint32	Objects				( ReadInt( File ) );				// how many sub-objects are in the model
	uint32	Lights				( ReadInt( File ) );				// how many lights are in the model
	uint32	Points				( ReadInt( File ) );				// how many glows are in the model
	uint32	Paths				( ReadInt( File ) );				// how many lights are in the model
	uint32	TotalMaterials		( ReadInt( File ) );				// how many individual materials are applied
	uint32	Textures			( ReadInt( File ) );				// how many unique textures appear

	m_TotalMaterials = TotalMaterials;

	uint32	m_Materials			( m_TotalMaterials );

	if ( m_TotalMaterials <= 0 )
	{
		m_TotalMaterials = 1;
	}

	// allocate a vector of pointers to the materials that will be loaded

	m_pMaterialReferences.resize( m_TotalMaterials );

	std::vector <std::string>	TextureNames			( Textures );

	std::vector <bool>			MaterialHasColorKey		( m_TotalMaterials );
	std::vector <DWORD>			MaterialColorKey		( m_TotalMaterials );
	std::vector <bool>			MaterialIsDoubleSided	( m_TotalMaterials );
	std::vector <int32>			MaterialCollideable		( m_TotalMaterials );
	std::vector <int32>			MaterialVisible			( m_TotalMaterials );
	std::vector <EMaterialType>	MaterialType			( m_TotalMaterials );
	std::vector <int32>			SubIndex				( m_TotalMaterials );

	// second block of the file - texture names are parsed
	for ( uint32 j = 0; j < Textures; j++ )
	{
		// we convert everything to upper case for ease of comparison later
		std::string	TexturePath	= StringUpper( ReadString( File ) );
		int32		id			= ReadInt( File );

		// trim off any TexturePath that appears before the filename 
		int32		last		( TexturePath.find_last_of( '\\' ) + 1 );
		TexturePath = TexturePath.substr( last, TexturePath.length() - last );

		TextureNames[id] = TexturePath;
	}


	// after the texture names have been parsed, we load the material data, which
	// references the textures

	m_MaterialNames.resize( m_TotalMaterials );

	m_pMaterialSegments.resize( KMaterialTypes );
	m_pSortedMaterialSegments.resize( KMaterialTypes );
	for( int32 j = 0; j < KMaterialTypes; j ++ )
	{
		m_pMaterialSegments[j].resize( m_TotalMaterials );
		m_pSortedMaterialSegments[j].resize( m_TotalMaterials );
	}


	// load each material
 	for( int32 j = 0; j < m_Materials ; j++ )
	{
		bool	Duplicate			( kFalse );
		bool	Opacity				( kFalse );
		int32	DoubleSided			( 0 );
		int32	HasColorKey			( 0 );
		int32	RenderLast			( 0 );
		int32	RenderFirst			( 0 );
		int32	DataID				( 0 );

		int32	BehaviorShifting	( 0 );
		int32	BehaviorFlipbook	( 0 );
		int32	FlipbookWFrames		( 0 );
		int32	FlipbookHFrames		( 0 );
		float32	ShiftU				( 0 );
		float32	ShiftV				( 0 );
		float32	FlipbookTime		( 0 );

		int32	BehaviorShifting2	( 0 );
		int32	BehaviorFlipbook2	( 0 );
		int32	FlipbookWFrames2	( 0 );
		int32	FlipbookHFrames2	( 0 );
		float32	ShiftU2				( 0 );
		float32	ShiftV2				( 0 );
		float32	FlipbookTime2		( 0 );

		// load the material's name
		m_MaterialNames[j] = ReadString( File );

		// parse the material ID (this is what the faces will reference, not the order in which it was loaded)
		int32	MaterialID	= ReadInt( File );

		// is the material double sided? (new polys will be generated if it is)
		DoubleSided = ReadInt( File );

		// should we put this material's faces in the collision volume?
		MaterialCollideable[MaterialID] = ReadInt( File );

		// should we render this material's faces - or only collide?
		MaterialVisible[MaterialID] = ReadInt( File );

		// user data, maybe to be used for LOD during rendering
		DataID = ReadInt( File );

		// has a colorkey 
		HasColorKey = ReadInt( File );
		if( HasColorKey )
		{
			uint32 r = ReadShort( File );
			uint32 g = ReadShort( File );
			uint32 b = ReadShort( File );

			MaterialColorKey[MaterialID] = D3DCOLOR_ARGB( 255, r, g, b );
		}
		// shifting behavior of the material (scrolling)
		BehaviorShifting = ReadInt( File );
		if( BehaviorShifting )
		{
			ShiftU = ReadFloat( File );
			ShiftV = ReadFloat( File );
		}
		// flipbook behavior of the material 
		BehaviorFlipbook = ReadInt( File );
		if( BehaviorFlipbook )
		{
			FlipbookWFrames = ReadShort( File );
			FlipbookHFrames = ReadShort( File );
			FlipbookTime = ReadFloat( File );
		}

		// shifting behavior of the material for the second layer (scrolling)
		BehaviorShifting2 = ReadInt( File );
		if( BehaviorShifting2 )
		{
			ShiftU2 = ReadFloat( File );
			ShiftV2 = ReadFloat( File );
		}
		// flipbook behavior of the material for the second layer 
		BehaviorFlipbook2 = ReadInt( File );
		if( BehaviorFlipbook2 )
		{
			FlipbookWFrames2 = ReadShort( File );
			FlipbookHFrames2 = ReadShort( File );
			FlipbookTime2 = ReadFloat( File );
		}

		// render this material after others
		RenderLast = ReadInt( File );
		// render this material before others
		RenderFirst = ReadInt( File );


		// now we parse material properties - diffuse, ambient, specular, spec power
		float32 DiffuseR( ReadShort( File ) / 255.0f );
		float32 DiffuseG( ReadShort( File ) / 255.0f );
		float32 DiffuseB( ReadShort( File ) / 255.0f );

		float32 AmbientR( ReadShort( File ) / 255.0f );
		float32 AmbientG( ReadShort( File ) / 255.0f );
		float32 AmbientB( ReadShort( File ) / 255.0f );

		float32 SpecularR( ReadShort( File ) / 255.0f );
		float32 SpecularG( ReadShort( File ) / 255.0f );
		float32 SpecularB( ReadShort( File ) / 255.0f );

		float32 EmissiveR( ReadShort( File ) / 255.0f );
		float32 EmissiveG( ReadShort( File ) / 255.0f );
		float32 EmissiveB( ReadShort( File ) / 255.0f );

		float32 Shininess( ReadFloat( File ) );
		float32 ShininessStrength( ReadFloat( File ) );

		// default to a standard material with no behavior
		MaterialType[MaterialID] = KMaterialStandard;

		// load all of the texture ids
		int32	DiffuseMapID		= ReadInt( File );
		int32	OpacityMapID		= ReadInt( File );
		int32	ReflectionMapID		= ReadInt( File );
		int32	IlluminationMapID	= ReadInt( File );
		int32	BumpMapID			= ReadInt( File );
		int32	SphereMapID			= ReadInt( File );
		int32	MultiplyMapID		= ReadInt( File );

		// store a reference to this material in the index as well.
		// submeshes will look up an ID in the index, and then apply the material
		// referenced at that index - we start out with NULL
		m_pMaterialReferences[MaterialID] = NULL;

		CMaterial*	Material			= NULL;

		// all paths are empty by default
		std::string	DiffusePath			= "";
		std::string	OpacityPath			= "";
		std::string	IlluminationPath	= "";
		std::string	NormalPath			= "";
		std::string	SphereMapPath		= "";
		std::string	MultiplyMapPath		= "";
		MaterialType[MaterialID] = KMaterialStandard;

		// for each texture, assemble a path to the actual texture file by prepending the
		// texture path passed into this mesh to the texture name we extracted earlier


		// if a diffuse map exists, set to a standard material
		if ( DiffuseMapID != -1 )
		{
			DiffusePath = TexturePath + TextureNames[DiffuseMapID];
			MaterialType[MaterialID] = KMaterialStandard;
		}

		if ( OpacityMapID != -1 )
		{
			OpacityPath = TexturePath + TextureNames[OpacityMapID];
			MaterialType[MaterialID] = KMaterialAlpha;
		}

		// if it has a colorkey, it must also be an alpha
		if ( HasColorKey > 0 )
		{
			MaterialType[MaterialID] = KMaterialAlpha;
			MaterialHasColorKey[MaterialID] = kTrue;
		}
		else
		{
			MaterialHasColorKey[MaterialID] = kFalse;
		}

		// if we have an illumination map, it can go two ways -
		if ( IlluminationMapID != -1 )
		{
			IlluminationPath = TexturePath + TextureNames[IlluminationMapID];
			// if we already have a diffuse map, then it will become a 'glow' map
			if ( DiffuseMapID != -1 )
			{
				MaterialType[MaterialID] = KMaterialStandard;
			}
			else // otherwise, it is a standalone glow (like a shaft of light)
			{
				MaterialType[MaterialID] = KMaterialIllumination;
			}
		}

		if( MultiplyMapID != -1 && 
			IlluminationMapID == -1)
		{
			IlluminationPath = TexturePath + TextureNames[MultiplyMapID];
			if ( DiffuseMapID != -1 )
			{
				MaterialType[MaterialID] = KMaterialStandard;
			}
			else // otherwise, it is a standalone multiplication ( like a shadow )
			{
				MaterialType[MaterialID] = KMaterialMultiply;
			}
		}

		if ( ReflectionMapID != -1 && DiffuseMapID != -1 )
		{
			OpacityPath = TexturePath + TextureNames[ReflectionMapID];
			MaterialType[MaterialID] = KMaterialReflect;
		}
		if ( SphereMapID != -1 && ReflectionMapID != -1 )
		{
			SphereMapPath = TexturePath + TextureNames[SphereMapID];
			MaterialType[MaterialID] = KMaterialSphereMap;
		}

		if ( BumpMapID != -1 && ReflectionMapID == -1 )
		{
			NormalPath = TexturePath + TextureNames[BumpMapID];
			MaterialType[MaterialID] = KMaterialNormalMap;
		}

		// set the double-sided state of this material
		if ( DoubleSided != 0 )
		{
			MaterialIsDoubleSided[MaterialID] = kTrue;
		}
		else
		{
			MaterialIsDoubleSided[MaterialID] = kFalse;
		}


		// only create the material if we have requested at creation time
		if( MaterialHasColorKey[MaterialID] )
		{
			Material = m_pRefManager.AddMaterial( pD3DDevice,
												   DiffusePath,
												   kTrue,
												   MaterialColorKey[MaterialID],
												   MaterialType[MaterialID] == KMaterialReflect );
		}
		else
		{
			Material = m_pRefManager.AddMaterial( pD3DDevice,
												   DiffusePath,
												   OpacityPath,
												   IlluminationPath,
												   NormalPath,
												   SphereMapPath,
												   MaterialType[MaterialID] == KMaterialReflect );
		}
		// if an alpha was embedded in the texture, we need to detect it
		if( MaterialType[MaterialID] == KMaterialStandard &&
			Material->HasAlpha() )
		{
			MaterialType[MaterialID] = KMaterialAlpha;

		}

		// these alpha textures always render last, and do not write to the zbuffer at all
		if ( MaterialType[MaterialID] == KMaterialAlpha && RenderLast )
		{
			MaterialType[MaterialID] = KMaterialShadow;
		}
		// these alpha textures always render first
		if ( MaterialType[MaterialID] == KMaterialAlpha && RenderFirst )
		{
			MaterialType[MaterialID] = KMaterialPreAlpha;
		}

		// if we have an illumination texture for a base texture, and it's a multiply,
		// we need to make sure we set the illumination type, which defaults to ADD
		if( MultiplyMapID != -1 )
		{
			Material->SetIlluminationType( KMaterialMultiply );
		}
		// assign our data id
		Material->SetDataID( DataID );

		// assign our material properties to the newly created material

		D3DMATERIAL8& D3DMaterial = Material->D3DMaterial();

		D3DMaterial.Diffuse.r	= DiffuseR;					
		D3DMaterial.Diffuse.g	= DiffuseG;
		D3DMaterial.Diffuse.b	= DiffuseB;


		D3DMaterial.Specular.r	= SpecularR * ShininessStrength;
		D3DMaterial.Specular.g	= SpecularG * ShininessStrength;
		D3DMaterial.Specular.b	= SpecularB * ShininessStrength;

		// glossiness - 
		D3DMaterial.Power		= Shininess*100;

		D3DMaterial.Ambient.r	= AmbientR;						
		D3DMaterial.Ambient.g	= AmbientG;
		D3DMaterial.Ambient.b	= AmbientB;

		D3DMaterial.Emissive.r	= EmissiveR;						
		D3DMaterial.Emissive.g	= EmissiveG;
		D3DMaterial.Emissive.b	= EmissiveB;

		// set any animation properties on the material
		if( BehaviorFlipbook )
		{
			Material->SetAnimationFlipbook( 0,
											FlipbookWFrames, 
											FlipbookHFrames,
											FlipbookTime );
		}
		if( BehaviorShifting )
		{
			Material->SetAnimationShifting( 0, ShiftU, ShiftV );
		}

		// set any animation properties on the material's second layer
		if( BehaviorFlipbook2 )
		{
			Material->SetAnimationFlipbook( 1,
											FlipbookWFrames2, 
											FlipbookHFrames2,
											FlipbookTime2 );
		}
		if( BehaviorShifting2 )
		{
			Material->SetAnimationShifting( 1, ShiftU2, ShiftV2 );
		}

		// store the reference in our reference list
		m_pMaterialReferences[MaterialID] = Material;
	

		if( MaterialType[MaterialID] == KMaterialReflect )
		{
			Material->SetReflective( kTrue );
			if( AllowRealtimeReflection )
			{
				Material->EnableDynamicTexture( pD3DDevice, ReflectionSize );
			}
		}

		if( MaterialType[MaterialID] == KMaterialNormalMap )
		{
			Material->SetNormalMapped( kTrue );
		}

		if( ( MaterialType[MaterialID] == KMaterialReflect ||
			MaterialType[MaterialID] == KMaterialSphereMap ) &&
			OpacityMapID != -1 )
		{
			if( RenderLast )
			{
				Material->SetAllowZWrite( kFalse );
				if( RenderLast )
				{
					Material->SetAllowZWrite( kFalse );
					// convert this material into a post-material
					switch( MaterialType[MaterialID] )
					{
						case KMaterialReflect :
							MaterialType[MaterialID] = KMaterialReflectPost;
						break;
						case KMaterialStaticCubeMap :
							MaterialType[MaterialID] = KMaterialStaticCubeMapPost;
						break;
						case KMaterialSphereMap :
							MaterialType[MaterialID] = KMaterialSphereMapPost;
						break;
					}
				}
			}
			// no base texture means a self-illuminating reflection
			if( Material->BlankBaseTexture() )
			{
				Material->SetAllowZWrite( kFalse );
				Material->SetAdditiveReflection( kTrue );
			}
			else
			{
				Material->SetTransparentReflection( kTrue );
			}
		}
		else if( ( MaterialType[MaterialID] == KMaterialReflect ||
			MaterialType[MaterialID] == KMaterialSphereMap ) &&
			OpacityMapID == -1 )
		{
			if( RenderLast )
			{
				Material->SetAllowZWrite( kFalse );
				if( RenderLast )
				{
					Material->SetAllowZWrite( kFalse );
					// convert this material into a post-material
					switch( MaterialType[MaterialID] )
					{
						case KMaterialReflect :
							MaterialType[MaterialID] = KMaterialReflectPost;
						break;
						case KMaterialStaticCubeMap :
							MaterialType[MaterialID] = KMaterialStaticCubeMapPost;
						break;
						case KMaterialSphereMap :
							MaterialType[MaterialID] = KMaterialSphereMapPost;
						break;
					}
				}
			}
			// no base texture means a self-illuminating reflection
			if( Material->BlankBaseTexture() )
			{
				Material->SetAdditiveReflection( kTrue );
			}
		}
		// now we store an index to this material's location in it's material category -
		// this is used for dividing parts of the mesh into sub-lists later, which is
		// essential to drawing pieces in the correct order with the correct settings

		SubIndex[MaterialID] = MaterialCounts[MaterialType[MaterialID]];
		MaterialCounts[MaterialType[MaterialID]] ++;
	}

	// if no materials were contained in the file, we make a 'default' material that does nothing
	if ( m_Materials <= 0 )
	{
		MaterialType[0] = KMaterialStandard;
		SubIndex[0] = MaterialCounts[KMaterialStandard];
		MaterialCounts[KMaterialStandard]++;
		m_pMaterialReferences[0] = NULL;
		MaterialIsDoubleSided[0] = kFalse;
		MaterialHasColorKey[0] = kFalse;
	}

	// we keep track of the smallest objects, 
	// so that we know how far down to go in our octree/quadtree
	float32 SmallestObject = 1000;

	// we keep track of the smallest face, 
	// so that we know how far down to go in our octree/quadtree
	float32 SmallestFace = 1000;

	m_pFaceList = new CFaceList();

	uint32 VerticesLoaded( 0 );	// need to keep track of how many verts parsed
								// so that we can keep triangle refs in sync

	// now parse the individual sub-models within the file
	for( int32 j = 0; j < Objects; j ++ )
	{

		CSubScene*	SubScene					= new CSubScene();
		m_ModelList.push_back( SubScene );

		// pares the pivot location of the mesh
		// making sure to scale it
		float32	PivotX	= ReadFloat( File ) * SceneScale;
		float32	PivotY	= ReadFloat( File ) * SceneScale;
		float32	PivotZ	= ReadFloat( File ) * SceneScale;
		// keep as a vector so that we can use it to construct reflection planes
		D3DXVECTOR3 CenterPoint( PivotX, PivotY, PivotZ );

		int32		a, b, c;
		float32		x, y, z;
		D3DXVECTOR3	MinBounds		( -1, -1, -1 );
		D3DXVECTOR3	MaxBounds		( -1, -1, -1 );		

		// has a second mapping channel?

		int32		ShadowMapping		= ReadShort( File );

		// parse the total vertex count
		uint32		TotalVertices	= ReadInt( File );

		MeshVertices += TotalVertices;

		// load the vertex indices
		std::vector <uint32>	VertexReferences		( TotalVertices );

		for ( uint32 k = 0; k < TotalVertices; k++ )
		{
			VertexReferences[k] = ReadInt( File );

		}

		uint32						ActualVertices	= ReadInt( File );

		std::vector <D3DXVECTOR3>	Vertices		( ActualVertices );

		// parse the actual vertex values 
		for( int32 k = 0; k < ActualVertices; k++ )
		{
			x = PivotX + ReadFloat( File ) * SceneScale;
			y = PivotY + ReadFloat( File ) * SceneScale;
			z = PivotZ + ReadFloat( File ) * SceneScale;

			Vertices[k] = D3DXVECTOR3( x, y, z );

			if ( MinBounds.x == -1 && MaxBounds.z == -1 )
			{
				MinBounds = Vertices[k];
				MaxBounds = Vertices[k];
			}
			else
			{
				ExpandBounds( MinBounds, MaxBounds, Vertices[k] );
			}
		}
		for( int32 k = 0; k < TotalVertices; k++ )
		{
			assert( VertexReferences[k] < ActualVertices );

		}
		float32 AxisDiagonal = D3DXVec3Length( &D3DXVECTOR3( MaxBounds - MinBounds ) );

		if( AxisDiagonal < SmallestObject )
		{
			SmallestObject = AxisDiagonal;
		}

		if ( m_HasCollision )
		{
			for( int32 k = 0; k < TotalVertices; k++ )
			{
				m_pFaceList->AddVertex( Vertices[VertexReferences[k]] );
			}
		}

		// parse UV references
		int32					HasUVs				= ReadShort( File );
		std::vector <uint32>	UVReferences		( TotalVertices );
		std::vector <uint32>	ShadowUVReferences	( TotalVertices );

		for( int32 k = 0; k < TotalVertices; k++ )
		{
			uint32	Index	( 0 );
			uint32	ShadowIndex	( 0 );

			if ( HasUVs == 1 )
			{
				Index = ReadInt( File );
				if( ShadowMapping )
				{
					ShadowIndex = ReadInt( File );
				}
			}

			UVReferences[k] = Index;
			
			if( ShadowMapping )
			{
				ShadowUVReferences[k] = ShadowIndex;
			}
		}

		// parse the acutal UV coordinates
		uint32						ActualUVs	= 1;
		std::vector <D3DXVECTOR3>	UVs			( 1 );

		if ( HasUVs == 1 )
		{
			ActualUVs = ReadInt( File );
			UVs.resize( ActualUVs );

			for( int32 k = 0 ; k < ActualUVs; k++ )
			{
				x = ReadFloat( File );
				y = ReadFloat( File ) * -1;

				UVs[k] = D3DXVECTOR3( x, y, 0 );
			}
		}
		else
		{
			for( int32 k = 0 ; k < ActualUVs; k++ )
			{
				UVs[k] = D3DXVECTOR3( 0, 0, 0 );
			}
		}


		// parse normals
		int32					HasNormals			= ReadShort( File );
		std::vector <uint32>	NormalReferences	( TotalVertices );
		uint32						ActualNormals	( 0 );
		std::vector <D3DXVECTOR3>	Normals			( 0 );

		if( HasNormals > 0 )
		{
			for( int32 k = 0; k < TotalVertices; k++ )
			{
				NormalReferences[k] = ReadInt( File );
			}

			// parse the actual normals
			ActualNormals = ReadInt( File );
			Normals.resize( ActualNormals );

			for( int32 k = 0 ; k < ActualNormals; k++ )
			{
				int32 AU = ReadByte( File );
				int32 AV = ReadByte( File );

				Normals[k] = D3DXVECTOR3( DecompressNormal( AU, AV, 0 ),
										  DecompressNormal( AU, AV, 2 ),
										  DecompressNormal( AU, AV, 1 ) );
			}
		}

		// parse vertex colors
		uint32						ActualVertexColors	= ReadInt( File );

		std::vector <D3DCOLOR>	VertexColors		( TotalVertices );
		for( int32 k = 0; k < TotalVertices; k++ )
		{
			if ( k >= ActualVertexColors )
			{
				VertexColors[k] = D3DXCOLOR( 1, 1, 1, 1 );
				if( m_HasCollision )
				{
					m_pFaceList->AddColor( D3DXVECTOR3( 1, 1, 1 ) );
				}
			}
			else
			{
				uint32 r = ReadShort( File );
				uint32 g = ReadShort( File );
				uint32 b = ReadShort( File );

				VertexColors[k] = D3DXCOLOR( r / 255.0f,
											 g / 255.0f,
											 b / 255.0f,
											 1.0f );
				if( m_HasCollision )
				{
					m_pFaceList->AddColor( D3DXVECTOR3( r / 255.0f,
													    g / 255.0f,
													    b / 255.0f ) );
				}
			}

		}

		SubScene->SetMinBounds( MinBounds );
		SubScene->SetMaxBounds( MaxBounds );

		// parse the number of triangles in the whole mesh
		uint32	TotalTriangles		= ReadInt( File );
		// and the number of submodelsegments in this object
		uint32	SubSceneSegments	= ReadInt( File );

		// create different VBs depending on whether we're using shadow mapping or not...
		if( ShadowMapping )
		{
			if( HasNormals > 0 )
			{
				SubScene->CreateBuffers( pD3DDevice,
										 TotalTriangles,
										 NormalReferences,
										 Normals,
										 VertexReferences,
										 UVReferences,
										 ShadowUVReferences,
										 Vertices,
										 UVs,
										 VertexColors);
			}
			else {
				SubScene->CreateBuffers( pD3DDevice,
										 TotalTriangles,
										 VertexReferences,
										 UVReferences,
										 ShadowUVReferences,
										 Vertices,
										 UVs,
										 VertexColors);
			}
		}
		else
		{
			if( HasNormals > 0 )
			{
				SubScene->CreateBuffers( pD3DDevice,
										 TotalTriangles,
										 NormalReferences,
										 Normals,
										 VertexReferences,
										 UVReferences,
										 Vertices,
										 UVs,
										 VertexColors);
			}
			else 
			{
				SubScene->CreateBuffers( pD3DDevice,
										 TotalTriangles,
										 VertexReferences,
										 UVReferences,
										 Vertices,
										 UVs,
										 VertexColors);
			}
		}

		if ( !BoundsSet )
		{
			BoundsSet = kTrue;
			m_LocalMinBounds = MinBounds;
			m_LocalMaxBounds = MaxBounds;
		}

		// constantly expand the bounding box to encompass these new subobjects
		ExpandBounds( m_LocalMinBounds, m_LocalMaxBounds, MinBounds );
		ExpandBounds( m_LocalMinBounds, m_LocalMaxBounds, MaxBounds );


		// parse each sub-mesh
		for ( uint32 m = 0; m < SubSceneSegments; m++ )
		{
			// keep a major normal for each object for use in reflection planes
			D3DXVECTOR3 MajorNormal( 0, 0, 0 );

			// parse the material ID of this submesh (each submesh only has 1)
			uint32	MaterialID	= ReadShort( File );
			uint32	ShadowMaterialID( 0 );
			// make sure its valid
			while ( MaterialID >= m_TotalMaterials )
			{
				MaterialID = m_TotalMaterials - 1;
			}

			// parse shadow map id if required
			if( ShadowMapping )
			{
				// parse the shadow material ID of this submesh (each submesh only has 1)
				ShadowMaterialID	= ReadShort( File );
				// make sure its valid
				while ( ShadowMaterialID >= m_TotalMaterials )
				{
					ShadowMaterialID = m_TotalMaterials - 1;
				}
			}



			uint32	ActualTriangles	= ReadInt( File );

			m_TotalFaces += ActualTriangles;
			std::vector <D3DXVECTOR4>	Triangles	( ActualTriangles );


			// parse the actual triangle indices for each corner vert
			for ( uint32 k = 0 ; k < ActualTriangles; k++ )
			{
				a = ReadInt( File );
				b = ReadInt( File );
				c = ReadInt( File );

				Triangles[k] = D3DXVECTOR4( (float32)a, 
											(float32)b, 
											(float32)c, 
											(float32)MaterialID );

				if( MaterialType[MaterialID] == KMaterialReflect )
				{
					D3DXVECTOR3 Side1 = SubScene->GetVertex( VertexReferences[c] );
					D3DXVECTOR3 Side2 = SubScene->GetVertex( VertexReferences[b] );
					Side1 -= SubScene->GetVertex( VertexReferences[b] );
					Side2 -= SubScene->GetVertex( VertexReferences[a] );

					D3DXVECTOR3 Normal;
					D3DXVec3Cross( &Normal, &Side1, &Side2 );
					D3DXVec3Normalize( &Normal, &Normal );
					MajorNormal += Normal;
				}

				if ( m_HasCollision && MaterialCollideable[MaterialID] )
				{
					m_pFaceList->AddFace( c + VerticesLoaded, 
										  b + VerticesLoaded, 
										  a + VerticesLoaded,
										  MaterialID );

					if( MaterialIsDoubleSided[MaterialID] )
					{
						m_pFaceList->AddFace( a + VerticesLoaded, 
											  b + VerticesLoaded, 
											  c + VerticesLoaded,
											  MaterialID );
					}

					float32 AxisDiagonal = D3DXVec3Length( &D3DXVECTOR3( m_pFaceList->GetVertex( a + VerticesLoaded ) -
																		 m_pFaceList->GetVertex( b + VerticesLoaded ) ) );
				
					if( AxisDiagonal < SmallestFace )
					{
						SmallestFace = AxisDiagonal;
					}
				}
			}

			// create this segment as part of the submodel
			// (this effectively makes an index buffer to the submodel's
			// vertex buffer)
			uint32	SegmentIndex	= SubScene->AddSubSceneSegment( m_pRefManager,
																	m_pSettings,
																    pD3DDevice,
																	VertexReferences,
																    Triangles,
																    MaterialID,
																    MaterialType[MaterialID],
																    MaterialIsDoubleSided[MaterialID] );

			// if we've actually got a material, apply it to this submodelsegment
			if ( m_TotalMaterials > 0 )
			{
				SubScene->SetMaterial( m_pMaterialReferences[MaterialID],
									   SegmentIndex );

				if( ShadowMapping )
				{
					SubScene->SetShadowMaterial( m_pMaterialReferences[ShadowMaterialID],
												 SegmentIndex );
				}

				if( MaterialType[MaterialID] == KMaterialReflect )
				{
					// average the normals to get the major normal
					MajorNormal /= (float32)ActualTriangles;
					m_pMaterialReferences[MaterialID]->SetDynamicTexturePlane( CenterPoint, MajorNormal );
				}

				if( !MaterialVisible[MaterialID] )
				{
					SubScene->SetVisible( SegmentIndex, kFalse );
				}
			}

			m_TotalSubMeshes++;
		}
		SubScene->CleanIB( pD3DDevice );

		// if this submesh has any normall mapping textures, then generate
		// tangent and binormal data for use in the calculation at render time
		if( SubScene->HasNormalMapping() )
		{
			SubScene->GenerateTangentBinormal( pD3DDevice );
		}

		VerticesLoaded += TotalVertices;

	}

	// do all setup calculations for this collision mesh
	// and sort it into quad/octrees
	if ( m_HasCollision )
	{

		float32 BoundDiagonal = D3DXVec3Length( &( m_LocalMinBounds - m_LocalMaxBounds ) );
		BoundDiagonal /= 1000.0f;
		if( SmallestFace < BoundDiagonal )
		{
			SmallestFace = BoundDiagonal;
		}

		m_pFaceList->CalculateFaceBounds();					
		m_pFaceList->CalculateNormals();

	}

	// sort the scene
	if( SortType == KSortOctree )
	{
		m_pOctree = new CSceneOctree( m_LocalMinBounds,
									  m_LocalMaxBounds,
									  SmallestObject );
		
		// sort references to each subsegment into a bucket
		// of the appropriate material type, for optimized rendering later
		for( int32 i = 0 ; i < m_ModelList.size() ; i++ )
		{
			for ( uint32 j = 0; j < m_ModelList[i]->SubSceneSegments() ; j++ )
			{
				CSubSceneSegment&	Segment			= m_ModelList[i]->GetSubSceneSegment( j );			
				uint32				SubMaterialIndex	= SubIndex[Segment.MaterialID()];
				Segment.SetSubIndex( SubMaterialIndex );
				
				m_pOctree->AddData( &Segment, Segment.MinBounds(), Segment.MaxBounds() );
				
				m_pMaterialSegments[Segment.MaterialType()][SubMaterialIndex].push_back( &Segment );
			}
		}
	}
	else
	{
		m_pQuadtree = new CSceneQuadtree( m_LocalMinBounds,
										  m_LocalMaxBounds,
										  SmallestObject );
		
		// sort references to each subsegment into a bucket
		// of the appropriate material type, for optimized rendering later
		for( int32 i = 0 ; i < m_ModelList.size() ; i++ )
		{
			for ( uint32 j = 0; j < m_ModelList[i]->SubSceneSegments() ; j++ )
			{
				CSubSceneSegment&	Segment			= m_ModelList[i]->GetSubSceneSegment( j );			
				uint32				SubMaterialIndex	= SubIndex[Segment.MaterialID()];
				Segment.SetSubIndex( SubMaterialIndex );
				
				m_pQuadtree->AddData( &Segment, Segment.MinBounds(), Segment.MaxBounds() );

				m_pMaterialSegments[Segment.MaterialType()][SubMaterialIndex].push_back( &Segment );
			}
		}
	}

    if( m_HasCollision )
	{
		// sort collision faces into quad or octree, as was requested
		// if we requested an octree
		if( SortType == KSortOctree )
		{
			m_pCollisionOctree = new COctree< uint32 >( m_LocalMinBounds,
														m_LocalMaxBounds,
														SmallestFace);

			uint32 Faces = m_pFaceList->Faces();
			// sort face data into an octtree
			for( int32 i = 0; i < Faces ; i++ )
			{
				m_pCollisionOctree->AddData( i, 
											 m_pFaceList->GetMinFaceBounds( i ),
											 m_pFaceList->GetMaxFaceBounds( i ) );
			}
		}
		else
		{
			m_pCollisionQuadtree = new CQuadtree< uint32 >( m_LocalMinBounds,
															m_LocalMaxBounds,
															SmallestFace);

			uint32 Faces = m_pFaceList->Faces();
			// sort face data into a quadtree
			for( int32 i = 0; i < Faces ; i++ )
			{
				m_pCollisionQuadtree->AddData( i,
											   m_pFaceList->GetMinFaceBounds( i ),
											   m_pFaceList->GetMaxFaceBounds( i ) );
			}
		}
	}

	for( int32 i = 0; i < KMaterialTypes; i ++ )
	{
		m_pMaterialSegments[i].resize( MaterialCounts[i] );
		m_pSortedMaterialSegments[i].resize( MaterialCounts[i] );
	}



	// read all light data in the file - a type, position, quaternion (which we'll convert to a ray )
	// and a color
	for( int32 i = 0; i < Lights; i ++ )
	{
		std::string Name = ReadString( File );

		ELightType Type = (ELightType)ReadShort(File);

		float32 X = ReadFloat( File ) * SceneScale;
		float32 Y = ReadFloat( File ) * SceneScale;
		float32 Z = ReadFloat( File ) * SceneScale;
		
			
		D3DXMATRIX Orientation;
		D3DXMatrixIdentity( &Orientation );

		Orientation._11 = ReadFloat( File );
		Orientation._12 = ReadFloat( File );
		Orientation._13 = ReadFloat( File );

		Orientation._21 = ReadFloat( File );
		Orientation._22 = ReadFloat( File );
		Orientation._23 = ReadFloat( File );

		Orientation._31 = ReadFloat( File );
		Orientation._32 = ReadFloat( File );
		Orientation._33 = ReadFloat( File );
	
		float32 R = ReadFloat( File );
		float32 G = ReadFloat( File );
		float32 B = ReadFloat( File );

		float32 Range = ReadFloat( File );

		m_Lights.push_back( new CDataLight( Name, 
											Type,
											Range,
											D3DXVECTOR3( X, Y, Z ),
											D3DXVECTOR3( Orientation._31, Orientation._32, Orientation._33 ),
											D3DXVECTOR3( R, G, B ) ) );

		m_Lights.back()->D3DLight().Range = Range;

		uint32 HasUserData = ReadShort( File );

		if( HasUserData > 0 )
		{
			m_Lights.back()->SetUserData( ReadString( File ) );
		}

	}

	// read all the points in the file - each has a position and location
	for( int32 i = 0; i < Points; i ++ )
	{
		std::string Name = ReadString( File );

		float32	X = ReadFloat( File ) * SceneScale;
		float32	Y = ReadFloat( File ) * SceneScale;
		float32	Z = ReadFloat( File ) * SceneScale;

		D3DXMATRIX Orientation;
		D3DXMatrixIdentity( &Orientation );

		Orientation._11 = ReadFloat( File );
		Orientation._12 = ReadFloat( File );
		Orientation._13 = ReadFloat( File );

		Orientation._21 = ReadFloat( File );
		Orientation._22 = ReadFloat( File );
		Orientation._23 = ReadFloat( File );

		Orientation._31 = ReadFloat( File );
		Orientation._32 = ReadFloat( File );
		Orientation._33 = ReadFloat( File );

		m_Points.push_back( new CDataPoint( Name, D3DXVECTOR3( X, Y, Z ), Orientation ) );

		uint32 HasUserData = ReadShort( File );

		if( HasUserData > 0 )
		{
			m_Points.back()->SetUserData( ReadString( File ) );
		}

	}


	// read all the paths in the file - each has a position and location
	for( int32 i = 0; i < Paths; i ++ )
	{
		std::string Name = ReadString( File );

		float32	X = ReadFloat( File ) * SceneScale;
		float32	Y = ReadFloat( File ) * SceneScale;
		float32	Z = ReadFloat( File ) * SceneScale;

		uint32 SubPaths = ReadShort( File );

		for( uint32 k = 0; k < SubPaths; k++ )
		{
			uint32 Points	= ReadShort( File );
			bool Closed		= ReadBool( File);

			if( k == 0 )
			{
				m_Paths.push_back( new CPath( Name, Closed, D3DXVECTOR3( X, Y, Z ) ) );
			}

			for( uint32 l = 0; l < Points; l++ )
			{
				float32 PX = ReadFloat( File ) * SceneScale;
				float32 PY = ReadFloat( File ) * SceneScale;
				float32 PZ = ReadFloat( File ) * SceneScale;

				if( k == 0 )
				{
					m_Paths.back()->AddPoint( D3DXVECTOR3( PX, PY, PZ ) );
				}
			}
			if( Closed )
			{
				m_Paths.back()->ClosePath();
			}
		}

		uint32 HasUserData = ReadShort( File );

		if( HasUserData > 0 )
		{
			m_Paths.back()->SetUserData( ReadString( File ) );
		}

	}

	fclose( File );

}	// CScene::Load()


// presorts faces for collision later.
// this is excellent to use if you are going to perform many collisions in a set area
// (for instance, colliding with four points on the ground to place a shadow )
// call SortForCollision, and then either PreSortedRayCollision or PreSortedSphereCollision
// to collide with this data.
void CScene::SortForCollision( const D3DXVECTOR3& MinBounds,	// min bounds of area to sort
							   const D3DXVECTOR3& MaxBounds )	// max bounds of area to sort
{
	m_SortedFaces.resize( 0 );

	if( m_pCollisionOctree )
	{
		m_pCollisionOctree->Sort( MinBounds,
								  MaxBounds,
								  m_SortedFaces);
	}
	else
	{
		m_pCollisionQuadtree->Sort( MinBounds,
						   MaxBounds,
						   m_SortedFaces);
	}

} // CScene::SortForCollision()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CScene::PreSortedRayCollision( const D3DXVECTOR3& Start,		// start of collision ray
									const D3DXVECTOR3& End,		// end of collision ray
									D3DXVECTOR3& ImpactPoint,		// impact point to be filled
									D3DXVECTOR3& ImpactNormal,		// impact normal to be filled
									uint32& FaceMaterial,			// material id of the impacted face
									D3DXVECTOR3& AverageColor )	// averaged colors of the face's verts
{
	float32		BestDistance	( KCollisionInvalid );

	D3DXVECTOR3	MinBounds		( Start );
	D3DXVECTOR3	MaxBounds		( Start );


	ExpandBounds( MinBounds, MaxBounds, End );

	m_pFaceList->RayCollision( m_SortedFaces,
							   Start,
							   End,
							   MinBounds,
							   MaxBounds,
							   ImpactPoint,
							   ImpactNormal,
							   FaceMaterial,
							   AverageColor,
							   BestDistance );

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CScene::PreSortedRayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CScene::PreSortedSphereCollision( const D3DXVECTOR3& Start,	// start of collision ray
									   const D3DXVECTOR3& End,		// end of collision ray
									   float32 Radius,				// radius of sphere to check along collision ray
									   D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
									   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
									   D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
									   uint32& FaceMaterial,		// material id of the impacted face
									   D3DXVECTOR3& AverageColor )	// averaged colors of the face's verts
{
	float32		BestDistance	( KCollisionInvalid );

	D3DXVECTOR3	MinBounds		( Start );
	D3DXVECTOR3	MaxBounds		( Start );

	ExpandBounds( MinBounds, MaxBounds, End );
 

	MinBounds.x -= Radius;
	MinBounds.y -= Radius;
	MinBounds.z -= Radius;
	MaxBounds.x += Radius;
	MaxBounds.y += Radius;
	MaxBounds.z += Radius;

	m_pFaceList->SphereCollision( m_SortedFaces,
								  Start,
								  End,
								  MinBounds,
								  MaxBounds,
								  Radius,
								  FinalPosition,
								  ImpactPoint,
								  ImpactNormal,
								  FaceMaterial,
								  AverageColor,
								  BestDistance );

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CScene::PreSortedSphereCollision()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CScene::RayCollision( const D3DXVECTOR3& Start,	// start of collision ray
						   const D3DXVECTOR3& End,		// end of collision ray
						   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
						   D3DXVECTOR3& ImpactNormal )	// impact normal to be filled
{
	D3DXVECTOR3 Color;
	uint32 Material;
	return RayCollision( Start, End, ImpactPoint, ImpactNormal, Material, Color );
} // CScene::RayCollision()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CScene::RayCollision( const D3DXVECTOR3& Start,	// start of collision ray
						   const D3DXVECTOR3& End,		// end of collision ray
						   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
						   D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
						   uint32& FaceMaterial,		// material id of the impacted face
						   D3DXVECTOR3& AverageColor )	// averaged colors of the face's verts
{
	float32		BestDistance	( KCollisionInvalid );

	D3DXVECTOR3	MinBounds		( Start );
	D3DXVECTOR3	MaxBounds		( Start );


	ExpandBounds( MinBounds, MaxBounds, End );

	std::vector < uint32 >	pSortedFaces;
	pSortedFaces.reserve( 500 );

	if( m_pCollisionOctree )
	{
		m_pCollisionOctree->Sort( MinBounds,
						 MaxBounds,
						 pSortedFaces);
	}
	else
	{
		m_pCollisionQuadtree->Sort( MinBounds,
						   MaxBounds,
						   pSortedFaces);
	}

	m_pFaceList->RayCollision( pSortedFaces,
							   Start,
							   End,
							   MinBounds,
							   MaxBounds,
							   ImpactPoint,
							   ImpactNormal,
							   FaceMaterial,
							   AverageColor,
							   BestDistance );

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CScene::RayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CScene::SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
							  const D3DXVECTOR3& End,		// end of collision ray
							  float32 Radius,				// radius of sphere to check along collision ray
							  D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
							  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
							  D3DXVECTOR3& ImpactNormal )	// impact normal to be filled
{
	D3DXVECTOR3 Color;
	uint32 Material;
	return SphereCollision( Start, 
		End, 
		Radius, 
		FinalPosition,
		ImpactPoint, 
		ImpactNormal, 
		Material, 
		Color );
} // CScene::SphereCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CScene::SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
							  const D3DXVECTOR3& End,		// end of collision ray
							  float32 Radius,				// radius of sphere to check along collision ray
							  D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
							  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
							  D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
							  uint32& FaceMaterial,			// material id of the impacted face
							  D3DXVECTOR3& AverageColor )	// averaged colors of the face's verts
{
	float32		BestDistance	( KCollisionInvalid );

	D3DXVECTOR3	MinBounds		( Start );
	D3DXVECTOR3	MaxBounds		( Start );

	ExpandBounds( MinBounds, MaxBounds, End );
 

	MinBounds.x -= Radius;
	MinBounds.y -= Radius;
	MinBounds.z -= Radius;
	MaxBounds.x += Radius;
	MaxBounds.y += Radius;
	MaxBounds.z += Radius;

	std::vector < uint32 >	pSortedFaces;
	pSortedFaces.reserve( 500 );

	if( m_pCollisionOctree )
	{
		m_pCollisionOctree->Sort( MinBounds,
								  MaxBounds,
								  pSortedFaces);
	}
	else
	{
		m_pCollisionQuadtree->Sort( MinBounds,
								    MaxBounds,
									pSortedFaces);
	}


	m_pFaceList->SphereCollision( pSortedFaces,
								  Start,
								  End,
								  MinBounds,
								  MaxBounds,
								  Radius,
								  FinalPosition,
								  ImpactPoint,
								  ImpactNormal,
								  FaceMaterial,
								  AverageColor,
								  BestDistance );

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CScene::SphereCollision()

// transforms vertex normals into light-space and stores them in the diffuse RGBA-
// removes any current vertex colors. For use in normal mapping
void CScene::CalculateTangentNormals( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									  const D3DXVECTOR3& LightDirection )	// light direction
{
	for( uint32 i = 0; i < KMaterialTypes; i++ )
	{
		for( uint32 j = 0; j < m_pSortedMaterialSegments[i].size(); j++ )
		{
			for( uint32 k = 0; k < m_pSortedMaterialSegments[i][j].size(); k++ )
			{
				CMaterial* Material =  m_pSortedMaterialSegments[i][j][k]->GetMaterial();
				// if this is a normal mapped mesh - we do the calculation
				if( Material != NULL && Material->HasNormalMap() )
				{	
					m_pSortedMaterialSegments[i][j][k]->Parent()->CalculateTangentNormals( pD3DDevice,
																						   LightDirection,
																						   m_pSortedMaterialSegments[i][j][k]->FirstVertIndex(),
																						   m_pSortedMaterialSegments[i][j][k]->LastVertIndex() );
				}
			}
		}
	}
}

// render a vector of segments
void CScene::RenderSegmentSet( LPDIRECT3DDEVICE8 pD3DDevice,										// direct3d device
							   std::vector <std::vector <CSubSceneSegment*> > pMaterialSegments )	// vector of segments to render
{
	
	CSubScene*	pCurrentSubScene = NULL;
	CMaterial*	CurrentShadowMaterial = NULL;
	CSubSceneSegment*	CurrentShadowSegment = NULL;
	// loop through the different material sets
	for ( uint32 i = 0 ; i < pMaterialSegments.size() ; i++ )
	{
		bool SetTexture = kFalse;
		// and then render the submodelsegments for each material set
		for ( uint32 j = 0 ; j < pMaterialSegments[i].size() ; j++ )
		{
			if( pMaterialSegments[i][j]->Visible() )
			{
				// set the stream if this is a new mesh
				if( pMaterialSegments[i][j]->Parent() != pCurrentSubScene )
				{
					pCurrentSubScene = pMaterialSegments[i][j]->Parent();
					pMaterialSegments[i][j]->SetStream( pD3DDevice );

				}
				// set the vertex shader if required
				if( m_CurrentVertexShader != pMaterialSegments[i][j]->Parent()->VertexShader() )
				{
					m_CurrentVertexShader = pMaterialSegments[i][j]->Parent()->VertexShader();
					pD3DDevice->SetVertexShader( m_CurrentVertexShader );
				}

				if( pMaterialSegments[i][j]->GetShadowMaterial() != 
					CurrentShadowMaterial )
				{
					if( CurrentShadowSegment != NULL )
					{
						CurrentShadowSegment->UnSetShadowMap( pD3DDevice );
					}

					CurrentShadowMaterial = pMaterialSegments[i][j]->GetShadowMaterial();

					if( CurrentShadowMaterial != NULL )
					{
						pMaterialSegments[i][j]->SetShadowMap( pD3DDevice );
						CurrentShadowSegment = pMaterialSegments[i][j];
					}
					else
					{
						CurrentShadowSegment = NULL;
					}
				}

				// only set the texture for the first mesh with this texture
				if ( !SetTexture )
				{
					pMaterialSegments[i][j]->Render( pD3DDevice );
					SetTexture = kTrue;
				}
				else
				{
					pMaterialSegments[i][j]->RenderNoTexture( pD3DDevice );
				}
			}
		}

		// if we set a texture, unset it (any alpha settings that got set)
		if ( pMaterialSegments[i].size() > 0 )
		{
			pMaterialSegments[i][0]->UnSet( pD3DDevice );
		}
	}

	// unset any shadow maps still remaining
	if( CurrentShadowSegment != NULL )
	{
		CurrentShadowSegment->UnSetShadowMap( pD3DDevice );
	}


} // CScene::RenderSegmentSet()

void CScene::RenderSegmentSetIllumination( LPDIRECT3DDEVICE8 pD3DDevice,										// direct3d device
										   std::vector <std::vector <CSubSceneSegment*> > pMaterialSegments,	// vector of segments to render
										   EMaterialType MaterialType )											// material type we will render
{
	
	CSubScene*	pCurrentSubScene = NULL;
	// loop through the different material sets
	for ( uint32 i = 0 ; i < pMaterialSegments.size() ; i++ )
	{
		bool SetTexture = kFalse;
		bool SetShifter = kFalse;
		// and then render the submodelsegments for each material set
		for ( uint32 j = 0 ; j < pMaterialSegments[i].size() ; j++ )
		{
			if( pMaterialSegments[i][j]->Visible() )
			{
				// only render meshes with an illumination map
				if( pMaterialSegments[i][j]->GetMaterial()->HasBaseTexture() &&
					pMaterialSegments[i][j]->GetMaterial()->HasIllumination() &&
					pMaterialSegments[i][j]->GetMaterial()->GetIlluminationType() == MaterialType )
				{
					// set the stream if this is a new mesh
					if( pMaterialSegments[i][j]->Parent() != pCurrentSubScene )
					{
						pCurrentSubScene = pMaterialSegments[i][j]->Parent();
						pMaterialSegments[i][j]->SetStream( pD3DDevice );

					}
					// set the vertex shader if required
					if( m_CurrentVertexShader != pMaterialSegments[i][j]->Parent()->VertexShader() )
					{
						m_CurrentVertexShader = pMaterialSegments[i][j]->Parent()->VertexShader();
						pD3DDevice->SetVertexShader( m_CurrentVertexShader );
					}

					// only set the texture for the first mesh with this texture
					if ( !SetTexture )
					{
						pD3DDevice->SetTexture( 0, 
												pMaterialSegments[i][j]->GetMaterial()->GetIlluminationTexture() );
						SetTexture = kTrue;
					}
					// set any second-layer shifters
					if ( pMaterialSegments[i][j]->GetMaterial()->IsAnimated( 1 ) )
					{
						pD3DDevice->SetTextureStageState( 0,
														D3DTSS_TEXTURETRANSFORMFLAGS,
														D3DTTFF_COUNT2 );

						D3DXMATRIX	matShift;
						D3DXMatrixIdentity( &matShift );

						// set the matrix using the shifting component of the material
						matShift._31 = pMaterialSegments[i][j]->GetMaterial()->GetU( 1 );
						matShift._32 = pMaterialSegments[i][j]->GetMaterial()->GetV( 1 );

						pD3DDevice->SetTransform( D3DTS_TEXTURE0, &matShift );
						SetShifter = kTrue;
					}
					// render the actual mesh
					pMaterialSegments[i][j]->RenderNoTexture( pD3DDevice );

					if( SetShifter )
					{
						pD3DDevice->SetTextureStageState( 0,
														  D3DTSS_TEXTURETRANSFORMFLAGS,
														  D3DTTFF_DISABLE );
						SetShifter = kFalse;
					}

				}
			}
		}

	}
} // CScene::RenderSegmentSetIllumination()


// renders the scene using the default reflection matrix
void CScene::Render( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
					 const D3DXMATRIX& ViewportMatrix,			// viewport matrix
					 LPDIRECT3DTEXTURE8 pReflectionMap1 )		// reflection map to use - NULL if none
{
	// calculate a default reflection matrix - if you want your own,
	D3DXMATRIX	InverseView;
	D3DXMATRIX	ProjectionMatrix;

	float32	Aspect	= ( float32 ) m_pSettings.GetSettings( KSetScreenWidth ) /
					  ( float32 ) m_pSettings.GetSettings( KSetScreenHeight );
	float32 FOV		= ( float32 )( m_pSettings.GetSettings( KSetFOV ) * KDeg2Rad );

	D3DXMatrixPerspectiveFovLH( &ProjectionMatrix,
								FOV,
								Aspect,
								0,
								(float32)m_pSettings.GetSettings( KSetFarClip ) );

	D3DXMatrixInverse( &InverseView, NULL, &ViewportMatrix );

	InverseView = InverseView * ViewportMatrix * ProjectionMatrix * m_ProjectorBias;
	
	Render( pD3DDevice, ViewportMatrix, &InverseView, pReflectionMap1 );

}

// renders the scene using a user-supplied reflection matrix
void CScene::Render( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
					 const D3DXMATRIX& ViewportMatrix,			// viewport matrix
					 D3DXMATRIX* pReflectionMatrix,				// reflection matrix - NULL if none
					 LPDIRECT3DTEXTURE8 pReflectionMap1 )		// reflection map to use - NULL if none
{
	Render( pD3DDevice, 
		    ViewportMatrix, 
			pReflectionMatrix, 
			pReflectionMap1, 
			m_pMaterialSegments );
} // CScene::Render()

// renders the scene using a user-supplied reflection matrix
void CScene::Render( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
					 const D3DXMATRIX& ViewportMatrix,			// viewport matrix
					 D3DXMATRIX* pReflectionMatrix,				// reflection matrix - NULL if none
					 LPDIRECT3DTEXTURE8 pReflectionMap,			// reflection map to use - NULL if none
					 std::vector <std::vector <std::vector <CSubSceneSegment*> > >&	pMaterialSegments ) // vector containing all segments to be rendered
{
	uint32 j( 0 );
	pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );
	m_CurrentVertexShader = D3DFVF_PLAINVERTEXSCENE;
	pD3DDevice->SetVertexShader( m_CurrentVertexShader );
	
	if( m_RenderOpaque )
	{
		RenderSegmentSet( pD3DDevice,
						pMaterialSegments[KMaterialStandard] );

		// set up to render normal mapped segments
		if ( pMaterialSegments[KMaterialNormalMap].size() > 0 &&
			m_pSettings.GetSettings( KSetNormalMapping ) > 0 )
		{

			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3 );

			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADDSIGNED );

		}

		RenderSegmentSet( pD3DDevice,
				 		pMaterialSegments[KMaterialNormalMap] );
		// turn off normal mapping states
		if ( pMaterialSegments[KMaterialNormalMap].size() > 0 &&
			m_pSettings.GetSettings( KSetNormalMapping ) > 0 )
		{

			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
		}


		// reflection mapped segments
		if ( pMaterialSegments[KMaterialReflect].size() > 0 &&
			m_pSettings.GetSettings( KSetSceneReflection ) &&
			pReflectionMatrix != NULL )
		{
			// use our reflection map override if necessary
			if( pReflectionMap != NULL )
			{
				pD3DDevice->SetTexture( 1, pReflectionMap );
			}

			pD3DDevice->SetTransform( D3DTS_TEXTURE1, pReflectionMatrix );

			// now we need to set up layer 1 to do the sort of projection we want when it comes time
			pD3DDevice->SetTextureStageState( 1, D3DTSS_MIPFILTER, D3DTEXF_NONE );
			
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_ADDRESSU,
											m_pSettings.GetSettings( KSetClampUVs ) );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_ADDRESSV,
											m_pSettings.GetSettings( KSetClampUVs ) );
			
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_TEXTURETRANSFORMFLAGS,
											D3DTTFF_COUNT3 |
											D3DTTFF_PROJECTED );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_TEXCOORDINDEX,
											D3DTSS_TCI_CAMERASPACEPOSITION |
											1 );

			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_COLOROP,
											D3DTOP_MODULATEALPHA_ADDCOLOR );
		}

		RenderSegmentSet( pD3DDevice,
						pMaterialSegments[KMaterialReflect] );

		if ( pMaterialSegments[KMaterialReflect].size() > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_TEXTURETRANSFORMFLAGS,
											D3DTTFF_DISABLE );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_MIPFILTER,
											m_pSettings.GetSettings( KSetMipFilter ) );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_ADDRESSU,
											D3DTADDRESS_WRAP );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_ADDRESSV,
											D3DTADDRESS_WRAP );

		}


		// spherically mapped segments
		if ( pMaterialSegments[KMaterialSphereMap].size() > 0 &&
			m_pSettings.GetSettings( KSetCubeMapping ) > 0 )

		{
			
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_TEXCOORDINDEX,
											D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );


			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_COLOROP,
											D3DTOP_MODULATEALPHA_ADDCOLOR );
		}

		RenderSegmentSet( pD3DDevice,
						pMaterialSegments[KMaterialSphereMap] );

		if ( pMaterialSegments[KMaterialSphereMap].size() > 0 &&
			m_pSettings.GetSettings( KSetCubeMapping ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );

			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

		}
	}

	if( m_RenderAlpha )
	{
		// if any alpha materials are included -
		if ( pMaterialSegments[KMaterialPreAlpha].size() > 0 ||
			pMaterialSegments[KMaterialAlpha].size() > 0 ||
			pMaterialSegments[KMaterialShadow].size() > 0 )
		{
			// set alpha states
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE );

			pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
			pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

			if ( m_pSettings.GetSettings( KSetAlphaTest ) )
			{
				pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kTrue );
				pD3DDevice->SetRenderState( D3DRS_ALPHAREF, ( DWORD ) 50 );
				pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
			}		

			// render the alphas
			RenderSegmentSet( pD3DDevice,
							pMaterialSegments[KMaterialPreAlpha] );

			RenderSegmentSet( pD3DDevice,
							pMaterialSegments[KMaterialAlpha] );


			if ( m_pSettings.GetSettings( KSetAlphaTest ) )
			{
				pD3DDevice->SetRenderState( D3DRS_ALPHAREF, ( DWORD ) 1 );
			}		

			// don't zwrite
			pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

			pD3DDevice->SetTextureStageState( 0,
											D3DTSS_MIPFILTER,
											m_pSettings.GetSettings( KSetMipFilter ) );

			RenderSegmentSet( pD3DDevice,
							pMaterialSegments[KMaterialShadow] );


			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
		}

		// render multiply materials
		pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

		RenderSegmentSet( pD3DDevice,
						pMaterialSegments[KMaterialMultiply] );

		// now render any multiply illumination maps that are on top of any previous materials
		for( int32 j = 0; j < KMaterialTypes; j ++ )
		{
			RenderSegmentSetIllumination( pD3DDevice, pMaterialSegments[j], KMaterialMultiply );
		}

		// render glow materials
		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

		RenderSegmentSet( pD3DDevice,
						pMaterialSegments[KMaterialIllumination] );



		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );


		
		// now we render any additive reflections that don't zwrite -
		// lots of state changes again, so bear with me

		// reflection mapped segments
		if ( pMaterialSegments[KMaterialReflectPost].size() > 0 &&
			m_pSettings.GetSettings( KSetSceneReflection ) &&
			pReflectionMatrix != NULL )
		{
			// use our reflection map override if necessary
			if( pReflectionMap != NULL )
			{
				pD3DDevice->SetTexture( 1, pReflectionMap );
			}

			pD3DDevice->SetTransform( D3DTS_TEXTURE1, pReflectionMatrix );

			// now we need to set up layer 1 to do the sort of projection we want when it comes time
			pD3DDevice->SetTextureStageState( 1, D3DTSS_MIPFILTER, D3DTEXF_NONE );
			
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_ADDRESSU,
											m_pSettings.GetSettings( KSetClampUVs ) );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_ADDRESSV,
											m_pSettings.GetSettings( KSetClampUVs ) );
			
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_TEXTURETRANSFORMFLAGS,
											D3DTTFF_COUNT3 |
											D3DTTFF_PROJECTED );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_TEXCOORDINDEX,
											D3DTSS_TCI_CAMERASPACEPOSITION |
											1 );

			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_COLOROP,
											D3DTOP_MODULATEALPHA_ADDCOLOR );
		}

		RenderSegmentSet( pD3DDevice,
						pMaterialSegments[KMaterialReflectPost] );

		if ( pMaterialSegments[KMaterialReflectPost].size() > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_TEXTURETRANSFORMFLAGS,
											D3DTTFF_DISABLE );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_MIPFILTER,
											m_pSettings.GetSettings( KSetMipFilter ) );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_ADDRESSU,
											D3DTADDRESS_WRAP );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_ADDRESSV,
											D3DTADDRESS_WRAP );

		}


		// spherically mapped segments
		if ( pMaterialSegments[KMaterialSphereMapPost].size() > 0 &&
			m_pSettings.GetSettings( KSetCubeMapping ) > 0 )
		{
			
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_TEXCOORDINDEX,
											D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );


			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											D3DTSS_COLOROP,
											D3DTOP_MODULATEALPHA_ADDCOLOR );
		}

		RenderSegmentSet( pD3DDevice,
						pMaterialSegments[KMaterialSphereMapPost] );

		if ( pMaterialSegments[KMaterialSphereMapPost].size() > 0 &&
			m_pSettings.GetSettings( KSetCubeMapping ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );

			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

		}

		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );


		// now render any illumination maps that are on top of any previous materials
		for( int32 j = 0; j < KMaterialTypes; j ++ )
		{
			RenderSegmentSetIllumination( pD3DDevice, pMaterialSegments[j], KMaterialIllumination );
		}

	}

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	if ( m_pSettings.GetSettings( KSetAlphaTest ) )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kFalse );
	}	
} // CScene::Render()

// renders the scene with all non-glowing objects as invisible, but written to the zbuffer.
// all glowing objects are visible, but do NOT write to the zbuffer.
// this is used for post effects like light-streaking.
void CScene::RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							 const D3DXMATRIX& ViewportMatrix,			// viewport matrix
							 std::vector <std::vector <std::vector <CSubSceneSegment*> > >&	pMaterialSegments,	 // vector containing all segments to be rendered
							 bool RenderGlows )							// render glows, or just zbuffer?
{
	CSubScene*	pCurrentSubScene = NULL;

	pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );
	m_CurrentVertexShader = D3DFVF_PLAINVERTEXSCENE;
	pD3DDevice->SetVertexShader( m_CurrentVertexShader );

	// first we set the blending mode so that it DOESN'T write to texture,
	// but only to the zbuffer.
	// we have to render the non-visible objects to the zbuffer so that
	// glowing objects will be properly occluded.
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );

	if ( m_pSettings.GetSettings( KSetAlphaTest ) )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kTrue );
	}	

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	for ( uint32 i = 0 ; i < pMaterialSegments.size() ; i++ )
	{
		// only render objects that are totally opaque
		if( i == KMaterialReflect ||
			i == KMaterialStandard ||
			i == KMaterialNormalMap ||
			i == KMaterialSphereMap ||
			i == KMaterialAlpha )
		{
			for ( uint32 k = 0 ; k < pMaterialSegments[i].size() ; k++ )
			{
				// and then render the submodelsegments for each material set
				for ( uint32 j = 0 ; j < pMaterialSegments[i][k].size() ; j++ )
				{
					if( m_pMaterialSegments[i][k][j]->GetMaterial() != NULL &&
						!m_pMaterialSegments[i][k][j]->GetMaterial()->AllowZWrite() )
					{
						break;
					}

					if( i == KMaterialAlpha )
					{
						if ( m_pSettings.GetSettings( KSetAlphaTest ) )
						{
							pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kTrue );
							pD3DDevice->SetRenderState( D3DRS_ALPHAREF, ( DWORD ) 50 );
							pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
						}		
						pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
					}

					if( pMaterialSegments[i][k][j]->Visible() )
					{
						// set the stream if this is a new mesh
						if( pMaterialSegments[i][k][j]->Parent() != pCurrentSubScene )
						{
							pCurrentSubScene = pMaterialSegments[i][k][j]->Parent();
							pMaterialSegments[i][k][j]->SetStream( pD3DDevice );

						}
						// set the vertex shader if required
						if( m_CurrentVertexShader != pMaterialSegments[i][k][j]->Parent()->VertexShader() )
						{
							m_CurrentVertexShader = pMaterialSegments[i][k][j]->Parent()->VertexShader();
							pD3DDevice->SetVertexShader( m_CurrentVertexShader );
						}

						// alphas are a special case - they need to set their texture,
						// because the transparency determines the z-write
						if( i == KMaterialAlpha )
						{
							pMaterialSegments[i][k][j]->Render( pD3DDevice );
							pMaterialSegments[i][k][j]->UnSet( pD3DDevice );
						}
						else
						{
							pMaterialSegments[i][k][j]->RenderNoTexture( pD3DDevice );
						}
					}

					if( i == KMaterialAlpha )
					{
						if ( m_pSettings.GetSettings( KSetAlphaTest ) )
						{
							pD3DDevice->SetRenderState( D3DRS_ALPHAREF, ( DWORD ) 1 );
						}	
						pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
					}

				}
			}
		}
	}
	pD3DDevice->SetTextureStageState( 1, D3DTSS_MIPFILTER, D3DTEXF_NONE );

	// now we render only the illumination textures. These will 'glow' and be properly occluded.
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	// render glow materials

	if( RenderGlows )
	{
		RenderSegmentSet( pD3DDevice,
						  pMaterialSegments[KMaterialIllumination] );
	}


	pCurrentSubScene = NULL;
	// now we render any meshes that are not additive, but are marked as 'glowing'.
	for( int32 i = 0 ; i < pMaterialSegments.size() ; i++ )
	{
		// only render objects that are totally opaque
		if( i == KMaterialReflect ||
			i == KMaterialStandard ||
			i == KMaterialNormalMap ||
			i == KMaterialSphereMap )
		{
			for ( uint32 k = 0 ; k < pMaterialSegments[i].size() ; k++ )
			{
				uint32 Renders( 0 ); // how many renders we did

				// and then render the submodelsegments for each material set
				for ( uint32 j = 0 ; j < pMaterialSegments[i][k].size() ; j++ )
				{
					if( m_pMaterialSegments[i][k][j]->GetMaterial() != NULL &&
						!m_pMaterialSegments[i][k][j]->GetMaterial()->AllowZWrite() )
					{
						break;
					}
					if( m_pMaterialSegments[i][k][j]->GetMaterial() != NULL &&
						pMaterialSegments[i][k][j]->GetMaterial()->GetDataID() != KGlowDataID )
					{
						break;
					}
					if( pMaterialSegments[i][k][j]->Visible() )
					{
						// set the stream if this is a new mesh
						if( pMaterialSegments[i][k][j]->Parent() != pCurrentSubScene )
						{
							pCurrentSubScene = pMaterialSegments[i][k][j]->Parent();
							pMaterialSegments[i][k][j]->SetStream( pD3DDevice );

						}
						// set the vertex shader if required
						if( m_CurrentVertexShader != pMaterialSegments[i][k][j]->Parent()->VertexShader() )
						{
							m_CurrentVertexShader = pMaterialSegments[i][k][j]->Parent()->VertexShader();
							pD3DDevice->SetVertexShader( m_CurrentVertexShader );
						}

						pMaterialSegments[i][k][j]->Render( pD3DDevice );
						Renders++;
					}
				}
				if( Renders > 0 )
				{
					pMaterialSegments[i][k][0]->UnSet( pD3DDevice );
				}
			}
		}
	}

	if( RenderGlows )
	{
		// now render any illumination maps that are on top of any previous materials
		for( uint32 j = 0; j < KMaterialTypes; j ++ )
		{
			RenderSegmentSetIllumination( pD3DDevice, pMaterialSegments[j], KMaterialIllumination );
		}
	}

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

	pD3DDevice->SetTextureStageState( 1,
									  D3DTSS_MIPFILTER,
									  m_pSettings.GetSettings( KSetMipFilter ) );

	if ( m_pSettings.GetSettings( KSetAlphaTest ) )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kFalse );
	}	
} // CScene::RenderGlowPass()

// Specifically for racing gamese, this renders the opaque portions of the scene
// with projected texture coordinates given a light projection matrix provided by the vehicle.
// It renders an additive layer with a texture that has been applied prior to calling this method
void CScene::RenderHeadlightPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								  const D3DXMATRIX& ViewportMatrix,			// viewport matrix
								  const D3DXMATRIX& LightMatrix,			// matrix for the headlight direction
								  std::vector <std::vector <std::vector <CSubSceneSegment*> > >&	pMaterialSegments ) // vector containing all segments to be rendered
{
	D3DXMATRIX ProjectionMatrix;
	D3DXMATRIX InverseView;

	float32 FOV		= ( float32 )( 90 * KDeg2Rad );

	D3DXMatrixPerspectiveFovLH( &ProjectionMatrix, 
								FOV, 
								1.0f, 
								1, 
								(float32)m_pSettings.GetSettings( KSetFarClip ) );

	D3DXMatrixInverse( &InverseView, NULL, &ViewportMatrix );

	InverseView = InverseView * LightMatrix * ProjectionMatrix * m_ProjectorBias;
		
	pD3DDevice->SetTransform( D3DTS_TEXTURE0, &InverseView );

	// now we need to set up layer 0 to do the sort of projection we want when it comes time

	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_ADDRESSU,
									  m_pSettings.GetSettings( KSetClampUVs ) );
	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_ADDRESSV,
									  m_pSettings.GetSettings( KSetClampUVs ) );
		
	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_TEXTURETRANSFORMFLAGS,
									  D3DTTFF_COUNT3 |
									  D3DTTFF_PROJECTED );
	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_TEXCOORDINDEX,
									  D3DTSS_TCI_CAMERASPACEPOSITION |
									  1 );

	CSubScene*	pCurrentSubScene = NULL;

	pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );
	m_CurrentVertexShader = D3DFVF_PLAINVERTEXSCENE;
	pD3DDevice->SetVertexShader( m_CurrentVertexShader );

	if ( m_pSettings.GetSettings( KSetAlphaTest ) )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kTrue );
	}	

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	for ( uint32 i = 0 ; i < pMaterialSegments.size() ; i++ )
	{
		// only render objects that are totally opaque
		if( i == KMaterialReflect ||
			i == KMaterialStandard ||
			i == KMaterialNormalMap ||
			i == KMaterialSphereMap )
		{
			for ( uint32 k = 0 ; k < pMaterialSegments[i].size() ; k++ )
			{
				// and then render the submodelsegments for each material set
				for ( uint32 j = 0 ; j < pMaterialSegments[i][k].size() ; j++ )
				{
					if( pMaterialSegments[i][k][j]->Visible() )
					{
						// set the stream if this is a new mesh
						if( pMaterialSegments[i][k][j]->Parent() != pCurrentSubScene )
						{
							pCurrentSubScene = pMaterialSegments[i][k][j]->Parent();
							pMaterialSegments[i][k][j]->SetStream( pD3DDevice );

						}
						// set the vertex shader if required
						if( m_CurrentVertexShader != pMaterialSegments[i][k][j]->Parent()->VertexShader() )
						{
							m_CurrentVertexShader = pMaterialSegments[i][k][j]->Parent()->VertexShader();
							pD3DDevice->SetVertexShader( m_CurrentVertexShader );
						}

						pMaterialSegments[i][k][j]->RenderNoTexture( pD3DDevice );
					}
				}
			}
		}
	}

	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );

	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_TEXTURETRANSFORMFLAGS,
									  D3DTTFF_DISABLE );
	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_ADDRESSU,
									  D3DTADDRESS_WRAP );
	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_ADDRESSV,
									  D3DTADDRESS_WRAP );

	if ( m_pSettings.GetSettings( KSetAlphaTest ) )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kFalse );
	}	
} // CScene::RenderHeadlightPass()

// renders a premade projected shadow map to the scene
void CScene::RenderShadowPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							   const D3DXMATRIX& ViewportMatrix,			// viewport matrix
							   const D3DXMATRIX& LightMatrix,				// matrix for the headlight direction
							   std::vector <std::vector <std::vector <CSubSceneSegment*> > >&	pMaterialSegments,  // vector containing all segments to be rendered
							   D3DXMATRIX* LightProjectionMatrix,			// null by default
							   bool Orthographic )							// is this orthographic? false by default
{
	D3DXMATRIX ProjectionMatrix;
	D3DXMATRIX InverseView;

	float32 FOV		= ( float32 )( 90 * KDeg2Rad );

	if( LightProjectionMatrix != NULL )
	{
		ProjectionMatrix = *LightProjectionMatrix;
	}
	else
	{
		if( Orthographic )
		{
			D3DXMatrixOrthoLH ( &ProjectionMatrix, 
								50,
								50,
								.01f, 
								(float32)m_pSettings.GetSettings( KSetFarClip ) );
		}
		else
		{
			D3DXMatrixPerspectiveFovLH( &ProjectionMatrix, 
										FOV, 
										1.0f, 
										.01f, 
										(float32)m_pSettings.GetSettings( KSetFarClip ) );
		}
	}

	D3DXMatrixInverse( &InverseView, NULL, &ViewportMatrix );

	InverseView = InverseView * LightMatrix * ProjectionMatrix * m_ProjectorBias;
		
	pD3DDevice->SetTransform( D3DTS_TEXTURE0, &InverseView );

	// now we need to set up layer 0 to do the sort of projection we want when it comes time

	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_ADDRESSU,
									  D3DTADDRESS_CLAMP );
	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_ADDRESSV,
									  D3DTADDRESS_CLAMP );
		

	if( Orthographic )
	{
		pD3DDevice->SetTextureStageState( 0,
										D3DTSS_TEXTURETRANSFORMFLAGS,
										D3DTTFF_COUNT3 );
	}
	else
	{
		pD3DDevice->SetTextureStageState( 0,
										D3DTSS_TEXTURETRANSFORMFLAGS,
										D3DTTFF_COUNT3 |
										D3DTTFF_PROJECTED );
	}

	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_TEXCOORDINDEX,
									  D3DTSS_TCI_CAMERASPACEPOSITION |
									  1 );

	CSubScene*	pCurrentSubScene = NULL;

	pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );
	m_CurrentVertexShader = D3DFVF_PLAINVERTEXSCENE;
	pD3DDevice->SetVertexShader( m_CurrentVertexShader );

	if ( m_pSettings.GetSettings( KSetAlphaTest ) )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kTrue );
	}	

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );


	for ( uint32 i = 0 ; i < pMaterialSegments.size() ; i++ )
	{
		// only render objects that are totally opaque
		if( i == KMaterialReflect ||
			i == KMaterialStandard ||
			i == KMaterialNormalMap ||
			i == KMaterialSphereMap )
		{
			for ( uint32 k = 0 ; k < pMaterialSegments[i].size() ; k++ )
			{
				// and then render the submodelsegments for each material set
				for ( uint32 j = 0 ; j < pMaterialSegments[i][k].size() ; j++ )
				{
					if( pMaterialSegments[i][k][j]->Visible() )
					{
						// set the stream if this is a new mesh
						if( pMaterialSegments[i][k][j]->Parent() != pCurrentSubScene )
						{
							pCurrentSubScene = pMaterialSegments[i][k][j]->Parent();
							pMaterialSegments[i][k][j]->SetStream( pD3DDevice );

						}
						// set the vertex shader if required
						if( m_CurrentVertexShader != pMaterialSegments[i][k][j]->Parent()->VertexShader() )
						{
							m_CurrentVertexShader = pMaterialSegments[i][k][j]->Parent()->VertexShader();
							pD3DDevice->SetVertexShader( m_CurrentVertexShader );
						}

						pMaterialSegments[i][k][j]->RenderNoTexture( pD3DDevice );
					}
				}
			}
		}
	}

	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );

	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_TEXTURETRANSFORMFLAGS,
									  D3DTTFF_DISABLE );
	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_ADDRESSU,
									  D3DTADDRESS_WRAP );
	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_ADDRESSV,
									  D3DTADDRESS_WRAP );

	if ( m_pSettings.GetSettings( KSetAlphaTest ) )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kFalse );
	}	
} // CScene::RenderShadowPass()

// renders the scene using the default reflection matrix
void CScene::RenderSorted( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						   const D3DXMATRIX& ViewportMatrix,			// viewport matrix
						   LPDIRECT3DTEXTURE8 pReflectionMap1 )		// reflection map to use - NULL if none
{
// calculate a default reflection matrix - if you want your own,
	D3DXMATRIX	InverseView;
	D3DXMATRIX	ProjectionMatrix;

	float32	Aspect	= ( float32 ) m_pSettings.GetSettings( KSetScreenWidth ) /
					  ( float32 ) m_pSettings.GetSettings( KSetScreenHeight );

	float32 FOV		= ( float32 )( m_pSettings.GetSettings( KSetFOV ) * KDeg2Rad );

	D3DXMatrixPerspectiveFovLH( &ProjectionMatrix,
								FOV,
								Aspect,
								0,
								(float32)m_pSettings.GetSettings( KSetFarClip ) );

	D3DXMatrixInverse( &InverseView, NULL, &ViewportMatrix );

	InverseView = InverseView * ViewportMatrix * ProjectionMatrix * m_ProjectorBias;


	RenderSorted( pD3DDevice, ViewportMatrix, &InverseView, pReflectionMap1 );


} // CScene::RenderSorted()

// renders the scene using a user-supplied reflection matrix
void CScene::RenderSorted( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
						   const D3DXMATRIX& ViewportMatrix,			// viewport matrix
						   D3DXMATRIX* pReflectionMatrix,				// reflection matrix - NULL if none
						   LPDIRECT3DTEXTURE8 pReflectionMap1 )		// reflection map to use - NULL if none
{

	Render( pD3DDevice,
			ViewportMatrix,
			pReflectionMatrix,
			pReflectionMap1,
			m_pSortedMaterialSegments );

} // CScene::RenderSorted()

// renders the scene with all non-glowing objects as invisible, but written to the zbuffer.
// all glowing objects are visible, but do NOT write to the zbuffer.
// this is used for post effects like light-streaking.
void CScene::RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							const D3DXMATRIX& ViewportMatrix,			// viewport matrix
							bool RenderGlows )							// render glows, or just zbuffer?
{
	RenderGlowPass( pD3DDevice, ViewportMatrix, m_pMaterialSegments, RenderGlows );
} // CScene::RenderGlowPass()

// renders the scene with all non-glowing objects as invisible, but written to the zbuffer.
// all glowing objects are visible, but do NOT write to the zbuffer.
// this is used for post effects like light-streaking.
void CScene::RenderGlowPassSorted( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								   const D3DXMATRIX& ViewportMatrix,			// viewport matrix
								   bool RenderGlows )							// render glows, or just zbuffer?
{
	RenderGlowPass( pD3DDevice, ViewportMatrix, m_pSortedMaterialSegments, RenderGlows );
} // CScene::RenderGlowPassSorted()

// Specifically for racing gamese, this renders the opaque portions of the scene
// with projected texture coordinates given a projection matrix provided by the vehicle.
// It renders an additive layer with a texture that has been applied prior to calling this method
void CScene::RenderHeadlightPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								  const D3DXMATRIX& ViewportMatrix, 		// viewport matrix
								  const D3DXMATRIX& LightMatrix )			// matrix for the headlight direction
{
	RenderHeadlightPass( pD3DDevice, ViewportMatrix, LightMatrix, m_pMaterialSegments );
} // CScene::RenderHeadlightPass()

// Specifically for racing gamese, this renders the opaque portions of the scene
// with projected texture coordinates given a projection matrix provided by the vehicle.
// It renders an additive layer with a texture that has been applied prior to calling this method
void CScene::RenderHeadlightPassSorted( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
										const D3DXMATRIX& ViewportMatrix,		// viewport matrix
										const D3DXMATRIX& LightMatrix )			// matrix for the headlight direction
{
	RenderHeadlightPass( pD3DDevice, ViewportMatrix, LightMatrix, m_pSortedMaterialSegments );
} // CScene::RenderHeadlightPassSorted()

// renders a projected shadow map to the scene
void CScene::RenderShadowPass( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							   const D3DXMATRIX& ViewportMatrix, 		// viewport matrix
							   const D3DXMATRIX& LightMatrix,			// matrix for the Shadow direction
							   D3DXMATRIX* LightProjectionMatrix,		// null by default
							   bool Orthographic )						// is this orthographic? false by default
{
	RenderShadowPass( pD3DDevice, ViewportMatrix, LightMatrix, m_pMaterialSegments, LightProjectionMatrix, Orthographic );
} // CScene::RenderShadowPass()

// renders a projected shadow map to the scene
void CScene::RenderShadowPassSorted( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									 const D3DXMATRIX& ViewportMatrix,		// viewport matrix
									 const D3DXMATRIX& LightMatrix,			// matrix for the Shadow direction
									 D3DXMATRIX* LightProjectionMatrix,		// null by default
									 bool Orthographic )						// is this orthographic? false by default
{
	RenderShadowPass( pD3DDevice, ViewportMatrix, LightMatrix, m_pSortedMaterialSegments, LightProjectionMatrix, Orthographic );
} // CScene::RenderShadowPassSorted()

// sort the scene for rendering
void CScene::Sort( CFrustum& pFrustum )					// frustum to cull to
{
	for( uint32 j = 0; j < KMaterialTypes; j ++ )
	{
		for( uint32 k = 0; k < m_pSortedMaterialSegments[j].size() ; k ++ )
		{
			m_pSortedMaterialSegments[j][k].resize( 0 );
		}
	}

	if( m_pOctree != NULL )
	{
		m_pOctree->Sort( pFrustum,
						 m_pSortedMaterialSegments);
	}
	else
	{
		m_pQuadtree->Sort( pFrustum,
						   m_pSortedMaterialSegments);
	}
} // CScene::Sort()

// sort the scene for rendering
void CScene::FindVisibleReflections( CFrustum& pFrustum ) // frustum to use to determine whether
{														  // the reflections are facing the camera
	m_pSortedReflectiveMaterials.resize( 0 );
	for( uint32 j = 0; j < m_pSortedMaterialSegments[KMaterialReflect].size(); j++ )
	{
		if( m_pSortedMaterialSegments[KMaterialReflect][j].size() > 0 )
		{
			// find out if the reflection is facing the camera
			CMaterial* pMaterial = m_pSortedMaterialSegments[KMaterialReflect][j][0]->GetMaterial();
			float32 Dot = D3DXVec3Dot( &m_pSortedMaterialSegments[KMaterialReflect][j][0]->GetMaterial()->DynamicTextureNormal(),
									   &pFrustum.Forward() );
			if( Dot > -.5f )
			{
				m_pSortedReflectiveMaterials.push_back( m_pSortedMaterialSegments[KMaterialReflect][j][0]->GetMaterial() );
			}
		}
	}
} // CScene::FindVisibleReflections()

// how many reflected surfaces are currently sorted to be rendered
uint32 CScene::ReflectionsVisible( void )
{
	return m_pSortedReflectiveMaterials.size();
} // CScene::ReflectionsVisible()

// return a pointer to a material in the sorted set of visible reflective materials
CMaterial*	CScene::GetReflectionMaterial( uint32 Index )	// index in the sorted set
{
	assert( Index < m_pSortedReflectiveMaterials.size() );

	return m_pSortedReflectiveMaterials[Index];
} // CScene::GetReflectionMaterial()

// render the mesh without any textures
void CScene::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							  const D3DXMATRIX& ViewportMatrix )	// viewport transformation matrix
{
	pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );


	for ( uint32 i = 0 ; i < m_ModelList.size() ; i++ )
	{
		if( m_ModelList[i]->SubSceneSegments() > 0 )
		{
			pD3DDevice->SetVertexShader( m_ModelList[i]->VertexShader() );
			m_ModelList[i]->GetSubSceneSegment( 0 ).SetStream( pD3DDevice );
			for ( uint32 j = 0 ; j < m_ModelList[i]->SubSceneSegments() ; j++ )
			{
				m_ModelList[i]->GetSubSceneSegment( j ).RenderNoTexture( pD3DDevice );
			}
		}
	}

} // CScene::RenderNoTexture()

CPath&	CScene::GetPath( uint32 Index )		// Index of point to retrieve
{
	assert( Index < m_Paths.size() );

	return *m_Paths[Index];
} // CScene::GetPath()

CDataPoint&	CScene::GetPoint( uint32 Index )		// Index of point to retrieve
{
	assert( Index < m_Points.size() );

	return *m_Points[Index];

} // CScene::GetPoint()

CDataLight&	CScene::GetLight( uint32 Index )		// Index of point to retrieve
{
	assert( Index < m_Lights.size() );

	return *m_Lights[Index];

} // CScene::GetLight()


