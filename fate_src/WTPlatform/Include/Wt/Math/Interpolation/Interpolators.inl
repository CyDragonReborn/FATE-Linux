//
//	Interpolators.inl
//	
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//


//
//	InterpolateTimedVal
//

template<typename V, typename T>
__forceinline void __fastcall InterpolateTimedVal<V,T>::Reset()
{
	m_tick_current = m_tick_end;
	m_val_current = m_val_end;
}

template<typename V, typename T>
__forceinline void __fastcall InterpolateTimedVal<V,T>::Reset(V targetval)
{
	m_val_end = targetval;
	Reset();
}

template<typename V, typename T>
__forceinline void __fastcall InterpolateTimedVal<V,T>::Move(T tickdelta)
{
	m_tick_current += tickdelta;
	if(m_tick_current > m_tick_end)
		m_val_current = m_val_end;
	else
		m_val_current = m_A*m_tick_current + m_val_start;	
}

template<typename V, typename T>
__forceinline void __fastcall InterpolateTimedVal<V,T>::Start(V startval, T maxtick, T mintick)
{
	m_tick_start = mintick;
	m_tick_current = mintick;
	m_tick_end = maxtick;
	m_val_current = startval;
	m_val_start = startval;
	m_A = (m_val_end-m_val_start)/(m_tick_end-mintick);
}

template<typename V, typename T>
__forceinline T __fastcall InterpolateTimedVal<V,T>::Status()
{
	return (m_tick_current-m_tick_start)/(m_tick_end-m_tick_start);
}


//
//	InterpolateLoopTimedVal
//

template<typename V, typename T>
__forceinline void __fastcall InterpolateLoopTimedVal<V,T>::Move(T tickdelta)
{
	if( (m_tick_current + tickdelta) > m_tick_end )
	{
		tickdelta = m_tick_end - (m_tick_current + tickdelta);
		m_tick_current = m_tick_start;
	}

	InterpolateTimedVal<V,T>::Move(tickdelta);
}

template<typename V, typename T>
__forceinline void __fastcall InterpolateLoopTimedVal<V,T>::Start(V startval, T maxtick, T mintick)
{
	InterpolateTimedVal<V,T>::Start(startval, maxtick, mintick);
}


//
//	InterpolateLoopTimedVal
//

template<typename V, typename T>
__forceinline void __fastcall InterpolateBounceTimedVal<V,T>::Move(T tickdelta)
{
	if(m_increasing)
	{
		if( (m_tick_current + tickdelta) > m_tick_end )
		{
			tickdelta = (m_tick_end - (m_tick_current + tickdelta));
			m_increasing = FALSE;
		}
	}
	else
	{
		if( (m_tick_current - tickdelta) < m_tick_start )
		{
			tickdelta = (m_tick_current + tickdelta)-m_tick_start;
			m_increasing = TRUE;
		}
	}

	if(!m_increasing)
		tickdelta = -tickdelta;

	InterpolateTimedVal<V,T>::Move(tickdelta);
}

template<typename V, typename T>
__forceinline void __fastcall InterpolateBounceTimedVal<V,T>::Start(V startval, T maxtick, T mintick)
{
	InterpolateTimedVal<V,T>::Start(startval, maxtick, mintick);
}
