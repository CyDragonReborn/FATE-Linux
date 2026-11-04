/*
	VectorQuantization.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#pragma once

#include "WT/System/Platform.h"
#include "WT/System/SSE.h"

namespace WT
{
namespace Graphics
{

//Number of dimensions used for vector quantization. This is fixed
//currently only to ensure maximum performance in the distortion function
//that computes squared distance between vectors.
int32 const KVQDimensions = 4; //DO NOT CHANGE unless you also reimplement ComputeDistortion()

float32 const KDefaultLBGTolerance = 0.01f;
int32 const KDefaultLBGMaxIter = 20;

//Quantize the input vectors and produce an approximately optimal codebook.
//Codebook and Input must be aligned to 16-byte boundaries for SSE optimization
float32 QuantizeVectorsLBG(int32 const InputSize, float32 const *Input,
            int32 const CodebookSize, float32* Codebook, float32 const Tolerance,
            int32 const MaxIter );

//Compute the distortion between two vectors. In this case, we are computing the squared distance between two vectors.
//We assume KVQDimensions == 4
__forceinline float32 ComputeDistortion(float32 const Codeword[], float32 const Input[])
{
	if (System::IsSSESupported())
	{
		float32 Result;
		__asm
		{
			mov esi, Codeword
			mov edi, Input
			movaps xmm0, [esi]
			movaps xmm1, [edi]
			subps xmm0, xmm1
			mulps xmm0, xmm0
			movaps xmm1, xmm0
			shufps xmm1, xmm1, (2 << 0) | (3 << 2)
			addps xmm1, xmm0
			movaps xmm0, xmm1
			shufps xmm0, xmm0, (1 << 0) | (0 << 2)
			addss xmm0, xmm1
			movss Result, xmm0
		}
		return Result;
	} else {
		float32 d, s;
		d = Codeword[0] - Input[0];
		s = d * d;
		d = Codeword[1] - Input[1];
		s += d * d;
		d = Codeword[2] - Input[2];
		s += d * d;
		d = Codeword[3] - Input[3];
		return s + d * d;
	}
}

} //namespace Graphics
} //namespace WT
