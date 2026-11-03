//
//	Rect.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

__forceinline CRect::CRect ( const CVector3f& AOrigin , const CVector3f& AWidth , const CVector3f& AHeight ) :
	Origin ( AOrigin) , Width ( AWidth ) , Height ( AHeight )
{
	Update();
}

__forceinline void __fastcall CRect::Set ( const CVector3f& AOrigin , const CVector3f& AWidth , const CVector3f& AHeight )
{
	Origin = AOrigin;
	Width = AWidth;
	Height = AHeight;
	Update();
}

__forceinline void __fastcall CRect::Update()
{
	WidthSqr = Width.Length();
	CVector3f tu = Width / WidthSqr;

	HeightSqr = Height.Length();
	CVector3f tv = Height / HeightSqr;

	Normal = tu.CrossProduct ( tv );
	Normal.Normalize();
	Distance = - ( Origin.DotProduct ( Normal ));
}

__forceinline CVector3f CRect::GetPointOn ( float32 U , float32 V ) const
{
	return Origin + Width * U + Height * V;
}

__forceinline bool __fastcall CRect::IsInside ( const CVector3f& /*Point*/ ) const
{
	return false;
}

__forceinline CVector3f __fastcall CRect::GenerateRandom ()
{
	return Origin + Width * Math_Randf() + Height * Math_Randf();
}
