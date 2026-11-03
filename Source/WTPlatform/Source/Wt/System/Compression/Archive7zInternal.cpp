/***************************************************************************************
	Archive7zInternal.cpp

	Copyright (C) 2002-2005 WildTangent, Inc. 
	All Rights Reserved 

	Description:  7z decoding stuff. 

	History:   
		01/21/2005	Rui Jiang	Created
		09/28/2005  Rui Jiang	Renamed from wt7z.cpp to Archive7zInternal.cpp, and 
				moved into the WTPlatform.
***************************************************************************************/
#include "WT/stdafx.h"
#include "Archive7zInternal.h"


//warning C4127: conditional expression is constant
#pragma warning(disable: 4127)

namespace WT
{
namespace System
{
namespace Compression
{
namespace Archive7zInternal
{

//static const UI32 CCRC::kCRCPoly = 0xEDB88320;
UI32 CCRC::Table[256];
bool CCRC::g_TableInited = false;

void CCRC::InitTable()
{
	if (g_TableInited) return;
	for (UI32 i = 0; i < 256; i++)
	{
		UI32 r = i;
		for (int j = 0; j < 8; j++)
			if (r & 1) 
				r = (r >> 1) ^ kCRCPoly;
			else     
				r >>= 1;
		CCRC::Table[i] = r;
	}
	g_TableInited = true;
}

/*

Don't do this. Do it in the archive initialization instead.

class CCRCTableInit
{
public:
	CCRCTableInit() { CCRC::InitTable(); }
} g_CRCTableInit;
*/

///////////////////////////////////////////////////////////////////////////////
// LzmaDecoder

//Properties must have size of 5.
//In LzmaDecoderInit, it will read first 5 bytes of the stream. So make sure we have
//Stream ready before calling init.
bool LzmaDecoder::Init(const unsigned char* Properties, int PropertySize, int TOutputSize)
{
	Clear();

	int lc, lp, pb;
	int lzmaInternalSize;
	char prop0;

	if (PropertySize != 5) return false;
	prop0 = Properties[0];
	if (prop0 >= (9*5*5))
	{
		return false;
	}
	for (pb = 0; prop0 >= (9 * 5); pb++, prop0 -= (9 * 5));
	for (lp = 0; prop0 >= 9; lp++, prop0 -= 9);
	lc = prop0;

	lzmaInternalSize = 
		(LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (lc + lp)))* sizeof(CProb);

	//WE use _LZMA_OUT_READ.
	lzmaInternalSize += 100;

	m_LzmaInternalData.resize(lzmaInternalSize);

	//Dictionary size.
	unsigned int DictionarySize = 0;
	for(int i = 0; i < 4; i++)
	{
		DictionarySize += ((unsigned int)Properties[i+1]) << (i * 8);
	}
	m_Dictionary.resize(DictionarySize);

	ReadCallBack.Read = ReadCallBackFunc;
	ReadCallBack.m_Self = this;
	if (LzmaDecoderInit((unsigned char*)(&m_LzmaInternalData[0]), lzmaInternalSize, lc, lp, pb, &m_Dictionary[0], DictionarySize, &ReadCallBack)
		!= 0) return false;

