/***************************************************************************************
	Archive7zInternal.h

	Copyright (C) 2002-2005 WildTangent, Inc. 
	All Rights Reserved 

	Description:  7z decoding stuff. 

	History:   
		01/21/2005	Rui Jiang	Created
		09/28/2005  Rui Jiang	Renamed from wt7z.h to Archive7zInternal.h, and 
				moved into the WTPlatform.
***************************************************************************************/

#pragma once

//Temporary disable it.
#pragma warning(disable:4244)

#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <vector>
#include <string>

extern "C"
{
#include "LzmaDecode.h"
}

namespace WT
{
namespace System
{
namespace Compression
{
namespace Archive7zInternal
{

#define DEBUG_INDENT "  "
typedef unsigned char		UI8;
typedef unsigned short		UI16;
typedef unsigned int		UI32;
typedef unsigned __int64	UI64;
typedef std::vector<UI8>	ByteVector;

//Copied from 7zHeader.h
enum Type7z
{
	kEnd,

	kHeader,

	kArchiveProperties,

	kAdditionalStreamsInfo,
	kMainStreamsInfo,
	kFilesInfo,

	kPackInfo,
	kUnPackInfo,
	kSubStreamsInfo,

	kSize,
	kCRC,

	kFolder,

	kCodersUnPackSize,
	kNumUnPackStream,

	kEmptyStream,
	kEmptyFile,
	kAnti,

	kName,
	kCreationTime,
	kLastAccessTime,
	kLastWriteTime,
	kWinAttributes,
	kComment,

	kEncodedHeader,

	kStartPos
};

///////////////////////////////////////////////////////////////////////////////
// CRC verification. Copied from 7zip project.
class CCRC
{
	UI32 _value;
public:
	static UI32 Table[256];
	static bool g_TableInited;
	static const UI32 kCRCPoly = 0xEDB88320;
	//Init CRC table if it isn't inited.
	static void InitTable();

	CCRC():  _value(0xFFFFFFFF){};
	void Init() { _value = 0xFFFFFFFF; }
	void UpdateByte(UI8 b)
	{  
		_value = Table[((UI8)(_value)) ^ b] ^ (_value >> 8);
	}
	void UpdateUInt16(UI16 v)
	{
		UpdateByte(UI8(v));
		UpdateByte(UI8(v >> 8));
	}
	void UpdateUInt32(UI32 v)
	{
		for (int i = 0; i < 4; i++)
			UpdateByte((UI8)(v >> (8 * i)));
	}

	void UpdateUInt64(const UI64 &v)
	{
		for (int i = 0; i < 8; i++)
			UpdateByte((UI8)(v >> (8 * i)));
	}
	void Update(const void *data, UI32 size)
	{
		UI32 v = _value;
		const UI8 *p = (const UI8 *)data;
		for (; size > 0 ; size--, p++)
			v = Table[((UI8)(v)) ^ *p] ^ (v >> 8);
		_value = v;
	}

	UI32 GetDigest() const { return _value ^ 0xFFFFFFFF; } 
	static UI32 CalculateDigest(const void *data, UI32 size)
	{
		CCRC crc;
		crc.Update(data, size);
		return crc.GetDigest();
	}
	static bool VerifyDigest(UI32 digest, const void *data, UI32 size)
	{
		return (CalculateDigest(data, size) == digest);
	}
};

//Print simple number type.
template < class T> void fprintfvector(FILE* f, const std::vector<T>& v)
{
	fprintf(f, "(size=%d: ", v.size());
	for(int i = 0; i < (int)v.size(); i++)
	{
		if (i > 0)
			fprintf(f, ",");
		fprintf(f, "%d", v[i]);		
	}
	fprintf(f, ")");
};

//T is usually a std::string, or std::string.
template<class T> T ConcatPath(const T& ParentDir, const T& FileName)
{
	T res = ParentDir;
	if (res.size() > 0 && res[res.size() - 1] != '\\' && res[res.size() - 1] != '/')
		res += "\\";
	res += FileName;
	return res;
}

//Extract the parent dir name.
template<class _String> _String GetParentDir(const _String& s)
{
	int i;
	for(i = (int)s.size() - 1; i >= 0; i--)
	{
		int c = s[i];
		if (c == '/' || c == '\\')
		{
			//in case \\computer1
			if (i > 0 && (s[i-1] == '/' || s[i-1] =='\\'))
				i--;
			return s.substr(0, i);
		}
	}

	return "";
};

//Determine if the specified name is a dir or file.
inline bool IsDirExist(const std::string & name)
{
      WIN32_FIND_DATA data;
      if (name.size() == 0) return false;
	  std::string s = ConcatPath(name, std::string(("*.*")));
      //Determine whether the source is a directory or file.
      HANDLE hFileFind = FindFirstFile(s.c_str(), &data);
      if (hFileFind == INVALID_HANDLE_VALUE)
      {
            return false;
      }

      FindClose(hFileFind);
      //return (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
      return true;
}

//Recursively create directory.
template<class _String> bool CreateDirRecursive(const _String& name)
{
      if (name.size() == 0) return true;
      if (IsDirExist(name)) return true;

      _String parentDir = GetParentDir(name);
      if (parentDir.size() == name.size())
      {
            return CreateDirectory(name.c_str(), NULL) != FALSE;
      }
      else
      {
            if (!CreateDirRecursive(parentDir))
            {
                  return false;
            }
            else
            {
                  return CreateDirectory(name.c_str(), NULL) != FALSE;
            }
      }
      return true;
}

///////////////////////////////////////////////////////////////////////////////
// Simple stream handling for my needs.

//All these operations should just throw an exception when fails.
class IStream
{
public:
	virtual ~IStream() {}

