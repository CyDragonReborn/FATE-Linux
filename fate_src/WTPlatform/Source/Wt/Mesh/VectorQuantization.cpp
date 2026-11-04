/*
	VectorQuantization.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#include "WT/stdafx.h"

#include <cmath>
#include <limits>
#include <vector>
#include <functional>
#include <algorithm>
#include <cassert>
#include "VectorQuantization.h"

namespace WT
{
namespace Graphics
{

//Return random number between 0.0 and 1.0 (inclusive)
float32 frand()
{
    return static_cast<float32>(rand()) * (1.0f / RAND_MAX);
}

struct UtilityLesser : public std::greater<int32>
{
    float32* m_pUtility;
    UtilityLesser( float32 Utility[] ) : m_pUtility( Utility ) {};
    bool operator()( int32 const& Codeword1, int32 const& Codeword2 ) const
    {
        return m_pUtility[Codeword1] < m_pUtility[Codeword2];
    }
};

//Quantize the input vectors and produce an approximately optimal codebook.
//This function implements an enhanced LBG algorithm similar to the one described in 
//http://ai.unime.it/~gp/publications/full/eblg.pdf
float32 QuantizeVectorsLBG( int32 const InputSize, float32 const *Input,
            int32 const CodebookSize, float32* Codebook, float32 const Tolerance,
            int32 const MaxIter )
{
	//Data must be SSE-aligned
	assert( System::IsSSEAligned( Input ) );
	assert( System::IsSSEAligned( Codebook ) );

    int32 const Dimensions = KVQDimensions;

    //Initialize codebook
    //Choose codewords randomly
    for (int32 i = 0; i < CodebookSize; ++i)
    {
        int32 InputIndex = rand() % InputSize;
        std::copy(&Input[InputIndex * Dimensions], &Input[InputIndex * Dimensions + Dimensions],
            &Codebook[i * Dimensions]);
    }

    float32 ResultDistortion = std::numeric_limits<float>::max();

	int32 iter = 0;
    for (; iter < MaxIter; ++iter)
    {
        //Partition input and compute new distortion 
        std::vector<std::vector<int> > Partitions;
        std::vector<float32> Distortions;
        Partitions.resize(CodebookSize);
        Distortions.resize(CodebookSize);
        std::fill(Distortions.begin(), Distortions.end(), 0.0f);

        float32 TotalDistortion = 0.0f;
        for (int32 i = 0; i < InputSize; ++i)
        {
            int32 PartitionIndex = 0; //Use fallback value

			int32 BestDistortion;
			*(float32*)&BestDistortion = std::numeric_limits<float>::max();
			
			float32 const* pCodebook = &Codebook[0];
			float32 const* pInput = &Input[i * Dimensions];

            for (int32 j = CodebookSize - 1; j >= 0; --j)
            {
				int32 Distortion;
                *(float32*)&Distortion = ComputeDistortion( &pCodebook[j * Dimensions], pInput );

				if (Distortion < BestDistortion)
                {
                    PartitionIndex = j;
                    BestDistortion = Distortion;
                }
            }

            Partitions[PartitionIndex].push_back(i);
            Distortions[PartitionIndex] += BestDistortion;
            TotalDistortion += BestDistortion;
        }


        float32 PrevDistortion = ResultDistortion;
        ResultDistortion = TotalDistortion;

        if (TotalDistortion <= 0.0f)
        {
            break;
        }

        //If there is not enough improvement in total distortion, then we're finished
        if (PrevDistortion - TotalDistortion <= TotalDistortion * Tolerance)
        {
            break;
        }

        //EBLG block
        {
            //Evaluate utility of each codeword - the utility is the ratio
            //if the codeword's distortion with the mean codeword distortion.
            std::vector<float32> Utility;
            Utility.resize(CodebookSize);
            float32 MeanDistortion = TotalDistortion / CodebookSize;
            int32 NumEmpty = 0;
            for (int32 i = 0; i < CodebookSize; ++i)
            {
                if (Partitions[i].empty())
                {
                    //The codeword is completely unused, and is less useful
                    //than a codeword representing only a single input.
                    //TODO find an elegant way of representing this fact numerically.
                    Utility[i] = -1.0f; 
                    NumEmpty++;
                } else {
                    Utility[i] = Distortions[i] / MeanDistortion;
                }
            }

            //Find a low utility codeword and high utility codeword
            //Later, we'll try to Move the low utility codeword into the high utility cell

            //We should process low utility codewords in order of decreasing utility
            //Construct a list of the codewords and then sort it.
            std::vector<int> LowUtilityCodewords;
            for (int32 i = 0; i < CodebookSize; ++i)
            {
                if (Utility[i] < 1.0f)
                {
                    LowUtilityCodewords.push_back(i);
                }
            }

            //Sort the codewords
            std::sort(LowUtilityCodewords.begin(), LowUtilityCodewords.end(),
                UtilityLesser(&Utility[0]));

            std::vector<bool> bUsed;
            bUsed.resize(CodebookSize);
            std::fill(bUsed.begin(), bUsed.end(), false);
            for (int32 i = 0; i < static_cast<int32>(LowUtilityCodewords.size()); ++i)
            {
                int32 Lo = LowUtilityCodewords[i];

                //The low utility cell must have utility < 1 and it must not
                //have been involved in shifting before.
                if (bUsed[Lo])
                {
                    continue;
                }

                //We now have low and high utility cells
                //Look for codeword nearest low utility codeword
                float32 BestDist2 = std::numeric_limits<float>::max();
                int32 Near = -1;
                for (int32 n = 0; n < CodebookSize; ++n)
                {
                    if (n == Lo || bUsed[n])
                    {
                        continue;
                    }

                    float32 Dist2 = ComputeDistortion( 
                        &Codebook[n * Dimensions], &Codebook[Lo * Dimensions]);
                    if (Dist2 < BestDist2)
                    {
                        BestDist2 = Dist2;
                        Near = n;
                    }
                }
                
                //If no unused nearby codeword, then give up
                if (Near < 0)
                {
                    continue;
                }

                float32 TotalWeight = 0.0f;
                int32 Hi;
                //Choose a high utility cell randomly weighted by utility
                for ( Hi = 0; Hi < CodebookSize; ++Hi)
                {
                    if (!bUsed[Hi] && Utility[Hi] > 1.0f)
                    {
                        //TODO "utility -1" seems most natural, is it the
                        //correct choice?
                        TotalWeight += Utility[Hi] - 1.0f;
                    }
                }

                //If no cells have high utility, then we're done with ELBG.
                if (TotalWeight == 0.0f)
                {
                    break;
                }

                float32 Weight = TotalWeight * frand();
                for (int32 j = 0; j < CodebookSize; ++j)
                {
                    //machine rounding causes a problem
                    if (!bUsed[j] && Utility[j] > 1.0f)
                    {
                        Hi = j; //Make sure we have a valid Hi value even if 
                        //machine rounding causes a problem
                        Weight -= Utility[Hi] - 1.0f;
                        if (Weight <= 0.0f)
                        {
                            break;
                        }
                    }
                }

                //Choose new location for codewords
                //Compute corners of hyperbox around cell Hi
                std::vector<float32> Min,Max;
                Min.resize(Dimensions);
                Max.resize(Dimensions);
                std::fill(Min.begin(), Min.end(), std::numeric_limits<float>::max());
                std::fill(Max.begin(), Max.end(), -std::numeric_limits<float>::max());

                for (int32 i = 0; i < static_cast<int32>(Partitions[Hi].size()); ++i)
                {
                    int32 InputIndex = Partitions[Hi][i];
                    for (int32 m = 0; m < Dimensions; ++m)
                    {
                        float32 Val = Input[InputIndex * Dimensions + m];
                        
                        if (Min[m] > Val)
                        {
                            Min[m] = Val;
                        }

                        if (Max[m] < Val)
                        {
                            Max[m] = Val;
                        }
                    }
                }

                //Choose an arbitrary diagonal and place the two codewords on it
                std::vector<uint8> CodewordHiRaw, CodewordLoRaw;
                CodewordHiRaw.resize( Dimensions * sizeof(float32) + (System::KSSEAlignmentSize - 1) );
                CodewordLoRaw.resize( Dimensions * sizeof(float32) + (System::KSSEAlignmentSize - 1) );
				float32* CodewordHi = static_cast<float32*>( System::SSEAlign( &CodewordHiRaw[0] ) );
				float32* CodewordLo = static_cast<float32*>( System::SSEAlign( &CodewordLoRaw[0] ) );
                
                for (int32 m = 0; m < Dimensions; ++m)
                {
                    CodewordHi[m] = Min[m] + 0.25f * (Max[m] - Min[m]);
                    CodewordLo[m] = Min[m] + 0.75f * (Max[m] - Min[m]);
                }

                //Apply a few iterations of LBG with tolerance 0.1 to single cell and two codewords
                float32 const KMyTolerance = 0.1f;
                int32 const KMyMaxIter = 5;
                float32 MyDistortion = std::numeric_limits<float>::max();
				float32 TotalDistortion = std::numeric_limits<float>::max(); //Initialize only to avoid warning
                std::vector<int> NewPartitionLo;
                std::vector<int> NewPartitionHi;
                for (int32 MyIter = 0; MyIter < KMyMaxIter; ++MyIter)
                {
	                float32 DistortionLo = 0.0f;
					float32 DistortionHi = 0.0f;

                    NewPartitionLo.clear();
                    NewPartitionHi.clear();

                    for (int32 i = 0; i < static_cast<int32>(Partitions[Hi].size()); ++i)
                    {
                        int32 InputIndex = Partitions[Hi][i];
                        float32 DistortionL = ComputeDistortion( &CodewordLo[0], &Input[InputIndex * Dimensions]);
                        float32 DistortionH = ComputeDistortion( &CodewordHi[0], &Input[InputIndex * Dimensions]);

                        if (DistortionL < DistortionH)
                        {
                            DistortionLo += DistortionL;
                            NewPartitionLo.push_back(InputIndex);
                        } else {
                            DistortionHi += DistortionH;
                            NewPartitionHi.push_back(InputIndex);
                        }
                    }

                    TotalDistortion = DistortionLo + DistortionHi;
                    if (MyDistortion - TotalDistortion <= TotalDistortion * KMyTolerance)
                    {
    		            break;
                    }
                    MyDistortion = TotalDistortion;

                    //New codebook calculation
                    if (NewPartitionLo.size())
                    {
                        std::fill(CodewordLo, CodewordLo + Dimensions, 0.0f);
                        for (int32 j = 0; j < static_cast<int32>(NewPartitionLo.size()); ++j)
                        {
                            for (int32 m = 0; m < Dimensions; ++m)
                            {
                                CodewordLo[m] += Input[NewPartitionLo[j] * Dimensions + m];
                            }
                        }

                        float32 Factor = 1.0f / static_cast<float32>(NewPartitionLo.size());
                        for (int32 m = 0; m < Dimensions; ++m)
                        {
                            CodewordLo[m] *= Factor;
                        }
                    } else {
                        //Choose a random input
                        int32 InputIndex = Partitions[Hi][rand() % Partitions[Hi].size()];
                        std::copy( &Input[InputIndex * Dimensions], &Input[InputIndex * Dimensions + Dimensions],
                            &CodewordLo[0]);
                    }

                    if (NewPartitionHi.size())
                    {
                        std::fill(CodewordHi, CodewordHi + Dimensions, 0.0f);
                        for (int32 j = 0; j < static_cast<int32>(NewPartitionHi.size()); ++j)
                        {
                            for (int32 m = 0; m < Dimensions; ++m)
                            {
                                CodewordHi[m] += Input[NewPartitionHi[j] * Dimensions + m];
                            }
                        }

                        float32 Factor = 1.0f / static_cast<float32>(NewPartitionHi.size());
                        for (int32 m = 0; m < Dimensions; ++m)
                        {
                            CodewordHi[m] *= Factor;
                        }
                    } else {
                        //Choose a random input
                        int32 InputIndex = Partitions[Hi][rand() % Partitions[Hi].size()];
                        std::copy( &Input[InputIndex * Dimensions], &Input[InputIndex * Dimensions + Dimensions],
                            &CodewordHi[0]);
                    }
                }

                //Assign inputs in cell Lo to the codeword Near
                std::vector<int> NewPartitionNear = Partitions[Near];
                for (int32 i = 0; i < static_cast<int32>(Partitions[Lo].size()); ++i)
                {
                    NewPartitionNear.push_back(Partitions[Lo][i]);
                }

                //Update centroid for the Near codeword
                std::vector<uint8> CodewordNearRaw;
                CodewordNearRaw.resize( Dimensions * sizeof(float32) + (System::KSSEAlignmentSize - 1) );
				float32* CodewordNear = static_cast<float32*>( System::SSEAlign( &CodewordNearRaw[0] ) );

                for (int32 m = 0; m < Dimensions; ++m)
                {
                    CodewordNear[m] = Codebook[Near * Dimensions + m] * Partitions[Near].size();
                }

                for (int32 i = 0; i < static_cast<int32>(Partitions[Lo].size()); ++i)
                {
                    for (int32 m = 0; m < Dimensions; ++m)
                    {
                        CodewordNear[m] += Input[Partitions[Lo][i] * Dimensions + m];
                    }
                }
                if (NewPartitionNear.size())
                {
                    float32 Factor = 1.0f / NewPartitionNear.size();
                    for (int32 m = 0; m < Dimensions; ++m)
                    {
                        CodewordNear[m] *= Factor;
                    }
                }

                //Check that change in distortion of the three cells has decreased
                //This is sufficient to show that the total distortion has decreased
                float32 DistortionNear = 0.0f;
                for (int32 i = 0; i < static_cast<int32>(NewPartitionNear.size()); ++i)
                {
                    DistortionNear += ComputeDistortion( &CodewordNear[0], &Input[NewPartitionNear[i] * Dimensions]);
                }

                if (TotalDistortion + DistortionNear >
                    Distortions[Lo] + Distortions[Hi] + Distortions[Near])
                {
                    continue;
                }

                //Commit the changes
                std::copy(CodewordLo, CodewordLo + Dimensions, &Codebook[Lo * Dimensions]);
                std::copy(CodewordHi, CodewordHi + Dimensions, &Codebook[Hi * Dimensions]);
                std::copy(CodewordNear, CodewordNear + Dimensions, &Codebook[Near * Dimensions]);

                Partitions[Lo] = NewPartitionLo;
                Partitions[Hi] = NewPartitionHi;
                Partitions[Near] = NewPartitionNear;

                bUsed[Lo] = true;
                bUsed[Hi] = true;
                bUsed[Near] = true;
            }
        }

        //New codebook calculation
        for (int32 i = 0; i < CodebookSize; ++i)
        {
            std::fill(&Codebook[i * Dimensions], &Codebook[i * Dimensions + Dimensions], 0.0f);

            if (Partitions[i].size())
            {
                for (int32 j = 0; j < static_cast<int32>(Partitions[i].size()); ++j)
                {
                    for (int32 m = 0; m < Dimensions; ++m)
                    {
                        Codebook[i * Dimensions + m] += Input[Partitions[i][j] * Dimensions + m];
                    }
                }

                float32 Factor = 1.0f / Partitions[i].size();
                for (int32 m = 0; m < Dimensions; ++m)
                {
                    Codebook[i * Dimensions + m] *= Factor;
                }
            }
        }

		//TODO report equal amounts of progress for each iteration
	}

	if (iter < MaxIter)
	{
		//TODO report any remaining progress here, if we ended iterations prematurely
	}

    return ResultDistortion;
}

} //namespace Graphics
} //namespace WT
