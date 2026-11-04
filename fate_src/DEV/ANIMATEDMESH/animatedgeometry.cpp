/*
	animatedgeometry.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	Change History

	Travis Baldree
	4/20/2004		- now takes advantage of OptimizeInPlace to generate a mesh
					  that has been compacted, takes advantage of vertex cacheing,
					  and is grouped by material in the IB

	Travis Baldree
	5/3/2004	-	Can now do a multiply layer. 
					Shifters now work on the second layer too.
					Materials can now be marked as visible

	Travis Baldree
	5/17/2004	-	textures with embedded alpha are now recognized and loaded properly
					into their appropriate material bank

	Travis Baldree
	6/29/2004	-	added many new methods for setting/getting/changing materials on an indexed
					and named basis, for much greater flexibility in modifying pre-existing materials.
					A change had to be made to the refmanager to accomodate this, so make
					sure that it is updated as well

	7/30/2004	-	upped to version 2.0, with support for a variable compression bias and post-scaling.
					support for 1.0 AMS files remains.

	8/24/2004	-	added support for builtin static cubemaps

	8/24/2004	-	Added support for post-rendered reflections (fully additive reflections)

	Travis Baldree
	10/08/2004	- ams files did not properly load and render meshes with no mesh data (only tags)
				  Fixed

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
#include "AnimatedGeometry.h"
#include "animatedsubmesh.h"
#include "animatedmeshsegment.h"
#include "activeanimation.h"
#include "tag.h"


// the following structures are used at model-load time only.
// they are not used for persistent data

typedef struct
{
	int8   Name[68];
	int32  MeshFrames;
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
	int32  Frames;
	int32  AnimationCompression;
	int32  Tags;
	int32  Meshes;
	int32  Vertices;
	int32  Textures;

} ModelHeader;

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

typedef struct
{

	int16	Vec[3];
	uint8	Normal[2];

} Vertex;

// end loading structures


CAnimatedGeometry::CAnimatedGeometry( CRefManager& pRefManager,	// pointer to the reference manager, for tracking submaterials
									  CSettings& pSettings ) :		// pointer to the settings manager, for keeping track of card/machine capabilities
																m_pRefManager( pRefManager ),
																m_pSettings( pSettings ),
																m_CompressionFactor( 1 ),
																m_CastsShadows( kFalse ),
																m_TotalMaterials( 0 ),
																m_AnimationFrames( 0 ),
																m_LocalMinBounds( 0, 0, 0 ),
																m_LocalMaxBounds( 0, 0, 0 ),
																m_pSubModel( NULL )
													
{

} // CAnimatedGeometry::CAnimatedGeometry()


CAnimatedGeometry::~CAnimatedGeometry()
{
	InvalidateDeviceObjects();

	DELETE_SAFELY( m_pSubModel );

	for( uint32 i = 0; i < m_TagList.size(); i++ )
	{
		DELETE_SAFELY( m_TagList[i] );
	}
	m_TagList.clear();

} // CAnimatedGeometry::~CAnimatedGeometry()

void CAnimatedGeometry::InvalidateDeviceObjects( void )
{

	m_pSubModel->InvalidateDeviceObjects();
} // CAnimatedGeometry::InvalidateDeviceObjects()

void CAnimatedGeometry::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	m_pSubModel->RestoreDeviceObjects( pD3DDevice );
} // CAnimatedGeometry::RestoreDeviceObjects()

// see if any reflective materials exist in this model
bool CAnimatedGeometry::HasReflection( void )
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
} // CAnimatedGeometry::HasReflection()


void CAnimatedGeometry::SetUserData( uint32 Id,			// id of material to set user data on
									 int32 Value )		// value to set for user data
{
	if ( Id < m_TotalMaterials )
	{
		m_UserData[Id] = Value;
	}
} // CMAnimatedGeometry::SetUserData()

void CAnimatedGeometry::SetUserData( std::string Name,	// name of material to set user data on
									 int32 Value )		// value to set for user data
{
	for ( uint32 i = 0; i < m_TotalMaterials; i++ )
	{
		if ( m_MaterialNames[i] == Name )
		{
			m_UserData[i] = Value;
		}
	}
} // CAnimatedGeometry::SetUserData()

bool CAnimatedGeometry::HasMaterial( const std::string& Name )	// name of material to check existence of
{
	for ( uint32 i = 0; i < m_TotalMaterials; i++ )
	{
		if ( m_MaterialNames[i] == Name )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CAnimatedGeometry::HasMaterial()

int32 CAnimatedGeometry::NearestMappedFrame( int32 UnmappedIndex )	// unmapped frame to find frame mapping for
{
	int32 NearestFrame( 0 );
	for( uint32 i = 0; i < m_FrameMap.size(); i++ )
	{
		if( m_FrameMap[i] <= UnmappedIndex )
		{
			NearestFrame = i;
		}
	}
	return NearestFrame;
} // CAnimatedGeometry::NearestMappedFrame()

// load geometry, animation data, and materials
bool CAnimatedGeometry::Load( LPDIRECT3DDEVICE8 pD3DDevice,	// the direct3d device - needed to create vert and index buffers
							  std::string FileName,		// path to the mesh file
				   			  std::string TexturePath )		// path to the textures (if "" is supplied, will use the mesh path)

{
	FILE *File = NULL;
	
	// old versions do not support mapping of compressed frames
	// to uncompressed frames
	bool SupportsFrameMapping( kFalse );
	bool SupportsUserData( kFalse );

	// Check if file exists
	if ( !FileExists( FileName ) )
	{
		return kFalse;
	}
	
	float32 ModelScale = 1.0f;
	float32 CompressionBias = 1 / 64.0f;

	File = fopen( FileName.c_str(), "rb" );

	std::string Version( "V1.0" );
	// try to get a version - might fail for old versions
	try
	{
		Version = ReadString( File );
	} 
	catch( ... )
	{
		Version = "V1.0";
	}

	if( Version == "V3.0" )
	{
		SupportsFrameMapping = kTrue;
	}
	if( Version == "V4.0" )
	{
		SupportsFrameMapping = kTrue;
		SupportsUserData = kTrue;
	}

	// if version 2, scale is done here
	if( Version == "V2.0" ||
		Version == "V3.0" ||
		Version == "V4.0" )
	{
		ModelScale = ReadFloat( File );				// scale of the entire model
		CompressionBias = 1.0f / (float32)ReadInt( File );	// compression bias lets you weight the detail of a model to small or large unit size
	}
	else	// old version one has scale down ahead of time. Yuck!
	{
		fseek( File, 0, SEEK_SET );
	}
		
	ModelHeader Header;
	
	// Read in the header
	fread(&Header, 1, sizeof(ModelHeader), File);

	m_AnimationFrames = Header.Frames;

	// factor at which the animation was compressed
	// we need this to re-map frames to their actual index at
	// animation time
	m_CompressionFactor = (float32)Header.AnimationCompression;
	m_CompressionFactor = 100 - m_CompressionFactor;
	m_CompressionFactor /= 100.0f;
	if( m_CompressionFactor < .1f )
	{
		m_CompressionFactor = .1f;
	}
	if( m_CompressionFactor > 1.0f )
	{
		m_CompressionFactor = 1.0f;
	}
	
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

	

	if( SupportsUserData )
	{
		// parse all the tags that this mesh will recognize
		for( int32 k = 0; k < Header.Tags; k++ )
		{
			CTag* pTag = new CTag( ReadString( File ) );
			m_TagList.push_back( pTag );
			
			uint32 HasUserData = ReadShort( File );

			if( HasUserData > 0 )
			{
				pTag->SetUserData( ReadString( File ) );
			}
			else
			{
				pTag->SetUserData( "" );
			}
		}

	}
	else
	{
		// Read in the tags
		TagName* TagNames = new TagName[Header.Tags];
		fread(TagNames, sizeof(TagName), Header.Tags, File);
		// convert the tag Position/orientation data to usable vectors and quaternions,
		// for quick calculation later
		for( int32 k = 0; k < Header.Tags; k++ )
		{
			m_TagList.push_back( new CTag( TagNames[k].Name ) );
		}
		DELETE_ARRAY_SAFELY(TagNames);
	}


	Tag* Tags = new Tag[Header.Frames * Header.Tags];
	fread(Tags, sizeof(Tag), Header.Frames*Header.Tags, File);

	for( uint32 l = 0; l < m_AnimationFrames; l++ )
	{
		for( int32 m = 0; m < Header.Tags; m++ )
		{
			uint32 index = l * Header.Tags + m;
			m_TagList[m]->AddPosition( D3DXVECTOR3( Tags[index].Position[0] * ModelScale,
													Tags[index].Position[1] * ModelScale,
													Tags[index].Position[2] * ModelScale ) );

			D3DXMATRIX Orientation;
			D3DXMatrixIdentity( &Orientation );
			D3DXQUATERNION Quaternion;
			for( uint32 row=0; row < 3; row++ )
			{
				for(uint32 col=0; col < 3; col++ )
				{
					Orientation.m[row][col] = Tags[index].Rotation[row][col];
				}
			}

			D3DXQuaternionRotationMatrix(&Quaternion,&Orientation);

			m_TagList[m]->AddOrientation( Quaternion );
		}
	}

	// in version three, we have an animation map that
	// maps corresponding compressed frames to their
	// uncompressed counterparts.
	if( SupportsFrameMapping )
	{
		for( l = 0; l < m_AnimationFrames; l++ )
		{
			m_FrameMap.push_back( ReadInt( File ) );
		}
	}
	
	DELETE_ARRAY_SAFELY(Tags);

	m_pMaterialSegments.resize( KMaterialTypes );

	
	int32 MeshOffset = ftell(File);
	
	bool	BoundsSet = kFalse;
	m_pSubModel					= new CAnimatedSubMesh();

	std::vector <D3DXVECTOR4>	FinalTriangles;
	std::vector <D3DXVECTOR3>	FinalVertices ( Header.Vertices * Header.Frames );
	std::vector <D3DXVECTOR3>	FinalUVs;
	std::vector <D3DXVECTOR3>	FinalNormals ( Header.Vertices * Header.Frames );

	uint32		VertexOffset( 0 );
	uint32		MaterialOffset( 0 );
	uint32		TotalFaces( 0 );
	// for each mesh
	for ( int32 i=0; i < Header.Meshes; i++ )
	{
		
		
		Triangle*				Triangles;
		TexCoord*				TexCoords;
		Vertex*					Vertices;
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
		Vertices =  new Vertex[SubHeader.Vertices * SubHeader.MeshFrames];
		fread(Vertices, sizeof(Vertex), SubHeader.MeshFrames*SubHeader.Vertices, File);
		
		D3DXVECTOR3	MinBounds		( -1, -1, -1 );
		D3DXVECTOR3	MaxBounds		( -1, -1, -1 );	

		uint32 CurrentVertex = 0;
		for( j = 0; j < SubHeader.MeshFrames; j++ )
		{
			for( int32 k = 0; k < SubHeader.Vertices; k++ )
			{
				FinalVertices[j*Header.Vertices + k + VertexOffset] = D3DXVECTOR3( Vertices[CurrentVertex].Vec[0] * CompressionBias * ModelScale,
																				   Vertices[CurrentVertex].Vec[1] * CompressionBias * ModelScale,
																				   Vertices[CurrentVertex].Vec[2] * CompressionBias * ModelScale );
				if( CurrentVertex == 0 )
				{
					MinBounds = FinalVertices[VertexOffset];
					MaxBounds = FinalVertices[VertexOffset];
				}
				else
				{
					ExpandBounds( MinBounds, MaxBounds, FinalVertices[j*Header.Vertices + k + VertexOffset] );
				}

				int32 AU = Vertices[CurrentVertex].Normal[0];
				int32 AV = Vertices[CurrentVertex].Normal[1];
				
				FinalNormals[j*Header.Vertices + k + VertexOffset] = D3DXVECTOR3( DecompressNormal( AU, AV, 0 ),
																				  DecompressNormal( AU, AV, 2 ),
																				  DecompressNormal( AU, AV, 1 ) );

				CurrentVertex++;
			}
		}

		VertexOffset += SubHeader.Vertices;


		if ( !BoundsSet )
		{
			BoundsSet = kTrue;
			m_LocalMinBounds = MinBounds;
			m_LocalMaxBounds = MaxBounds;
		}

		// constantly expand the bounding box to encompass these new subobjects
		ExpandBounds( m_LocalMinBounds, m_LocalMaxBounds, MinBounds );
		ExpandBounds( m_LocalMinBounds, m_LocalMaxBounds, MaxBounds );

				
		DELETE_SAFELY( Triangles );
		DELETE_SAFELY( TexCoords );
		DELETE_SAFELY( Vertices );
		
		MeshOffset += SubHeader.MeshSize;	
		


	}
		
	// we're done, close the file
	fclose(File);

	m_pSubModel->SetMinBounds( m_LocalMinBounds );
	m_pSubModel->SetMaxBounds( m_LocalMaxBounds );
		
	m_pSubModel->CreateVB( pD3DDevice,
						   Header.Vertices,
						   TotalFaces,
						   FinalVertices,
						   FinalUVs,
						   FinalNormals );

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
	for ( uint32 m = 0; m < m_pSubModel->AnimatedMeshSegments() ; m++ )
	{
		CAnimatedMeshSegment&	Segment			= m_pSubModel->GetAnimatedMeshSegment( m );			

		m_pMaterialSegments[Segment.MaterialType()].push_back( &Segment );
	}

	m_UserData.resize( m_TotalMaterials );

	for( l = 0; l < MaterialReferences.size(); l++ )
	{
		m_pRefManager.RemoveMaterial( MaterialReferences[l] );
		MaterialReferences[l] = NULL;
	}


	return kTrue;
		
} // CAnimatedGeometry::Load()


// render a vector of segments with a specific userdata value
void CAnimatedGeometry::RenderSegmentSetUserData( LPDIRECT3DDEVICE8 pD3DDevice,										// direct3d device
												  std::vector <CAnimatedMeshSegment*> pMaterialSegments,	// vector of segments to render
												  int32 UserDataValue )										// userdata id to render
{
	CAnimatedMeshSegment* pCurrentSegment = NULL;
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

} // CAnimatedGeometry::RenderSegmentSetUserData()

void CAnimatedGeometry::RenderSegmentSetIllumination( LPDIRECT3DDEVICE8 pD3DDevice,							// direct3d device
													  std::vector <CAnimatedMeshSegment*> pMaterialSegments,// vector of segments to render
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
} // CAnimatedGeometry::RenderSegmentSetIllumination()

// render all submodels
void CAnimatedGeometry::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
							    const D3DXMATRIX& CameraSpaceMatrix, 	// camera space matrix 
								const std::deque < CActiveAnimation* >& ActiveAnimations, // animations to blend
								const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
							    bool UpdateVB )							// update the vertex buffer?
{
	RenderUserData( pD3DDevice, CameraSpaceMatrix, ActiveAnimations, NULL, KAllUserData, Light, UpdateVB );

} // CAnimatedGeometry::Render()

// render all m_pSubModeles
void CAnimatedGeometry::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
							    const D3DXMATRIX& CameraSpaceMatrix, 	// camera space matrix 
								const std::deque < CActiveAnimation* >& ActiveAnimations, // animations to blend
								LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use - NULL if none
								const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
								bool UpdateVB )												// update the vertex buffer?
{
	RenderUserData( pD3DDevice, CameraSpaceMatrix, ActiveAnimations, pCubeMap, KAllUserData, Light, UpdateVB );

} // CAnimatedGeometry::Render()

// render only subsegments with materials whose userdata has been set to the requested
// value
void CAnimatedGeometry::RenderUserData( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
										const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
										const std::deque < CActiveAnimation* >& ActiveAnimations, // animations to blend
										LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use for reflection maps - NULL if none
										int32 UserDataValue,					// userdata value that each segment must have set to be rendered
										const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
										bool UpdateVB )												// update the vertex buffer?
{
	if( m_pSubModel->GetVB() == NULL )
	{
		return;
	}
	pD3DDevice->SetTransform( D3DTS_VIEW, &CameraSpaceMatrix );
	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );

	if( UpdateVB )
	{
		m_pSubModel->UpdateVB( ActiveAnimations );
	}

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

} //  CAnimatedGeometry::RenderUserData()

// renders the model with all non-glowing objects as invisible, but written to the zbuffer.
// all glowing objects are visible, but do NOT write to the zbuffer.
// this is used for post effects like light-streaking.
void  CAnimatedGeometry::RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,									// direct3d device
										  const D3DXMATRIX& ViewportMatrix,								// viewport matrix
										  const std::deque < CActiveAnimation* >& ActiveAnimations,		// animations to blend
										  bool UpdateVB )												// update the vertex buffer?
							 
{
	if( m_pSubModel->GetVB() == NULL )
	{
		return;
	}

	if( UpdateVB )
	{
		m_pSubModel->UpdateVB( ActiveAnimations );
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
} //  CAnimatedGeometry::RenderGlowPass()

// renders the model highlighted
void  CAnimatedGeometry::RenderIlluminated( LPDIRECT3DDEVICE8 pD3DDevice,									// direct3d device
											 const D3DXMATRIX& ViewportMatrix,								// viewport matrix
											 const std::deque < CActiveAnimation* >& ActiveAnimations,		// animations to blend
											 const D3DXVECTOR3& Light,	// direction of light for use in normal mapping
											 bool UpdateVB )												// update the vertex buffer?
							 
{
	if( m_pSubModel->GetVB() == NULL )
	{
		return;
	}

	if( UpdateVB )
	{
		m_pSubModel->UpdateVB( ActiveAnimations );
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
} //  CAnimatedGeometry::RenderIlluminated()

// render all m_pSubModeles with no texture
void CAnimatedGeometry::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
										 const D3DXMATRIX& CameraSpaceMatrix, 	// camera space matrix 
										 const std::deque < CActiveAnimation* >& ActiveAnimations, // animations to blend
										 bool UpdateVB )						// update the vertex buffer?
{
	if( m_pSubModel->GetVB() == NULL )
	{
		return;
	}
	
	pD3DDevice->SetTransform(D3DTS_VIEW, &CameraSpaceMatrix);
	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );

	if( UpdateVB )
	{
		m_pSubModel->UpdateVB( ActiveAnimations );
		}

	pD3DDevice->SetStreamSource( 0,
								 m_pSubModel->GetVB(),
								 sizeof( PLAINVERTEXMODEL ) );

	pD3DDevice->SetIndices( m_pSubModel->GetIB(), 0L );

	m_pSubModel->RenderNoTexture( pD3DDevice );

} // CAnimatedGeometry::RenderNoTexture()

// returns a pointer to the requested material by index
CMaterial*  CAnimatedGeometry::GetMaterial( uint32 MaterialID )	// id of material to retrieve
{
	for ( uint32 j = 0 ; j < m_pSubModel->AnimatedMeshSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we 
		// return a pointer to it
		if( m_pSubModel->GetAnimatedMeshSegment( j ).MaterialID() == MaterialID )
		{
			return m_pSubModel->GetAnimatedMeshSegment( j ).GetMaterial();
		}
	}
	return NULL;
} //  CAnimatedGeometry::GetMaterial()

// returns a pointer to the requested material by index
CMaterial*  CAnimatedGeometry::GetMaterialByIndex( uint32 Index )	// index of material to retrieve
{
	for ( uint32 j = 0 ; j < m_pSubModel->AnimatedMeshSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we 
		// return a pointer to it
		if( j == Index )
		{
			return m_pSubModel->GetAnimatedMeshSegment( j ).GetMaterial();
		}
	}
	return NULL;
} //  CAnimatedGeometry::GetMaterialByIndex()

// returns the material id of the indexed material
int32  CAnimatedGeometry::GetMaterialIDByIndex( uint32 Index )	// index of material to retrieve
{
	for ( uint32 j = 0 ; j < m_pSubModel->AnimatedMeshSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we 
		// return a pointer to it
		if( j == Index )
		{
			return m_pSubModel->GetAnimatedMeshSegment( j ).MaterialID();
		}
	}
	return 0;
} //  CAnimatedGeometry::GetMaterialIDByIndex()

// replaces a material by index with a pre-existing material
void  CAnimatedGeometry::SetMaterial( uint32 MaterialID,		// id of material to overwrite
									  CMaterial* pMaterial )	// material to replace it with
{
	for ( uint32 j = 0 ; j < m_pSubModel->AnimatedMeshSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we simply fill
		// in its local bounds, and return true
		if( m_pSubModel->GetAnimatedMeshSegment( j ).MaterialID() == MaterialID )
		{
			// only swap materials if this is actually a different material
			if( m_pSubModel->GetAnimatedMeshSegment( j ).GetMaterial() != pMaterial )
			{
				// we first need to remove the reference to the previously existing material,
				// so that it will be cleaned up properly
				m_pRefManager.RemoveMaterial( m_pSubModel->GetAnimatedMeshSegment( j ).GetMaterial() );
				// we then need to increase the references to the material we're applying 
				m_pSubModel->GetAnimatedMeshSegment( j ).SetMaterial( pMaterial );
			}
		}
	}
} //  CAnimatedGeometry::SetMaterial()

// replaces a material by index with a pre-existing material
void  CAnimatedGeometry::SetMaterialByIndex( uint32 Index,				// index of material to overwrite
											 CMaterial* pMaterial )	// material to replace it with
{
	for ( uint32 j = 0 ; j < m_pSubModel->AnimatedMeshSegments() ; j++ )
	{
		// if the submodelsegment has the requested material applied, we simply fill
		// in its local bounds, and return true
		if( j == Index )
		{
			// only swap materials if this is actually a different material
			if( m_pSubModel->GetAnimatedMeshSegment( j ).GetMaterial() != pMaterial )
			{
				// we first need to remove the reference to the previously existing material,
				// so that it will be cleaned up properly
				m_pRefManager.RemoveMaterial( m_pSubModel->GetAnimatedMeshSegment( j ).GetMaterial() );
				// we then need to increase the references to the material we're applying 
				m_pSubModel->GetAnimatedMeshSegment( j ).SetMaterial( pMaterial );
			}
		}
	}
} //  CAnimatedGeometry::SetMaterialByIndex()


D3DXMATRIX CAnimatedGeometry::GetTransformedTag( uint32 Index,				// index of tag to get transformed info for
												 uint32 CurrentFrame,			// current frame
												 uint32 NextFrame,			// next frame
												 float32 Interpolation )		// interpolation between the frames
{
	assert( Index < m_TagList.size() );

	return m_TagList[Index]->GetTransformedTag( CurrentFrame, 
												NextFrame, 
												Interpolation );			
} // CAnimatedGeometry::GetTransformedTag()

D3DXMATRIX CAnimatedGeometry::GetTransformedTag( uint32 Index,				// index of tag to get transformed info for
												 const std::deque < CActiveAnimation* >& ActiveAnimations ) // animations to blend
{
	assert( Index < m_TagList.size() );

	return m_TagList[Index]->GetTransformedTag( ActiveAnimations );			
} // CAnimatedGeometry::GetTransformedTag()

D3DXVECTOR3 CAnimatedGeometry::GetTagPosition( uint32 Index,				// index of tag to get transformed info for
											   const std::deque < CActiveAnimation* >& ActiveAnimations ) // animations to blend
{
	assert( Index < m_TagList.size() );

	return m_TagList[Index]->GetTagPosition( ActiveAnimations );			
} // CAnimatedGeometry::GetTagPosition()

D3DXVECTOR3	CAnimatedGeometry::GetTagPosition( uint32 Index,			// index of tag to get transformed info for
											   uint32 CurrentFrame,		// current frame
											   uint32 NextFrame,		// next frame
											   float32 Interpolation )	// interpolation between the frames
{
	assert( Index < m_TagList.size() );

	return m_TagList[Index]->GetTagPosition( CurrentFrame,
											 NextFrame,
											 Interpolation );
} // CAnimatedGeometry::GetTagPosition()

const std::string& CAnimatedGeometry::GetTagName( uint32 Index )
{
	assert( Index < m_TagList.size() );

	return m_TagList[Index]->Name();
} // CAnimatedGeometry::GetTagName()

const std::string& CAnimatedGeometry::GetTagUserData( uint32 Index )
{
	assert( Index < m_TagList.size() );

	return m_TagList[Index]->UserData();
} // CAnimatedGeometry::GetTagUserData()

void CAnimatedGeometry::EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{
	m_CastsShadows = kTrue;

	m_pSubModel->EnableShadows( pD3DDevice );
} // CAnimatedGeometry::EnableShadows()


// render shadows
void CAnimatedGeometry::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									   const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
									   const D3DXVECTOR3& Light )				// direction of light shadow is being cast from
{
	m_pSubModel->RenderShadows( pD3DDevice,
								CameraSpaceMatrix,
								Light );
} // CAnimatedGeometry::RenderShadows()
