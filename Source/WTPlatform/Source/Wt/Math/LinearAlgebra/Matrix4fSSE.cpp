//
//	Matrix4fSSE.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "Matrix4f.h"

namespace WT
{
namespace Math
{


//
// Matrix4f_SSE Optimizations
//

void __fastcall CMatrix4f::TransformVec3dArraySSE( const CVector3f *Source, CVector3f *Dest, int32 Count ) const
{
	assert( Source != NULL );
	assert( Dest != NULL );
	
	if (Count <= 0)										// Early out if possible
		return;

	static uint32 zooo[4] = {0xffffffff, 0xffffffff, 0xffffffff, 0x0000};
	
	_asm
	{
			mov			ecx, Count						// number of geVec3d's to process
			test		ecx, ecx
			jz			Done
			
			mov			esi, Source						// point32er to Source
			mov			edi, Dest						// point32er to Dest
			mov			eax, this						// point32er to matrix
			
			// preload values
			prefetchnta	[eax]
			prefetchnta	[esi]
			prefetchnta	[edi]

#define MXRow		xmm0
#define MYRow		xmm1
#define MZRow		xmm2
#define MTran		xmm7
#define ZOOO		xmm3
															// |  High	 |        |        |  Low   |
			movups		MXRow,	[eax]						// |  M.RowX
			movups		MYRow,	[eax+16]					// |  M.RowY
			movups		MZRow,	[eax+32]					// |  M.RowZ
			movups		MTran,	[eax+48]					// |  M.RowT
			movups		ZOOO,	zooo
			
TransformLoop:
#define VXRow		xmm4
#define VYRow		xmm5
#define VZRow		xmm6

			movups		VXRow,	[esi]
			andps		VXRow,	ZOOO			// zero out V.W, since we're dealing with 3 element verts
	
			movaps		VZRow,	VXRow
			movaps		VYRow,	VXRow
			shufps		VZRow,	VXRow, 0xEA		//{11,10,10,10}, = {0.0, V.Z, V.Z, V.Z}
			shufps		VYRow,	VXRow, 0xD5		//{11,01,01,01}, = {0.0, V.Y, V.Y, V.Y}	
			shufps		VXRow,	VXRow, 0xC0		//{11,00,00,00}, = {0.0, V.X, V.X, V.X}	

			add			esi,	12				// sizeof(*Source)

			// V#Row = {0, v# * M#.Z, v# * M#.Y, v# * M#.X}
			mulps		VXRow,	MXRow
			mulps		VYRow,	MYRow
			mulps		VZRow,	MZRow
	
			prefetchnta	[esi]					// ping for the next vertex

			// sum the SHIT UP!!!
			addps		VXRow,	MTran			// T + x
			addps		VYRow,	VZRow			// VYRow is trashed now!!!
			addps		VXRow,	VYRow

			movups		[edi],	VXRow

			add			edi,	12				// sizeof(*Dest)

			dec			ecx
			jnz			TransformLoop
Done:
	}

#undef MXRow
#undef MYRow
#undef MZRow
#undef MWRow

#undef VXRow
#undef VYRow
#undef VZRow
#undef VWRow
}


void __fastcall CMatrix4f::TransformVec4dArraySSE( const CVector4f *Source, CVector4f *Dest, int32 Count ) const
{
	assert( Source != NULL );
	assert( Dest != NULL );
	
	if (Count <= 0)										// Early out if possible
		return;
		
	_asm
	{
			mov			ecx, Count						// number of geVec3d's to process
			test		ecx, ecx
			jz			Done
			
			mov			esi, Source						// point32er to Source
			mov			edi, Dest						// point32er to Dest
			mov			eax, this						// point32er to matrix
			
			// preload values
			prefetchnta	[eax]
			prefetchnta	[esi]
			prefetchnta	[edi]
			
#define MXRow		xmm0
#define MYRow		xmm1
#define MZRow		xmm2
#define MWRow		xmm7

			// |  High	 |        |        |  Low   |
			movups		MXRow,	[eax]						// |  M.RowX
			movups		MYRow,	[eax+16]					// |  M.RowY
			movups		MZRow,	[eax+32]					// |  M.RowZ
			movups		MWRow,	[eax+48]					// |  M.RowT
						
TransformLoop:
#define VXRow		xmm4
#define VYRow		xmm5
#define VZRow		xmm6
#define VWRow		xmm3
		
			movups		VXRow,	[esi]
			movaps		VZRow,	VXRow
			movaps		VYRow,	VXRow
			movaps		VWRow,	VWRow

			shufps		VZRow,	VXRow, 0xAA		//{10,10,10,10}, = {V.Z...}
			shufps		VYRow,	VXRow, 0x55		//{01,01,01,01}, = {V.Y...}	
			shufps		VWRow,	VXRow, 0xFF		//{11,11,11,11}, = {V.W...}	
			shufps		VXRow,	VXRow, 0x00		//{00,00,00,00}, = {V.X...}	
			
			add			esi,	16				// sizeof(*Source)
			
			// V#Row = {0, v# * M#.Z, v# * M#.Y, v# * M#.X}
			mulps		VXRow,	MXRow
			mulps		VYRow,	MYRow
			mulps		VZRow,	MZRow
			mulps		VWRow,	MWRow
			
			prefetchnta	[esi]					// ping for the next vertex
			
			// sum the SHIT UP!!!
			addps		VXRow,	VWRow			// w + x
			addps		VYRow,	VZRow			// y + z
			addps		VXRow,	VYRow			// w + x + y + z
			
			movups		[edi],	VXRow
			
			add			edi,	16				// sizeof(*Dest)
			
			dec			ecx
			jnz			TransformLoop
Done:
	}
#undef MXRow
#undef MYRow
#undef MZRow
#undef MWRow
	
#undef VXRow
#undef VYRow
#undef VZRow
#undef VWRow
}

#define	BS_WEIGHTOFFSET		12
#define	BS_INDEXOFFSET		24
#define	BS_NORMALOFFSET		28
#define	BS_SIZE				40
#define BD_SIZE				24

void __fastcall CMatrix4f_TranformVec3dMultiBlendSSE(const CMatrix4f * xforms, const VertexBlendSrc * vert, VertexBlendDst * destvert, int32 stride, int32 count)

{
	float32	one = 1.0f;
	
	if( count <= 0 )
		return;

	int32		stackptr;
	
	int32		extradata = stride - sizeof(VertexBlendSrc);
	assert(0 == (extradata % 4));
		
	_asm
	{
		mov				stackptr,	esp		// i just need one more lousy register!!!
		mov				ebx,	vert
		prefetchnta		[ebx]
		
		mov				esp,	count
		mov				edi,	destvert
		mov				ecx,	extradata;
		

TransformLoop:


		///////////////////////////////////////////////////
		//
		//		Phase One:
		//	Build the blend matrix. Blend matrix =
		//
		//	M[0]*w[0] + M[1]*w[1] + M[2]*w[2] + M[3]*w[3]
		//
		//	Yes this Will work, we don't need to transform
		//	the vert 4 times. And it also means we can
		//	get a full propa' blended normal too.
		//
		///////////////////////////////////////////////////

		movss			xmm7,	one
		shufps			xmm7,	xmm7,	0x0		// xmm7 = [1,1,1,1]
												// xmm7 is used to find out the 3rd beta

		// load matrix zero
		movzx			eax,	byte ptr [ebx+BS_INDEXOFFSET+0]
		mov				esi,	64
		mul				esi				
		add				eax,	xforms	// eax = eax * 64 + xforms
		prefetchnta		[eax]
		mov				esi, eax		// esi = mat 0

		// load matriz one
		movzx			eax,	byte ptr [ebx+BS_INDEXOFFSET+1]
		mov				edx,	64
		mul				edx				
		add				eax,	xforms	// eax = eax * 64 + xforms
		prefetchnta		[eax]
		mov				edx, eax		// esi = mat 1
		
#define MXRow		xmm1
#define MYRow		xmm2
#define MZRow		xmm3
#define MWRow		xmm4

		
		// do zero xform
		movss			xmm0,	[ebx+BS_WEIGHTOFFSET+0]
		shufps			xmm0,	xmm0,	0x0		// xmm0 = [w0, w0, w0, w0]
		
		movups			MXRow,	[esi]			// load xform0 -> MXRow-4
		movups			MYRow,	[esi+16]
		movups			MZRow,	[esi+32]
		movups			MWRow,	[esi+48]

		mulps			MXRow,	xmm0			// scale xform0 by w0
		mulps			MYRow,	xmm0
		mulps			MZRow,	xmm0
		mulps			MWRow,	xmm0

		subps			xmm7,	xmm0

		// do one xform
		movss			xmm0,	[ebx+BS_WEIGHTOFFSET+4]
		shufps			xmm0,	xmm0,	0x0		// xmm0 = [w1, w1, w1, w1]
		
		movups			xmm5,	[edx]			// load partial xform1 -> xmm5-7
		movups			xmm6,	[edx+16]

		mulps			xmm5,	xmm0			// scale xform1 by w1
		mulps			xmm6,	xmm0

		addps			MXRow,	xmm5			// add partial scaled xf1 to xf0
		addps			MYRow,	xmm6

		movups			xmm5,	[edx+32]		// finish xf1 load scale and add
		movups			xmm6,	[edx+48]		// finish xf1 load scale and add
		
		mulps			xmm5,	xmm0			
		mulps			xmm6,	xmm0
		
		addps			MZRow,	xmm5
		addps			MWRow,	xmm6

		subps			xmm7,	xmm0

		// load matriz two
		movzx			eax,	byte ptr [ebx+BS_INDEXOFFSET+2]
		mov				esi,	64
		mul				esi				
		add				eax,	xforms	// eax = eax * 64 + xforms
		prefetchnta		[eax]
		mov				esi, eax		// esi = mat 2

		// load matriz three
		movzx			eax,	byte ptr [ebx+BS_INDEXOFFSET+3]
		mov				edx,	64
		mul				edx				
		add				eax,	xforms	// eax = eax * 64 + xforms
		prefetchnta		[eax]
		mov				edx, eax		// esi = mat 3

		
		// do two xform
		movss			xmm0,	[ebx+BS_WEIGHTOFFSET+8]
		shufps			xmm0,	xmm0,	0x0		// xmm0 = [w2, w2, w2, w2]
		
		movups			xmm5,	[esi]			// load partial xform1 -> xmm5-7
		movups			xmm6,	[esi+16]
		
		mulps			xmm5,	xmm0			// scale xform1 by w1
		mulps			xmm6,	xmm0
		
		addps			MXRow,	xmm5			// add partial scaled xf1 to xf0
		addps			MYRow,	xmm6
		
		movups			xmm5,	[esi+32]		// finish xf1 load scale and add
		movups			xmm6,	[esi+48]		// finish xf1 load scale and add
		
		mulps			xmm5,	xmm0			
		mulps			xmm6,	xmm0
		
		addps			MZRow,	xmm5
		addps			MWRow,	xmm6

		subps			xmm7,	xmm0


		// do three xform
		movups			xmm5,	[edx]			// load partial xform1 -> xmm5-7
		movups			xmm6,	[edx+16]
		
		mulps			xmm5,	xmm7			// scale xform1 by w1
		mulps			xmm6,	xmm7
		
		addps			MXRow,	xmm5			// add partial scaled xf1 to xf0
		addps			MYRow,	xmm6
		
		movups			xmm5,	[edx+32]		// finish xf1 load scale and add
		movups			xmm6,	[edx+48]		// finish xf1 load scale and add
		
		mulps			xmm5,	xmm7			
		mulps			xmm6,	xmm7
		
		addps			MZRow,	xmm5
		addps			MWRow,	xmm6


		//////////////////////////////////////////////////
		//
		//			Phase Two:
		//	we have the final blend xform in xmm1-4
		//	and the vert and normal ptrs in ESI/EDX
		//	transform the vert and ROTATE the normal
		//
		//////////////////////////////////////////////////


#define VXRow		xmm0
#define VYRow		xmm5
#define VZRow		xmm6
#define VWRow		xmm7
		
		// transform the vertex
		movups		VXRow,	[ebx]
		movaps		VZRow,	VXRow
		movaps		VYRow,	VXRow
		movaps		VWRow,	VWRow
		
		shufps		VZRow,	VXRow, 0xAA		//{10,10,10,10}, = {V.Z...}
		shufps		VYRow,	VXRow, 0x55		//{01,01,01,01}, = {V.Y...}	
		shufps		VWRow,	VXRow, 0xFF		//{11,11,11,11}, = {V.W...}	
		shufps		VXRow,	VXRow, 0x00		//{00,00,00,00}, = {V.X...}	
		
		// V#Row = {0, v# * M#.Z, v# * M#.Y, v# * M#.X}
		mulps		VXRow,	MXRow
		mulps		VYRow,	MYRow
		mulps		VZRow,	MZRow
		
		// sum the SHIT UP!!!
		addps		VXRow,	MWRow			// w + x, (assume normal matrices, and that vert.w = 1)
		addps		VYRow,	VZRow			// y + z
		addps		VXRow,	VYRow			// w + x + y + z
		
		movups		[edi],	VXRow

		// load the normal
		movups		VXRow,	[ebx+BS_NORMALOFFSET]
		movaps		VZRow,	VXRow
		movaps		VYRow,	VXRow

		// swizzle normal registers
		shufps		VZRow,	VXRow, 0xAA		//{10,10,10,10}, = {V.Z...}
		shufps		VYRow,	VXRow, 0x55		//{01,01,01,01}, = {V.Y...}	
		shufps		VXRow,	VXRow, 0x00		//{00,00,00,00}, = {V.X...}		
		
		// rotate the normal
		// V#Row = {0, v# * M#.Z, v# * M#.Y, v# * M#.X}
		mulps		VXRow,	MXRow
		mulps		VYRow,	MYRow
		mulps		VZRow,	MZRow
		
		// sum the SHIT UP!!!
		addps		VXRow,	VYRow			// y + x
		addps		VXRow,	VZRow			// y + x + z

		// ping for the next vertex and and prep for extra data copy
		add			ebx,		BS_SIZE			
		movups		[edi+12],	VXRow
		prefetchnta	[ebx]
		add			edi,		BD_SIZE

		// copy extra data
		mov			esi,	ebx
		shr			ecx,	2		// get extradata count in dword's
		rep	movs	dword ptr [edi],	dword ptr [esi]		// copy extra data, this ALSO increments edi/esi to the correct spot!!!
		mov			ecx,	extradata	// reload ecx
		mov			ebx,	esi
		
		dec			esp
		jnz			TransformLoop
//Done:

		mov			esp,	stackptr		// restore the stack ptr
	}
#undef MXRow
#undef MYRow
#undef MZRow
#undef MWRow
	
#undef VXRow
#undef VYRow
#undef VZRow
#undef VWRow
}

void CMatrix4f::MultiplySSE( const CMatrix4f& M1, const CMatrix4f& M2 )
            // fastcall:                    src1 = ecx,   src2= edx,   dst = esp+4
            // stdcall:                     src1 = esp+4, src2= esp+8, dst = esp+0Ch
{
                                        //      +0      +4      +8      +C
                                        //      +10     +14     +18     +1C
                                        //      +20     +24     +28     +2C
                                        //      +30     +34     +38     +3C

	float32 * src1 = (float32*)&M1;
	float32 * src2 = (float32*)&M2;
	float32 * dst = (float32*)this;

    __asm {

        #define SRC1 ecx
        #define SRC2 edx

		//TODO write assembly code that works for __fastcall

        mov SRC1,dword ptr [src1]
        mov SRC2,dword ptr [src2]

		#define DST eax
        mov DST, dword ptr [dst] 

        movss  xmm0, dword ptr [SRC1]           //  xmm0 =  ?           ?           ?           S1.RowX.x
        movups xmm1, xmmword ptr [SRC2]         //  xmm1 =  S2.RowX.x   S2.RowX.y   S2.RowX.z   S2.RowX.w       (S2.RowX[])
        shufps xmm0, xmm0, 0                    //  xmm0 =  S1.RowX.x   S1.RowX.x   S1.RowX.x   S1.RowX.x
        movss  xmm2, dword ptr [SRC1+4]         //  xmm2 =  ?           ?           ?           S1.RowX.y 
        mulps  xmm0, xmm1                       //  xmm0 =  S1.RowX.x * S2.RowX[]
        shufps xmm2, xmm2, 0                    //  xmm2 =  S1.RowX.y   S1.RowX.y   S1.RowX.y   S1.RowX.y
        movups xmm3, xmmword ptr [SRC2+10h]     //  xmm3 =  S2.RowY.x   S2.RowY.y   S2.RowY.z   S2.RowY.w       (S2.RowY[])
        movss  xmm7, dword ptr [SRC1+8]         //  xmm7 =  ?           ?           ?           S1.RowX.z
        mulps  xmm2, xmm3                       //  xmm2 =  S1.RowX.y * S2.RowY[]
        shufps xmm7, xmm7, 0                    //  xmm7 =  S1.RowX.z   S1.RowX.z   S1.RowX.z   S1.RowX.z
        addps  xmm0, xmm2                       //  xmm0 =  (S1.RowX.x * S2.RowX[]) + (S1.RowX.y * S2.RowY[])
        movups xmm4, xmmword ptr [SRC2+20h]     //  xmm4 =  S2.RowZ.x   S2.RowZ.y   S2.RowZ.z   S2.RowZ.w       (S2.RowZ[])
        mulps  xmm7, xmm4                       //  xmm7 =  S1.RowX.z * S2.RowZ[]
        addps  xmm0, xmm7                       //  xmm0 =  (S1.RowX.x * S2.RowX[]) + (S1.RowX.y * S2.RowY[]) + (S1.RowX.z * S2.RowZ[])
        movups xmm5, xmmword ptr [SRC2+30h]     //  xmm5 =  S2.RowT.x   S2.RowT.y   S2.RowT.z   S2.RowT.w       (S2.RowT[])         Only needed once
        movss  xmm6, dword ptr [SRC1+10h]       //  xmm6 =  ?           ?           ?           S1.RowY.x
        movss  xmm7, dword ptr [SRC1+14h]       //  xmm7 =  ?           ?           ?           S1.RowY.y
        shufps xmm6, xmm6, 0                    //  xmm6 =  S1.RowY.x   S1.RowY.x   S1.RowY.x   S1.RowY.x
        shufps xmm7, xmm7, 0                    //  xmm7 =  S1.RowY.y   S1.RowY.y   S1.RowY.y   S1.RowY.y
        movlps qword ptr [DST], xmm0            //    
        movhps qword ptr [DST+8], xmm0          //  completed D1.RowX[] = xmm0  

        mulps  xmm7, xmm3                       //  xmm7 = S1.RowY.y * S2.RowY[]
        movss  xmm0, dword ptr [SRC1+18h]       //  xmm0 = ?            ?           ?           S1.RowY.z
        mulps  xmm6, xmm1                       //  xmm6 = S1.RowY.x * S2.RowX[]
        shufps xmm0, xmm0, 0                    //  xmm0 = S1.RowY.z    S1.RowY.z   S1.RowY.z   S1.RowY.z
        addps  xmm6, xmm7                       //  xmm6 = (S1.RowY.x * S2.RowX[]) + (S1.RowY.y * S2.RowY[])
        mulps  xmm0, xmm4                       //  xmm0 = S1.RowY.z * S2.RowZ[]
        movss  xmm2, dword ptr [SRC1+24h]       //  xmm2 = ?            ?           ?           S1.RowZ.y       
        addps  xmm6, xmm0                       //  xmm6 = (S1.RowY.x * S2.RowX[]) + (S1.RowY.y * S2.RowY[]) + (S1.RowY.z * S2.RowZ[])
        movss  xmm7, dword ptr [SRC1+20h]       //  xmm7 = ?            ?           ?           S1.RowZ.x
        shufps xmm7, xmm7, 0                    //  xmm7 = S1.RowZ.x    S1.RowZ.x   S1.RowZ.x   S1.RowZ.x
        mulps  xmm7, xmm1                       //  xmm7 = S1.RowZ.x * S2.RowX[]
        shufps xmm2, xmm2, 0                    //  xmm2 = S1.RowZ.y    S1.RowZ.y   S1.RowZ.y   S1.RowZ.y
        movlps qword ptr [DST+10h], xmm6        //
        movhps qword ptr [DST+18h], xmm6        //  completed D1.RowY[] = xmm6
        mulps  xmm2, xmm3                       //  xmm2 = S1.RowZ.y * S2.RowY[]
        movss  xmm6, dword ptr [SRC1+28h]       //  xmm6 = ?            ?           ?           S1.RowZ.z
        addps  xmm7, xmm2                       //  xmm7 = (S1.RowZ.x * S2.RowX[]) + (S1.RowZ.y * S2.RowY[])
        shufps xmm6, xmm6, 0                    //  xmm6 = S1.RowZ.z    S1.RowZ.z   S1.RowZ.z   S1.RowZ.z   
        mulps  xmm6, xmm4                       //  xmm6 = S1.RowZ.z * S2.RowZ[]
        addps  xmm7, xmm6                       //  xmm7 = (S1.RowZ.x * S2.RowX[]) + (S1.RowZ.y * S2.RowY[]) + (S1.RowZ.z * S2.RowZ[])
        movss  xmm0, dword ptr [SRC1+34h]       //  xmm0 = ?            ?           ?           S1.RowT.y
        shufps xmm0, xmm0, 0                    //  xmm0 = S1.RowT.y    S1.RowT.y   S1.RowT.y   S1.RowT.y    
        movlps qword ptr [DST+20h], xmm7        //  
        movss  xmm2, dword ptr [SRC1+30h]       //  xmm2 = ?            ?           ?           S1.RowT.x
        movhps qword ptr [DST+28h], xmm7        //  completed D1.RowZ[] = xmm7
        mulps  xmm0, xmm3                       //  xmm0 = S1.RowT.y * S2.RowY[]
        shufps xmm2, xmm2, 0                    //  xmm2 = S1.RowT.x    S1.RowT.x   S1.RowT.x   S1.RowT.x
        movss  xmm6, dword ptr [SRC1+38h]       //  xmm6 = ?            ?           ?           S1.RowT.z
        mulps  xmm2, xmm1                       //  xmm2 = S1.RowT.x * S2.RowX[]
        shufps xmm6, xmm6, 0                    //  xmm6 = S1.RowT.z    S1.RowT.z   S1.RowT.z   S1.RowT.z
        addps  xmm2, xmm0                       //  xmm2 = (S1.RowT.x * S2.RowX[]) + (S1.RowT.y * S2.RowY[])
        mulps  xmm6, xmm4                       //  xmm6 = S1.RowT.z * S2.RowZ[]
        addps  xmm2, xmm6                       //  xmm2 = (S1.RowT.x * S2.RowX[]) + (S1.RowT.y * S2.RowY[]) + (S1.RowT.z * S2.RowZ[])
        addps  xmm2, xmm5                       //  xmm2 = (S1.RowT.x * S2.RowX[]) + (S1.RowT.y * S2.RowY[]) + (S1.RowT.z * S2.RowZ[]) + (S2.RowT[])
        movups xmmword ptr [DST+30h], xmm2      //  completed D1.RowT[] = xmm2
    }

}


} // namespace Math
} // WT
