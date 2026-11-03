/*
	skinnedgeometry.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/4/2004

*/

#pragma warning(disable: 4786) 

#include <assert.h>
#include <stdio.h>
#include <queue>
#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/textfile.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/capabilities.h"
#include "../UTILITIES/vertexdefinitions.h"
#include "../MEDIA/material.h"
#include "../MEDIA/materialconstants.h"
#include "../ANIMATEDMESH/tag.h"
#include "skeleton.h"
#include "activeskinnedanimation.h"
#include "skinnedgeometry.h"
#include "weightedvertex.h"
#include "skinnedmeshsegment.h"
#include "skinnedsubmesh.h"



// the following structures are used at model-load time only.
// they are not used for persistent data

typedef struct
{
	int8   Name[68];
	int32  Textures;
	int32  Vertices;
	int32  Triangles;
	int32  TriangleStart;
	int32  HeaderSize;
	int32  UVStart;
	int32  Verticestart;
	int32  MeshSize;

} MeshHeader;


typedef struct 
{
	int32 Vertex[4];

} Triangle;

typedef struct
{
	float32 UV[2];

} TexCoord;

typedef struct
{
	float32	Position[3];
	float32	Rotation[3][3];

} Tag;

typedef struct
{
	int8	Name[64];
} TagName;

// end loading structures


CSkinnedGeometry::CSkinnedGeometry( CRefManager& pRefManager,	// pointer to the reference manager, for tracking submaterials
									  CSettings& pSettings ):		// pointer to the settings manager, for keeping track of card/machine capabilities
																m_pRefManager( pRefManager ),
																m_pSettings( pSettings ),
																m_CastsShadows( kFalse ),
																m_TotalMaterials( 0 ),
																m_Scale( 1 ),
																m_pSkeleton( NULL ),
																m_pSubModel( NULL )
													
{
	ZeroMemory ( &m_LocalMaxBounds, sizeof ( m_LocalMaxBounds ));
	ZeroMemory ( &m_LocalMinBounds, sizeof ( m_LocalMinBounds ));
} // CSkinnedGeometry::CSkinnedGeometry()


CSkinnedGeometry::~CSkinnedGeometry()
{
	InvalidateDeviceObjects();

	DELETE_SAFELY( m_pSkeleton );

	DELETE_SAFELY( m_pSubModel );


} // CSkinnedGeometry::~CSkinnedGeometry()

void CSkinnedGeometry::InvalidateDeviceObjects( void )
{
	m_pSubModel->InvalidateDeviceObjects();
} // CSkinnedGeometry::InvalidateDeviceObjects()

void CSkinnedGeometry::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	m_pSubModel->RestoreDeviceObjects( pD3DDevice );
} // CSkinnedGeometry::RestoreDeviceObjects()

// see if any reflective materials exist in this model
bool CSkinnedGeometry::HasReflection( void )
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
} // CSkinnedGeometry::HasReflection()


void CSkinnedGeometry::SetUserData( uint32 Id,			// id of material to set user data on
									 int32 Value )		// value to set for user data
{
	if ( Id < m_TotalMaterials )
	{
		m_UserData[Id] = Value;
	}
} // CMSkinnedGeometry::SetUserData()

void CSkinnedGeometry::SetUserData( std::string Name,	// name of material to set user data on
									 int32 Value )		// value to set for user data
{
	for ( uint32 i = 0; i < m_TotalMaterials; i++ )
	{
		if ( m_MaterialNames[i] == Name )
		{
			m_UserData[i] = Value;
		}
	}
} // CSkinnedGeometry::SetUserData()

