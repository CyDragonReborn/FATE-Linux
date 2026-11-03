/*
	materialconstants.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _MATERIALCONSTANTS_H
#define _MATERIALCONSTANTS_H

const int8	KAllUserData( -1 );

enum EMaterialType
{
	KMaterialStandard,
	KMaterialPreAlpha,
	KMaterialAlpha,
	KMaterialShadow,
	KMaterialReflect,
	KMaterialStaticCubeMap,
	KMaterialIllumination,
	KMaterialMultiply,
	KMaterialNormalMap,
	KMaterialSphereMap,
	KMaterialReflectPost,
	KMaterialStaticCubeMapPost,
	KMaterialSphereMapPost,
	KMaterialTypes
};

const uint32 KGlowDataID( 100 );

const uint8	KBehaviorNone( 0 );
const uint8	KBehaviorShifting( 2 );


#endif