	virtual void Read(void *buffer, int size) = 0;
	virtual void Write(const void* buffer, int size) = 0;
	virtual void Seek(int offset, int origin) = 0;
	virtual int Tell() = 0;
	virtual int GetLength() = 0;
	virtual void SeekSet(int offset) { return Seek(offset, SEEK_SET); }
	virtual void SeekCur(int offset) { return Seek(offset, SEEK_CUR); }
	
	UI8	ReadByte()
	{
		UI8 b;
		Read(&b, sizeof(b));
		return b;
	}
	UI32 ReadUI32()
	{
		UI32 value = 0;
		for (int i = 0; i < 4; i++)
		{
			value |= (UI32(ReadByte()) << (8 * i));
		}
		return value;
	}
	//This is specific for 7z. It's a variable length number. See doc of 7z.
	UI64 Read7zNumber()
	{
		UI8 firstByte = ReadByte();
		UI8 mask = 0x80;
		UI64 value = 0;
		for (int i = 0; i < 8; i++)
		{
			if ((firstByte & mask) == 0)
			{
				UI64 highPart = firstByte & (mask - 1);
				value += (highPart << (i * 8));
				return value;
			}
			UI8 b = ReadByte();
			value |= (UI64(b) << (8 * i));
			mask >>= 1;
		}
		return value;
	}
	//Actually it must an unsigned. But I like signed int. How much time was wasted just
	//debugging error like (v.size() - 1) when v.size() == 0?
	int Read7zInt()
	{
		int res = (int)Read7zNumber();
		if (res < 0)
		{
			throw "Number overflowed";
		}
		return res;
	}
	//Although type seems like a BYTE, The 7z program read it using the long format. So play safe.
	Type7z ReadType()
	{
		return (Type7z)Read7zNumber();
	}
	//Ignore all numbers until we see the typeid.
	void ReadUntilType(Type7z t)
	{
		while(ReadType() != t)
		{
		}
	}
	void ReadBoolVector(int numItems, std::vector<bool> &v)
	{
		v.clear();
		v.reserve(numItems);
		UI8 b = 0;
		UI8 mask = 0;
		for(int i = 0; i < numItems; i++)
		{
			if (mask == 0)
			{
				b = ReadByte();
				mask = 0x80;
			}
			v.push_back((b & mask) != 0);
			mask >>= 1;
		}
	}
	//A little bit different. Read in the first byte, if it is not 0, then all are true.
	void ReadBoolVector2(int numItems, std::vector<bool> &v)
	{
		UI8 b = ReadByte();
		if (b)
		{
			v.clear();
			v.resize(numItems, true);
		}
		else
		{
			ReadBoolVector(numItems, v);
		}
	}
	void ReadHashDigests(int numItems, std::vector<bool> &v, std::vector<UI32>& digests)
	{
		UI8 allAreDefined = ReadByte();
		if (allAreDefined == 0)
		{
			ReadBoolVector(numItems, v);
		}
		else
		{
			v.clear();
			v.resize(numItems, true);
		}
		digests.clear();
		digests.resize(numItems, 0);
		for(int i = 0; i < numItems; i++)
		{
			if (v[i])
			{
				digests[i] = ReadUI32();
			}
		}
	}
	void SkipData(int size)
	{
		if (size > 0)
		{
			UI8* buf = new UI8[size];
			try
			{
				Read(buf, size);
			}
			catch(...)
			{
				delete [] buf;
				buf = NULL;
				throw;
			}
			delete [] buf;
		}
	}
	wchar_t ReadWideChar()
	{
		UI8 b1 = ReadByte();
		UI8 b2 = ReadByte();
		return (((unsigned short)b2) << 8) + b1;
	}
	//This comes as one byte. If not 0, mean to switch another data stream.
	//We don't support it now.
	void ReadStreamSwitch()
	{
		UI8 external = ReadByte();
		if (external)
		{
			throw "Stream switch not supported in file time.";
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// File stream handling for my needs.

class FileStream : public IStream
{
public:
	FileStream()
	{
		m_File = NULL;
	}
	~FileStream()
	{
		Close();
	}
	virtual bool Open(const char* filename, const char* access)
	{
		Close();
		m_File = fopen(filename, access);
		return m_File != NULL;
	}
	virtual void Close()
	{
		if (m_File)
		{
			fclose(m_File);
			m_File = NULL;
		}
	}

	//////////////  IStream stuff ///////////////////////
	virtual void Read(void *buffer, int size)
	{
		CheckFile();
		int res = (int)fread(buffer, 1, size, m_File);
		if (res != size)
		{
			throw "Read in size is different than specified";
		}
	}
	virtual void Write(const void* buffer, int size)
	{		
		CheckFile();
		int res = (int)fwrite(buffer, 1, size, m_File);
		if (res != size)
		{
			throw "Write size is different than specified";
		}
	}
	virtual void Seek(int offset, int origin)
	{
		CheckFile();
		int res = fseek(m_File, offset, origin);
		if (res != 0)
		{
			throw "Seek failed";
		}
	}
	virtual int Tell()
	{
		CheckFile();
		return ftell(m_File);
	}
	virtual int GetLength()
	{
		CheckFile();
		int pos = Tell();
		Seek(0, SEEK_END);
		int res = Tell();
		Seek(pos, SEEK_SET);
		return res;
	}
protected:
	FILE* m_File;

	void CheckFile()
	{
		assert(m_File);
		if (!m_File)
		{
			throw "File is not opened";
		}
	}
};

class MemStream : public IStream
{
public:
	MemStream(int size = 0):
		m_Buffer(size)
	{
		m_CurPos = 0;
	}
	~MemStream()
	{
	}
	virtual void Resize(int size)
	{
		m_Buffer.resize(size);
	}
	//Read from another stream to create myself.
	virtual void Create(IStream* is, int offset, int size)
	{
		is->SeekSet(offset);
		Create(is, size);
	}
	virtual void Create(IStream* is, int size)
	{
		Resize(size);
		is->Read(&(m_Buffer[0]), size);
	}
	virtual const UI8* GetBuffer() { return &(m_Buffer[0]); }

	//////////////  IStream stuff ///////////////////////
	virtual void Read(void *buffer, int size)
	{
		int res = __min((int)m_Buffer.size() - m_CurPos, size);
		if (res != size)
		{
			throw "Read in size is different than specified";
		}
		memcpy(buffer, &(m_Buffer[0]) + m_CurPos, res);
		m_CurPos += res;
	}
	virtual void Write(const void* buffer, int size)
	{		
		int res = __min((int)m_Buffer.size() - m_CurPos, size);
		if (res != size)
		{
			throw "Read in size is different than specified";
		}
		memcpy(&(m_Buffer[0]) + m_CurPos, buffer, res);
		m_CurPos += res;
	}
	virtual void Seek(int offset, int origin)
	{
		int Pos;
		switch(origin)
		{
		case SEEK_SET:
			Pos = offset; 
			break;
		case SEEK_END:
			Pos = (int)m_Buffer.size() + offset;
			break;
		case SEEK_CUR:
			Pos = m_CurPos + offset;
			break;
		default:
			throw "Error origin";
			break;
		}
		if (Pos < 0 || Pos > (int)m_Buffer.size())
			throw "Seek error";
		m_CurPos = Pos;
	}
	virtual int Tell()
	{
		return m_CurPos;
	}
	virtual int GetLength()
	{
		return (int)m_Buffer.size();
	}
	
protected:
	ByteVector	m_Buffer;
	int			m_CurPos;
};

//The original LZMADecode.h APIs are difficult to use. Wrap them here.
//Be carefull APIs of this don't throw exceptions!!!!!! (to conform with LZMA SDK).
//So check return result!!!!
class LzmaDecoder
{
public:
	LzmaDecoder()
	{
		Clear();
	}
	~LzmaDecoder()
	{
		Clear();
	}
	void Clear()
	{
		m_LzmaInternalData.clear();
		m_Dictionary.clear();
	}
	void ReleaseBuffer()
	{
		m_LzmaInternalData.clear();
		m_Dictionary.clear();
	}
	//Properties must have size of 5.
	//In LzmaDecoderInit, it will read first 5 bytes of the stream. So make sure we have
	//Stream ready before calling init.
	bool Init(const unsigned char* Properties, int PropertySize, int TOutputSize);
	//Process one piece of output.
	bool ProcessOneBlock(unsigned char* Output, int OutSize, int* OutSizeProcessed);
protected:
	//This is what lzma decoder needed for its internal use.
	std::vector<unsigned char> m_LzmaInternalData;
	std::vector<unsigned char> m_Dictionary;

	//I have to wrap around. Because the callback is working differently. The first (void*) object 
	//param in the "Read" call back is the pointer we passed to LzmaDecoderInit. Be careful, it
	// doesn't have any type information. So use the simplest struct as possible here so we can
	// safely cast it!!!
	struct MyCallBack : public ILzmaInCallback
	{
		LzmaDecoder*	m_Self;
	};
	MyCallBack	ReadCallBack;
	static int ReadCallBackFunc(void *object, unsigned char **buffer, UInt32 *bufferSize)
	{
		//Here, the object is a pointer to MyCallBack.
		MyCallBack* mcb = (MyCallBack*)object;
		assert(mcb->m_Self);
		return mcb->m_Self->OnRead(buffer, bufferSize);
	}

	//Will be called by LzmaDecode.
	virtual int OnRead(unsigned char **buffer, UInt32 *bufferSize) = 0;
};

//BCJ Filter.
class CBCJ_x86_Decoder
{ 
protected:
	UI32 _bufferPos;
	UI32 _prevMask;
	UI32 _prevPos;

public: 
	CBCJ_x86_Decoder()
	{
		SubInit();
	}
	UI32 SubFilter(UI8 *data, UI32 size)
	{
		UI32 res = x86_Convert(data, size, _bufferPos, &_prevMask, &_prevPos, 0);
		_bufferPos += res;
		//_prevPos -= res;
		return res;
	}
	void SubInit() 
	{ 
		_bufferPos = 0;
		_prevMask = 0;
		_prevPos = (UI32)(-5);
	}
	static int Test86MSByte(UI8 b)
	{
		return (b == 0 || b == 0xFF);
	}
	static UI32 x86_Convert(UI8 *buffer, UI32 endPos, UI32 nowPos, 
		UI32 *prevMask, UI32 *prevPos, int encoding);
};

//This one decode from and to memory.
class LzmaDecoderMem : protected LzmaDecoder
{
public:
	//This single function is to decode everything in memory. 
	bool DecodeMemory(const unsigned char* properties, int PropertySize, const unsigned char* InputBuffer, int InputSize, unsigned char* OutBuffer, int* OutSize)
	{
		//The LzmaDecoderInit will call OnRead. So setting these must happen before Init!!!
		assert(InputBuffer);		
		m_InputBuffer = InputBuffer;
		m_InputSize = InputSize;
		m_Pos = 0;

		if (!Init(properties, PropertySize, *OutSize)) return false;
		
		if (!ProcessOneBlock(OutBuffer, *OutSize, OutSize)) return false;
		return true;
	}
protected:
	const unsigned char* m_InputBuffer;
	int	m_InputSize;
	int m_Pos;

	//Will be called by LzmaDecode.
	virtual int OnRead(unsigned char **buffer, UInt32 *bufferSize)
	{
		int RestSize = m_InputSize - m_Pos;
		
		*buffer = (unsigned char*)(m_InputBuffer + m_Pos);
		*bufferSize = RestSize;

		m_Pos += RestSize;
		return 0;
	}
};

//Read input from an IStream.
class LzmaDecoderStream : protected LzmaDecoder
{
public:
};

/* Single function to decode.
inline bool LzmaDecodeMemory(const unsigned char* properties, const unsigned char* InputBuffer, int InputSize, unsigned char* OutBuffer, int* OutSize)
{
	int lc, lp, pb;
	int lzmaInternalSize;
	void *lzmaInternalData;
	char prop0;

	prop0 = properties[0];
	if (prop0 >= (9*5*5))
	{
		return false;
	}
	for (pb = 0; prop0 >= (9 * 5); pb++, prop0 -= (9 * 5));
	for (lp = 0; prop0 >= 9; lp++, prop0 -= 9);
	lc = prop0;

	lzmaInternalSize = 
		(LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (lc + lp)))* sizeof(CProb);

	lzmaInternalData = malloc(lzmaInternalSize);
	if (lzmaInternalData == 0)
	{
		return false;
	}

	unsigned int outSizeProcessed;
	int res = LzmaDecode((unsigned char *)lzmaInternalData, lzmaInternalSize,
		lc, lp, pb,
		(unsigned char *)InputBuffer, InputSize,
		(unsigned char *)OutBuffer, *OutSize, &outSizeProcessed);
	*OutSize = outSizeProcessed;

	free(lzmaInternalData);

	return (res == 0);
}
*/

#pragma pack(push, 1)
struct SignatureHeader
{
	//Must be '7z...'
	UI8	Signature[6];
	UI8 VerMajor;
	UI8 VerMinor;
	UI32	StartHeaderCRC;
	//Be careful of the encoding
	UI64	NextHeaderOffset;
	UI64	NextHeaderSize;
	UI32	NextHeaderCRC;
};
#pragma pack(pop)

//Packed input streams.
struct PackStream
{
	PackStream(int ps = 0, bool crcd = false, UI32 crc = 0)
	{
		PackSize = ps;
		CRC = crc;
		CRCDefined = crcd;
	}
	int		PackSize;
	bool	CRCDefined;
	UI32	CRC;

	void Debug(FILE* f, const std::string& indent)
	{
		fprintf(f, "%sPackSize: %d\n", indent.c_str(), PackSize);
		fprintf(f, "%sCRCDefiend: %d\n", indent.c_str(), CRCDefined);
		fprintf(f, "%sCRC: %d\n", indent.c_str(), CRC);
	}
};


struct Coder
{
	Coder()
	{
		MainByte = 0;
		NumInStreams = NumOutStreams = 0;
	}
	UI8 MainByte;
	std::vector<UI8>	MethodId;
	int	NumInStreams;
	int NumOutStreams;
	std::vector<UI8>	Properties;
	//Out put streams' unpacksize.
	std::vector<int>	UnPackSizes;

	void Debug(FILE* f, const std::string& indent)
	{
		//int i;
		fprintf(f, "%sMainByte: %d\n", indent.c_str(), MainByte);

		fprintf(f, "%sMethodId: ", indent.c_str());
		fprintfvector(f, MethodId);
		fprintf(f, "\n");

		fprintf(f, "%sNumInStreams: %d\n", indent.c_str(), NumInStreams);
		fprintf(f, "%sNumOutStreams: %d\n", indent.c_str(), NumOutStreams);

		fprintf(f, "%sProperties: ", indent.c_str());
		fprintfvector(f, Properties);
		fprintf(f, "\n");

		fprintf(f, "%sUnPackSizes: ", indent.c_str());
		fprintfvector(f, UnPackSizes);
		fprintf(f, "\n");
	}
	void Parse(IStream* is)
	{
		UI8 byte = is->ReadByte();

		//Method ID
		int MethodIdSize = byte & 0xF;
		MethodId.resize(MethodIdSize);
		is->Read(&MethodId[0], MethodIdSize);

		if (byte & 0x10)
		{
			//Not simple.
			NumInStreams = is->Read7zInt();
			NumOutStreams = is->Read7zInt();
		}
		else
		{
			//Simple
			NumInStreams = 1;
			NumOutStreams = 1;
		}

		//Properties.
		if (byte & 0x20)
		{
			int PropertySize = is->Read7zInt();
			Properties.resize(PropertySize);
			is->Read(&Properties[0], PropertySize);
		}
	}
	int GetUnpackSize()
	{
		int total = 0;
		for(int i = 0; i < (int)UnPackSizes.size(); i++)
		{
			total += UnPackSizes[i];
		}
		return total;
	}
	bool IsLZMA()
	{
		return MethodId.size() == 3 && MethodId[0] == 3 && MethodId[1] == 1 && MethodId[2] == 1;
	}
	bool IsBCJ()
	{
		return MethodId.size() == 4 && MethodId[0] == 3 && MethodId[1] == 3 && MethodId[2] == 1 && MethodId[3] == 3;
	}
	//Must have 5 bytes properites.
	bool IsValidLZMA()
	{
		return IsLZMA() && (Properties.size() == 5);
	}
};

struct BindPair
{
	BindPair()
	{
		InIndex = OutIndex = 0;
	}
	int InIndex, OutIndex;
	
	void Debug(FILE* f, const std::string& indent)
	{
		fprintf(f, "%sInIndex: %d   OutIndex: %d\n", indent.c_str(), InIndex, OutIndex);
	}
};

struct UnpackStream
{
	UnpackStream(int sz = 0, bool crcd = false, UI32 crc = 0)
	{
		Size = sz;
		CRC = crc;
		CRCDefined = crcd;
	}
	int		Size;
	bool	CRCDefined;
	UI32	CRC;

	void Debug(FILE* f, const std::string& indent)
	{
		//int i;
		fprintf(f, "%sSize: %d\n", indent.c_str(), Size);
		fprintf(f, "%sCRCDefined: %d\n", indent.c_str(), CRCDefined);
		fprintf(f, "%sCRC: %d\n", indent.c_str(), CRC);
	}
};

//One encoding folder
struct Folder
{
	Folder()
	{
		UnpackCRCDefined = false;
		UnpackCRC = 0;
		NumUnpackStreams = 0;
	}
	std::vector<Coder>	Coders;
	std::vector<BindPair>	BindPairs;
	//Indexes of packed streams.
	std::vector<int>	PackedStreamIndexes;
	bool	UnpackCRCDefined;
	UI32	UnpackCRC;

	//This are equal size.
	int		NumUnpackStreams;
	std::vector<UnpackStream>	UnpackStreams;

	void Debug(FILE* f, const std::string& indent)
	{
		int i;
		fprintf(f, "%sCoders: %d\n", indent.c_str(), Coders.size());
		for(i = 0; i < (int)Coders.size(); i++)
		{
			fprintf(f, "%sCoder %d:\n", (indent+DEBUG_INDENT).c_str(), i);
			Coders[i].Debug(f, indent + DEBUG_INDENT + DEBUG_INDENT);
		}

		fprintf(f, "%sBindPairs: %d\n", indent.c_str(), BindPairs.size());
		for(i = 0; i < (int)BindPairs.size(); i++)
		{
			fprintf(f, "%sBindPair %d:\n", (indent+DEBUG_INDENT).c_str(), i);
			BindPairs[i].Debug(f, indent + DEBUG_INDENT + DEBUG_INDENT);
		}

		fprintf(f, "%sNumUnpackStreams: %d\n", indent.c_str(), UnpackStreams.size());
		for(i = 0; i < (int)UnpackStreams.size(); i++)
		{
			fprintf(f, "%sUnpackStream %d:\n", (indent+DEBUG_INDENT).c_str(), i);
			UnpackStreams[i].Debug(f, indent + DEBUG_INDENT+ DEBUG_INDENT);
		}
	}
	//Will throw an exception if anything is wrong.
	void Parse(IStream* is)
	{
		//Coders
		int NumCoders = is->Read7zInt();
		Coders.resize(NumCoders);
		int NumInStreamsTotal = 0;
		int NumOutStreamsTotal = 0;
		for(int i = 0; i < NumCoders; i++)
		{
			Coders[i].Parse(is);
			NumInStreamsTotal += Coders[i].NumInStreams;
			NumOutStreamsTotal += Coders[i].NumOutStreams;
			//Note: this doesn't seem to be right. Continue.
			//Trick here. See the source code. If it is 0, means last method. should break.
			//if ((Coders[i].MainByte & 0x80) == 0) break;
		}

		//Bind Pairs
		int NumBindPairs = NumOutStreamsTotal - 1;
		if (NumBindPairs > 0)
		{
			//We have to support them now.
			//TODO: let's not support bind pairs.
			//throw "Binded streams are not supported.";
		}
		BindPairs.resize(NumBindPairs);
		for(int i = 0 ; i < NumBindPairs; i++)
		{
			BindPairs[i].InIndex = is->Read7zInt();
			BindPairs[i].OutIndex = is->Read7zInt();
		}

		int NumPackedStreams = NumInStreamsTotal - NumBindPairs;
		if (NumPackedStreams == 1)
		{
			for(int i = 0; i < NumInStreamsTotal; i++)
			{
				//It's not binded to another coder?
				if (FindBindPairForInStream(i) < 0)
					PackedStreamIndexes.push_back(i);
			}
		}
		else
		{
			//More than 1 packed streams. read in index.
			for(int i = 0; i < NumPackedStreams; i++)
				PackedStreamIndexes.push_back(is->Read7zInt());
		}

	}

	void ParseUnpackSizes(IStream* is)
	{
		for(int i = 0; i < (int)Coders.size(); i++)
		{
			for(int j = 0; j < Coders[i].NumOutStreams; j++)
			{
				Coders[i].UnPackSizes.push_back(is->Read7zInt());
			}
		}
	}

	int FindBindPairForInStream(int StreamIndex)
	{
		for(int i = 0; i < (int)BindPairs.size(); i++)
			if (BindPairs[i].InIndex == StreamIndex) return i;
		return -1;
	}
	int GetNumOutStreams()
	{
		int total = 0;
		for(int i = 0; i < (int)Coders.size(); i++)
		{
			total += Coders[i].NumOutStreams;
		}
		return total;
	}
	//This is for coder's unpackSize.
	int GetUnpackSize()
	{
		int total = 0;
		for(int i = 0; i < (int)Coders.size(); i++)
		{
			total += Coders[i].GetUnpackSize();
			//TODO: Now let's just think one coder only.
			break;
		}
		return total;
	}
	//This is for UnpackStream ' total size.
	int GetUnpackStreamSize()
	{
		int total = 0;
		for(int i = 0; i < (int)UnpackStreams.size(); i++)
		{
			total += UnpackStreams[i].Size;
		}
		return total;
	}
};

class StreamsInfo
{
public:
	//Start position of subsequent streams. 
	int					PackPos;
	std::vector<PackStream>	PackedStreams;
	std::vector<Folder>		Folders;

	void Debug(FILE* f, const std::string& indent);
	void Clear();
	//Will throw an exception if anything is wrong.
	void Parse(IStream* is);
	//Decode from the stream to a Memory Stream. Folders will init Memory stream.
	//Caller should seek IStream to the correct position.
	void DecodeInMemory(IStream* is, MemStream& ms);
	//All folders' unpack size.
	int GetUnpackSize();
	//All folders UnpackStream ' total size.
	int GetUnpackStreamSize();
protected:
	void ParsePackInfo(IStream* is);
	void ParseCodersInfo(IStream* is);
	void ParseCRCDigests(IStream* is);
	void ParseSubStreamsInfo(IStream* is);
};

struct FileDateTime
{
	FileDateTime()
	{
		Defined = false;
		Low = 0;
		High = 0;
	}
	bool	Defined;
	UI32	Low, High;
};

struct FileInfo
{
	FileInfo()
	{
		EmptyStream = false;
		EmptyFile = false;
		WinAttribute = 0;
	}
	//If EmptyStream is false, then it must be a file.
	bool		EmptyStream;
	//If EmptyStream is true and EmptyFile is true, then this is a file with 0 size.
	bool		EmptyFile;
	std::wstring		Name;
	UI32		WinAttribute;
	FileDateTime	CreationTime, LastWriteTime, LastAccessTime;
	bool		IsDirectory()
	{
		return EmptyStream && !EmptyFile;
	}
	//We need to do the conversion.
	std::string	GetAnsiName()
	{
		char* buf;
		int size = WideCharToMultiByte(CP_ACP, 0, Name.c_str(), -1, NULL, 0, NULL, NULL);
		if (size <= 0)
			return "";
		buf = new char[size];
		
		int ret = WideCharToMultiByte(CP_ACP, 0, Name.c_str(), (int)Name.size(), buf, size, NULL, NULL);
		ret;
		assert(ret);
		
		buf[size-1] = 0;
		std::string res = buf;
		delete [] buf;
		
		//Change '/' to '\';
		for(int i = 0; i < (int)res.size(); i++)
			if (res[i] == '/') res[i] = '\\';
		return res;
	}
};

struct Header
{
	StreamsInfo	MainStreamsInfo;
	std::vector<FileInfo>	FilesInfo;
	
	void Debug(FILE* f, const std::string& indent);
	void Parse(IStream* is);
	void Clear();
protected:
	//The current 7z doing nothing with archive properties. I am following him.
	void ParseArchiveProperties(IStream* is);
	void ParseFilesInfo(IStream* is);
	void ParseFileNames(IStream* is);
	void ParseFileTimes(IStream* is, Type7z type);
	void ParseWinAttributes(IStream* is);
};

//This is for internal use only. It's a composition of things in UnpackInfo and FileInfo.
struct InternalUnpackStream
{
	InternalUnpackStream()
	{
		UnpackStreamPtr = NULL;
		FileInfoPtr = NULL;
	}
	UnpackStream*	UnpackStreamPtr;
	FileInfo*		FileInfoPtr;
	//This will updated while writing the output, if we have CRCDefined.
	CCRC			Crc;
};

//This is for internal use only. It's a composition of things in StreamsInfo, Folder, etc. 
struct InternalPackStream
{
	InternalPackStream()
	{
		StartPos = 0;
		PackStreamPtr = NULL;
		CoderPtr = NULL;
		BcjEncoded = false;
		TotalUnpackSize = 0;
		TotalUnpackedBytes = 0;
	}
	//Start position of the file. 
	int StartPos;
	//Point to PackStream, so we know Size and CRC.
	PackStream*	PackStreamPtr;
	//Point to a single Coder in Folder, so we know the stream's property.
	Coder*		CoderPtr;
	//Whether we need to use BCJ FILTER.
	bool		BcjEncoded;
	//The unpack streams this stream is going to generate.
	std::vector<InternalUnpackStream>	InternalUnpackStreams;
	//Total unpack stream size.
	int	TotalUnpackSize;

	//This will updated while reading from the stream, if we have CRCDefined.
	CCRC			Crc;	
	//Updated during processing.
	int		TotalUnpackedBytes;
};

class ArchiveBase
{
public:
	ArchiveBase()
	{
		ClearStatus();
	}
	void ClearStatus()
	{
		m_Status = stUninited;
		m_LastError = ecOK;
		m_ErrorObject.clear();
	}
	enum Status
	{
		//Nothing happened yet. 
		stUninited,
		//ExtractInit has been called.
		stInited, 
		//ExtractInit has been called, and processing works well!
		//If ExtractInit failed, it will still be in ecReady state.
		stProcessing,
		//Error happend in extraction. (ExtractInit works OK though).
		stErrored,
		//User aborted.
		stAborted,
		//Correctly extracted all files.
		stDone
	};
	enum ErrorCodes
	{
		ecOK,
		ecUnknown,
		ecInvalidHeader,
		ecFailedToOpenArchive,
		ecFailedToReadArchive,
		ecFailedToCreateDirectory,
		ecFailedToCreateFile,
		ecFailedToWriteFile,
		ecDecoding, 
		ecPackedDataCRCError,
		ecUnpackedDataCRCError
	};

	Status GetStatus() { return m_Status; } 
	bool IsUninited()
	{
		return m_Status == stUninited;
	}
	ErrorCodes GetLastError()
	{
		return m_LastError;
	}
protected:
	//My current status.
	Status		m_Status;
	ErrorCodes	m_LastError;
	//Either file name or directory name
	std::string		m_ErrorObject;

	virtual void SetStatus(Status st)
	{
		m_Status = st;
	}
	virtual void SetError(ErrorCodes ec)
	{
		m_LastError = ec;
	}
	virtual void SetErrorObject(const char* obj)
	{
		m_ErrorObject = obj;
	}
};

///////////////////////////////////////////////////////////////////////////////
// This is the class that can extract 7z archive with LZMA decoding.
class Archive : virtual public ArchiveBase, protected LzmaDecoder
{
public:
	Archive(int InputBufferSize = 0x10000, int OutputBufferSize = 0x10000)
	{
		assert(InputBufferSize > 0 && OutputBufferSize > 0);
		m_InputBufferSize = InputBufferSize;
		m_OutputBufferSize = OutputBufferSize;
		m_hCurrentFile = INVALID_HANDLE_VALUE;
		Clear();
	}
	~Archive()
	{
		CloseCurrentFile();
	}
	//This is the total size of all files.
	int GetUnpackSize() const
	{
		return m_TotalUnpackBytes;
	}
	//This is acurately how many bytes have been extracted.
	int GetUnpackBytesProcessed() const
	{ 
		return m_TotalUnpackBytesProcessed; 
	}
	//This is not accurate. LzmaDecoder will read more than needed. 
	//So this only represents bytes have been read.
	int GetPackBytesProcessed() const
	{ 
		return m_TotalPackBytesProcessed; 
	}
	//The number of files in the archive.
	int GetTotalFiles() const { return m_TotalFiles; }
	//The number of files that have been successfully extracted.
	int GetTotalFilesExtracted() const { return m_FilesExtracted; }
	// Retrieves the file length, in bytes, of the current file being extracted
	int GetCurrentFileSize() const{ return m_CurrentFileSize; }
	//The bytes extracted for the current file only.
	int GetCurrentFileBytesExtracted() const{ return m_ProcessingUnpackStreamProcessed; }
	//The size of the archive or input stream.
	int GetArchiveSize() const{ return m_ArchiveFileSize; }
	//Return the current file's name that is being extracted.
	std::string GetCurrentFileName() const { return m_CurrentFileName; }

	bool ExtractInit(const char* FileName);
	bool ExtractInit(IStream* is);
	//Init extraction and call ProcessBlock until it's done.
	bool Extract(const char* DestDir, bool override, int BlockSize);
	void SetExtractOptions(const char* DestDir, bool Override)
	{
		m_DestDir = DestDir;
		m_Override = Override;
	}
	bool IsFinished()
	{
		return GetStatus() != stProcessing;
	}
	void Abort()
	{
		CloseCurrentFile();
		SetStatus(stAborted);
	}
	//Process maximum of OutSize then return.
	bool ProcessBlock(int OutSize);
	//Output decoded stream to files. Note there might be multiple files involved.
	bool FlushDecodedStream(UI8* buffer, int size);
	void Clear()
	{
		m_Status = stUninited;
		m_ArchStream.Close();
		m_DecodeStream = NULL;
		m_DestDir.clear();
		SetError(ecOK);
		m_Override = false;

		m_Header.Clear();
		m_InternalPackStreams.clear();
		m_ProcessingPackStreamIndex = 0;
		m_ProcessingPackStreamProcessed = 0;
		m_ProcessingUnpackStreamIndex = 0;
		m_ProcessingUnpackStreamProcessed = 0;
		m_TotalPackBytesProcessed = 0;
		m_TotalUnpackBytesProcessed = 0;
		m_OutputBufferUnfiltered = 0;
		
		m_TimesProcessBlockCalled = 0;
		
		CloseCurrentFile();

		m_ArchiveFileName.clear();
		m_ArchiveFileSize = 0;
		m_TotalUnpackBytes = 0;
		m_TotalFiles = 0;
		m_FilesExtracted = 0;
	}
	void ReleaseBuffer()
	{
		m_InputBuffer.clear();
		m_OutputBuffer.clear();
		LzmaDecoder::ReleaseBuffer();
	}
	static bool VerifySignature(const unsigned char* buf, int bufSize)
	{
		static unsigned char Sig[6] = {'7', 'z', 0xBC, 0xAF, 0x27, 0x1C};
		if (bufSize < 6) return false;
		return memcmp(Sig, buf, 6) == 0;
	}
	void Debug(FILE* f, const std::string& indent)
	{
		fprintf(f, "%sArchive: \n", indent.c_str());
		m_Header.Debug(f, indent);
	}

protected:
	FileStream	m_ArchStream;

	//Never release this pointer!!!
	IStream*	m_DecodeStream;
	SignatureHeader	m_SHeader;
	Header		m_Header;
	//After ExtractInit, m_DestDir should not have a "\\" at the end.
	std::string		m_DestDir;
	//Whether we can override existing files.
	bool		m_Override;

	//My own generated mapping between packstream->Unpackstreams.
	std::vector<InternalPackStream>	m_InternalPackStreams;
	//Which stream I am processing.
	int	m_ProcessingPackStreamIndex;
	//How many bytes from the current pack stream has been processed.
	int m_ProcessingPackStreamProcessed;
	//The index into the vector<InternalUnpackStream> of PackStream
	int m_ProcessingUnpackStreamIndex;
	int m_ProcessingUnpackStreamProcessed;
	int m_TotalPackBytesProcessed;
	int m_TotalUnpackBytesProcessed;

	//How many times ProcessBlock has been called.
	int m_TimesProcessBlockCalled;
	HANDLE	m_hCurrentFile;

	//Inited in the constructor.
	int m_InputBufferSize;
	int m_OutputBufferSize;

	std::vector<UI8>	m_OutputBuffer;
	//This is the size of the output buffer chars that are still not filtered. 
	int	m_OutputBufferUnfiltered;
	std::vector<UI8>	m_InputBuffer;

	//These members are only for statistics. 
	std::string	m_ArchiveFileName;
	std::string	m_CurrentFileName;
	int		m_CurrentFileSize;
	int		m_ArchiveFileSize;
	int		m_TotalUnpackBytes;
	//How many files in total, including empty files.
	int		m_TotalFiles;
	int		m_FilesExtracted;

	//Helper class for BCJ Decoding.
	CBCJ_x86_Decoder	m_BcjDecoder;

	void CloseCurrentFile();
	//Override the default.
	virtual void SetStatus(Status st);	
	bool CreateDirectories();
	bool SetFileTime(HANDLE h, FileInfo& fi);
	//Always use this function to start writing a file.
	bool StartFile(FileInfo& fi, int size, HANDLE& hFile);
	bool EndFile(FileInfo& fi, HANDLE& hFile, int FileSize, int BytesWritten);
	bool CreateEmptyFiles();
	//generate mapping between packstream->Unpackstreams.
	bool CreateInternalMapping();
	//Override LzmaDecoder.
	virtual int OnRead(unsigned char **buffer, UInt32 *bufferSize);

	//First started..
	virtual void OnStartExtraction()
	{
	}
	//These are events called by processing. If return 0, continue; 1: skip.
	virtual int OnStartExtractFile(const char* FileName, int FileSize)
	{
		return 0;
	}
	virtual void OnWriteFile(const char* FileName, int FileSize, int BytesWritten)
	{
	}
	virtual void OnEndExtractFile(const char* FileName, int FileSize, int BytesWritten)
	{
	}
	//Everything stopped.
	virtual void OnEndExtraction()
	{
	}
};

}
}
}
}