bool CSkinnedGeometry::HasMaterial( const std::string& Name )	// name of material to check existence of
{
	for ( uint32 i = 0; i < m_TotalMaterials; i++ )
	{
		if ( m_MaterialNames[i] == Name )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CSkinnedGeometry::HasMaterial()


// load geometry, animation data, and materials
bool CSkinnedGeometry::Load( LPDIRECT3DDEVICE8 pD3DDevice,	// the direct3d device - needed to create vert and index buffers
							  std::string FileName,		// path to the mesh file
				   			  std::string TexturePath )		// path to the textures (if "" is supplied, will use the mesh path)

{
	FILE *File = NULL;
	
	// old versions do not support mapping of compressed frames
	// to uncompressed frames
	bool SupportsFrameMapping( kFalse );

	// Check if file exists
	if ( !FileExists( FileName ) )
	{
		return kFalse;
	}
	
	m_Scale = 1.0f;
	float32 CompressionBias = 1 / 64.0f;

	File = fopen( FileName.c_str(), "rb" );

	uint32 VersionNumber( 1 );
	std::string Version( "V1.0" );
	Version = ReadString( File );
	if( Version == "V2.0" )
	{
		VersionNumber = 2;
	}

	m_Scale = ReadFloat( File );				// scale of the entire model
	
	int32	Meshes				( ReadInt( File ) );
	int32	TotalVertices		( ReadInt( File ) );

	int32	Tags				( ReadInt( File ) );

	int32	TotalMaterials		( ReadInt( File ) );				// how many individual materials are applied


	int32	Textures			( ReadInt( File ) );				// how many unique textures appear

	m_TotalMaterials = TotalMaterials;

	int32	Materials			( m_TotalMaterials );

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
	for ( int32 j = 0; j < Textures; j++ )
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
	for ( uint32 d = 0; d < m_TotalMaterials; d++ )
	{
		m_UserData[d] = 0;
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
		if( Material->HasCubeMap() && ReflectionMapID != -1)
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
	}

	// if no materials were contained in the file, we make a 'default' material that does nothing
	if ( Materials <= 0 )
	{
		m_MaterialNames[0] = "NONE";
		MaterialType[0] = KMaterialStandard;
		MaterialReferences[0] = NULL;
		MaterialIsDoubleSided[0] = kFalse;
		MaterialHasColorKey[0] = kFalse;
	}

	
	// parse all the tags that this mesh will recognize
	for( int32 i = 0; i < Tags; i++ )
	{
		m_TagNames.push_back( ReadString( File ) );
		

		if( VersionNumber >= 2 )
		{
			uint32 HasUserData = ReadShort( File );

			if( HasUserData > 0 )
			{
				m_TagUserData.push_back( ReadString( File ) );
			}
			else
			{
				m_TagUserData.push_back( "" );
			}
		}
		else
		{
			m_TagUserData.push_back( "" );
		}
	}


	m_pMaterialSegments.resize( KMaterialTypes );

	
	int32 MeshOffset = ftell(File);
	
	bool	BoundsSet = kFalse;
	m_pSubModel					= new CSkinnedSubMesh();

	std::vector <D3DXVECTOR4>	FinalTriangles;
	std::vector <CWeightedVertex*>	FinalVertices ( TotalVertices );
	std::vector <D3DXVECTOR3>	FinalUVs;

	uint32		VertexOffset( 0 );
	uint32		MaterialOffset( 0 );
	uint32		TotalFaces( 0 );
	// for each mesh
	for ( i=0; i < Meshes; i++ )
	{		
		Triangle*				Triangles;
		TexCoord*				TexCoords;
		MeshHeader	SubHeader;
		
		// Load the mesh's header
		fseek(File, MeshOffset, SEEK_SET);
		fread(&SubHeader, sizeof(MeshHeader), 1, File);

		// we name the model after the first submesh we find
		if( i == 0 )
		{ 
			m_pSubModel->SetName( SubHeader.Name );
		}

		// allocate space for and load in the faces
		fseek(File, MeshOffset+SubHeader.TriangleStart, SEEK_SET);
		Triangles = new Triangle[SubHeader.Triangles];
		fread(Triangles, sizeof(Triangle), SubHeader.Triangles, File);

		for( j = 0; j < SubHeader.Triangles; j++ )
		{
			TotalFaces++;

			uint32 MaterialID = Triangles[j].Vertex[3];

			// make sure its valid
			if ( MaterialID >= m_TotalMaterials )
			{
				MaterialID = m_TotalMaterials - 1;
			}

			MaterialReferenced[MaterialID] = kTrue;

			FinalTriangles.push_back( D3DXVECTOR4( (float32)Triangles[j].Vertex[0] + VertexOffset, 
												   (float32)Triangles[j].Vertex[1] + VertexOffset, 
												   (float32)Triangles[j].Vertex[2] + VertexOffset, 
												   (float32)MaterialID ) );
		}

		// make sure to offset our vertex references, because we're combining the submodels into one VB

		// allocate space and load in the texture coordinates
		fseek(File, MeshOffset+SubHeader.UVStart, SEEK_SET);
		TexCoords = new TexCoord[SubHeader.Vertices];
		fread(TexCoords, sizeof(TexCoord), SubHeader.Vertices, File);
		
		for( j = 0; j < SubHeader.Vertices; j++ )
		{
			FinalUVs.push_back( D3DXVECTOR3( TexCoords[j].UV[0],
								TexCoords[j].UV[1],
								0 ) );
		}
		
		// allocate space and load in the Vertices
		fseek(File, MeshOffset+SubHeader.Verticestart, SEEK_SET);
		
		D3DXVECTOR3	MinBounds		( -1, -1, -1 );
		D3DXVECTOR3	MaxBounds		( -1, -1, -1 );	

		uint32 CurrentVertex = 0;
		for( int32 k = 0; k < SubHeader.Vertices; k++ )
		{
			CWeightedVertex* pWeightedVertex = new CWeightedVertex();
			FinalVertices[ k + VertexOffset ] = pWeightedVertex;

			int32	Bones				( ReadInt( File ) );
			for( int32 w = 0; w < Bones; w++ )
			{
				int32 Index( ReadInt( File ) );

				float32 VX( ReadFloat( File ) * m_Scale );
				float32 VY( ReadFloat( File ) * m_Scale );
				float32 VZ( ReadFloat( File ) * m_Scale );

				int32 AU = ReadByte( File );
				int32 AV = ReadByte( File );
	
				D3DXVECTOR3 Normal( DecompressNormal( AU, AV, 0 ),
									DecompressNormal( AU, AV, 2 ),
									DecompressNormal( AU, AV, 1 ) );
				float32 Weight( ReadFloat( File ) );

				pWeightedVertex->AddInfluence( Index,
											   Weight,
											   D3DXVECTOR3( VX, VY, VZ ),
											   Normal );
			}


			CurrentVertex++;
		}

		VertexOffset += SubHeader.Vertices;


		DELETE_SAFELY( Triangles );
		DELETE_SAFELY( TexCoords );
		
		MeshOffset += SubHeader.MeshSize;	
		


	}

	// parse the skeleton information
	int32 Bones( ReadInt( File ) );
	m_pSkeleton = new CSkeleton();

	std::vector< std::string >	BoneNames( Bones );
	std::vector< int32 >		BoneParents( Bones );
	std::vector< D3DXMATRIX >	BoneXForm( Bones );
	std::vector< bool >			BoneParsed( Bones );
	for( int32 k = 0; k < Bones; k++ )
	{
		BoneParsed[k] = kFalse;
		BoneNames[k] = ReadString( File );

		BoneParents[k] = ReadInt( File );

		D3DXMatrixIdentity( &BoneXForm[k] );
		BoneXForm[k]._41 = ReadFloat( File ) * m_Scale;
		BoneXForm[k]._42 = ReadFloat( File ) * m_Scale;
		BoneXForm[k]._43 = ReadFloat( File ) * m_Scale;

		BoneXForm[k]._11 = ReadShort( File ) / 32767.0f;
		BoneXForm[k]._12 = ReadShort( File ) / 32767.0f;
		BoneXForm[k]._13 = ReadShort( File ) / 32767.0f;

		BoneXForm[k]._21 = ReadShort( File ) / 32767.0f;
		BoneXForm[k]._22 = ReadShort( File ) / 32767.0f;
		BoneXForm[k]._23 = ReadShort( File ) / 32767.0f;

		BoneXForm[k]._31 = ReadShort( File ) / 32767.0f;
		BoneXForm[k]._32 = ReadShort( File ) / 32767.0f;
		BoneXForm[k]._33 = ReadShort( File ) / 32767.0f;
	}

	bool Done( kFalse );

	while( !Done )
	{
		Done = kTrue;
		for( k = 0; k < Bones; k++ )
		{
			if( !BoneParsed[k] )
			{
				Done = kFalse;
			}
			if( !BoneParsed[k] &&
				( BoneParents[k] == -1 ||
				  m_pSkeleton->BoneExists( BoneNames[BoneParents[k]] ) ) )
			{
				BoneParsed[k] = kTrue;
				CBoneNode* pBone = new CBoneNode( BoneNames[k],
												  BoneXForm[k] );
				if( BoneParents[k] == -1 )
				{
					m_pSkeleton->AddBone( pBone, k, "" );
				}
				else
				{
					m_pSkeleton->AddBone( pBone, k, BoneNames[BoneParents[k]] );
				}
			}
		}
	}


	m_pSkeleton->UpdateTransformation();


	// we're done, close the file
	fclose(File);

	// find any unassigned verts, and assign them to the root bone;
	int32 RootBoneIndex = m_pSkeleton->RootBoneIndex();
	for( uint32 p = 0; p < FinalVertices.size(); p++ )
	{
		if( FinalVertices[p]->Influences() == 0 )
		{
			FinalVertices[p]->AddInfluence( RootBoneIndex,
										    0,
										    D3DXVECTOR3( 0, 0, 0 ),
										    D3DXVECTOR3( 0, 0, 0 ) );
		}
	}
	
	m_pSubModel->SetSkeleton( m_pSkeleton );

	m_pSubModel->CreateVB( pD3DDevice,
						   TotalFaces,
						   FinalVertices,
						   FinalUVs );

	m_LocalMinBounds = m_pSubModel->MinBounds();
	m_LocalMaxBounds = m_pSubModel->MaxBounds();

	for( int32 s = 0; s < Materials; s ++ )
	{
		if( MaterialReferenced[s] )
		{
			uint32 SegmentIndex = m_pSubModel->AddAnimatedMeshSegment( m_pRefManager,
																	   m_pSettings,	
																	   pD3DDevice,
																	   FinalTriangles,
																	   s,
																	   MaterialType[s],
																	   MaterialIsDoubleSided[s] );
					
			// if we've actually got a material, apply it to this submodelsegment
			if ( m_TotalMaterials > 0 )
			{
				m_pSubModel->SetMaterial( MaterialReferences[s],
										  SegmentIndex );
			}

			if( !MaterialVisible[s] )
			{
				m_pSubModel->SetVisible( SegmentIndex, kFalse );
			}
		}
	}

	// clean up extra indices in the Index Buffer
	m_pSubModel->CleanIB( pD3DDevice );
	
	// sort references to each subsegment into a bucket
	// of the appropriate material type, for optimized rendering later
	for ( uint32 m = 0; m < m_pSubModel->SkinnedMeshSegments() ; m++ )
	{
		CSkinnedMeshSegment&	Segment			= m_pSubModel->GetSkinnedMeshSegment( m );			

		m_pMaterialSegments[Segment.MaterialType()].push_back( &Segment );
	}

	m_UserData.resize( m_TotalMaterials );

	for( uint32 l = 0; l < MaterialReferences.size(); l++ )
	{
		m_pRefManager.RemoveMaterial( MaterialReferences[l] );
		MaterialReferences[l] = NULL;
	}

	return kTrue;
		
} // CSkinnedGeometry::Load()


// render a vector of segments with a specific userdata value
void CSkinnedGeometry::RenderSegmentSetUserData( LPDIRECT3DDEVICE8 pD3DDevice,										// direct3d device
												  std::vector <CSkinnedMeshSegment*> pMaterialSegments,	// vector of segments to render
												  int32 UserDataValue )										// userdata id to render
{
	CSkinnedMeshSegment* pCurrentSegment = NULL;
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

} // CSkinnedGeometry::RenderSegmentSetUserData()

void CSkinnedGeometry::RenderSegmentSetIllumination( LPDIRECT3DDEVICE8 pD3DDevice,							// direct3d device
													  std::vector <CSkinnedMeshSegment*> pMaterialSegments,// vector of segments to render
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
				if( pMaterialSegments[i]->GetMaterial()->HasBaseTexture() &&
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
} // CSkinnedGeometry::RenderSegmentSetIllumination()

// render all submodels
void CSkinnedGeometry::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
							    const D3DXMATRIX& CameraSpaceMatrix, 	// camera space matrix 
								const std::deque < CActiveSkinnedAnimation* >& ActiveAnimations, // animations to blend
								CPose* pBlendedPose,					// final blended pose
								const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
								bool UpdateVB )							// update the vb?
{
	RenderUserData( pD3DDevice, CameraSpaceMatrix, ActiveAnimations, pBlendedPose, NULL, KAllUserData, Light, UpdateVB );

} // CSkinnedGeometry::Render()

// render all m_pSubModeles
void CSkinnedGeometry::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
							    const D3DXMATRIX& CameraSpaceMatrix, 	// camera space matrix 
								const std::deque < CActiveSkinnedAnimation* >& ActiveAnimations, // animations to blend
								CPose* pBlendedPose,					// final blended pose
								LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use - NULL if none
								const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
								bool UpdateVB )							// update the vb?
{
	RenderUserData( pD3DDevice, CameraSpaceMatrix, ActiveAnimations, pBlendedPose, pCubeMap, KAllUserData, Light, UpdateVB );

} // CSkinnedGeometry::Render()

// render only subsegments with materials whose userdata has been set to the requested
// value
void CSkinnedGeometry::RenderUserData( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
										const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
										const std::deque < CActiveSkinnedAnimation* >& ActiveAnimations, // animations to blend
										CPose* pBlendedPose,					// final blended pose
										LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use for reflection maps - NULL if none
										int32 UserDataValue,					// userdata value that each segment must have set to be rendered
										const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
										bool UpdateVB )							// update the vertex buffer?
{
	if( UpdateVB )
	{
		m_pSubModel->UpdateVB( pBlendedPose );
	}

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

	// reflection mapped segments
	if ( m_pMaterialSegments[KMaterialStaticCubeMap].size() > 0 )
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

	if ( m_pMaterialSegments[KMaterialSphereMap].size() > 0  )
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
		pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

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

	// reflection mapped segments
	if ( m_pMaterialSegments[KMaterialStaticCubeMapPost].size() > 0 )
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
		RenderSegmentSetIllumination( pD3DDevice, m_pMaterialSegments[j], UserDataValue, KMaterialIllumination );
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

} //  CSkinnedGeometry::RenderUserData()

// renders the model with all non-glowing objects as invisible, but written to the zbuffer.
// all glowing objects are visible, but do NOT write to the zbuffer.
// this is used for post effects like light-streaking.
void  CSkinnedGeometry::RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,									// direct3d device
										const D3DXMATRIX& ViewportMatrix,								// viewport matrix
										const std::deque < CActiveSkinnedAnimation* >& ActiveAnimations,	// animations to blend
										CPose* pBlendedPose,					// final blended pose
										bool UpdateVB )							// update the vertex buffer?
							 