	return true;
}

//Process one piece of output.
bool LzmaDecoder::ProcessOneBlock(unsigned char* Output, int OutSize, int* OutSizeProcessed)
{
	unsigned int ui;
	int res = LzmaDecode(&m_LzmaInternalData[0], Output, OutSize, &ui);
	*OutSizeProcessed = ui;
	return res == 0;
}

///////////////////////////////////////////////////////////////////////////////
// CBCJ_x86_Decoder

UI32 CBCJ_x86_Decoder::x86_Convert(UI8 *buffer, UI32 endPos, UI32 nowPos, 
	UI32 *prevMask, UI32 *prevPos, int encoding)
{
	const int kMaskToAllowedStatus[8] = {1, 1, 1, 0, 1, 0, 0, 0};
	const UI8 kMaskToBitNumber[8] = {0, 1, 2, 2, 3, 3, 3, 3};

	UI32 bufferPos = 0;

	if (endPos < 5)
		return 0;

	if (nowPos - *prevPos > 5)
		*prevPos = nowPos - 5;

	UI32 limit = endPos - 5;
	while(bufferPos <= limit)
	{
		UI8 b = buffer[bufferPos];
		if (b != 0xE8 && b != 0xE9)
		{
			bufferPos++;
			continue;
		}
		UI32 offset = (nowPos + bufferPos - *prevPos);
		*prevPos = (nowPos + bufferPos);
		if (offset > 5)
			*prevMask = 0;
		else
		{
			for (UI32 i = 0; i < offset; i++)
			{
				*prevMask &= 0x77;
				*prevMask <<= 1;
			}
		}
		UI8 &nextByte = buffer[bufferPos + 4];
		if (Test86MSByte(nextByte) && kMaskToAllowedStatus[(*prevMask >> 1) & 0x7] && 
			(*prevMask >> 1) < 0x10)
		{
			UI32 src = 
				(UI32(nextByte) << 24) |
				(UI32(buffer[bufferPos + 3]) << 16) |
				(UI32(buffer[bufferPos + 2]) << 8) |
				(buffer[bufferPos + 1]);

			UI32 dest;
			while(true)
			{
				if (encoding)
					dest = (nowPos + bufferPos + 5) + src;
				else
					dest = src - (nowPos + bufferPos + 5);
				if (*prevMask == 0)
					break;
				UI32 index = kMaskToBitNumber[*prevMask >> 1];
				if (!Test86MSByte(dest >> (24 - index * 8)))
					break;
				src = dest ^ ((1 << (32 - index * 8)) - 1);

			}
			nextByte = ~(((dest >> 24) & 1) - 1);
			buffer[bufferPos + 3] = (dest >> 16);
			buffer[bufferPos + 2] = (dest >> 8);
			buffer[bufferPos + 1] = dest;
			bufferPos += 5;
			*prevMask = 0;
		}
		else
		{
			bufferPos++;
			*prevMask |= 1;
			if (Test86MSByte(nextByte))
				*prevMask |= 0x10;
		}
	}
	return bufferPos;
}

///////////////////////////////////////////////////////////////////////////////
// StreamsInfo

void StreamsInfo::Debug(FILE* f, const std::string& indent)
{
	fprintf(f, "%sPackPos: %d\n", indent.c_str(), PackPos);
	fprintf(f, "%sPacked Streams: %d\n", indent.c_str(), PackedStreams.size());
	int i;
	for(i = 0; i < (int)PackedStreams.size(); i++)
	{
		fprintf(f, "%sPackedStream %d:\n", (indent+DEBUG_INDENT).c_str(), i);
		PackedStreams[i].Debug(f, indent + DEBUG_INDENT+ DEBUG_INDENT);
	}
	fprintf(f, "%sFolders: %d\n", indent.c_str(), Folders.size());
	for(i = 0; i < (int)Folders.size(); i++)
	{
		fprintf(f, "%sFolder %d:\n", (indent+DEBUG_INDENT).c_str(), i);
		Folders[i].Debug(f, indent + DEBUG_INDENT + DEBUG_INDENT);
	}

	//m_Header.Debug(indent, f);
}

void StreamsInfo::Clear()
{
	PackPos = 0;
	PackedStreams.clear();
	Folders.clear();
}

//Will throw an exception if anything is wrong.
void StreamsInfo::Parse(IStream* is)
{
	while(true)
	{
		Type7z type = is->ReadType();
		switch(type)
		{
		case kEnd:
			return;
			break;
		case kPackInfo:
			ParsePackInfo(is);
			break;
		case kUnPackInfo:
			ParseCodersInfo(is);

			//Finally, set each folder's default UnpackStream. We need this step because 
			// if one folder's have only one unpack stream, there might not be much unpack 
			// stream information at all!!!!
			for(int i = 0; i < (int)Folders.size(); i++)
			{
				Folder& folder = Folders[i];
				folder.NumUnpackStreams = 1;
				folder.UnpackStreams.resize(folder.NumUnpackStreams);
				folder.UnpackStreams[0].Size = folder.GetUnpackSize();
				folder.UnpackStreams[0].CRCDefined = folder.UnpackCRCDefined;
				folder.UnpackStreams[0].CRC = folder.UnpackCRC;
			}
			break;
		case kSubStreamsInfo:
			ParseSubStreamsInfo(is);
			break;
		}
	}		
}
//Decode from the stream to a Memory Stream. Folders will init Memory stream.
//Caller should seek IStream to the correct position.
void StreamsInfo::DecodeInMemory(IStream* is, MemStream& ms)
{
	int size = GetUnpackSize();
	//Move to start of Packed streams.
	is->SeekSet(sizeof(SignatureHeader) + this->PackPos);
	
	//Init memstream.
	ms.Resize(size);
	ms.SeekSet(0);
	
	int streamIndex = 0;
	for(int i = 0; i < (int)Folders.size(); i++)
	{
		Folder& folder = Folders[i];
		for(int j = 0; j < (int)folder.Coders.size(); j++)
		{
			Coder& coder = folder.Coders[j];
			if (!coder.IsLZMA())
			{
				throw "Coders other than LZMA are not supported.";
			}
			if (coder.NumInStreams != (int)coder.UnPackSizes.size())
			{
				throw "In stream size more than outstream size.";
			}
			for(int k = 0; k < coder.NumInStreams; k++)
			{
				if (streamIndex >= (int)PackedStreams.size()) 
				{
					throw "We have less packed streams in stream info than needed.";
				}
				PackStream& ps = PackedStreams[streamIndex];
				streamIndex++;

				int OutSize = coder.UnPackSizes[k];
				MemStream InputMS;
				InputMS.Create(is, ps.PackSize);
				if (ps.CRCDefined)
				{
					//Verify CRC.
					if (!CCRC::VerifyDigest(ps.CRC, InputMS.GetBuffer(), InputMS.GetLength()))
					{
						throw "CRC verify failed for input stream.";
					}
				}
				LzmaDecoderMem ldm;
				if (!ldm.DecodeMemory(&coder.Properties[0], (int)coder.Properties.size(), (unsigned char*)InputMS.GetBuffer(), InputMS.GetLength(), 
					(unsigned char*)ms.GetBuffer(), &OutSize))
				{
					throw "Failed to decode LZMA.";
				}
				if (OutSize != coder.UnPackSizes[k])
				{
					throw "Decoded length is not the same as header says.";
				}
			}
		}
	}
}
//All folders' unpack size.
int StreamsInfo::GetUnpackSize()
{
	int total = 0;
	for(int i = 0; i < (int)Folders.size(); i++)
	{
		total += Folders[i].GetUnpackSize();
	}
	return total;
}
//All folders UnpackStream ' total size.
int StreamsInfo::GetUnpackStreamSize()
{
	int total = 0;
	for(int i = 0; i < (int)Folders.size(); i++)
	{
		total += Folders[i].GetUnpackStreamSize();
	}
	return total;
}

void StreamsInfo::ParsePackInfo(IStream* is)
{
	PackPos = is->Read7zInt();
	int packStream = is->Read7zInt();
	if (packStream < 0)
	{
		throw "Invalid pack stream size";
	}
	PackedStreams.resize(packStream);
	int i;
	while(true)
	{
		Type7z type = is->ReadType();
		switch(type)
		{
		case kEnd:
			return;
			break;
		case kSize:
			for(i = 0; i < packStream; i++)
			{
				PackedStreams[i].PackSize = is->Read7zInt();
			}
			break;
		case kCRC:
			for(i = 0; i < packStream; i++)
			{
				PackedStreams[i].CRCDefined = true;
				PackedStreams[i].CRC = is->ReadUI32();
			}
			break;
		}
	}
}
void StreamsInfo::ParseCodersInfo(IStream* is)
{
	is->ReadUntilType(kFolder);
	int numFolders = is->Read7zInt();
	UI8 External = is->ReadByte();
	if (External != 0)
	{
		//Need to switch stream. But Let's just don't support it.  WARNING!!!!
		throw "Stream switch in Coders Info is not supported.";
	}
	Folders.resize(numFolders);
	for(int i = 0; i < numFolders; i++)
	{
		Folders[i].Parse(is);
	}
	
	//Unpack size for each folder.
	is->ReadUntilType(kCodersUnPackSize);
	for(int i = 0; i < (int)Folders.size(); i++)
	{
		Folders[i].ParseUnpackSizes(is);
	}

	//Could have CRC
	while(true)
	{
		Type7z type = is->ReadType();
		switch(type)
		{
		case kEnd:
			return;
			break;
		case kCRC:
			ParseCRCDigests(is);
			break;
		}	
	}
}
void StreamsInfo::ParseCRCDigests(IStream* is)
{
	int i;

	//Depends on the first byte. If it is 0, then we have a bit vector denoting
	// Which folders have CRC.
	UI8 allDefined = is->ReadByte();
	if (allDefined == 0)
	{
		//TODO: need to verify this piece of code.
		std::vector<bool> vb;
		is->ReadBoolVector((int)Folders.size(), vb);
		for(i = 0; i < (int)Folders.size(); i++)
		{
			Folders[i].UnpackCRCDefined = vb[i];
		}
	}
	else
	{
		for(i = 0; i < (int)Folders.size(); i++)
		{
			Folders[i].UnpackCRCDefined = true;
		}
	}
	for(i = 0; i < (int)Folders.size(); i++)
	{
		if (Folders[i].UnpackCRCDefined)
		{
			Folders[i].UnpackCRC = is->ReadUI32();
		}
	}
}
void StreamsInfo::ParseSubStreamsInfo(IStream* is)
{
	//TODO!!
	int i,j;
	while(true)
	{
		Type7z type = is->ReadType();
		switch(type)
		{
		case kEnd:
			return;
			break;
		case kNumUnPackStream:
			for(i = 0; i < (int)Folders.size(); i++)
			{
				Folders[i].NumUnpackStreams = is->Read7zInt();
				Folders[i].UnpackStreams.resize(Folders[i].NumUnpackStreams);
			}
			break;
		case kSize:
			for(i = 0; i < (int)Folders.size(); i++)
			{
				int total = 0;
				Folder& folder = Folders[i];
				//printf("\n");
				//Careful! Actually we count 1 less, the rest is the last unpack stream. Shit!
				for(j = 0; j + 1 < (int)folder.UnpackStreams.size(); j++)
				{
					folder.UnpackStreams[j].Size = is->Read7zInt();
					total += folder.UnpackStreams[j].Size;
					//printf("%d\t", (int)folder.UnpackStreams[j].Size);
				}
				if (folder.UnpackStreams.size() == 0)
				{
					throw "Invalid header";
				}
				folder.UnpackStreams[folder.UnpackStreams.size() - 1].Size = folder.GetUnpackSize() - total;
			}
			break;
		case kCRC:
			int numDigests;
			numDigests = 0;
			//int numDigestsTotal = 0;
			for(i = 0; i < (int)Folders.size(); i++)
			{
				if (Folders[i].NumUnpackStreams != 1 || !Folders[i].UnpackCRCDefined)
					numDigests += Folders[i].NumUnpackStreams;
				//numDigestsTotal += numSubstreams;
			}
			std::vector<bool> v;
			std::vector<UI32> digests;
			is->ReadHashDigests(numDigests, v, digests);
			int digestIndex = 0;
			for(i = 0; i < (int)Folders.size(); i++)
			{
				Folder& folder = Folders[i];
				//Only one stream, so we grab from the folder.
				if (folder.NumUnpackStreams == 1 && folder.UnpackCRCDefined)
				{
					assert(folder.UnpackStreams.size());
					folder.UnpackStreams[0].CRCDefined = true;
					folder.UnpackStreams[0].CRC = folder.UnpackCRC;
				}
				else
				{
					for(int j = 0; j < folder.NumUnpackStreams; j++)
					{
						folder.UnpackStreams[j].CRCDefined = v[digestIndex];
						folder.UnpackStreams[j].CRC = digests[digestIndex];
						digestIndex++;
					}
				}
			}
			assert(digestIndex == numDigests);
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Header

void Header::Debug(FILE* f, const std::string& indent)
{
	fprintf(f, "%sMainStreams Information: \n", indent.c_str());
	MainStreamsInfo.Debug(f, indent + DEBUG_INDENT);

	fprintf(f, "%sTotal FilesInfo: %d\n", indent.c_str(), FilesInfo.size());
	//m_Header.Debug(indent, f);
}
void Header::Parse(IStream* is)
{
	while(true)
	{
		Type7z type = is->ReadType();
		switch(type)
		{
		case kEnd:
			return;
			break;
		case kArchiveProperties:
			//TODO
			throw "Archive properties is not supported.";
			ParseArchiveProperties(is);
			break;
		case kAdditionalStreamsInfo:
			//TODO.
			throw "Additional stream is not supported.";
			break;
		case kMainStreamsInfo:
			MainStreamsInfo.Parse(is);
			break;
		case kFilesInfo:
			ParseFilesInfo(is);
			break;
		}	
	}
}
void Header::Clear()
{
	FilesInfo.clear();
	MainStreamsInfo.Clear();
}

//The current 7z doing nothing with archive properties. I am following him.
void Header::ParseArchiveProperties(IStream* is)
{
	while(true)
	{
		Type7z type = is->ReadType();
		switch(type)
		{
		case kEnd:
			return;
			break;
		default:
			int size;
			size = is->Read7zInt();
			is->SkipData(size);
		}	
	}
}
void Header::ParseFilesInfo(IStream* is)
{
	int NumFiles = is->Read7zInt();
	int i;
	FilesInfo.resize(NumFiles);
	while(true)
	{
		Type7z type = is->ReadType();
		if (type == kEnd) break;

		//This size seems to be the datasize followed it.
		int size = is->Read7zInt();
		size = size; //To remove the warning about size not used.
		switch(type)
		{
		case kEnd:
			return;
			break;
		case kEmptyStream:
			{
				std::vector<bool> v;
				is->ReadBoolVector(NumFiles, v);
				for(i = 0; i < (int)v.size(); i++)
				{
					FilesInfo[i].EmptyStream = v[i];
				}
			}
			break;
		case kEmptyFile:
			{
				int EmptyStreamCount = 0;
				for(int i = 0; i < (int)FilesInfo.size(); i++)
				{
					EmptyStreamCount += FilesInfo[i].EmptyStream? 1 : 0;
				}
				std::vector<bool> v;
				is->ReadBoolVector(EmptyStreamCount, v);
				int Index = 0;
				assert((int)v.size() == EmptyStreamCount);
				for(int i = 0; i < (int)FilesInfo.size(); i++)
				{
					if (FilesInfo[i].EmptyStream)
					{
						FilesInfo[i].EmptyFile = v[Index];
						Index ++;
					}
				}					
			}
			break;
		case kName:
			ParseFileNames(is);
			break;
		case kCreationTime:
		case kLastWriteTime:
		case kLastAccessTime:
			ParseFileTimes(is, type);
			break;
		case kWinAttributes:
			ParseWinAttributes(is);
			break;
		}	

	}
}

void Header::ParseFileNames(IStream* is)
{
	UI8 external;
	external = is->ReadByte();
	if (external != 0)
	{
		throw "Stream switch in file name not supported!";
	}
	else
	{
		for(int i = 0; i < (int)FilesInfo.size(); i++)
		{
			FileInfo& fi = FilesInfo[i];
			fi.Name.clear();
			while(true)
			{
				wchar_t wc = is->ReadWideChar();
				if (wc == 0) 
					break;
				fi.Name += wc;				
			}
		}
	}
}

void Header::ParseFileTimes(IStream* is, Type7z type)
{
	std::vector<bool> v;
	is->ReadBoolVector2((int)FilesInfo.size(), v);
	is->ReadStreamSwitch();
	for(int i = 0; i < (int)v.size(); i++)
		if (v[i])
	{
		FileDateTime fdt;
		fdt.Low = is->ReadUI32();
		fdt.High = is->ReadUI32();
		fdt.Defined = true;
		switch(type)
		{
		case kCreationTime:
			FilesInfo[i].CreationTime = fdt;
			break;
		case kLastWriteTime:
			FilesInfo[i].LastWriteTime = fdt;
			break;
		case kLastAccessTime:
			FilesInfo[i].LastAccessTime = fdt;
			break;
		}
	}
}

void Header::ParseWinAttributes(IStream* is)
{
	std::vector<bool> v;
	is->ReadBoolVector2((int)FilesInfo.size(), v);
	is->ReadStreamSwitch();
	for(int i = 0; i < (int)v.size(); i++)
		if (v[i])
	{
		FilesInfo[i].WinAttribute = is->ReadUI32();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Archive

bool Archive::ExtractInit(const char* FileName)
{
	Clear();
	m_ArchiveFileName = FileName;
	if (!m_ArchStream.Open(FileName, "rb"))
	{
		SetError(ecFailedToOpenArchive);
		SetErrorObject(FileName);
		return false;
	}
	return ExtractInit(&m_ArchStream);
}

bool Archive::ExtractInit(IStream* is)
{
	//remove last "\\";
	if (m_DestDir.size() > 0 && m_DestDir[m_DestDir.size() - 1] == '\\')
	{
		m_DestDir.resize(m_DestDir.size() - 1);
	}
	assert(is);
	try
	{
		m_ArchiveFileSize = is->GetLength();

		//Read in Signature header.
		is->Read(&m_SHeader, sizeof(m_SHeader));
		if (!VerifySignature(m_SHeader.Signature, 6)) 
		{
			throw "Not a 7z file.";
		}

		//Now, read in header.
		MemStream ms;
		is->SeekCur(m_SHeader.NextHeaderOffset);
		ms.Create(is, m_SHeader.NextHeaderSize);

		//Verify Digest.
		if (!CCRC::VerifyDigest(m_SHeader.NextHeaderCRC, ms.GetBuffer(), ms.GetLength()))
		{
			throw "CRC Error";
		}
		
		//Now we switch to the memory stream.
		Type7z type = ms.ReadType();
		if (type == kHeader)
		{
			m_Header.Parse(&ms);
		}
		else if (type == kEncodedHeader)
		{
			//In this case, it's a StreamsInfo that need to be decoded first.
			StreamsInfo si;
			//Parse directly from memory stream.
			si.Parse(&ms);
			if (si.Folders.size() != 1)
			{
				throw "Invalid header. Contains more than one folders.";
			}
			MemStream HeaderStream;
			si.DecodeInMemory(is, HeaderStream);
			m_Header.Parse(&HeaderStream);
		}
		else
		{
			throw "Invalid header";
		}
		/* These two numbers should be the same !!!!!!
		printf("UnpackStream Size: %d. Unpack Size %d\n", m_Header.MainStreamsInfo.GetUnpackStreamSize(), 
			m_Header.MainStreamsInfo.GetUnpackSize());
		*/
		if (!CreateInternalMapping()) 
		{
			if (GetLastError() == ecInvalidHeader)
				SetErrorObject(m_ArchiveFileName.c_str());
			return false;
		}


		m_DecodeStream = is;
		m_OutputBuffer.resize(m_OutputBufferSize);
		m_InputBuffer.resize(m_InputBufferSize);
		
		m_TotalUnpackBytes = m_Header.MainStreamsInfo.GetUnpackSize();
		for(int i = 0; i <(int) m_Header.FilesInfo.size(); i++)
		{
			FileInfo& fi = m_Header.FilesInfo[i];
			if (!fi.IsDirectory())
				m_TotalFiles ++ ;
		}
		SetStatus(stProcessing);
	}
	catch(const char*)
	{
		Clear();
		//Corrupted file.
		SetError(ecInvalidHeader);
		SetErrorObject(m_ArchiveFileName.c_str());
	}
	catch(...)
	{
		Clear();
		SetError(ecUnknown);
		return false;
	}
	return true;
}

bool Archive::Extract(const char* DestDir, bool override, int BlockSize)
{
	if (GetStatus() != stProcessing) return false;
	SetExtractOptions(DestDir, override);
	//Start the loop.
	while(! this->IsFinished())
	{
		if (! this->ProcessBlock(BlockSize)) 
		{				
			return false;
		}
	}
	return true;
}

//Process maximum of OutSize then return.
bool Archive::ProcessBlock(int OutSize)
{
	//assert(GetStatus() == stProcessing);
	if (GetStatus() != stProcessing) return false;

	if (m_TimesProcessBlockCalled == 0)
	{
		OnStartExtraction();
		/*
		let's just create dir on the fly.
		if (!CreateDirectories())
		{
			SetStatus(stErrored);
			return false;
		}*/
		if (!CreateEmptyFiles()) 
		{
			SetStatus(stErrored);
			return false;
		}
	}
	m_TimesProcessBlockCalled++;

	if (m_DecodeStream == NULL)
	{
		SetError(ecUnknown);
		SetStatus(stErrored);
		return false;
	}
	int ProcessedSize = 0;
	while(ProcessedSize < OutSize)
	{
		//We are already finished?
		if (m_ProcessingPackStreamIndex >= (int)m_InternalPackStreams.size()) 
		{
			SetStatus(stDone);
			return true;
		}

		InternalPackStream& ips = m_InternalPackStreams[m_ProcessingPackStreamIndex];

		if (m_ProcessingPackStreamProcessed == 0)
		{
			m_DecodeStream->SeekSet(ips.StartPos);
			//Just started! Init decoder.
			if (!LzmaDecoder::Init(&ips.CoderPtr->Properties[0], (int)ips.CoderPtr->Properties.size(), ips.CoderPtr->GetUnpackSize()))
			{
				SetError(ecDecoding);
				SetStatus(stErrored);
				return false;
			}
			if (ips.BcjEncoded)
			{
				m_BcjDecoder.SubInit();
			}
		}

		if (m_ProcessingUnpackStreamIndex >= (int)ips.InternalUnpackStreams.size())
		{
			//Actually this may not be necessary. But let's play safe.
			CloseCurrentFile();

			//Move Pack pointer up one. 
			m_ProcessingPackStreamIndex ++;
			m_ProcessingPackStreamProcessed = 0;

			m_ProcessingUnpackStreamIndex = 0;
			m_ProcessingUnpackStreamProcessed = 0;
			continue;
		}

		//How many bytes have we processed.
		int ThisProcessed = 0;
		int WantedSize = std::min((int)m_OutputBuffer.size() - m_OutputBufferUnfiltered, ips.TotalUnpackSize - ips.TotalUnpackedBytes);
		//This should never happen.
		if (WantedSize <= 0)
		{
			assert(false);
			SetError(ecDecoding);
			SetStatus(stErrored);
			return false;
		}
		if (!LzmaDecoder::ProcessOneBlock(&m_OutputBuffer[0] + m_OutputBufferUnfiltered, WantedSize, &ThisProcessed))
		{
			//It could be because we failed to read from file, CRC verification of source file failed. 
			if (GetLastError() == ecOK)
			{
				SetError(ecDecoding);
			}
			SetStatus(stErrored);
			return false;
		}
		m_OutputBufferUnfiltered += ThisProcessed;
		ips.TotalUnpackedBytes += ThisProcessed;

		//May need BCJ filter.
		int FilteredSize = 0;
		if (ips.BcjEncoded)
		{				
			FilteredSize = m_BcjDecoder.SubFilter(&m_OutputBuffer[0], m_OutputBufferUnfiltered);
		}
		else
		{
			FilteredSize = m_OutputBufferUnfiltered;
		}
		//If we are finished with the stream, then filter should run to end.
		if (ips.TotalUnpackedBytes == ips.TotalUnpackSize)
		{
			FilteredSize = m_OutputBufferUnfiltered;
		}
		//Flush everything. 
		if (!FlushDecodedStream(&m_OutputBuffer[0], FilteredSize)) 
			return false;
		//Move the rest portion to beggining. Should be very small!
		for(int i = 0; i < m_OutputBufferUnfiltered - FilteredSize; i++)
		{
			m_OutputBuffer[i] = m_OutputBuffer[FilteredSize + i];
		}
		m_OutputBufferUnfiltered -= FilteredSize;
		//m_BcjDecoder.SetBufferPos(0);

		ProcessedSize += FilteredSize;
		m_TotalUnpackBytesProcessed += FilteredSize;
	}
	return true;
}

//Output decoded stream to files. Note there might be multiple files involved.
bool Archive::FlushDecodedStream(UI8* buffer, int size)
{
	//This shouldn't happen.
	if (m_ProcessingPackStreamIndex < 0 || m_ProcessingPackStreamIndex >= (int)m_InternalPackStreams.size())
		return false;
	InternalPackStream& ips = m_InternalPackStreams[m_ProcessingPackStreamIndex];
	while(size > 0)
	{
		//This shouldn't happen either. We have more bytes than unpackstream?
		if (m_ProcessingUnpackStreamIndex < 0 || m_ProcessingUnpackStreamIndex >= (int)ips.InternalUnpackStreams.size())
			return false;
		InternalUnpackStream& ius = ips.InternalUnpackStreams[m_ProcessingUnpackStreamIndex];
		
		//Are we done with the previous stream?
		if (ius.UnpackStreamPtr->Size <= m_ProcessingUnpackStreamProcessed)
		{
			//Actually this may not be necessary. But let's play safe.
			CloseCurrentFile();

			//We already finished this file.
			m_ProcessingUnpackStreamIndex++;
			m_ProcessingUnpackStreamProcessed = 0;
			continue;
		}

		//Has this file been created?
		if (m_ProcessingUnpackStreamProcessed == 0)
		{
			CloseCurrentFile();				
			//It's the start. We need to create the file.
			if (!StartFile(*ius.FileInfoPtr, ius.UnpackStreamPtr->Size, m_hCurrentFile))
			{
				SetStatus(stErrored);
				return false;
			}
		}
		int FileRestSize = ius.UnpackStreamPtr->Size - m_ProcessingUnpackStreamProcessed;
		int WantedSize = std::min(size, FileRestSize);
		if (WantedSize == 0)
		{
			//TODO: This should never happen!
			assert(false);
			return false;
		}

		//If we don't want to override, then this handle could be invalid.
		if (m_hCurrentFile != INVALID_HANDLE_VALUE)
		{
			DWORD Written = 0;
			WriteFile(m_hCurrentFile, buffer, WantedSize, &Written, NULL);				
			if ((int)Written != WantedSize)
			{
				SetError(ecFailedToWriteFile);
				SetErrorObject(m_CurrentFileName.c_str());
				SetStatus(stErrored);
				return false;
			}
			m_ProcessingUnpackStreamProcessed += WantedSize;
			OnWriteFile(m_CurrentFileName.c_str(), m_CurrentFileSize, m_ProcessingUnpackStreamProcessed);
		}
		else
		{
			m_ProcessingUnpackStreamProcessed += WantedSize;
		}

		//Update CRC.
		if (ius.UnpackStreamPtr->CRCDefined)
		{
			ius.Crc.Update(buffer, WantedSize);
		}

		if  (m_ProcessingUnpackStreamProcessed >= ius.UnpackStreamPtr->Size)
		{
			//Now change file time.
			EndFile(*ius.FileInfoPtr, m_hCurrentFile, m_CurrentFileSize, m_ProcessingUnpackStreamProcessed);
			
			m_ProcessingUnpackStreamIndex ++;
			m_ProcessingUnpackStreamProcessed = 0;

			//Verify CRC.
			if (ius.UnpackStreamPtr->CRCDefined)
			{
				if (ius.Crc.GetDigest() != ius.UnpackStreamPtr->CRC)
				{
					SetError(ecUnpackedDataCRCError);
					SetErrorObject(m_CurrentFileName.c_str());
					SetStatus(stErrored);
					return false;
				}
			}
		}

		//Update buffer and size.
		buffer += WantedSize;
		size -= WantedSize;
	} // while (size > 0)
	return true;
}

void Archive::CloseCurrentFile()
{
	if (m_hCurrentFile != INVALID_HANDLE_VALUE)
	{
		OnEndExtractFile(m_CurrentFileName.c_str(), m_CurrentFileSize, m_ProcessingUnpackStreamProcessed);

		m_CurrentFileName.clear();
		m_CurrentFileSize = 0;

		CloseHandle(m_hCurrentFile);
		m_hCurrentFile = INVALID_HANDLE_VALUE;
	}
	m_CurrentFileName.clear();
	m_CurrentFileSize = 0;
}

//Override the default.
void Archive::SetStatus(Status st)
{
	if (m_Status == stProcessing && st != stProcessing)
	{
		m_Status = st;
		ReleaseBuffer();
		if (st != stDone)
		{
			//We met an error. So we need to clean up the last file that has been finished correctly.
			if (m_hCurrentFile != INVALID_HANDLE_VALUE && m_CurrentFileName.size())
			{
				//We decide not to clean up unfinished file. But in case we want it in the future.
				
				//string fileName = m_CurrentFileName;
				CloseCurrentFile();

				//DeleteFile(fileName.c_str());
			}
		}
		//Make sure this is last call. In case the notification receiver delete me in 
		// the callback.
		OnEndExtraction();
	}
	else
	{
		m_Status = st;
	}		
}

bool Archive::CreateDirectories()
{
	if (!CreateDirRecursive(m_DestDir)) 
	{
		SetError(ecFailedToCreateDirectory);
		SetErrorObject(m_DestDir.c_str());
		return false;
	}
	for(int i = 0; i < (int)m_Header.FilesInfo.size(); i++)
	{
		FileInfo& fi = m_Header.FilesInfo[i];
		if (fi.IsDirectory())
		{
			std::string FullPath = ConcatPath(m_DestDir, fi.GetAnsiName()); 
			if (!CreateDirRecursive(FullPath)) 
			{
				SetError(ecFailedToCreateDirectory);
				SetErrorObject(FullPath.c_str());
				return false;
			}
		}
	}
	return true;
}
bool Archive::SetFileTime(HANDLE h, FileInfo& fi)
{
	FILETIME Creation, Access, Write;
	Creation.dwHighDateTime = fi.CreationTime.High;
	Creation.dwLowDateTime = fi.CreationTime.Low;
	Access.dwHighDateTime = fi.LastAccessTime.High;
	Access.dwLowDateTime = fi.LastAccessTime.Low;
	Write.dwHighDateTime = fi.LastWriteTime.High;
	Write.dwLowDateTime = fi.LastWriteTime.Low;
	return ::SetFileTime(h, fi.CreationTime.Defined? &Creation : NULL, fi.LastAccessTime.Defined? &Access : NULL,
		fi.LastWriteTime.Defined? &Write: NULL) != FALSE;
}
//Always use this function to start writing a file.
bool Archive::StartFile(FileInfo& fi, int size, HANDLE& hFile)
{
	m_CurrentFileName = ConcatPath(m_DestDir, fi.GetAnsiName());
	m_CurrentFileSize = size;

	int ret = OnStartExtractFile(m_CurrentFileName.c_str(), m_CurrentFileSize);
	if (ret == 1)
	{
		//Skip.
		hFile = INVALID_HANDLE_VALUE;
		return true;
	}

	//Sometimes the 7zip will not encode directory information. like using the *.* to compress a folder.
	std::string  Dir = GetParentDir(m_CurrentFileName);
	if (!CreateDirRecursive(Dir))
	{
		SetError(ecFailedToCreateDirectory);
		SetErrorObject(Dir.c_str());
	}

	hFile = ::CreateFile(m_CurrentFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, 
		(m_Override? CREATE_ALWAYS : CREATE_NEW), fi.WinAttribute, NULL);
	if (hFile == INVALID_HANDLE_VALUE)	
	{
		if (m_Override)
		{
			SetError(ecFailedToCreateFile);
			SetErrorObject(m_CurrentFileName.c_str());
			return false;
		}
		else
		{
			//File should already exist. Don't bother.
			return true;
		}
	}
	else
	{
		return true;
	}
}
bool Archive::EndFile(FileInfo& fi, HANDLE& hFile, int FileSize, int BytesWritten)
{
	if (hFile != INVALID_HANDLE_VALUE)
	{
		::SetEndOfFile(hFile);
		SetFileTime(hFile, fi);
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;

		OnEndExtractFile(m_CurrentFileName.c_str(), FileSize, BytesWritten);

		m_CurrentFileName.clear();
		m_CurrentFileSize = 0;
		m_FilesExtracted ++;
	}
	return true;
}

bool Archive::CreateEmptyFiles()
{
	for(int i = 0; i < (int)m_Header.FilesInfo.size(); i++)
	{
		FileInfo& fi = m_Header.FilesInfo[i];
		if (fi.EmptyFile)
		{
			HANDLE hFile;
			if (!StartFile(fi, 0, hFile)) return false;
			if (hFile != INVALID_HANDLE_VALUE)	
			{
				EndFile(fi, hFile, 0, 0);
			}
		}
	}
	return true;
}

//generate mapping between packstream->Unpackstreams.
bool Archive::CreateInternalMapping()
{
	int i, j, k;
	int PackStreamIndex = 0;
	int FileInfoIndex = 0;
	int LastStartPos = sizeof(SignatureHeader) + m_Header.MainStreamsInfo.PackPos;
	m_InternalPackStreams.resize(m_Header.MainStreamsInfo.PackedStreams.size());

	for(i = 0; i < (int)m_Header.MainStreamsInfo.Folders.size(); i++)
	{
		Folder& folder = m_Header.MainStreamsInfo.Folders[i];
		int UnpackStreamIndex = 0;
		for(j = 0; j < (int)folder.Coders.size(); j++)
		{
			Coder& coder = folder.Coders[j];
			if (j == 1)
			{
				//Only BCJ can be there.
				if (!coder.IsBCJ())
				{
					SetError(ecInvalidHeader);
					SetErrorObject(m_ArchiveFileName.c_str());
					return false;
				}
				if (PackStreamIndex - 1 >= 0 && PackStreamIndex - 1 < (int)m_InternalPackStreams.size())
				{
					InternalPackStream& ips = m_InternalPackStreams[PackStreamIndex-1];
					ips.BcjEncoded = true;
				}
				continue;
			}
			else if (j > 1)
			{
				//Don't support more than 2 coders.
				SetError(ecInvalidHeader);
				return false;
			}
			for(k = 0; k < coder.NumInStreams; k++)
			{
				if (PackStreamIndex >= (int)m_InternalPackStreams.size())
				{
					SetError(ecInvalidHeader);
					return false;
				}
				InternalPackStream& ips = m_InternalPackStreams[PackStreamIndex];
				ips.CoderPtr = &coder;
				ips.PackStreamPtr = &m_Header.MainStreamsInfo.PackedStreams[PackStreamIndex];
				ips.StartPos = LastStartPos;
				ips.TotalUnpackSize = coder.GetUnpackSize();
				PackStreamIndex++;
				
				LastStartPos += ips.PackStreamPtr->PackSize;

				//Now get substreams (file) info.
				if (k >= (int)coder.UnPackSizes.size())
				{
					SetError(ecInvalidHeader);
					return false;
				}
                int UnpackSize = coder.UnPackSizes[k];
				while(UnpackSize > 0)
				{
					//Add one unpack stream.
					InternalUnpackStream ius;
					if (UnpackStreamIndex >= (int)folder.UnpackStreams.size())
					{
						SetError(ecInvalidHeader);
						return false;
					}
					ius.UnpackStreamPtr = &folder.UnpackStreams[UnpackStreamIndex];
					UnpackStreamIndex ++;
					UnpackSize -= ius.UnpackStreamPtr->Size;

					//Skip empty file/dir s.
					for(; m_Header.FilesInfo[FileInfoIndex].EmptyStream && FileInfoIndex < (int)m_Header.FilesInfo.size(); FileInfoIndex++)
					{
					}
					if (FileInfoIndex >= (int)m_Header.FilesInfo.size())
					{
						SetError(ecInvalidHeader);
						return false;
					}
					ius.FileInfoPtr = &m_Header.FilesInfo[FileInfoIndex];
					FileInfoIndex++;

					ips.InternalUnpackStreams.push_back(ius);
				}
				//It should be exactly 0!
				if (UnpackSize != 0)
				{
					SetError(ecInvalidHeader);
					return false;
				}
			}
		}
	}
	if (PackStreamIndex != (int)m_Header.MainStreamsInfo.PackedStreams.size())
	{
		SetError(ecInvalidHeader);
		return false;
	}
	return true;
}

//Override LzmaDecoder.
int Archive::OnRead(unsigned char **buffer, UInt32 *bufferSize)
{
	//Always need to set the buffer pointer!! Otherwise the LZMA decoder will break.
	*buffer = (unsigned char*)&m_InputBuffer[0];
	*bufferSize = 0;

	if (m_ProcessingPackStreamIndex >= (int)m_InternalPackStreams.size()) return 1;
	InternalPackStream& ips = m_InternalPackStreams[m_ProcessingPackStreamIndex];
	int StreamRest = ips.PackStreamPtr->PackSize - m_ProcessingPackStreamProcessed;
	if (StreamRest <= 0) return 1;
	
	int Wanted = std::min(StreamRest, (int)m_InputBuffer.size());

	if (m_DecodeStream == NULL) return 1;
	try
	{
		m_DecodeStream->Read(&m_InputBuffer[0], Wanted);
	}
	catch(...)
	{			
		SetError(ecFailedToReadArchive);
		SetErrorObject(m_ArchiveFileName.c_str());
		return 1;
	}
	
	*bufferSize = Wanted;

	m_ProcessingPackStreamProcessed += Wanted;
	
	//CRC.
	if (ips.PackStreamPtr->CRCDefined)
	{
		ips.Crc.Update(&m_InputBuffer[0], Wanted);
		if (m_ProcessingPackStreamProcessed >= ips.PackStreamPtr->PackSize)
		{
			//Verify Crc.
			if (ips.Crc.GetDigest() != ips.PackStreamPtr->CRC)
			{
				SetError(ecPackedDataCRCError);
				SetErrorObject(m_ArchiveFileName.c_str());
				return 1;
			}
		}
	}
	return 0;
}

}
}
}
}
