#include "WT/stdafx.h"

#include "WT/System/IO/MemoryStream.h"

namespace WT
{
namespace System
{
namespace IO
{
using namespace KConstants;

WT_DEFINE_CLASS(CMemoryStream)

CMemoryStream::CMemoryStream() 
	: m_pBuffer( NULL )
	, m_Capacity( 0 )
	, m_Length( 0 )
	, m_bOwnBuffer( true )
{
	std::fill( m_Cur, m_Cur + KMaxCur, 0 );
}

CMemoryStream::CMemoryStream( uint8 Buffer[], uint32 Length )
	: m_pBuffer( Buffer )
	, m_Capacity( Length )
	, m_Length( Length )
	, m_bOwnBuffer( false )
{
	std::fill( m_Cur, m_Cur + KMaxCur, 0 );
}

CMemoryStream::CMemoryStream( std::vector<uint8>& Buffer )
	: m_pBuffer( &Buffer[0] )
	, m_Capacity( static_cast<int32>( Buffer.size() ) )
	, m_Length( static_cast<int32>( Buffer.size() ) )
	, m_bOwnBuffer( false )
{
	std::fill( m_Cur, m_Cur + KMaxCur, 0 );
}

CMemoryStream::~CMemoryStream()
{
	if (m_bOwnBuffer)
	{
		free( m_pBuffer );
	}
}

bool CMemoryStream::CanRead() const
{
	return true;
}

bool CMemoryStream::CanSeek() const
{
	return true;
}

bool CMemoryStream::CanWrite() const
{
	return true;
}

void* CMemoryStream::GetHandle()
{
	return m_pBuffer;
}

bool CMemoryStream::IsAsync() const
{
	return false;
}

int32 CMemoryStream::GetCapacity() const
{
	return m_Capacity;
}

int32 CMemoryStream::GetLength() const
{
	return m_Length;
}

string const& CMemoryStream::GetName() const
{
	static std::string const Name( "MemoryStream" );
	return Name;
}

void CMemoryStream::SetPosition( int32 position )
{
	//allow position == m_Length
	if (position < 0 || position > m_Length)
	{
		throw std::out_of_range( "memory stream position" );
	}

	m_Cur[KDefaultCur] = position;
}

void CMemoryStream::SetPositionCur( int32 Cur, int32 position )
{
	//allow position == m_Length
	if (position < 0 || position > m_Length)
	{
		throw std::out_of_range( "memory stream position" );
	}

	m_Cur[Cur] = position;
}

int32 CMemoryStream::GetPosition() const
{
	return m_Cur[KDefaultCur];
}

int32 CMemoryStream::GetPositionCur( int32 Cur ) const
{
	if (Cur < 0 || Cur >= KMaxCur)
	{
		throw std::out_of_range ( "memory stream position cursor" );
	}
	return m_Cur[Cur];
}

//BeginRead
//BeginWrite
void CMemoryStream::Close()
{
	if (m_bOwnBuffer)
	{
		free( m_pBuffer );
		m_bOwnBuffer = false;
	}

	m_pBuffer = NULL;
	m_Length = 0;
	for (int32 Cur=0; Cur<KMaxCur; Cur++)
	{
		m_Cur[Cur] = 0;
	}
}

//EndRead
//EndWrite
void CMemoryStream::Flush()
{
}

void CMemoryStream::ChangeCapacityTo ( const int32 Value )
{
	assert (m_bOwnBuffer);
	assert (Value >= 0);
	if (Value == m_Capacity)
	{
		return;
	}

	const uint8* const pOldBuffer = m_pBuffer; // in case realloc fails
	m_pBuffer = static_cast<uint8*>( realloc( m_pBuffer, Value ) );
	
	if (m_pBuffer == NULL)
	{
		assert (m_pBuffer);
		free ((char*)pOldBuffer);
		Close();
		throw std::bad_alloc( "memory stream buffer" );
	}

	// If the capacity is shrinking, the length may have
	// to shink down to the right value.
	if (m_Length > m_Capacity)
	{
		m_Length = m_Capacity;
	}

	for (int32 Cur=0; Cur<KMaxCur; Cur++)
	{
		if (m_Cur[Cur] > Value)
		{
			m_Cur[Cur] = Value;
		}
	}
}

int32 CMemoryStream::Read ( uint8 buffer[], int32 count )
{
	return ReadCur ( KDefaultCur, buffer, count);
}

int32 CMemoryStream::ReadCur ( int32 Cur, uint8 buffer[], int32 count )
{
	if (count < 0)
	{
		throw std::domain_error( "bad memory stream read count" );
	}

	if (Cur < 0 || Cur >= KMaxCur)
	{
		throw std::out_of_range ( "memory stream position cursor" );
	}

	int32 BytesRead = count;
	if (BytesRead > m_Length - m_Cur[Cur])
	{
		BytesRead = m_Length - m_Cur[Cur];
	}
	if (BytesRead < 0)
	{
		BytesRead = 0;
	}

	if (buffer != NULL)
	{
		memcpy( buffer, m_pBuffer + m_Cur[Cur], BytesRead );
	}
	
	m_Cur[Cur] += BytesRead;
	return BytesRead;
}

int32 CMemoryStream::ReadByte ()
{
	uint8 B;
	int32 NumRead = ReadCur( KDefaultCur, &B, 1 );
	return NumRead == 1 ? B : -1;
}

int32 CMemoryStream::ReadByteCur ( int32 Cur)
{
	uint8 B;
	int32 NumRead = ReadCur( Cur, &B, 1 );
	return NumRead == 1 ? B : -1;
}


int32 CMemoryStream::Seek ( int32 offset , SeekOrigin::Enum origin )
{
	return SeekCur ( KDefaultCur, offset, origin);
}

int32 CMemoryStream::SeekCur ( int32 Cur, int32 offset, SeekOrigin::Enum origin )
{
	int32 Dest = m_Cur[Cur];

	if (Cur < 0 || Cur >= KMaxCur)
	{
		throw std::out_of_range( "memory stream position cursor" );
	}
	switch (origin)
	{
	case SeekOrigin::Begin:
		Dest = offset;
		break;
	case SeekOrigin::Current:
		Dest += offset;
		break;
	case SeekOrigin::End:
		Dest = m_Length - offset;
	default:
		throw std::invalid_argument( "seek origin" );
	}

	if (Dest < 0 || Dest > m_Length)
	{
		return -1;
	}
		
	m_Cur[Cur] = Dest;
	return m_Cur[Cur];
}

void CMemoryStream::SetCapacity ( int32 Value )
{
	if (Value < 0)
	{
		throw std::domain_error( "memory stream length" );
	}

	if (!m_bOwnBuffer)
	{
		throw std::invalid_argument( "SetCapacity on externally-owned memory stream" );
	}

	if (Value <= m_Length)
	{
		// Must not change
		return;
	}

	ChangeCapacityTo (Value);
}


void CMemoryStream::SetLength ( int32 length )
{
	if (length < 0)
	{
		throw std::domain_error( "memory stream length" );
	}

	if (!m_bOwnBuffer)
	{
		throw std::invalid_argument( "SetLength on externally-owned memory stream" );
	}

	if (length == m_Length)
	{
		//No change needed
		return;
	}

	if (length > m_Capacity)
	{
		ChangeCapacityTo (length*2);
	}
	m_Length = length;
}

void CMemoryStream::Write( uint8 const buffer[], int32 count )
{
	return WriteCur ( KDefaultCur, buffer, count);
}

void CMemoryStream::WriteCur ( int32 Cur, uint8 const buffer[], int32 count)
{
	if (count == 0)
	{
		return;
	}

	if (buffer == NULL)
	{
		throw std::invalid_argument( "NULL buffer pointer" );
	}
	if (Cur < 0 || Cur >= KMaxCur)
	{
		throw std::out_of_range( "memory stream position cursor" );
	}

	if (m_Cur[Cur] + count > m_Length )
	{
		SetLength( m_Cur[Cur] + count );
	}

	memcpy( m_pBuffer + m_Cur[Cur], buffer, count );
	m_Cur[Cur] += count;
}

void CMemoryStream::WriteByte ( uint8 value )
{
	WriteCur( KDefaultCur, &value, 1 );
}

void CMemoryStream::WriteByteCur ( int32 Cur,uint8 value )
{
	WriteCur( Cur, &value, 1 );
}

}
}
}