{
	if( UpdateVB )
	{
		m_pSubModel->UpdateVB( pBlendedPose );
	}

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
				if( m_pMaterialSegments[i][k]->GetMaterial() != NULL &&
					!m_pMaterialSegments[i][k]->GetMaterial()->AllowZWrite() )
				{
					break;
				}
				// and then render the submodelsegments for each material set
				if( m_pMaterialSegments[i][k]->GetMaterial() != NULL &&
					m_pMaterialSegments[i][k]->GetMaterial()->GetDataID() != KGlowDataID )
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
} //  CSkinnedGeometry::RenderGlowPass()


// renders the model with all non-glowing objects as invisible, but written to the zbuffer.
// all glowing objects are visible, but do NOT write to the zbuffer.
// this is used for post effects like light-streaking.
void  CSkinnedGeometry::RenderIlluminated( LPDIRECT3DDEVICE8 pD3DDevice,									// direct3d device
										   const D3DXMATRIX& ViewportMatrix,								// viewport matrix
										   const std::deque < CActiveSkinnedAnimation* >& ActiveAnimations,	// animations to blend
										   CPose* pBlendedPose,					// final blended pose
										   const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
										   bool UpdateVB )							// update the vertex buffer?
							 
{
	if( UpdateVB )
	{
		m_pSubModel->UpdateVB( pBlendedPose );
	}

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

} //  CSkinnedGeometry::RenderIlluminated()

