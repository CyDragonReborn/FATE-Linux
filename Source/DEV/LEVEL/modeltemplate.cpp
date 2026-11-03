/*
	modeltemplate.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/9/2004 

*/

#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/math3D.h"
#
#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "modeltemplate.h"
#include "../MODEL/facelist.h"




CModelTemplate::CModelTemplate( CRefManager& pRefManager ) :	// the reference manager, for tracking submaterials
																m_HasAlpha( kFalse ),
																m_IsTextured( kFalse ),
																m_HasReflection( kFalse ),
																m_TotalSubMeshes( 0 ),
																m_TotalFaces( 0 ),
																m_TotalMaterials( 0 ),
																m_pRefManager( pRefManager ),
																m_pCollisionList( NULL ),
																m_CastsShadows ( kFalse ),
																m_LocalMaxBounds( 0, 0, 0 ),
																m_LocalMinBounds( 0, 0, 0 ),
																m_MaxBounds( 0, 0, 0 ),
																m_MinBounds( 0, 0, 0 )
{

} // CModelTemplate::CModelTemplate()


CModelTemplate::~CModelTemplate()
{
	for( uint32 i = 0; i < m_pMaterialReferences.size(); i++ )
	{
		m_pRefManager.RemoveMaterial( m_pMaterialReferences[i] );
		m_pMaterialReferences[i] = NULL;
	}

	DELETE_SAFELY( m_pCollisionList );

	for( i =0; i < m_Lights.size(); i++ )
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

} // CModelTemplate::~CModelTemplate()


void CModelTemplate::Load( LPDIRECT3DDEVICE8 pD3DDevice,	// the direct3d device - needed to create vert and index buffers
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

	m_pMaterialReferences.resize( m_TotalMaterials );
	m_MaterialReferenced.resize( m_TotalMaterials );

	std::vector <bool>			MaterialHasColorKey		( m_TotalMaterials );
	std::vector <DWORD>			MaterialColorKey		( m_TotalMaterials );
	std::vector <bool>			MaterialIsDoubleSided	( m_TotalMaterials );
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
		m_pMaterialReferences[MaterialID] = NULL;

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
		m_MaterialReferenced[MaterialID] = kFalse;

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
			m_pMaterialReferences[MaterialID] = Material;


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
		m_pMaterialReferences[0] = NULL;
		MaterialIsDoubleSided[0] = kFalse;
		MaterialHasColorKey[0] = kFalse;
		MaterialCollideable[0] = kTrue;
		MaterialVisible[0] = kTrue;
	}

	// we're going to just merge all sub-objects in the model into a single one,
	// to reduce the number of VBs
	m_pCollisionList = new CFaceList();

	// now parse the individual sub-models within the file
	for ( j = 0; j < Objects; j ++ )
	{
		// offset into the list of merged vertices
		int32 VerticesLoaded = m_Vertices.size();
		int32 VertexReferencesLoaded = m_VertexReferences.size();
		int32 UVsLoaded = m_UVs.size();


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
			m_VertexReferences.push_back( ReadInt( File ) + VerticesLoaded );
		}

		int32						ActualVertices	= ReadInt( File );

		// parse the actual vertex values 
		for ( k = 0; k < ActualVertices; k++ )
		{
			x = PivotX + ReadFloat( File ) * ModelScale;
			y = PivotY + ReadFloat( File ) * ModelScale;
			z = PivotZ + ReadFloat( File ) * ModelScale;

			m_Vertices.push_back( D3DXVECTOR3( x, y, z ) );
			
			m_pCollisionList->AddVertex( D3DXVECTOR3( x, y, z ) );

			if ( MinBounds.x == -1 && MaxBounds.z == -1 )
			{
				MinBounds = m_Vertices[k];
				MaxBounds = m_Vertices[k];
			}
			else
			{
				ExpandBounds( MinBounds, MaxBounds, m_Vertices[k] );
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

			m_UVReferences.push_back( Index );
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

				m_UVs.push_back( D3DXVECTOR3( x, y * -1.0f, 0 ) );
			}
		}
		else
		{
			for ( k = 0 ; k < ActualUVs; k++ )
			{
				m_UVs.push_back( D3DXVECTOR3( 0, 0, 0 ) );
			}
		}

		// parse normals
		int32						HasNormals		= ReadShort( File );

		if( HasNormals > 0 )
		{
			for ( k = 0; k < TotalVertices; k++ )
			{
				m_NormalReferences.push_back( ReadInt( File ) + m_Normals.size() );
			}

			// parse the actual normals
			int32 ActualNormals = ReadInt( File );

			for ( k = 0 ; k < ActualNormals; k++ )
			{
				int32 AU = ReadByte( File );
				int32 AV = ReadByte( File );

				m_Normals.push_back( D3DXVECTOR3( DecompressNormal( AU, AV, 0 ),
												DecompressNormal( AU, AV, 2 ),
												DecompressNormal( AU, AV, 1 ) ) );
			}
		}
		else
		{
			if( m_Normals.size() == 0)
			{
				m_Normals.push_back( D3DXVECTOR3( 0, 1, 0 ) );
			}
			for( int32 n = 0; n < TotalVertices; n++ )
			{
				m_NormalReferences.push_back( m_Normals.size() - 1 );
			}
		}

		// parse vertex colors
		int32						ActualVertexColors	= ReadInt( File );

		for ( k = 0; k < TotalVertices; k++ )
		{
			if ( k >= ActualVertexColors )
			{
				m_VertexColors.push_back( D3DXVECTOR3( 1, 1, 1 ) );
			}
			else
			{
				uint32 r = ReadShort( File );
				uint32 g = ReadShort( File );
				uint32 b = ReadShort( File );
				
				m_VertexColors.push_back( D3DXVECTOR3( r / 255.0f,
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
			m_MaterialReferenced[MaterialID] = kTrue;

			uint32	ActualTriangles	= ReadInt( File );

			m_TotalFaces += ActualTriangles;


			// parse the actual triangle indices for each corner vert
			for ( uint32 k = 0 ; k < ActualTriangles; k++ )
			{

				a = ReadInt( File ) + VertexReferencesLoaded;
				b = ReadInt( File ) + VertexReferencesLoaded;
				c = ReadInt( File ) + VertexReferencesLoaded;

				m_Triangles.push_back( D3DXVECTOR4( (float32)a, 
												  (float32)b, 
												  (float32)c, 
												  (float32)MaterialID ) );

				if( MaterialCollideable[MaterialID] )
				{
					m_pCollisionList->AddFace( m_VertexReferences[c], 
											   m_VertexReferences[b], 
											   m_VertexReferences[a] );

				}
			}

			m_TotalSubMeshes++;
		}

	}

	// do all setup calculations for the face list (usable in collisions
	// optimize out any redundant verts, since we merged many meshes together - may not need to do this
	m_pCollisionList->Optimize();
	m_pCollisionList->CalculateFaceBounds();					
	m_pCollisionList->CalculateNormals();


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
	
}	// CModelTemplate::Load()


// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not

bool CModelTemplate::RayCollision( const D3DXVECTOR3& Start,	// start of collision ray
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
} // CModelTemplate::RayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CModelTemplate::SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
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
} // CModelTemplate::SphereCollision()


