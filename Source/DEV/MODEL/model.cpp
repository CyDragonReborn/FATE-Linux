/*
	model.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004 

	Change History :

	Travis Baldree
	4/14/2004	-	changed return type for lights/points/paths from pointers to refs
					changed return type of min/max bounds to refs
					big changes to vert/index buffer arrangement! consolidated them
					for the entire model, and subsegments now only have an index
					to their position in the parent buffer. Minimizes vert buffer changes

	Travis Baldree
	4/16/2004	-	Added support for point orientations

	Travis Baldree
	5/3/2004	-	Can now do a multiply layer. 
					Shifters now work on the second layer too.
					Materials can now be marked as visible

	Travis Baldree
	5/17/2004	-	textures with embedded alpha are now recognized and loaded properly
					into their appropriate material bank

	Travis Baldree
	5/27/2004	-	fixed a bug during loading with multi-sub object materials where the mesh only used
					some of those materials. Exclusive to this media type, as the others already took care of it.

					faces were being added to the collision list with verts in the wrong order.
					Fixed.

	Travis Baldree
	6/29/2004	-	added many new methods for setting/getting/changing materials on an indexed
					and named basis, for much greater flexibility in modifying pre-existing materials.
					A change had to be made to the refmanager to accomodate this, so make
					sure that it is updated as well

	8/24/2004	-	added support for builtin static cubemaps

	8/24/2004	-	Added support for post-rendered reflections (fully additive reflections)

    12/26/2004	-	Bounds were being improperly calculated for models with sub-objects!

	1/18/2004	-	Added methods for getting/setting materials by index instead of material ID

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

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "model.h"
#include "submodel.h"
#include "submodelsegment.h"
#include "shadowvolume.h"
#include "facelist.h"




CModel::CModel( CRefManager& pRefManager,	// the reference manager, for tracking submaterials
				CSettings& pSettings ):		// the settings manager, for keeping track of card/machine capabilities
		m_pRefManager( pRefManager ),
		m_pSettings( pSettings ),
		m_HasAlpha( kFalse ),
		m_IsTextured( kFalse ),
		m_HasReflection( kFalse ),
		m_TotalSubMeshes( 0 ),
		m_TotalFaces( 0 ),
		m_TotalMaterials( 0 ),
		m_pSubModel( NULL ),
		m_pCollisionList( NULL ),
		m_pShadowVolume( NULL ),
		m_CastsShadows( kFalse )

{
	ZeroMemory ( &m_LocalMaxBounds, sizeof( m_LocalMaxBounds ));
	ZeroMemory ( &m_LocalMinBounds, sizeof( m_LocalMinBounds ));

	ZeroMemory ( &m_MaxBounds, sizeof( m_MaxBounds ));
	ZeroMemory ( &m_MinBounds, sizeof( m_MinBounds ));

	for ( int x = 0; x < 8; x++ )
	{
		ZeroMemory ( &m_Bounds[x], sizeof ( m_Bounds[x] ));
	}
} // CModel::CModel()


CModel::~CModel()
{
	InvalidateDeviceObjects();

	DELETE_SAFELY( m_pCollisionList );

	DELETE_SAFELY( m_pSubModel );

	for( uint32 i =0; i < m_Lights.size(); i++ )
	{
		DELETE_SAFELY( m_Lights[i] );
	}
	m_Lights.clear();

	for( i =0; i < m_Paths.size(); i++ )
	{
		DELETE_SAFELY( m_Paths[i] );
	}
	m_Paths.clear();

	for( i =0; i < m_Points.size(); i++ )
	{
		DELETE_SAFELY( m_Points[i] );
	}
	m_Points.clear();

} // CModel::~CModel()

// see if any reflective materials exist in this model
bool CModel::HasReflection( void )
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
} // CModel::HasReflection()


void CModel::InvalidateDeviceObjects( void )
{
	m_pSubModel->InvalidateDeviceObjects();

} // CModel::InvalidateDeviceObjects()

void CModel::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	m_pSubModel->RestoreDeviceObjects( pD3DDevice );

} // CModel::RestoreDeviceObjects()

void CModel::Load( LPDIRECT3DDEVICE8 pD3DDevice,	// the direct3d device - needed to create vert and index buffers
				   std::string FileName,			// path to the mesh file
				   std::string TexturePath,			// path to the textures (if "" is supplied, will use the mesh path)
				   bool LocalSpace,					// ignore worldspace positioning in the file?
				   bool LoadTextures,				// should the mesh load and apply textures
				   bool FlipNormals )				// flip the normals at load time?
{
	m_IsTextured = LoadTextures;

	m_TotalFaces = 0;


	DELETE_SAFELY( m_pCollisionList );

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

	// first block of the model file - basic info about the mesh
	float32	ModelScale			( ReadFloat( File ) );				// scale of the entire model
	uint32	Objects				( ReadInt( File ) );				// how many sub-objects are in the model
	uint32	Lights				( ReadInt( File ) );				// how many lights are in the model
	uint32	Points				( ReadInt( File ) );				// how many glows are in the model
	uint32	Paths				( ReadInt( File ) );				// how many lights are in the model
	uint32	TotalMaterials		( ReadInt( File ) );				// how many individual materials are applied
	uint32	Textures			( ReadInt( File ) );				// how many unique textures appear

	m_TotalMaterials = TotalMaterials;

	uint32	Materials			( m_TotalMaterials );

	if ( m_TotalMaterials <= 0 )
	{
		m_TotalMaterials = 1;
	}


	std::vector <std::string>	TextureNames			( Textures );

	std::vector <CMaterial*>	MaterialReferences		( m_TotalMaterials );
	std::vector <bool>			MaterialHasColorKey		( m_TotalMaterials );
	std::vector <DWORD>			MaterialColorKey		( m_TotalMaterials );
	std::vector <bool>			MaterialIsDoubleSided	( m_TotalMaterials );
	std::vector <bool>			MaterialReferenced		( m_TotalMaterials );
	std::vector <int32>			MaterialCollideable		( m_TotalMaterials );
	std::vector <int32>			MaterialVisible			( m_TotalMaterials );
	std::vector <EMaterialType>	MaterialType			( m_TotalMaterials );

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
	m_UserData.resize( m_TotalMaterials );

	m_pMaterialSegments.resize( KMaterialTypes );


	// reset user data for each material
	for ( uint32 i = 0; i < m_TotalMaterials; i++ )
	{
		m_UserData[i] = 0;
	}

	// load each material
	for ( j = 0; j < Materials ; j++ )
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
		MaterialReferences[MaterialID] = NULL;

		CMaterial*	Material			= NULL;

		// all paths are empty by default
		std::string	DiffusePath			= "";
		std::string	OpacityPath			= "";
		std::string	IlluminationPath	= "";
		std::string	NormalPath	= "";
		std::string	SphereMapPath		= "";
		std::string	MultiplyMapPath		= "";
		MaterialType[MaterialID] = KMaterialStandard;
		// by default, the material is not 'used' by the mesh, until some part of the mesh asks for it
		MaterialReferenced[MaterialID] = kFalse;

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
			else // otherwise, it is a standalone glow (like a shaft of light)
			{
				MaterialType[MaterialID] = KMaterialMultiply;
			}
		}

		if ( ReflectionMapID != -1 )
		{
			OpacityPath = TexturePath + TextureNames[ReflectionMapID];
			MaterialType[MaterialID] = KMaterialReflect;
		}
		if ( SphereMapID != -1 && ReflectionMapID != -1 )
		{
			SphereMapPath = TexturePath + TextureNames[SphereMapID];
			MaterialType[MaterialID] = KMaterialSphereMap;
		}

		if ( BumpMapID != -1 )
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
		if ( LoadTextures )
		{
			if( MaterialHasColorKey[MaterialID] )
			{
				Material = m_pRefManager.AddMaterial( pD3DDevice,
													   DiffusePath,
													   kTrue,
													   MaterialColorKey[MaterialID] );
			}
			else
			{
				Material = m_pRefManager.AddMaterial( pD3DDevice,
													   DiffusePath,
													   OpacityPath,
													   IlluminationPath,
													   NormalPath,
													   SphereMapPath );
			}

			// if a sphere-map was turned into a non-dynamic cubemap,
			// change material type to KMaterialStaticCubeMap
			if( Material->HasCubeMap() && ReflectionMapID != -1 )
			{
				MaterialType[MaterialID] = KMaterialStaticCubeMap;
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
			MaterialReferences[MaterialID] = Material;


			if( MaterialType[MaterialID] == KMaterialReflect )
			{
				Material->SetReflective( kTrue );
			}

			if( MaterialType[MaterialID] == KMaterialNormalMap )
			{
				Material->SetNormalMapped( kTrue );
			}

			if( ( MaterialType[MaterialID] == KMaterialReflect ||
				MaterialType[MaterialID] == KMaterialStaticCubeMap ||
				MaterialType[MaterialID] == KMaterialSphereMap ) &&
				OpacityMapID != -1 )
			{
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
				// no base texture means a self-illuminating reflection
				if( Material->BlankBaseTexture() )
				{
					Material->SetAdditiveReflection( kTrue );
				}
				else
				{
					Material->SetTransparentReflection( kTrue );
				}
			}
			else if( ( MaterialType[MaterialID] == KMaterialReflect ||
				MaterialType[MaterialID] == KMaterialStaticCubeMap ||
				MaterialType[MaterialID] == KMaterialSphereMap ) &&
				OpacityMapID == -1 )
			{
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
				// no base texture means a self-illuminating reflection
				if( Material->BlankBaseTexture() )
				{
					Material->SetAdditiveReflection( kTrue );
				}
			}
		}
	}

	// if no materials were contained in the file, we make a 'default' material that does nothing
	if ( Materials <= 0 )
	{
		Materials = 1;
		m_MaterialNames[0] = "NONE";
		MaterialType[0] = KMaterialStandard;
		MaterialReferences[0] = NULL;
		MaterialIsDoubleSided[0] = kFalse;
		MaterialHasColorKey[0] = kFalse;
		MaterialCollideable[0] = kTrue;
		MaterialVisible[0] = kTrue;
	}

	// we're going to just merge all sub-objects in the model into a single one,
	// to reduce the number of VBs
	m_pCollisionList = new CFaceList();

	m_pSubModel					= new CSubModel();

	std::vector <D3DXVECTOR3>	Vertices;
	std::vector <uint32>		VertexReferences;
	std::vector <D3DXVECTOR4>	Triangles;
	std::vector <uint32>		UVReferences;
	std::vector <D3DXVECTOR3>	UVs;
	std::vector <D3DXVECTOR3>	Normals;
	std::vector <uint32>		NormalReferences;
	std::vector <D3DXVECTOR3>	VertexColors;


	// now parse the individual sub-models within the file
	for ( j = 0; j < Objects; j ++ )
	{
		// offset into the list of merged vertices
		int32 VerticesLoaded = Vertices.size();
		int32 VertexReferencesLoaded = VertexReferences.size();
		int32 UVsLoaded = UVs.size();


		// pares the pivot location of the mesh
		// making sure to scale it
		float32	PivotX	= ReadFloat( File ) * ModelScale;
		float32	PivotY	= ReadFloat( File ) * ModelScale;
		float32	PivotZ	= ReadFloat( File ) * ModelScale;

		// if we are ignoring the mesh's world location, reset it to zero
		if ( LocalSpace )
		{
			PivotX = 0;
			PivotY = 0;
			PivotZ = 0;
		}

		int32		a, b, c;
		float32		x, y, z;
		D3DXVECTOR3	MinBounds		( -1, -1, -1 );
		D3DXVECTOR3	MaxBounds		( -1, -1, -1 );		

		// parse the total vertex count
		int32		TotalVertices	= ReadInt( File );

		MeshVertices += TotalVertices;

		// load the vertex indices
		for ( int32 k = 0; k < TotalVertices; k++ )
		{
			VertexReferences.push_back( ReadInt( File ) + VerticesLoaded );
		}

		int32						ActualVertices	= ReadInt( File );

		// parse the actual vertex values 
		for ( k = 0; k < ActualVertices; k++ )
		{
			x = PivotX + ReadFloat( File ) * ModelScale;
			y = PivotY + ReadFloat( File ) * ModelScale;
			z = PivotZ + ReadFloat( File ) * ModelScale;

			Vertices.push_back( D3DXVECTOR3( x, y, z ) );
			
			m_pCollisionList->AddVertex( D3DXVECTOR3( x, y, z ) );

			if ( MinBounds.x == -1 && MaxBounds.z == -1 )
			{
				MinBounds = Vertices[k];
				MaxBounds = Vertices[k];
			}
			else
			{
				ExpandBounds( MinBounds, MaxBounds, D3DXVECTOR3( x, y, z ) );
			}
		}

		// parse UV references
		int32					HasUVs			= ReadShort( File );

		for ( k = 0; k < TotalVertices; k++ )
		{
			uint32	Index	( 0 );

			if ( HasUVs == 1 )
			{
				Index = ReadInt( File ) + UVsLoaded;
			}

			UVReferences.push_back( Index );
		}

		// parse the acutal UV coordinates
		int32						ActualUVs	= 1;

		if ( HasUVs == 1 )
		{
			ActualUVs = ReadInt( File );

			for ( k = 0 ; k < ActualUVs; k++ )
			{
				x = ReadFloat( File );
				y = ReadFloat( File );

				UVs.push_back( D3DXVECTOR3( x, y * -1.0f, 0 ) );
			}
		}
		else
		{
			for ( k = 0 ; k < ActualUVs; k++ )
			{
				UVs.push_back( D3DXVECTOR3( 0, 0, 0 ) );
			}
		}

		// parse normals
		int32						HasNormals		= ReadShort( File );

		if( HasNormals > 0 )
		{
			for ( k = 0; k < TotalVertices; k++ )
			{
				NormalReferences.push_back( ReadInt( File ) + Normals.size() );
			}

			// parse the actual normals
			int32 ActualNormals = ReadInt( File );

			for ( k = 0 ; k < ActualNormals; k++ )
			{
				int32 AU = ReadByte( File );
				int32 AV = ReadByte( File );

				Normals.push_back( D3DXVECTOR3( DecompressNormal( AU, AV, 0 ),
												DecompressNormal( AU, AV, 2 ),
												DecompressNormal( AU, AV, 1 ) ) );
			}
		}
		else
		{
			if( Normals.size() == 0)
			{
				Normals.push_back( D3DXVECTOR3( 0, 1, 0 ) );
			}
			for( int32 n = 0; n < TotalVertices; n++ )
			{
				NormalReferences.push_back( Normals.size() - 1 );
			}
		}

		// parse vertex colors
		int32						ActualVertexColors	= ReadInt( File );

		for ( k = 0; k < TotalVertices; k++ )
		{
			if ( k >= ActualVertexColors )
			{
				VertexColors.push_back( D3DXVECTOR3( 1, 1, 1 ) );
			}
			else
			{
				uint32 r = ReadShort( File );
				uint32 g = ReadShort( File );
				uint32 b = ReadShort( File );
				
				VertexColors.push_back( D3DXVECTOR3( r / 255.0f,
												     g / 255.0f,
												     b / 255.0f ) );
			}
		}

		// parse the number of triangles in the whole mesh
		uint32	TotalTriangles		= ReadInt( File );
		// and the number of submodelsegments in this object
		uint32	SubModelSegments	= ReadInt( File );


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
		for ( uint32 m = 0; m < SubModelSegments; m++ )
		{
			// parse the material ID of this submesh (each submesh only has 1)
			uint32	MaterialID	= ReadShort( File );
			// make sure its valid
			if ( MaterialID >= m_TotalMaterials )
			{
				MaterialID = m_TotalMaterials - 1;
			}
			MaterialReferenced[MaterialID] = kTrue;

			uint32	ActualTriangles	= ReadInt( File );

			m_TotalFaces += ActualTriangles;


			// parse the actual triangle indices for each corner vert
			for ( uint32 k = 0 ; k < ActualTriangles; k++ )
			{

				a = ReadInt( File ) + VertexReferencesLoaded;
				b = ReadInt( File ) + VertexReferencesLoaded;
				c = ReadInt( File ) + VertexReferencesLoaded;

				Triangles.push_back( D3DXVECTOR4( (float32)a, 
												  (float32)b, 
												  (float32)c, 
												  (float32)MaterialID ) );

				if( MaterialCollideable[MaterialID] )
				{
					m_pCollisionList->AddFace( VertexReferences[c], 
											   VertexReferences[b], 
											   VertexReferences[a] );

				}
			}

			m_TotalSubMeshes++;
		}

	}

	m_pSubModel->SetMinBounds( m_LocalMinBounds );
	m_pSubModel->SetMaxBounds( m_LocalMaxBounds );

	m_pSubModel->CreateBuffers( pD3DDevice,
								m_TotalFaces,
								VertexReferences,
								UVReferences,
								Vertices,
								UVs,
								NormalReferences,
								Normals,
								VertexColors  );




	for( uint32 m = 0; m < Materials; m++ )
	{
		// we only add a segment if the material is actually in use by the model
		if( MaterialReferenced[m] )
		{
			// create this segment as part of the submodel
			// (this effectively makes an index buffer to the submodel's
			// vertex buffer)
			uint32	SegmentIndex	= m_pSubModel->AddSubModelSegment( m_pRefManager,
																		m_pSettings,																	pD3DDevice,
																		VertexReferences,
																		Triangles,
																		m,
																		MaterialType[m],
																		MaterialIsDoubleSided[m] );

			// if we've actually got a material, apply it to this submodelsegment
			if ( m_TotalMaterials > 0 )
			{
				m_pSubModel->SetMaterial( MaterialReferences[m],
										  SegmentIndex );
			}

			if( !MaterialVisible[m] )
			{
				m_pSubModel->SetVisible( SegmentIndex, kFalse );
			}
		}
	}

	// clean up extra indices in the index buffer 
	m_pSubModel->CleanIB( pD3DDevice );

	// do all setup calculations for the face list (usable in collisions
	// optimize out any redundant verts, since we merged many meshes together - may not need to do this
	m_pCollisionList->Optimize();
	m_pCollisionList->CalculateFaceBounds();					
	m_pCollisionList->CalculateNormals();

	// sort references to each subsegment into a bucket
	// of the appropriate material type, for optimized rendering later
	for ( uint32 s = 0; s < m_pSubModel->SubModelSegments() ; s++ )
	{
		CSubModelSegment&	Segment				= m_pSubModel->GetSubModelSegment( s );			
		m_pMaterialSegments[Segment.MaterialType()].push_back( &Segment );
	}

	// read all light data in the file - a type, position, quaternion (which we'll convert to a ray )
	// and a color
	for( i = 0; i < Lights; i ++ )
	{
		std::string Name = ReadString( File );

		ELightType Type = (ELightType)ReadShort(File);

		float32 X = ReadFloat( File ) * ModelScale;
		float32 Y = ReadFloat( File ) * ModelScale;
		float32 Z = ReadFloat( File ) * ModelScale;
		
			
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
	for( i = 0; i < Points; i ++ )
	{
		std::string Name = ReadString( File );

		float32	X = ReadFloat( File ) * ModelScale;
		float32	Y = ReadFloat( File ) * ModelScale;
		float32	Z = ReadFloat( File ) * ModelScale;

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
	for( i = 0; i < Paths; i ++ )
	{
		std::string Name = ReadString( File );

		float32	X = ReadFloat( File ) * ModelScale;
		float32	Y = ReadFloat( File ) * ModelScale;
		float32	Z = ReadFloat( File ) * ModelScale;

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
				float32 PX = ReadFloat( File ) * ModelScale;
				float32 PY = ReadFloat( File ) * ModelScale;
				float32 PZ = ReadFloat( File ) * ModelScale;

				if( k == 0 )
				{
					m_Paths.back()->AddPoint( D3DXVECTOR3( PX, PY, PZ ) );
				}
			}
		}

		uint32 HasUserData = ReadShort( File );

		if( HasUserData > 0 )
		{
			m_Paths.back()->SetUserData( ReadString( File ) );
		}

	}
	fclose( File );

	for( i = 0; i < MaterialReferences.size(); i++ )
	{
		m_pRefManager.RemoveMaterial( MaterialReferences[i] );
		MaterialReferences[i] = NULL;
	}
	
}	// CModel::Load()


// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not

bool CModel::RayCollision( const D3DXVECTOR3& Start,	// start of collision ray
						   const D3DXVECTOR3& End,		// end of collision ray
						   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
						   D3DXVECTOR3& ImpactNormal )	// impact normal to be filled
{
	float32		BestDistance	( KCollisionInvalid );

	D3DXVECTOR3	MinBounds		( Start );
	D3DXVECTOR3	MaxBounds		( Start );


	ExpandBounds( MinBounds, MaxBounds, End );

	m_pCollisionList->RayCollision( Start,
								    End,
									MinBounds,
									MaxBounds,
									ImpactPoint,
									ImpactNormal,
									BestDistance );

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CModel::RayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CModel::SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
							  const D3DXVECTOR3& End,		// end of collision ray
							  float32 Radius,				// radius of sphere to check along collision ray
							  D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
							  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
							  D3DXVECTOR3& ImpactNormal )	// impact normal to be filled
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


	m_pCollisionList->SphereCollision( Start,
									   End,
									   MinBounds,
									   MaxBounds,
									   Radius,
									   FinalPosition,
									   ImpactPoint,
									   ImpactNormal,
									   BestDistance );

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CModel::SphereCollision()


void CModel::SetUserData( uint32 Id,			// id of material to set user data on
						  int32 Value )		// value to set for user data
{
	if ( Id < m_TotalMaterials )
	{
		m_UserData[Id] = Value;
	}
} // CModel::SetUserData()

void CModel::SetUserData( std::string Name,	// name of material to set user data on
						  int32 Value )		// value to set for user data
{
	for ( uint32 i = 0; i < m_TotalMaterials; i++ )
	{
		if ( m_MaterialNames[i] == Name )
		{
			m_UserData[i] = Value;
		}
	}
} // CModel::SetUserData()

bool CModel::HasMaterial( const std::string& Name )	// name of material to check existence of
{
	for ( uint32 i = 0; i < m_TotalMaterials; i++ )
	{
		if ( m_MaterialNames[i] == Name )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CModel::HasMaterial()

void CModel::ResetMorph( void )
{
	m_pSubModel->ResetMorph();
} // CModel::ResetMorph()

void CModel::SetMorphable( void )
{
	m_pSubModel->SetMorphable();
} // CModel::SetMorphable()

void CModel::EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{
	m_CastsShadows = kTrue;

	m_pSubModel->EnableShadows( pD3DDevice );

} // CModel::EnableShadows()

// render shadows
void CModel::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
							const D3DXVECTOR3& Light )				// direction of light shadow is being cast from
{
	m_pSubModel->RenderShadows( pD3DDevice,
								*m_pCollisionList,
								CameraSpaceMatrix,
								Light );
} // CModel::RenderShadow()


// morph to the target mesh, given a point, radius, and amount of morph
void CModel::Morph( CModel* target,		// target mesh to morph to
					D3DXVECTOR3 vPoint,	// starting point of the morph
					float32 radius,		// radius of morph influence
					float32 percent )	// percentage to morph
{

	m_pSubModel->Morph( target->m_pSubModel,
					   vPoint,
					   radius,
					   percent );
} // CModel::Morph()


// render a vector of segments with a specific userdata value
void CModel::RenderSegmentSetUserData( LPDIRECT3DDEVICE8 pD3DDevice,										// direct3d device
									   std::vector <CSubModelSegment*> pMaterialSegments,	// vector of segments to render
									   int32 UserDataValue )												// userdata id to render
{
	CSubModelSegment* pCurrentSegment = NULL;
	for ( uint32 i = 0 ; i < pMaterialSegments.size() ; i++ )
	{
		if ( UserDataValue == KAllUserData ||
			 m_UserData[pMaterialSegments[i]->MaterialID()] == UserDataValue )
		{
			if ( pMaterialSegments[i]->Visible() )
			{
				if( pCurrentSegment == NULL ||
					pCurrentSegment->GetMaterial() != pMaterialSegments[i]->GetMaterial() )
				{
					if( pCurrentSegment != NULL )
					{
						pCurrentSegment->UnSet( pD3DDevice );
					}
					pMaterialSegments[i]->Render( pD3DDevice );
					
				}
				else
				{
					pMaterialSegments[i]->RenderNoTexture( pD3DDevice );
				}
				pCurrentSegment = pMaterialSegments[i];
			}
		}
	}
	if( pCurrentSegment != NULL )
	{
		pCurrentSegment->UnSet( pD3DDevice );
	}

} // CModel::RenderSegmentSetUserData()

void CModel::RenderSegmentSetIllumination( LPDIRECT3DDEVICE8 pD3DDevice,						// direct3d device
										   std::vector <CSubModelSegment*> pMaterialSegments,	// vector of segments to render
										   int32 UserDataValue,									// userdata id to render
										   EMaterialType MaterialType )							// material type we will render
{
	bool SetShifter = kFalse;
	for ( uint32 i = 0 ; i < pMaterialSegments.size() ; i++ )
	{
		if ( UserDataValue == KAllUserData ||
			 m_UserData[pMaterialSegments[i]->MaterialID()] == UserDataValue  )
		{
			if( pMaterialSegments[i]->Visible() )
			{
				// only render meshes with an illumination map
				if( pMaterialSegments[i]->GetMaterial() != NULL &&
					pMaterialSegments[i]->GetMaterial()->HasBaseTexture() &&
					pMaterialSegments[i]->GetMaterial()->HasIllumination() &&
					pMaterialSegments[i]->GetMaterial()->GetIlluminationType() == MaterialType )
				{
					pD3DDevice->SetTexture( 0, 
											pMaterialSegments[i]->GetMaterial()->GetIlluminationTexture() );

					// set any second-layer shifters
					if ( pMaterialSegments[i]->GetMaterial()->IsAnimated( 1 ) )
					{
						pD3DDevice->SetTextureStageState( 0,
														D3DTSS_TEXTURETRANSFORMFLAGS,
														D3DTTFF_COUNT2 );

						D3DXMATRIX	matShift;
						D3DXMatrixIdentity( &matShift );

						// set the matrix using the shifting component of the material
						matShift._31 = pMaterialSegments[i]->GetMaterial()->GetU( 1 );
						matShift._32 = pMaterialSegments[i]->GetMaterial()->GetV( 1 );

						pD3DDevice->SetTransform( D3DTS_TEXTURE0, &matShift );
						SetShifter = kTrue;
					}

					pMaterialSegments[i]->RenderNoTexture( pD3DDevice );
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
} // CModel::RenderSegmentSetIllumination()


void CModel::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
					 const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
					 const D3DXVECTOR3& Light )				// direction of light for use in normal mapping
{
	RenderUserData( pD3DDevice, CameraSpaceMatrix, NULL, KAllUserData, Light );
} // CModel::Render()

void CModel::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
					 const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
					 LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use - NULL if none
					 const D3DXVECTOR3& Light )				// direction of light for use in normal mapping
{
	RenderUserData( pD3DDevice, CameraSpaceMatrix, pCubeMap, KAllUserData, Light );
} // CModel::Render()

// render only subsegments with materials whose userdata has been set to the requested
// value
void CModel::RenderUserData( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							 const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
							 LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use for reflection maps - NULL if none
							 int32 UserDataValue,					// userdata value that each segment must have set to be rendered
							 const D3DXVECTOR3& Light )				// direction of light for use in normal mapping

{
	pD3DDevice->SetTransform( D3DTS_VIEW, &CameraSpaceMatrix );
	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );

	pD3DDevice->SetStreamSource( 0,
								 m_pSubModel->GetVB(),
								 sizeof( PLAINVERTEXMODEL ) );

	pD3DDevice->SetIndices( m_pSubModel->GetIB(), 0L );


	RenderSegmentSetUserData( pD3DDevice,
							  m_pMaterialSegments[KMaterialStandard],
							  UserDataValue );

	// set up to render normal mapped segments
	if ( m_pMaterialSegments[KMaterialNormalMap].size() > 0 &&
		 m_pSettings.GetSettings( KSetNormalMapping ) > 0 )
	{

		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3 );

		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, m_pSettings.GetSettings( KSetNormalMapAdditive ) );

		pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_MODULATE );

		// calculate our texture factor from the local light direction
		D3DXVECTOR3 LightDir = Light;
		D3DXVec3Normalize( &LightDir, &LightDir );
		DWORD TFactor = VectorToRGB( LightDir );								
		pD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR, TFactor );	

		// no specular - it looks funny
		if ( m_pSettings.GetSettings( KSetSpecular ) )
		{
			pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
		}
	}

	RenderSegmentSetUserData( pD3DDevice,
				 			  m_pMaterialSegments[KMaterialNormalMap],
							  UserDataValue );
	// turn off normal mapping states
	if ( m_pMaterialSegments[KMaterialNormalMap].size() > 0 &&
		 m_pSettings.GetSettings( KSetNormalMapping ) > 0 )
	{

		if ( m_pSettings.GetSettings( KSetSpecular ) )
		{
			pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
		}

		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
		pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
	}

	// static reflection mapped segments
	if ( m_pMaterialSegments[KMaterialStaticCubeMap].size() > 0 )
	{
		if ( m_pSettings.GetSettings( KSetCubeMapping ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_TEXCOORDINDEX,
											  D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			// try to do specular mapping at the same time
			if ( m_pSettings.GetSettings( KSetReflectionSpecularMapping ) &&
				 m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_SPECULAR );
				pD3DDevice->SetTextureStageState( 2,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			}

		}	// we try to fall back to just  specular mapping if we can't do this one....
		else if( m_pSettings.GetSettings( KSetSpecularMapping ) > 0 &&
				 m_pSettings.GetSettings( KSetSpecular ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_SPECULAR );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			// turn specular off...
			if ( m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
			}

		}
	}
	RenderSegmentSetUserData( pD3DDevice,
							  m_pMaterialSegments[KMaterialStaticCubeMap],
							  UserDataValue );

	if ( m_pMaterialSegments[KMaterialStaticCubeMap].size() > 0 )
	{
		if( m_pSettings.GetSettings( KSetCubeMapping ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_TEXTURETRANSFORMFLAGS,
											  D3DTTFF_DISABLE );

			// if we had specular mapping too, turn it off
			if ( m_pSettings.GetSettings( KSetReflectionSpecularMapping ) &&
				 m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
			}
		}
		else if( m_pSettings.GetSettings( KSetSpecularMapping ) > 0 &&
				 m_pSettings.GetSettings( KSetSpecular ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	
			// turn real specular back on...
			if ( m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
			}
		}
	}

	// reflection mapped segments
	if ( m_pMaterialSegments[KMaterialReflect].size() > 0 )
	{
		if ( m_pSettings.GetSettings( KSetCubeMapping ) > 0 &&
			 pCubeMap != NULL )
		{
			pD3DDevice->SetTexture( 1, pCubeMap );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_TEXCOORDINDEX,
											  D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			// try to do specular mapping at the same time
			if ( m_pSettings.GetSettings( KSetReflectionSpecularMapping ) &&
				 m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_SPECULAR );
				pD3DDevice->SetTextureStageState( 2,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			}

		}	// we try to fall back to just  specular mapping if we can't do this one....
		else if( m_pSettings.GetSettings( KSetSpecularMapping ) > 0 &&
				 m_pSettings.GetSettings( KSetSpecular ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_SPECULAR );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			// turn specular off...
			if ( m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
			}

		}
	}
	RenderSegmentSetUserData( pD3DDevice,
							  m_pMaterialSegments[KMaterialReflect],
							  UserDataValue );

	if ( m_pMaterialSegments[KMaterialReflect].size() > 0 )
	{
		if( m_pSettings.GetSettings( KSetCubeMapping ) > 0 &&
			pCubeMap != NULL )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_TEXTURETRANSFORMFLAGS,
											  D3DTTFF_DISABLE );

			// if we had specular mapping too, turn it off
			if ( m_pSettings.GetSettings( KSetReflectionSpecularMapping ) &&
				 m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
			}
		}
		else if( m_pSettings.GetSettings( KSetSpecularMapping ) > 0 &&
				 m_pSettings.GetSettings( KSetSpecular ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	
			// turn real specular back on...
			if ( m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
			}
		}
	}



	// spherically mapped segments
	if ( m_pMaterialSegments[KMaterialSphereMap].size() > 0 )
	{
		if ( m_pSettings.GetSettings( KSetCubeMapping ) > 0 )
		{
			pD3DDevice->SetTexture( 1, pCubeMap );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_TEXCOORDINDEX,
											  D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			// try to do specular mapping at the same time
			if ( m_pSettings.GetSettings( KSetReflectionSpecularMapping ) &&
				 m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_SPECULAR );
				pD3DDevice->SetTextureStageState( 2,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			}

		}	// we try to fall back to just  specular mapping if we can't do this one....
		else if( m_pSettings.GetSettings( KSetSpecularMapping ) > 0 &&
				 m_pSettings.GetSettings( KSetSpecular ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_SPECULAR );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			// turn specular off...
			if ( m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
			}

		}
	}

	RenderSegmentSetUserData( pD3DDevice,
							  m_pMaterialSegments[KMaterialSphereMap],
							  UserDataValue );

	if ( m_pMaterialSegments[KMaterialSphereMap].size() > 0 )
	{
		if( m_pSettings.GetSettings( KSetCubeMapping ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_TEXTURETRANSFORMFLAGS,
											  D3DTTFF_DISABLE );

			// if we had specular mapping too, turn it off
			if ( m_pSettings.GetSettings( KSetReflectionSpecularMapping ) &&
				 m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
			}
		}
		else if( m_pSettings.GetSettings( KSetSpecularMapping ) > 0 &&
				 m_pSettings.GetSettings( KSetSpecular ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	
			// turn real specular back on...
			if ( m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
			}
		}
	}

	// if any alpha materials are included -
	if ( m_pMaterialSegments[KMaterialPreAlpha].size() > 0 ||
		 m_pMaterialSegments[KMaterialAlpha].size() > 0 ||
		 m_pMaterialSegments[KMaterialShadow].size() > 0 )
	{
		// set alpha states
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE );

		pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		if ( m_pSettings.GetSettings( KSetAlphaTest ) )
		{
			pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kTrue );
			pD3DDevice->SetRenderState( D3DRS_ALPHAREF, ( DWORD ) 50 );
			pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
		}	

		// render the alphas
		RenderSegmentSetUserData( pD3DDevice,
								  m_pMaterialSegments[KMaterialPreAlpha],
								  UserDataValue );
		RenderSegmentSetUserData( pD3DDevice,
								  m_pMaterialSegments[KMaterialAlpha],
								  UserDataValue );


		if ( m_pSettings.GetSettings( KSetAlphaTest ) )
		{
			pD3DDevice->SetRenderState( D3DRS_ALPHAREF, ( DWORD ) 1 );
		}

		// don't zwrite
		pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

		pD3DDevice->SetTextureStageState( 0,
										  D3DTSS_MIPFILTER,
										  m_pSettings.GetSettings( KSetMipFilter ) );

		RenderSegmentSetUserData( pD3DDevice,
								  m_pMaterialSegments[KMaterialShadow],
								  UserDataValue );


		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

	}

	// render multiply materials
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

	RenderSegmentSetUserData( pD3DDevice,
							  m_pMaterialSegments[KMaterialMultiply],
							  UserDataValue );

	// now render any multiply illumination maps that are on top of any previous materials
	for( uint32 j = 0; j < KMaterialTypes; j ++ )
	{
		RenderSegmentSetIllumination( pD3DDevice,
									  m_pMaterialSegments[j],
									  UserDataValue,
									  KMaterialMultiply );
	}

	// render glow materials
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );



	RenderSegmentSetUserData( pD3DDevice,
							  m_pMaterialSegments[KMaterialIllumination],
							  UserDataValue );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	// now we render any additive reflections that don't zwrite -
	// lots of state changes again, so bear with me

	// static reflection mapped segments
	if ( m_pMaterialSegments[KMaterialStaticCubeMapPost].size() > 0 )
	{
		if ( m_pSettings.GetSettings( KSetCubeMapping ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_TEXCOORDINDEX,
											  D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			// try to do specular mapping at the same time
			if ( m_pSettings.GetSettings( KSetReflectionSpecularMapping ) &&
				 m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_SPECULAR );
				pD3DDevice->SetTextureStageState( 2,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			}

		}	// we try to fall back to just  specular mapping if we can't do this one....
		else if( m_pSettings.GetSettings( KSetSpecularMapping ) > 0 &&
				 m_pSettings.GetSettings( KSetSpecular ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_SPECULAR );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			// turn specular off...
			if ( m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
			}

		}
	}
	RenderSegmentSetUserData( pD3DDevice,
							  m_pMaterialSegments[KMaterialStaticCubeMapPost],
							  UserDataValue );

	if ( m_pMaterialSegments[KMaterialStaticCubeMapPost].size() > 0 )
	{
		if( m_pSettings.GetSettings( KSetCubeMapping ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_TEXTURETRANSFORMFLAGS,
											  D3DTTFF_DISABLE );

			// if we had specular mapping too, turn it off
			if ( m_pSettings.GetSettings( KSetReflectionSpecularMapping ) &&
				 m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
			}
		}
		else if( m_pSettings.GetSettings( KSetSpecularMapping ) > 0 &&
				 m_pSettings.GetSettings( KSetSpecular ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	
			// turn real specular back on...
			if ( m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
			}
		}
	}

	// reflection mapped segments
	if ( m_pMaterialSegments[KMaterialReflectPost].size() > 0 )
	{
		if ( m_pSettings.GetSettings( KSetCubeMapping ) > 0 &&
			 pCubeMap != NULL )
		{
			pD3DDevice->SetTexture( 1, pCubeMap );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_TEXCOORDINDEX,
											  D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			// try to do specular mapping at the same time
			if ( m_pSettings.GetSettings( KSetReflectionSpecularMapping ) &&
				 m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_SPECULAR );
				pD3DDevice->SetTextureStageState( 2,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			}

		}	// we try to fall back to just  specular mapping if we can't do this one....
		else if( m_pSettings.GetSettings( KSetSpecularMapping ) > 0 &&
				 m_pSettings.GetSettings( KSetSpecular ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_SPECULAR );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			// turn specular off...
			if ( m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
			}

		}
	}
	RenderSegmentSetUserData( pD3DDevice,
							  m_pMaterialSegments[KMaterialReflectPost],
							  UserDataValue );

	if ( m_pMaterialSegments[KMaterialReflectPost].size() > 0 )
	{
		if( m_pSettings.GetSettings( KSetCubeMapping ) > 0 &&
			pCubeMap != NULL )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_TEXTURETRANSFORMFLAGS,
											  D3DTTFF_DISABLE );

			// if we had specular mapping too, turn it off
			if ( m_pSettings.GetSettings( KSetReflectionSpecularMapping ) &&
				 m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
			}
		}
		else if( m_pSettings.GetSettings( KSetSpecularMapping ) > 0 &&
				 m_pSettings.GetSettings( KSetSpecular ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	
			// turn real specular back on...
			if ( m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
			}
		}
	}



	// spherically mapped segments
	if ( m_pMaterialSegments[KMaterialSphereMapPost].size() > 0 )
	{
		if ( m_pSettings.GetSettings( KSetCubeMapping ) > 0 )
		{
			pD3DDevice->SetTexture( 1, pCubeMap );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_TEXCOORDINDEX,
											  D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			// try to do specular mapping at the same time
			if ( m_pSettings.GetSettings( KSetReflectionSpecularMapping ) &&
				 m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_SPECULAR );
				pD3DDevice->SetTextureStageState( 2,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			}

		}	// we try to fall back to just  specular mapping if we can't do this one....
		else if( m_pSettings.GetSettings( KSetSpecularMapping ) > 0 &&
				 m_pSettings.GetSettings( KSetSpecular ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_SPECULAR );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_COLOROP,
											  D3DTOP_MODULATEALPHA_ADDCOLOR );
			// turn specular off...
			if ( m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
			}

		}
	}

	RenderSegmentSetUserData( pD3DDevice,
							  m_pMaterialSegments[KMaterialSphereMapPost],
							  UserDataValue );

	if ( m_pMaterialSegments[KMaterialSphereMapPost].size() > 0 )
	{
		if( m_pSettings.GetSettings( KSetCubeMapping ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
			pD3DDevice->SetTextureStageState( 1,
											  D3DTSS_TEXTURETRANSFORMFLAGS,
											  D3DTTFF_DISABLE );

			// if we had specular mapping too, turn it off
			if ( m_pSettings.GetSettings( KSetReflectionSpecularMapping ) &&
				 m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
			}
		}
		else if( m_pSettings.GetSettings( KSetSpecularMapping ) > 0 &&
				 m_pSettings.GetSettings( KSetSpecular ) > 0 )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	
			// turn real specular back on...
			if ( m_pSettings.GetSettings( KSetSpecular ) )
			{
				pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
			}
		}
	}

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );


	// now render any illumination maps that are on top of any previous materials
	for( j = 0; j < KMaterialTypes; j ++ )
	{
		RenderSegmentSetIllumination( pD3DDevice,
									  m_pMaterialSegments[j],
									  UserDataValue,
									  KMaterialIllumination );
	}

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	if ( m_pSettings.GetSettings( KSetAlphaTest ) )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kFalse );
	}	

} // CModel::RenderUserData()

// renders the model with all non-glowing objects as invisible, but written to the zbuffer.
// all glowing objects are visible, but do NOT write to the zbuffer.
// this is used for post effects like light-streaking.
void CModel::RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							 const D3DXMATRIX& ViewportMatrix )			// viewport matrix
							 
{

	pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );
	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );

	pD3DDevice->SetStreamSource( 0,
								 m_pSubModel->GetVB(),
								 sizeof( PLAINVERTEXMODEL ) );

	pD3DDevice->SetIndices( m_pSubModel->GetIB(), 0L );

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

	for ( uint32 i = 0 ; i < m_pMaterialSegments.size() ; i++ )
	{
		// only render objects that are totally opaque
		if( i == KMaterialReflect ||
			i == KMaterialStaticCubeMap ||
			i == KMaterialStandard ||
			i == KMaterialNormalMap ||
			i == KMaterialSphereMap ||
			i == KMaterialAlpha )
		{
			for ( uint32 k = 0 ; k < m_pMaterialSegments[i].size() ; k++ )
			{
				if( m_pMaterialSegments[i][k]->GetMaterial() != NULL &&
					!m_pMaterialSegments[i][k]->GetMaterial()->AllowZWrite() )
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

				if( m_pMaterialSegments[i][k]->Visible() )
				{

					// alphas are a special case - they need to set their texture,
					// because the transparency determines the z-write
					if( i == KMaterialAlpha )
					{
						m_pMaterialSegments[i][k]->Render( pD3DDevice );
						m_pMaterialSegments[i][k]->UnSet( pD3DDevice );
					}
					else
					{
						m_pMaterialSegments[i][k]->RenderNoTexture( pD3DDevice );
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
	pD3DDevice->SetTextureStageState( 1, D3DTSS_MIPFILTER, D3DTEXF_NONE );

	// now we render only the illumination textures. These will 'glow' and be properly occluded.
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	// render glow materials

	RenderSegmentSetUserData( pD3DDevice,
							  m_pMaterialSegments[KMaterialIllumination],
							  KAllUserData );


	// now we render any meshes that are not additive, but are marked as 'glowing'.
	for ( i = 0 ; i < m_pMaterialSegments.size() ; i++ )
	{
		// only render objects that are totally opaque
		if( i == KMaterialReflect ||
			i == KMaterialStaticCubeMap ||
			i == KMaterialStandard ||
			i == KMaterialNormalMap ||
			i == KMaterialSphereMap )
		{
			for ( uint32 k = 0 ; k < m_pMaterialSegments[i].size() ; k++ )
			{
				uint32 Renders( 0 ); // how many renders we did
				// and then render the submodelsegments for each material set
				if( m_pMaterialSegments[i][k]->GetMaterial() != NULL &&
					m_pMaterialSegments[i][k]->GetMaterial()->GetDataID() != KGlowDataID )
				{
					break;
				}
				if( m_pMaterialSegments[i][k]->GetMaterial() != NULL &&
					!m_pMaterialSegments[i][k]->GetMaterial()->AllowZWrite() )
				{
					break;
				}
				if( m_pMaterialSegments[i][k]->Visible() )
				{
					m_pMaterialSegments[i][k]->Render( pD3DDevice );
					Renders++;
				}
					m_pMaterialSegments[i][k]->UnSet( pD3DDevice );
			}
		}
	}

	// now render any illumination maps that are on top of any previous materials
	for( uint32 j = 0; j < KMaterialTypes; j ++ )
	{
		RenderSegmentSetIllumination( pD3DDevice, 
									  m_pMaterialSegments[j], 
									  KAllUserData, 
									  KMaterialIllumination );
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
} // CModel::RenderGlowPass()

// renders the model with highlights
void CModel::RenderIlluminated( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								const D3DXMATRIX& ViewportMatrix,			// viewport matrix
								const D3DXVECTOR3& Light )					// direction of light for use in normal mapping
							 
{

	pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );
	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );

	pD3DDevice->SetStreamSource( 0,
								 m_pSubModel->GetVB(),
								 sizeof( PLAINVERTEXMODEL ) );

	pD3DDevice->SetIndices( m_pSubModel->GetIB(), 0L );

	// first we set the blending mode so that it DOESN'T write to texture,
	// but only to the zbuffer.
	// we have to render the non-visible objects to the zbuffer so that
	// glowing objects will be properly occluded.
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );


	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	for ( uint32 i = 0 ; i < m_pMaterialSegments.size() ; i++ )
	{
		// only render objects that are totally opaque
		if( i == KMaterialReflect ||
			i == KMaterialStaticCubeMap ||
			i == KMaterialSphereMap ||
			i == KMaterialReflectPost ||
			i == KMaterialStaticCubeMapPost ||
			i == KMaterialSphereMapPost ||
			i == KMaterialStandard ||
			i == KMaterialNormalMap ||
			i == KMaterialAlpha ||
			i == KMaterialPreAlpha ||
			i == KMaterialShadow )
		{
			for ( uint32 k = 0 ; k < m_pMaterialSegments[i].size() ; k++ )
			{

				if( i == KMaterialReflectPost ||
					i == KMaterialStaticCubeMapPost ||
					i == KMaterialSphereMapPost )
				{
					if( m_pSettings.Capabilities().IsCapable( KCanDoModulate2X ) )
					{
						pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
					}
				}

				if( i == KMaterialAlpha ||
					i == KMaterialPreAlpha ||
					i == KMaterialShadow ||
					i == KMaterialReflectPost ||
					i == KMaterialStaticCubeMapPost ||
					i == KMaterialSphereMapPost )
				{
					pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
				}


				if ( i == KMaterialNormalMap &&
					m_pSettings.GetSettings( KSetNormalMapping ) > 0 )
				{

					pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
					pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
					pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3 );

					pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
					pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
					pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, m_pSettings.GetSettings( KSetNormalMapAdditive ) );

					pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
					pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
					pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_MODULATE );

					// calculate our texture factor from the local light direction
					D3DXVECTOR3 LightDir = Light;
					D3DXVec3Normalize( &LightDir, &LightDir );
					DWORD TFactor = VectorToRGB( LightDir );								
					pD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR, TFactor );

					// no specular - it looks funny
					if ( m_pSettings.GetSettings( KSetSpecular ) )
					{
						pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
					}
				}

				if( m_pMaterialSegments[i][k]->Visible() )
				{

					// alphas are a special case - they need to set their texture,
					// because the transparency determines the z-write
					m_pMaterialSegments[i][k]->Render( pD3DDevice );
				}

				// turn off normal mapping states
				if ( i == KMaterialNormalMap &&
					m_pSettings.GetSettings( KSetNormalMapping ) > 0 )
				{

					// turn it back on...
					if ( m_pSettings.GetSettings( KSetSpecular ) )
					{
						pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
					}

					pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
					pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
					pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
					pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
					pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
					pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
					pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
				}

				if( i == KMaterialAlpha ||
					i == KMaterialPreAlpha ||
					i == KMaterialShadow ||
					i == KMaterialReflectPost ||
					i == KMaterialStaticCubeMapPost ||
					i == KMaterialSphereMapPost )
				{
					pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
				}

				if( i == KMaterialReflectPost ||
					i == KMaterialStaticCubeMapPost ||
					i == KMaterialSphereMapPost )
				{
					pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
				}
			}
		}
	}

	// render glow materials

	RenderSegmentSetUserData( pD3DDevice,
							  m_pMaterialSegments[KMaterialIllumination],
							  KAllUserData );


	// now render any illumination maps that are on top of any previous materials
	for( uint32 j = 0; j < KMaterialTypes; j ++ )
	{
		RenderSegmentSetIllumination( pD3DDevice, 
									  m_pMaterialSegments[j], 
									  KAllUserData, 
									  KMaterialIllumination );
	}

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

	pD3DDevice->SetTextureStageState( 1,
									  D3DTSS_MIPFILTER,
									  m_pSettings.GetSettings( KSetMipFilter ) );
} // CModel::RenderIlluminated()

// render the mesh without any textures
void CModel::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
							  const D3DXMATRIX& CameraSpaceMatrix )		// camera space matrix
{
	pD3DDevice->SetTransform( D3DTS_VIEW, &CameraSpaceMatrix );

	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );

	pD3DDevice->SetStreamSource( 0,
								 m_pSubModel->GetVB(),
								 sizeof( PLAINVERTEXMODEL ) );

	pD3DDevice->SetIndices( m_pSubModel->GetIB(), 0L );


	for ( uint32 j = 0 ; j < m_pSubModel->SubModelSegments() ; j++ )
	{
		m_pSubModel->GetSubModelSegment( j ).RenderNoTexture( pD3DDevice );
	}

} // CModel::RenderNoTexture()

// render only subsegments with materials whose userdata has been set to the requested
// value, but don't set the textures
void CModel::RenderUserDataNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									  const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
									  int32 UserDataValue )					// userdata value that each segment must have set to be rendered
{
	pD3DDevice->SetTransform( D3DTS_VIEW, &CameraSpaceMatrix );

	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );



	for ( uint32 j = 0 ; j < m_pSubModel->SubModelSegments() ; j++ )
	{
		if ( m_UserData[m_pSubModel->GetSubModelSegment( j ).MaterialID()] == UserDataValue )
		{
			m_pSubModel->GetSubModelSegment( j ).RenderNoTexture( pD3DDevice );
		}
	}

} // CModel::RenderUserData()

// calculates the local bounds of all faces with a given material applied to them
// returns kTrue if the material was found, kFalse if not
bool CModel::GetBoundsOfMaterial( const std::string& Name,	// name of the material to check for
								  D3DXVECTOR3* Min,			// min bounds to fill
								  D3DXVECTOR3* Max )		// max bounds to fill
{
	for ( uint32 j = 0 ; j < m_pSubModel->SubModelSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we simply fill
		// in its local bounds, and return true
		if ( m_MaterialNames[m_pSubModel->GetSubModelSegment( j ).MaterialID()] == Name )
		{
			*Min = m_pSubModel->GetSubModelSegment( j ).MinBounds();
			*Max = m_pSubModel->GetSubModelSegment( j ).MaxBounds();
			return kTrue;
		}
	}
	return kFalse;
} // CModel::GetBoundsOfMaterial()

// returns a pointer to the requested material by index
CMaterial* CModel::GetMaterial( uint32 MaterialID )	// material ID to retrieve
{
	for ( uint32 j = 0 ; j < m_pSubModel->SubModelSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we 
		// return a pointer to it
		if( m_pSubModel->GetSubModelSegment( j ).MaterialID() == MaterialID )
		{
			return m_pSubModel->GetSubModelSegment( j ).GetMaterial();
		}
	}
	return NULL;
} // CModel::GetMaterial()

// returns a pointer to the requested material by index
CMaterial* CModel::GetMaterialByIndex( uint32 Index )	// index of material to retrieve
{
	for ( uint32 j = 0 ; j < m_pSubModel->SubModelSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we 
		// return a pointer to it
		if( j == Index )
		{
			return m_pSubModel->GetSubModelSegment( j ).GetMaterial();
		}
	}
	return NULL;
} // CModel::GetMaterialByIndex()

// returns the material id of the indexed material
int32 CModel::GetMaterialIDByIndex( uint32 Index )	// index of material to retrieve
{
	for ( uint32 j = 0 ; j < m_pSubModel->SubModelSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we 
		// return a pointer to it
		if( j == Index )
		{
			return m_pSubModel->GetSubModelSegment( j ).MaterialID();
		}
	}
	return 0;
} // CModel::GetMaterialIDByIndex()

// replaces a material by index with a pre-existing material
void CModel::SetMaterial( uint32 MaterialID,		// material ID to overwrite
						  CMaterial* pMaterial )	// material to replace it with
{
	for ( uint32 j = 0 ; j < m_pSubModel->SubModelSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we simply fill
		// in its local bounds, and return true
		if( m_pSubModel->GetSubModelSegment( j ).MaterialID() == MaterialID )
		{
			// only swap materials if this is actually a different material
			if( m_pSubModel->GetSubModelSegment( j ).GetMaterial() != pMaterial )
			{
				// we first need to remove the reference to the previously existing material,
				// so that it will be cleaned up properly
				m_pRefManager.RemoveMaterial( m_pSubModel->GetSubModelSegment( j ).GetMaterial() );
				// we then need to increase the references to the material we're applying 
				m_pSubModel->GetSubModelSegment( j ).SetMaterial( pMaterial );
			}
		}
	}
} // CModel::SetMaterial()

// replaces a material by index with a pre-existing material
void CModel::SetMaterialByIndex( uint32 Index,				// index of material to overwrite
								 CMaterial* pMaterial )	// material to replace it with
{
	for ( uint32 j = 0 ; j < m_pSubModel->SubModelSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we simply fill
		// in its local bounds, and return true
		if( j == Index )
		{
			// only swap materials if this is actually a different material
			if( m_pSubModel->GetSubModelSegment( j ).GetMaterial() != pMaterial )
			{
				// we first need to remove the reference to the previously existing material,
				// so that it will be cleaned up properly
				m_pRefManager.RemoveMaterial( m_pSubModel->GetSubModelSegment( j ).GetMaterial() );
				// we then need to increase the references to the material we're applying 
				m_pSubModel->GetSubModelSegment( j ).SetMaterial( pMaterial );
			}
		}
	}
} // CModel::SetMaterialByIndex()

CPath&	CModel::GetPath( uint32 Index )		// Index of point to retrieve
{
	assert( Index < m_Paths.size() );

	return *m_Paths[Index];
} // CModel::GetPath()

CDataPoint&	CModel::GetPoint( uint32 Index )		// Index of point to retrieve
{
	assert( Index < m_Points.size() );

	return *m_Points[Index];

} // CModel::GetPoint()

CDataLight&	CModel::GetLight( uint32 Index )		// Index of point to retrieve
{
	assert( Index < m_Lights.size() );

	return *m_Lights[Index];

} // CModel::GetLight()


