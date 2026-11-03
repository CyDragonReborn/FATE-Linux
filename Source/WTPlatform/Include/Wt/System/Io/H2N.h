#pragma once

namespace WT
{
namespace System
{
namespace IO
{

// H2N() converts various types from Host byte order to network byte order
// N2H() converts various types from network byte order to host byte order

// define this if we ever build on a big endian archtecture
#ifdef WT_ON_BIG_ENDIAN_CPU

inline int16 H2N(int16 value)
{
	return value;
}

inline int32 H2N(int32 value);
{
	return value;
}

inline int64 H2N(int64 value)
{
	return value;
}

inline int16 N2H(int16 value)
{
	return value;
}

inline int32 N2H(int32 value)
{
	return value;
}

inline int64 N2H(int64 value)
{
	return value;
}

#else 

// For Little-Endian CPUs like x86:

inline int16 H2N(int16 value)
{
	int16 newValue;
	reinterpret_cast<int8*>(&newValue)[1] = reinterpret_cast<int8*>(&value)[0];
	reinterpret_cast<int8*>(&newValue)[0] = reinterpret_cast<int8*>(&value)[1];
	return newValue;
}

inline int32 H2N(int32 value)
{
	int32 newValue;
	reinterpret_cast<int8*>(&newValue)[3] = reinterpret_cast<int8*>(&value)[0];
	reinterpret_cast<int8*>(&newValue)[2] = reinterpret_cast<int8*>(&value)[1];
	reinterpret_cast<int8*>(&newValue)[1] = reinterpret_cast<int8*>(&value)[2];
	reinterpret_cast<int8*>(&newValue)[0] = reinterpret_cast<int8*>(&value)[3];
	return newValue;
}

inline int64 H2N(int64 value)
{
	int64 newValue;
	reinterpret_cast<int8*>(&newValue)[7] = reinterpret_cast<int8*>(&value)[0];
	reinterpret_cast<int8*>(&newValue)[6] = reinterpret_cast<int8*>(&value)[1];
	reinterpret_cast<int8*>(&newValue)[5] = reinterpret_cast<int8*>(&value)[2];
	reinterpret_cast<int8*>(&newValue)[4] = reinterpret_cast<int8*>(&value)[3];
	reinterpret_cast<int8*>(&newValue)[3] = reinterpret_cast<int8*>(&value)[4];
	reinterpret_cast<int8*>(&newValue)[2] = reinterpret_cast<int8*>(&value)[5];
	reinterpret_cast<int8*>(&newValue)[1] = reinterpret_cast<int8*>(&value)[6];
	reinterpret_cast<int8*>(&newValue)[0] = reinterpret_cast<int8*>(&value)[7];
	return newValue;
}

inline int16 N2H(int16 value)
{
	// this operation is reversible so just call H2N()
	return H2N(value);
}

inline int32 N2H(int32 value)
{
	// this operation is reversible so just call H2N()
	return H2N(value);
}

inline int64 N2H(int64 value)
{
	// this operation is reversible so just call H2N()
	return H2N(value);
}

#endif

} // IO namespace
} // System namespace
} // WT namespace

