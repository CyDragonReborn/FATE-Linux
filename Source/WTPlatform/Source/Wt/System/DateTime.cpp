#include "WT/stdafx.h"

#include "WT/System/DateTime.h"

#include <windows.h>
#include <stdio.h>

namespace WT
{
namespace System
{
// -------------------------------------------------------------------------------------
// DayOfWeek
// -------------------------------------------------------------------------------------

const char* const DayOfWeek::ToStringRFC822 (const enum Enum Day)
{
	switch (Day)
	{
	case Sunday: return "Sun";
	case Monday: return "Mon";
	case Tuesday: return "Tue";
	case Wednesday: return "Wed";
	case Thursday: return "Thu";
	case Friday: return "Fri";
	case Saturday: return "Sat";
	}
	assert (false);
	return "?d?";
}



// -------------------------------------------------------------------------------------
// MonthOfYear
// -------------------------------------------------------------------------------------


const char* const MonthOfYear::ToStringRFC822 (const int Month)
{
	switch (Month)
	{
	case 1: return "Jan";
	case 2: return "Feb";
	case 3: return "Mar";
	case 4: return "Apr";
	case 5: return "May";
	case 6: return "Jun";
	case 7: return "Jul";
	case 8: return "Aug";
	case 9: return "Sep";
	case 10: return "Oct";
	case 11: return "Nov";
	case 12: return "Dec";
	}
	assert (false);
	return "?m?";
}

// -------------------------------------------------------------------------------------
// TimeSpan
// -------------------------------------------------------------------------------------

CTimeSpan::CTimeSpan()
{
	m_Ticks = 0;
}

CTimeSpan::CTimeSpan ( int64 Ticks )
{
	m_Ticks = Ticks;
}

CTimeSpan::CTimeSpan ( int32 Hours , int32 Minutes , int32 Seconds )
{
	m_Ticks = TimeToTicks ( Hours , Minutes, Seconds );
}

CTimeSpan::CTimeSpan ( int32 Days , int32 Hours , int32 Minutes , int32 Seconds )
{
	CTimeSpan NewTimeSpan ( Days , Hours , Minutes , Seconds , 0 );
	m_Ticks = NewTimeSpan.m_Ticks;
}

CTimeSpan::CTimeSpan ( int32 Days , int32 Hours , int32 Minutes , int32 Seconds , int32 Milliseconds )
{
	int64 TotalMilliSeconds = ((int64) Days * 3600 * 24 + (int64) Hours * 3600 + (int64) Minutes * 60 + (int64) Seconds ) * 1000 + Milliseconds;
	if ( TotalMilliSeconds > MaxMilliseconds || TotalMilliSeconds < MinMilliseconds )
		throw "TimeSpan too big";
	m_Ticks = TotalMilliSeconds * TicksPerMillisecond;
}

const CTimeSpan CTimeSpan::Zero = CTimeSpan ( 0 );
const CTimeSpan CTimeSpan::MaxValue = CTimeSpan ( Int64MaxValue );
const CTimeSpan CTimeSpan::MinValue = CTimeSpan ( Int64MinValue );

int64 WTCall CTimeSpan::TimeToTicks ( int Hours , int Minutes , int Seconds )
{
	int64 TotalSeconds = (int64) Hours * 3600 + (int64) Minutes * 60 + (int64) Seconds;
	if ( TotalSeconds > MaxSeconds || TotalSeconds < MinSeconds )
		throw "TimeSpan too big";
	return TotalSeconds * TicksPerSecond;
}

int32 CTimeSpan::Days () const
{
	return (int32) ( m_Ticks / TicksPerDay );
}

int32 CTimeSpan::Hours () const
{
	return (int32) ( ( m_Ticks / TicksPerHour ) % 24 );
}

int32 CTimeSpan::Minutes () const
{
	return (int32) ( ( m_Ticks / TicksPerMinute ) % 60 );
}

int32 CTimeSpan::Seconds () const
{
	return (int32) ( ( m_Ticks / TicksPerSecond ) % 60 );
}

int32 CTimeSpan::Milliseconds () const
{
	return (int32) ( ( m_Ticks / TicksPerMillisecond ) % 1000 );
}

int64 CTimeSpan::Ticks () const
{
	return m_Ticks;
}

float64 CTimeSpan::TotalDays () const
{
	return ( (float64) m_Ticks ) / (float64) TicksPerDay;
}

float64 CTimeSpan::TotalHours () const
{
	return ( (float64) m_Ticks ) / (float64) TicksPerHour;
}

float64 CTimeSpan::TotalMinutes () const
{
	return ( (float64) m_Ticks ) / (float64) TicksPerMinute;
	
}

float64 CTimeSpan::TotalSeconds () const
{
	return ( (float64) m_Ticks ) / (float64) TicksPerSecond;
}

float64 CTimeSpan::TotalMilliseconds () const
{
	float64 Temp = ((float64) m_Ticks ) / (float64) TicksPerMillisecond;
	if ( Temp > MaxMilliseconds )
		return (float64) MaxMilliseconds;
	else if ( Temp < MinMilliseconds )
		return (float64) MinMilliseconds;
	else
		return Temp;
}

CTimeSpan CTimeSpan::Add ( const CTimeSpan& Other ) const
{
	int64 Result = m_Ticks + Other.m_Ticks;

	// Overflow if signs of operands were identical and result's
	// sign was opposite.
	// >> 63 gives the sign bit (either 64 1's or 64 0's).

	if (( m_Ticks >> 63 == Other.m_Ticks >> 63 ) && ( m_Ticks >> 63 != Result >> 63 ))
		throw "TimeSpan too big";

	return CTimeSpan ( Result );
}

CTimeSpan CTimeSpan::Negate () const
{
	if ( m_Ticks == MinValue.m_Ticks )
		throw "TimeSpan too big";
	return CTimeSpan ( -m_Ticks );
}
 
CTimeSpan CTimeSpan::Subtract ( const CTimeSpan& Other ) const
{
	int64 Result = m_Ticks - Other.m_Ticks;

	// Overflow if signs of operands was different and result's
	// sign was opposite from the first argument's sign.
	// >> 63 gives the sign bit (either 64 1's or 64 0's).

	if (( m_Ticks >> 63 != Other.m_Ticks >> 63 ) && ( m_Ticks >> 63 != Result >> 63 ))
		throw "TimeSpan too big";

	return CTimeSpan ( Result );
}

CTimeSpan CTimeSpan::Multiply ( float32 Value ) const
{
	return CTimeSpan ( (int64) ( m_Ticks * Value ));
}

CTimeSpan CTimeSpan::Divide ( float32 Value ) const
{
	return CTimeSpan ( (int64) ( m_Ticks / Value ));
}

int32 WTCall CTimeSpan::Compare ( const CTimeSpan& TimeSpan1 , const CTimeSpan& TimeSpan2 )
{
	if ( TimeSpan1.m_Ticks > TimeSpan2.m_Ticks )
		return 1;
	else if ( TimeSpan1.m_Ticks < TimeSpan2.m_Ticks )
		return -1;
	else
		return 0;
}

int32 CTimeSpan::CompareTo ( const CTimeSpan& Other ) const
{
	if ( m_Ticks > Other.m_Ticks )
		return 1;
	else if ( m_Ticks < Other.m_Ticks )
		return -1;
	else
		return 0;
}

CTimeSpan CTimeSpan::Duration () const
{
	if ( m_Ticks == MinValue.m_Ticks )
		throw "TimeSpan too big";
	return CTimeSpan ( m_Ticks >= 0 ? m_Ticks : -m_Ticks );
}

bool CTimeSpan::Equals ( const CTimeSpan& Other ) const
{
	return m_Ticks == Other.m_Ticks ? true : false;
}

string CTimeSpan::ToString () const
{
	char Buffer[32];
	const char Sign = m_Ticks < 0 ? '-' : '+';

	::sprintf( Buffer, "%c%04d:%02d:%02d:%02d.%03d", Sign, Days(), Hours(), Minutes(), Seconds(), Milliseconds() );
	return string( Buffer );
}


//conversion routine to a TimeSpan from a cmString
//String must be in the format "ndddd:hh:mm:ss.uuu", where n=sign, d=days, h=hours, m=minutes, s=seconds, u=milliseconds
CTimeSpan WTCall CTimeSpan::FromString( const string& String )
{
	char Sign;
	int32 Days, Hours, Minutes, Seconds, Milliseconds;
	if( 6 != ::sscanf( String.c_str(), "%c%04d:%02d:%02d:%02d.%03d", &Sign, &Days, &Hours, &Minutes, &Seconds, &Milliseconds ) )
		return CTimeSpan( 0 );

	CTimeSpan Span( Days, Hours, Minutes, Seconds, Milliseconds );
	if( Sign == '-' )
		Span = Span.Negate();

	return Span;
}

CTimeSpan WTCall CTimeSpan::Interval ( float64 Value , int Scale )
{
	float64 Temp = Value * Scale;
	int64 Millis = (int64) ( Temp + ( Value >= 0 ? 0.5f : -0.5f ));
	// When value is NaN or -Inf, it becomes Int64.MinValue,
	// and for +Inf it becomes Int64.MaxValue.  The overflow check works.
	// Scale is always positive, which simplifies 1 of the 2 overflow checks
	if (( Millis > Int64MaxValue / TicksPerMillisecond ) || ( Millis < Int64MinValue / TicksPerMillisecond ) ||
	    ( Temp < 0 && Value > 0 ) || ( Temp > 0 && Value < 0 ))
		throw "TimeSpan too big";
	return CTimeSpan ( Millis * TicksPerMillisecond );
}

CTimeSpan WTCall CTimeSpan::FromDays ( float64 Value )
{
	return Interval ( Value , MillisPerDay );
}

CTimeSpan WTCall CTimeSpan::FromHours ( float64 Value )
{
	return Interval ( Value , MillisPerHour );
}

CTimeSpan WTCall CTimeSpan::FromMinutes ( float64 Value )
{
	return Interval ( Value , MillisPerMinute );
}

CTimeSpan WTCall CTimeSpan::FromSeconds ( float64 Value )
{
	return Interval ( Value , MillisPerSecond );
}

CTimeSpan WTCall CTimeSpan::FromMilliseconds ( float64 Value )
{
	return Interval ( Value , 1 );
}

CTimeSpan WTCall CTimeSpan::FromTicks ( int64 Value )
{
	return CTimeSpan ( Value );
}

CTimeSpan CTimeSpan::operator- () const
{
	return Negate ();
}

CTimeSpan CTimeSpan::operator+ ( const CTimeSpan& Other ) const
{
	return Add ( Other );
}

CTimeSpan CTimeSpan::operator- ( const CTimeSpan& Other ) const
{
	return Subtract ( Other );
}

CTimeSpan CTimeSpan::operator*  ( float32 Value ) const
{
	return Multiply ( Value );
}

CTimeSpan CTimeSpan::operator/  ( float32 Value ) const
{
	return Divide ( Value );
}

bool CTimeSpan::operator== ( const CTimeSpan& Other ) const
{
	return m_Ticks == Other.m_Ticks ? true : false;
}

bool CTimeSpan::operator!= ( const CTimeSpan& Other ) const
{
	return m_Ticks != Other.m_Ticks ? true : false;
}

bool CTimeSpan::operator< ( const CTimeSpan& Other ) const
{
	return m_Ticks < Other.m_Ticks ? true : false;
}

bool CTimeSpan::operator<= ( const CTimeSpan& Other ) const
{
	return m_Ticks <= Other.m_Ticks ? true : false;
}

bool CTimeSpan::operator> ( const CTimeSpan& Other ) const
{
	return m_Ticks > Other.m_Ticks ? true : false;
}

bool CTimeSpan::operator>= ( const CTimeSpan& Other ) const
{
	return m_Ticks >= Other.m_Ticks ? true : false;
}

CTimeSpan& CTimeSpan::operator+= ( const CTimeSpan& Other )
{
	m_Ticks += Other.m_Ticks;
	return *this;
}

CTimeSpan& CTimeSpan::operator-= ( const CTimeSpan& Other )
{
	m_Ticks -= Other.m_Ticks;
	return *this;
}

// -------------------------------------------------------------------------------------
// DateTime
// -------------------------------------------------------------------------------------

const CDateTime CDateTime::MinValue( CDateTime::MinTicks );
const CDateTime CDateTime::MaxValue( CDateTime::MaxTicks );

const int32 CDateTime::DaysToMonth365[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
const int32 CDateTime::DaysToMonth366[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

CDateTime::CDateTime ()
{
	m_Ticks = 0;
}

CDateTime::CDateTime ( int64 Ticks )
{
	m_Ticks = Ticks;
}

CDateTime::CDateTime ( int32 Year , int32 Month , int32 Day )
{
	m_Ticks = DateToTicks( Year , Month , Day );
}

CDateTime::CDateTime ( int32 Year , int32 Month , int32 Day , int32 Hour , int32 Minute , int32 Second )
{
	m_Ticks = DateToTicks ( Year , Month , Day ) + TimeToTicks ( Hour , Minute , Second );
}

CDateTime::CDateTime ( int32 Year , int32 Month , int32 Day , int32 Hour , int32 Minute , int32 Second , int32 Millisecond )
{
	m_Ticks = DateToTicks ( Year , Month , Day ) + TimeToTicks ( Hour , Minute , Second );
	m_Ticks += Millisecond * TicksPerMillisecond;
}

CDateTime CDateTime::Add ( const CTimeSpan& Span ) const
{
	return CDateTime ( m_Ticks + Span.m_Ticks );
}

CDateTime CDateTime::Subtract ( const CTimeSpan& Span ) const
{
	return CDateTime ( m_Ticks - Span.m_Ticks );
}

CTimeSpan CDateTime::Subtract ( const CDateTime& Other ) const
{
	return CTimeSpan ( m_Ticks - Other.m_Ticks );
}

CDateTime CDateTime::AddYears ( int32 Value ) const
{
	return AddMonths ( Value * 12 );
}

CDateTime CDateTime::AddMonths ( int32 Value ) const
{
	int32 y = GetDatePart ( DatePartYear );
	int32 m = GetDatePart ( DatePartMonth );
	int32 d = GetDatePart ( DatePartDay );
	int32 i = m - 1 + Value;
	if ( i >= 0 )
	{
		m = i % 12 + 1;
		y = y + i / 12;
	}
	else
	{
		m = 12 + (i + 1) % 12;
		y = y + (i - 11) / 12;
	}
	int32 days = DaysInMonth ( y , m );
	if ( d > days )
		d = days;
	return CDateTime ( DateToTicks ( y , m , d ) + m_Ticks % TicksPerDay );
}

CDateTime CDateTime::AddDays ( float64 Value ) const
{
	return Add ( Value , MillisPerDay );
}

CDateTime CDateTime::AddHours ( float64 Value ) const
{
	return Add ( Value , MillisPerHour );
}

CDateTime CDateTime::AddMinutes ( float64 Value ) const
{
	return Add ( Value , MillisPerMinute );
}

CDateTime CDateTime::AddSeconds ( float64 Value ) const
{
	return Add ( Value , MillisPerSecond );
}

CDateTime CDateTime::AddMilliseconds ( float64 Value ) const
{
	return Add ( Value , 1 );
}

CDateTime CDateTime::AddTicks ( int64 Value ) const
{
	return CDateTime ( m_Ticks + Value );
}

int32 WTCall CDateTime::DaysInMonth ( int32 Year , int32 Month )
{
	const int32* Days = IsLeapYear ( Year ) ? DaysToMonth366 : DaysToMonth365;
	return Days[Month] - Days[Month - 1];
}

int32 WTCall CDateTime::Compare ( const CDateTime& DateTime1 , const CDateTime& DateTime2 )
{
	if ( DateTime1.m_Ticks > DateTime2.m_Ticks )
		return 1;
	else if ( DateTime1.m_Ticks < DateTime2.m_Ticks )
		return -1;
	else
		return 0;
}

int32 CDateTime::CompareTo ( const CDateTime& Other ) const
{
	return Compare ( *this , Other );
}

bool CDateTime::Equals ( const CDateTime& Other ) const
{
	return m_Ticks == Other.m_Ticks ? true : false;
}

CDateTime CDateTime::Date () const
{
	return CDateTime ( m_Ticks - m_Ticks % TicksPerDay );
}

CTimeSpan CDateTime::TimeOfDay () const
{
	return CTimeSpan ( m_Ticks % TicksPerDay);
}

int32 CDateTime::Year () const
{
	return GetDatePart ( DatePartYear );
}

int32 CDateTime::Month () const
{
	return GetDatePart ( DatePartMonth );
}

int32 CDateTime::Day () const
{
	return GetDatePart ( DatePartDay );
}

int32 CDateTime::Hour () const
{
	return (int32) (( m_Ticks / TicksPerHour ) % 24); 
}

int32 CDateTime::Minute () const
{
	return (int32) (( m_Ticks / TicksPerMinute ) % 60);
}

int32 CDateTime::Second () const
{
	return (int32) (( m_Ticks / TicksPerSecond ) % 60);
}

int32 CDateTime::Millisecond () const
{
	return (int32)(( m_Ticks / TicksPerMillisecond ) % 1000);
}

int64 CDateTime::Ticks () const
{
	return m_Ticks;
}

DayOfWeek::Enum CDateTime::DayOfWeek() const
{
	return (DayOfWeek::Enum) (( m_Ticks / TicksPerDay + 1) % 7);
}

int32 CDateTime::DayOfYear () const
{
	return GetDatePart ( DatePartDayOfYear );
}

CDateTime CDateTime::StartOfWeek () const
{
	CDateTime ThisDay = Date();
	CTimeSpan DaysToSubtract = CTimeSpan::FromDays ( DayOfWeek() );
	return ThisDay - DaysToSubtract;
}

CDateTime CDateTime::EndOfWeek () const
{
	//get the start of the week and add 7 days to it.

	return StartOfWeek() + CTimeSpan::FromDays ( 7 );
}

CDateTime WTCall CDateTime::Today()
{
	return Now().Date();
}
CDateTime WTCall CDateTime::FromFileTime ( int64 FileTime)
{
	assert (false);
	return CDateTime (1963, 10, 19); // return Peter's birthday.  
}
	/*
	// This is the ticks in Universal time for this fileTime.
	int64 UniversalTicks = FileTime + FileTimeOffset;
	CDateTime UnivDT = new CDateTime ( UniversalTicks );
	// We can safely cast TimeZone.CurrentTimeZone to CurrentSystemTimeZone since CurrentTimeZone is a static method in TimeZone class.
		00459             CurrentSystemTimeZone tz = (CurrentSystemTimeZone)TimeZone.CurrentTimeZone;
	00460             long localTicks = universalTicks + tz.GetUtcOffsetFromUniversalTime(univDT);
	00461             return new DateTime(localTicks, 0);
	00462         }
	*/


CDateTime WTCall CDateTime::Now ()
{
	SYSTEMTIME UTCSystemTime;
	FILETIME UTCFileTime, LocalFileTime;
	GetSystemTime ( &UTCSystemTime );
	SystemTimeToFileTime ( &UTCSystemTime , &UTCFileTime );
	FileTimeToLocalFileTime ( &UTCFileTime , &LocalFileTime );
	return CDateTime ( *((int64 *) &LocalFileTime) + FileTimeOffset );
}

CDateTime WTCall CDateTime::UTCNow ()
{
	SYSTEMTIME UTCSystemTime;
	GetSystemTime ( &UTCSystemTime );
	FILETIME FileTime;
	SystemTimeToFileTime  ( &UTCSystemTime , &FileTime );
	return CDateTime ( *((int64 *) &FileTime) + FileTimeOffset );
}

CDateTime CDateTime::ToLocalTime() const
{
	int64 AdjustedTicks = m_Ticks - FileTimeOffset;
	FILETIME UTCFileTime , LocalFileTime;
	memcpy ( &UTCFileTime , &AdjustedTicks , sizeof (FILETIME));
	FileTimeToLocalFileTime ( &UTCFileTime , &LocalFileTime );
	return CDateTime ( *((int64 *) &LocalFileTime) + FileTimeOffset );
}

CDateTime CDateTime::ToUniversalTime() const
{
	int64 AdjustedTicks = m_Ticks - FileTimeOffset;
	FILETIME UTCFileTime , LocalFileTime;
	memcpy ( &LocalFileTime , &AdjustedTicks , sizeof (FILETIME));
	LocalFileTimeToFileTime ( &LocalFileTime , &UTCFileTime );
	return CDateTime ( *((int64 *) &UTCFileTime) + FileTimeOffset );
}

string CDateTime::ToString () const
{
	char Buffer[1024];
	sprintf ( Buffer , "%02d:%02d:%02d.%03d %02d/%02d/%04d" , Hour() , Minute() , Second() , Millisecond() ,
	                                                         Month() , Day() , Year() );
	string Result(Buffer);
	return Result;
}

string CDateTime::ToString (const string& Format) const
{
	char Buffer [1024];

	if (Format == "r" || Format == "R") // RFC 1123
	{
		const char* const DayName = DayOfWeek::ToStringRFC822 (DayOfWeek());
		const char* const MonthName = MonthOfYear::ToStringRFC822 (Month());
		sprintf (Buffer, "%s, %d %s %4d %02d:%02d:%02d UT", DayName, Day(), MonthName, Year(), Hour(), Minute(), Second());
	}
	else
	{
		assert (false);
		return ToString();
	}

	string Result(Buffer);
	return Result;
}

//conversion routine to a DateTime from a cmString
//String must be in the format "hh:mm:ss.uuu mm/dd/yyyy", where h=hours, m=minutes, s=seconds, u=milliseconds, d=day,m=month,y=year
CDateTime WTCall CDateTime::FromString( const string& String )
{
    int32 Year, Month, Day, Hour, Minute, Second, Millisecond;       
    if( 7 != ::sscanf( String.c_str(), "%02d:%02d:%02d.%03d %02d/%02d/%4d", &Hour, &Minute, &Second, &Millisecond, &Month, &Day, &Year ) )
        return CDateTime( 0 );

	return CDateTime( Year, Month, Day, Hour, Minute, Second, Millisecond );
}

int32 CDateTime::MonthsInterval ( const CDateTime& Other ) const
{
	int32 ThisYear = Year();
	int32 ThisMonth = Month();
	int32 DateTimeYear = Other.Year();
	int32 DateTimeMonth = Other.Month();

	return ( ThisYear - DateTimeYear - 1) * 12 + 12 - DateTimeMonth + ThisMonth;
}

int32 CDateTime::WeeksInterval ( const CDateTime& Other ) const
{
	//get the start of this week.

	CDateTime StartOfThisWeek = StartOfWeek();

	//get the start of the week for the DateTime

	CDateTime DateTimeStartOfWeek = Other.StartOfWeek();

	//the difference between the two weeks starts should be an even multiple of 7 days
	//so, the range of weeks should be an even multiple of the weekly interval * 7 days

	CTimeSpan WeeksSpan = StartOfThisWeek - DateTimeStartOfWeek;
	return (int32) ((float32) (WeeksSpan.TotalDays() / 7) );
}

DayOfWeek::Enum WTCall CDateTime::FirstDayOfMonth ( int32 Year , int32 Month )
{
	CDateTime FirstDay( Year , Month , 1 );
	return FirstDay.DayOfWeek ();
}

DayOfWeek::Enum WTCall CDateTime::LastDayOfMonth ( int32 Year , int32 Month )
{
	CDateTime LastDay( Year , Month , DaysInMonth ( Year , Month ) );
	return LastDay.DayOfWeek ();
}

int32 WTCall CDateTime::SpecificWeekDay ( int32 Year , int32 Month , int32 Count )
{
	//first see if we need the 1,2,3,4 or the last one (5)
	if ( Count < 5 )
	{
		int32 FirstWeekDay = 0;
		for ( int32 i = 1 ; i <= Count ; i++ )
		{
			FirstWeekDay ++;
			CDateTime WeekDay( Year , Month , FirstWeekDay );
			if ( WeekDay.DayOfWeek() == DayOfWeek::Saturday )
				FirstWeekDay += 2;
			else if ( WeekDay.DayOfWeek() ==  DayOfWeek::Sunday )
				FirstWeekDay ++;
		}
		return FirstWeekDay;
	}
	else
	{
		//we're interested in the last one in the month
		int32 LastWeekDay = DaysInMonth ( Year , Month );
		CDateTime WeekDay( Year , Month , LastWeekDay );
		if ( WeekDay.DayOfWeek() ==  DayOfWeek::Saturday )
			return LastWeekDay-1;
		else if ( WeekDay.DayOfWeek() ==  DayOfWeek::Sunday )
			return LastWeekDay-2;
		else
			return LastWeekDay;
	}
}

int32 WTCall CDateTime::SpecificWeekendDay ( int32 Year , int32 Month , int32 Count )
{
	//first see if we need the 1,2,3,4 or the last one (5)
	if ( Count < 5 )
	{
		int32 FirstWeekendDay = 0;
		for ( int32 i = 1 ; i <= Count ; i++ )
		{
			FirstWeekendDay ++;
			CDateTime WeekendDay( Year , Month , FirstWeekendDay );
			if ( WeekendDay.DayOfWeek() >= DayOfWeek::Monday && WeekendDay.DayOfWeek() <= DayOfWeek::Friday )
				FirstWeekendDay += 6 - WeekendDay.DayOfWeek();
		}
		return FirstWeekendDay;
	}
	else
	{
		//we're interested in the last one in the month
		int32 LastWeekendDay = DaysInMonth ( Year , Month );
		CDateTime WeekendDay = CDateTime ( Year , Month , LastWeekendDay );
		if ( WeekendDay.DayOfWeek() >= DayOfWeek::Monday && WeekendDay.DayOfWeek() <= DayOfWeek::Friday )
			return LastWeekendDay - WeekendDay.DayOfWeek();
		else
			return LastWeekendDay;
	}
}

int32 WTCall CDateTime::SpecificDay ( int32 Year , int32 Month , DayOfWeek::Enum Day , int32 Count )
{
	//first see if we need the 1,2,3,4 or the last one (5)
	if ( Count < 5 )
	{
		int32 FirstDay = 0;
		for ( int32 i = 1 ; i <= Count ; i++ )
		{
			FirstDay ++;
			CDateTime WeekDay( Year , Month , FirstDay );
			if ( WeekDay.DayOfWeek() != Day )
				FirstDay += Day > WeekDay.DayOfWeek() ? Day - WeekDay.DayOfWeek() : 7 + Day - WeekDay.DayOfWeek();
		}
		return FirstDay;
	}
	else
	{
		//we're interested in the last one in the month
		int32 LastDay = DaysInMonth ( Year , Month );
		CDateTime WeekDay( Year , Month , LastDay );
		if ( WeekDay.DayOfWeek() != Day )
			return Day > WeekDay.DayOfWeek() ? LastDay - 7 + ( Day - WeekDay.DayOfWeek()) : LastDay - ( WeekDay.DayOfWeek() - Day );
		else
			return LastDay;
	}
}

// -------------------------------------------------------------------------------------
// Operators
// -------------------------------------------------------------------------------------

CDateTime CDateTime::operator+ ( const CTimeSpan& Span ) const
{
	return CDateTime ( m_Ticks + Span.m_Ticks );
}

CDateTime CDateTime::operator- ( const CTimeSpan& Span ) const
{
	return CDateTime ( m_Ticks - Span.m_Ticks );
}

CTimeSpan CDateTime::operator- ( const CDateTime& Other ) const
{
	return CTimeSpan ( m_Ticks - Other.m_Ticks );
}

bool CDateTime::operator== ( const CDateTime& Other ) const
{
	return m_Ticks == Other.m_Ticks ? true : false;
}

bool CDateTime::operator!= ( const CDateTime& Other ) const
{
	return m_Ticks != Other.m_Ticks ? true : false;
}

bool CDateTime::operator< ( const CDateTime& Other ) const
{
	return m_Ticks < Other.m_Ticks ? true : false;
}

bool CDateTime::operator<= ( const CDateTime& Other ) const
{
	return m_Ticks <= Other.m_Ticks ? true : false;
}

bool CDateTime::operator> ( const CDateTime& Other ) const
{
	return m_Ticks > Other.m_Ticks ? true : false;
}

bool CDateTime::operator>= ( const CDateTime& Other ) const
{
	return m_Ticks >= Other.m_Ticks ? true : false;
}

CDateTime& CDateTime::operator+= ( const CTimeSpan& Span )
{
	m_Ticks += Span.m_Ticks;
	return *this;
}

CDateTime& CDateTime::operator-= ( const CTimeSpan& Span )
{
	m_Ticks -= Span.m_Ticks;
	return *this;
}

// -------------------------------------------------------------------------------------
// Private Methods
// -------------------------------------------------------------------------------------

int64 WTCall CDateTime::DateToTicks ( int32 Year , int32 Month , int32 Day )
{
	const int32 *Days = IsLeapYear ( Year ) ? DaysToMonth366 : DaysToMonth365;
	int32 y = Year - 1;
	int32 n = y * 365 + y / 4 - y / 100 + y / 400 + Days[Month - 1] + Day - 1;
	return n * TicksPerDay;
}

int64 WTCall CDateTime::TimeToTicks ( int32 Hour, int32 Minute , int Second )
{
	return CTimeSpan::TimeToTicks ( Hour , Minute, Second );
}

bool WTCall CDateTime::IsLeapYear ( int32 Year )
{
	return Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0) ? true : false;
}

CDateTime CDateTime::Add ( float64 Value , int32 Scale ) const
{
	int64 Millis = (int64) ( Value * Scale + ( Value >= 0? 0.5: -0.5 ));
	return CDateTime ( m_Ticks + Millis * TicksPerMillisecond );
}

int32 CDateTime::GetDatePart ( int32 Part ) const
{
	// n = number of days since 1/1/0001
	int32 n = (int32) ( m_Ticks / TicksPerDay );
	// y400 = number of whole 400-year periods since 1/1/0001
	int32 y400 = n / DaysPer400Years;
	// n = day number within 400-year period
	n -= y400 * DaysPer400Years;
	// y100 = number of whole 100-year periods within 400-year period
	int32 y100 = n / DaysPer100Years;
	// Last 100-year period has an extra day, so decrement result if 4
	if ( y100 == 4 )
		y100 = 3;
	// n = day number within 100-year period
	n -= y100 * DaysPer100Years;
	// y4 = number of whole 4-year periods within 100-year period
	int32 y4 = n / DaysPer4Years;
	// n = day number within 4-year period
	n -= y4 * DaysPer4Years;
	// y1 = number of whole years within 4-year period
	int32 y1 = n / DaysPerYear;
	// Last year has an extra day, so decrement result if 4
	if ( y1 == 4 )
		y1 = 3;

	// If year was requested, compute and return it

	if ( Part == DatePartYear )
	{
		return y400 * 400 + y100 * 100 + y4 * 4 + y1 + 1;
	}

	// n = day number within year
	n -= y1 * DaysPerYear;

	// If day-of-year was requested, return it

	if ( Part == DatePartDayOfYear )
		return n + 1;

	// Leap year calculation looks different from IsLeapYear since y1, y4,
	// and y100 are relative to year 1, not year 0

	bool LeapYear = y1 == 3 && (y4 != 24 || y100 == 3);
	const int32 *Days = LeapYear ? DaysToMonth366 : DaysToMonth365;

	// All months have less than 32 days, so n >> 5 is a good conservative
	// estimate for the month

	int32 m = ( n >> 5 ) + 1;
	// m = 1-based month number

	while ( n >= Days[m] )
		m++;

	// If month was requested, return it
	if ( Part == DatePartMonth )
		return m;

	// Return 1-based day-of-month

	return n - Days[m - 1] + 1;
}


}
}