// render all m_pSubModeles with no texture
void CSkinnedGeometry::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
										const D3DXMATRIX& CameraSpaceMatrix, 	// camera space matrix 
										const std::deque < CActiveSkinnedAnimation* >& ActiveAnimations, // animations to blend
										CPose* pBlendedPose,					// final blended pose
										bool UpdateVB )							// update the vertex buffer?

{
	if( UpdateVB )
	{
		m_pSubModel->UpdateVB( pBlendedPose );
	}

	pD3DDevice->SetTransform( D3DTS_VIEW, &CameraSpaceMatrix);
	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );


	pD3DDevice->SetStreamSource( 0,
								 m_pSubModel->GetVB(),
								 sizeof( PLAINVERTEXMODEL ) );

	pD3DDevice->SetIndices( m_pSubModel->GetIB(), 0L );

	m_pSubModel->RenderNoTexture( pD3DDevice );

} // CSkinnedGeometry::RenderNoTexture()

// returns a pointer to the requested material by index
CMaterial*  CSkinnedGeometry::GetMaterial( uint32 MaterialID )	// index of material to retrieve
{
	for ( uint32 j = 0 ; j < m_pSubModel->SkinnedMeshSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we 
		// return a pointer to it
		if( m_pSubModel->GetSkinnedMeshSegment( j ).MaterialID() == MaterialID )
		{
			return m_pSubModel->GetSkinnedMeshSegment( j ).GetMaterial();
		}
	}
	return NULL;
} //  CSkinnedGeometry::GetMaterial()