void CModelTemplate::SetUserData( uint32 Id,			// id of material to set user data on
						  int32 Value )		// value to set for user data
{
	if ( Id < m_TotalMaterials )
	{
		m_UserData[Id] = Value;
	}
} // CModelTemplate::SetUserData()

void CModelTemplate::SetUserData( std::string Name,	// name of material to set user data on
						  int32 Value )		// value to set for user data
{
	for ( uint32 i = 0; i < m_TotalMaterials; i++ )
	{
		if ( m_MaterialNames[i] == Name )
		{
			m_UserData[i] = Value;
		}
	}
} // CModelTemplate::SetUserData()

bool CModelTemplate::HasMaterial( const std::string& Name )	// name of material to check existence of
{
	for ( uint32 i = 0; i < m_TotalMaterials; i++ )
	{
		if ( m_MaterialNames[i] == Name )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CModelTemplate::HasMaterial()


// returns a pointer to the requested material by index
CMaterial* CModelTemplate::GetMaterial( uint32 Index )	// index of material to retrieve
{
	if( Index < m_TotalMaterials && m_MaterialReferenced[Index] )
	{
		return m_pMaterialReferences[Index];
	}
	return NULL;
} // CModelTemplate::GetMaterial()

CPath&	CModelTemplate::GetPath( uint32 Index )		// Index of point to retrieve
{
	assert( Index < m_Paths.size() );

	return *m_Paths[Index];
} // CModelTemplate::GetPath()

CDataPoint&	CModelTemplate::GetPoint( uint32 Index )		// Index of point to retrieve
{
	assert( Index < m_Points.size() );

	return *m_Points[Index];

} // CModelTemplate::GetPoint()

CDataLight&	CModelTemplate::GetLight( uint32 Index )		// Index of point to retrieve
{
	assert( Index < m_Lights.size() );

	return *m_Lights[Index];

} // CModelTemplate::GetLight()


