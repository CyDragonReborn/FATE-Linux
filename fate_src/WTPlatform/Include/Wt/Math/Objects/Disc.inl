//
//	CDisc.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

__forceinline CDisc::CDisc ( const CVector3f& ACenter , const CVector3f& ANormal , float32 ARadius , float32 ARadius2 ) :
	Center ( ACenter ) , Normal ( ANormal ) , Radius ( ARadius ) , Radius2 ( ARadius2 )
{
	Update();
}

__forceinline void __fastcall CDisc::Set ( const CVector3f& ACenter , const CVector3f& ANormal , float32 ARadius , float32 ARadius2 )
{
	Center = ACenter;
	Normal = ANormal;
	Radius = ARadius;
	Radius2 = ARadius2;
	Update();
}

__forceinline void __fastcall CDisc::Update()
{
	Normal.Normalize();

	// Find a vector orthogonal to n.

	CVector3f Basis ( 1.0f , 0.0f , 0.0f );
	
	if ( fabs ( Basis.DotProduct( Normal) ) > 0.999f )
		Basis.Set ( 0.0f , 1.0f , 0.0f );
	
	// Project away N component, normalize and cross to get
	// second orthonormal vector.

	U = Basis - Normal * ( Basis.DotProduct( Normal ));
	U.Normalize();
	V = Normal.CrossProduct( U );
	RealRadiusSqr = -( Center.DotProduct( Normal ) );
	RadiusSqr = Radius * Radius;
	Radius2Sqr = Radius2 * Radius2;
}

__forceinline bool __fastcall CDisc::IsInside ( const CVector3f& /*Point*/ ) const
{
	return false;
}

__forceinline CVector3f __fastcall CDisc::GenerateRandom ()
{
	float32 Alpha = Math_Randf ( 0 , KTwoPi );
	float32 r = Math_Randf ( Radius2 , Radius );
	float32 x = r * cosf ( Alpha );
	float32 y = r * sinf ( Alpha );
	return Center + U * x + V * y;
}