// returns a pointer to the requested material by index
CMaterial*  CSkinnedGeometry::GetMaterialByIndex( uint32 Index )	// index of material to retrieve
{
	for ( uint32 j = 0 ; j < m_pSubModel->SkinnedMeshSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we 
		// return a pointer to it
		if( j == Index )
		{
			return m_pSubModel->GetSkinnedMeshSegment( j ).GetMaterial();
		}
	}
	return NULL;
} //  CSkinnedGeometry::GetMaterialByIndex()

// returns the material id of the indexed material
int32  CSkinnedGeometry::GetMaterialIDByIndex( uint32 Index )	// index of material to retrieve
{
	for ( uint32 j = 0 ; j < m_pSubModel->SkinnedMeshSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we 
		// return a pointer to it
		if( j == Index )
		{
			return m_pSubModel->GetSkinnedMeshSegment( j ).MaterialID();
		}
	}
	return 0;
} //  CSkinnedGeometry::GetMaterialIDByIndex()


// replaces a material by index with a pre-existing material
void  CSkinnedGeometry::SetMaterial( uint32 MaterialID,			// materialID of material to overwrite
									  CMaterial* pMaterial )	// material to replace it with
{
	for ( uint32 j = 0 ; j < m_pSubModel->SkinnedMeshSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we simply fill
		// in its local bounds, and return true
		if( m_pSubModel->GetSkinnedMeshSegment( j ).MaterialID() == MaterialID )
		{
			// only swap materials if this is actually a different material
			if( m_pSubModel->GetSkinnedMeshSegment( j ).GetMaterial() != pMaterial )
			{
				// we first need to remove the reference to the previously existing material,
				// so that it will be cleaned up properly
				m_pRefManager.RemoveMaterial( m_pSubModel->GetSkinnedMeshSegment( j ).GetMaterial() );
				// we then need to increase the references to the material we're applying 
				m_pSubModel->GetSkinnedMeshSegment( j ).SetMaterial( pMaterial );
			}
		}
	}
} //  CSkinnedGeometry::SetMaterial()

// replaces a material by index with a pre-existing material
void  CSkinnedGeometry::SetMaterialByIndex( uint32 Index,			// index of material to overwrite
											CMaterial* pMaterial )	// material to replace it with
{
	for ( uint32 j = 0 ; j < m_pSubModel->SkinnedMeshSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we simply fill
		// in its local bounds, and return true
		if( j == Index )
		{
			// only swap materials if this is actually a different material
			if( m_pSubModel->GetSkinnedMeshSegment( j ).GetMaterial() != pMaterial )
			{
				// we first need to remove the reference to the previously existing material,
				// so that it will be cleaned up properly
				m_pRefManager.RemoveMaterial( m_pSubModel->GetSkinnedMeshSegment( j ).GetMaterial() );
				// we then need to increase the references to the material we're applying 
				m_pSubModel->GetSkinnedMeshSegment( j ).SetMaterial( pMaterial );
			}
		}
	}
} //  CSkinnedGeometry::SetMaterialByIndex()

const std::string& CSkinnedGeometry::GetTagName( uint32 Index )
{
	assert( Index < m_TagNames.size() );

	return m_TagNames[Index];
} // CSkinnedGeometry::GetTagName()

const std::string& CSkinnedGeometry::GetTagUserData( uint32 Index )
{
	assert( Index < m_TagUserData.size() );

	return m_TagUserData[Index];
} // CSkinnedGeometry::GetTagUserData()

int32 CSkinnedGeometry::TagIndex( const std::string& Name )	// tag name to find index for
{
	for( uint32 i = 0; i < m_TagNames.size(); i++ )
	{
		if( m_TagNames[i] == Name )
		{
			return i;
		}
	}

	return -1;
} // CSkinnedGeometry::GetTagIndex()


void CSkinnedGeometry::EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{
	m_CastsShadows = kTrue;

	m_pSubModel->EnableShadows( pD3DDevice );
} // CSkinnedGeometry::EnableShadows()


// render shadows
void CSkinnedGeometry::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									   const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
									   const D3DXVECTOR3& Light )				// direction of light shadow is being cast from
{

	m_pSubModel->RenderShadows( pD3DDevice,
								CameraSpaceMatrix,
								Light );
} // CSkinnedGeometry::RenderShadows()
