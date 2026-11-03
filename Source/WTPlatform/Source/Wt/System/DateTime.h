#pragma once

#include "WT/System/Object.h"
#include <string>

namespace WT
{
namespace System
{

namespace DayOfWeek
{
	enum Enum
	{ 
		Sunday    = 0,
		Monday    = 1,
		Tuesday   = 2,
		Wednesday = 3,
		Thursday  = 4,
		Friday    = 5,
		Saturday  = 6
	};
	// Returns RFC 822 (and RFC 1123) compatible day names
	const char* const WTCall ToStringRFC822 (const enum Enum);
}

namespace MonthOfYear
{
	// Returns RFC 822 (and RFC 1123) compatible month names given a 1-based month number
	const char* const WTCall ToStringRFC822 (const int Month);
}

class CDateTime;

class CTimeSpan
{
public:

	//Number of Ticks per millisecond ( 100 nano ticks, ten thousand or 10,000 )

	static const int64 TicksPerMillisecond = 10000;

	//Number of Ticks per second ( 10 million or 10,000,000 )

	static const int64 TicksPerSecond = TicksPerMillisecond * 1000;

	//number of Ticks per Minute ( 600 million or 600,000,000 )

	static const int64 TicksPerMinute = TicksPerSecond * 60;

	//number of Ticks per hour ( 36 billion or 36,000,000,000 )

	static const int64 TicksPerHour = TicksPerMinute * 60;

	//number of Ticks per day ( 864 billion or 864,000,000,000 )

	static const int64 TicksPerDay = TicksPerHour * 24;

	static const int64 Int64MaxValue = 0x7FFFFFFFFFFFFFFF;
	static const int64 Int64MinValue = 0x8000000000000000;

	static const int64 MaxSeconds = Int64MaxValue / TicksPerSecond;
	static const int64 MinSeconds = Int64MinValue / TicksPerSecond;

	static const int64 MaxMilliseconds = Int64MaxValue / TicksPerMillisecond;
	static const int64 MinMilliseconds = Int64MinValue / TicksPerMillisecond;

	CTimeSpan ();
	CTimeSpan ( int64 Ticks );
	CTimeSpan ( int32 Hours , int32 Minutes , int32 Seconds );
	CTimeSpan ( int32 Days , int32 Hours , int32 Minutes , int32 Seconds );
	CTimeSpan ( int32 Days , int32 Hours , int32 Minutes , int32 Seconds , int32 Milliseconds );

	//A zero time span value

	static const CTimeSpan Zero;

	//Maximum TimeSpan allowed

	static const CTimeSpan MaxValue;

	//Minimum TimeSpan allowed

	static const CTimeSpan MinValue;

	//return the Day component of the datetime

	int32 Days () const;

	//return the Hour component of the datetime

	int32 Hours () const;

	//return the Minutes component of the datetime

	int32 Minutes () const;

	//return the Second component of the datetime

	int32 Seconds () const;

	//return the Millisecond component of the datetime

	int32 Milliseconds () const;

	//return the actual Ticks value

	int64 Ticks () const;

	//get the total days

	float64 TotalDays () const;

	//get total hours

	float64 TotalHours () const;

	//get total minutes

	float64 TotalMinutes () const;

	//get total seconds

	float64 TotalSeconds () const;

	//get total milliseconds

	float64 TotalMilliseconds () const;

	//add the supplied timespan to this span, result in a new TimeSpan value

	CTimeSpan Add ( const CTimeSpan& Other ) const;

	//Negate the timespan and return a new value

	CTimeSpan Negate () const;

	//Subtract the supplied timespan from this span, result in a new timespan value

	CTimeSpan Subtract ( const CTimeSpan& Other ) const;

	//multiply the timespan by a given amount

	CTimeSpan Multiply ( float32 Value ) const;

	//Divide the timespan by a given amount

	CTimeSpan Divide ( float32 Value ) const;

	//compare two TimeSpans objects

	static int32 WTCall Compare ( const CTimeSpan& TimeSpan1 , const CTimeSpan& TimeSpan2 );

	//compare this timespan to another one, return 1 if greater, -1 if smaller, 0 if equal.

	int32 CompareTo ( const CTimeSpan& Other ) const;

	//return a TimeSpan that is the absolute duration of this time span.

	CTimeSpan Duration () const;

	//return cmKTrue if the timespans are equal.

	bool Equals ( const CTimeSpan& Other ) const;

    //convert to a simple string representation (mostly for debug and text serialization purposes)

	string ToString () const;

    //conversion routine to a TimeSpan from a cmString
    //String must be in the format "ndddd:hh:mm:ss.uuu", where n=sign, d=days, h=hours, m=minutes, s=seconds, u=milliseconds
    static CTimeSpan WTCall FromString( const string& String );
    
	//conversion function to get the a TimeSpan value from a time unit.

	static CTimeSpan WTCall FromDays ( float64 Value );
	static CTimeSpan WTCall FromHours ( float64 Value );
	static CTimeSpan WTCall FromMinutes ( float64 Value );
	static CTimeSpan WTCall FromSeconds ( float64 Value );
	static CTimeSpan WTCall FromMilliseconds ( float64 Value );
	static CTimeSpan WTCall FromTicks ( int64 Value );

	//operators:

    CTimeSpan  operator -  () const;
	CTimeSpan  operator +  ( const CTimeSpan& Other ) const;
	CTimeSpan  operator -  ( const CTimeSpan& Other ) const;
	CTimeSpan  operator *  ( float32 Value ) const;
	CTimeSpan  operator /  ( float32 Value ) const;
	bool   operator == ( const CTimeSpan& Other ) const;
	bool   operator != ( const CTimeSpan& Other ) const;
	bool   operator <  ( const CTimeSpan& Other ) const;
	bool   operator <= ( const CTimeSpan& Other ) const;
	bool   operator >  ( const CTimeSpan& Other ) const;
	bool   operator >= ( const CTimeSpan& Other ) const;
	CTimeSpan& operator += ( const CTimeSpan& Other );
	CTimeSpan& operator -= ( const CTimeSpan& Other );

private:	//consts

	static const int32 MillisPerSecond = 1000;
	static const int32 MillisPerMinute = MillisPerSecond * 60;
	static const int32 MillisPerHour = MillisPerMinute * 60;
	static const int32 MillisPerDay = MillisPerHour * 24;

private:	//functions

	static int64 WTCall TimeToTicks ( int Hours , int Minutes , int Seconds );
	static CTimeSpan WTCall Interval ( float64 Value , int Scale );

private:	//data

	int64 m_Ticks;
	friend CDateTime;
};

class CDateTime
{
public:

	static const CDateTime MinValue;
	static const CDateTime MaxValue;

	//constructors:

	//default init to 0 (start of time 1/1/0001)

	CDateTime ();

	//Init the instance to a specific 100-nanoseconds unit

	CDateTime ( int64 Ticks );

	//Init the instance with a Year, month and day values. Time is set to midnight (00:00:00)

	CDateTime ( int32 Year , int32 Month , int32 Day );

	//Init the instance with a date of Year, month, day values. Time to hour, minute, second

	CDateTime ( int32 Year , int32 Month , int32 Day , int32 Hour , int32 Minute , int32 Second );

	//Init the instance with a date of Year, month, day values. Time to hour, minute, second, millisecond

	CDateTime ( int32 Year , int32 Month , int32 Day , int32 Hour , int32 Minute , int32 Second , int32 Millisecond );

	//Add a time span to this date time, result in a new date time

	CDateTime Add ( const CTimeSpan& Span ) const;

	//Subtract a time span to this date time, result in a new date time

	CDateTime Subtract ( const CTimeSpan& Span ) const;

	//Subtract a another datetime from this one to result of a timespan

	CTimeSpan Subtract ( const CDateTime& Other ) const;

	// Returns the CDateTime resulting from adding a fractional number of
	// years to this CDateTime.

	CDateTime AddYears ( int32 Value ) const;

	// Returns the CDateTime resulting from adding a fractional number of
	// Months to this CDateTime.

	CDateTime AddMonths ( int32 Value ) const;

	// Returns the CDateTime resulting from adding a fractional number of
    // days to this CDateTime.

	CDateTime AddDays ( float64 Value ) const;

	// Returns the CDateTime resulting from adding a fractional number of
	// hours to this CDateTime.

	CDateTime AddHours ( float64 Value ) const;

	// Returns the CDateTime resulting from adding a fractional number of
	// minutes to this CDateTime.

	CDateTime AddMinutes ( float64 Value ) const;

	// Returns the CDateTime resulting from adding a fractional number of
	// seconds to this CDateTime.

	CDateTime AddSeconds ( float64 Value ) const;

	// Returns the CDateTime resulting from adding a fractional number of
	// milliseconds to this CDateTime.

	CDateTime AddMilliseconds ( float64 Value ) const;

	// Returns the CDateTime resulting from adding a number of
	// 100 nano-seconds ticks to this CDateTime.

	CDateTime AddTicks ( int64 Value ) const;

	//get the number of days in a specific month in a specific year.

	static int32 WTCall DaysInMonth ( int32 Year , int32 Month );

	//compate two CDateTime values to each other.

	static int32 WTCall Compare ( const CDateTime& DateTime1 , const CDateTime& DateTime2 );

	//compare this CDateTime to another

	int32 CompareTo ( const CDateTime& Other ) const;

	//return true if this CDateTime is equal

	bool Equals ( const CDateTime& Other ) const;

	//return the Date part of this DateTime - Time is set to 00:00:00 midnight

	CDateTime Date () const;

	//return a TimeSpan that represent the time of day (only the time component)

	CTimeSpan TimeOfDay () const;

	//return the Year component of the datetime

	int32 Year () const;

	//return the Month component of the datetime (1-12)

	int32 Month () const;

	//return the Day component of the datetime (1-31)

	int32 Day () const;

	//return the Hour component of the datetime (0-24)

	int32 Hour () const;

	//return the Minutes component of the datetime (0-59)

	int32 Minute () const;

	//return the Second component of the datetime (0-59)

	int32 Second () const;

	//return the Millisecond component of the datetime (0-999)

	int32 Millisecond () const;

	//return the Ticks of the datetime

	int64 Ticks () const;

	//return the Day of week from this CDateTime

	DayOfWeek::Enum DayOfWeek() const;

	//return the day number of the year of this CDateTime

	int32 DayOfYear () const;

	//return a datetime which is the start of the week containing this datetime
	//time is set to Sunday 12:00 midnight

	CDateTime StartOfWeek () const;

	//return a datetime which is the end of the week containing this datetime
	//time is set to Sunday 12:00 midnight

	CDateTime EndOfWeek () const;

	// Creates a CDateTime from a Windows filetime. A Windows filetime is
	// an int64 representing the date and time as the number of
	// 100-nanosecond intervals that have elapsed since 1/1/1601 12:00am.

	static CDateTime WTCall FromFileTime ( int64 FileTime );

	//Return a CDateTime that is now in local time

	static CDateTime WTCall Now ();

	//Return a CDateTime that is now in UTC time

	static CDateTime WTCall UTCNow ();

	//return a CDateTime that represent today - only the date part without the time

	static CDateTime WTCall Today();

	//Convert the UTC time to local time

	CDateTime ToLocalTime() const;

	//convert from local time to UTC time

	CDateTime ToUniversalTime() const;

	//convert to a simple string representation (mostly for debug purposes)

	string ToString () const;

	// convert to a specified format; as of 8/25/2005 only 'r' and 'R' (RFC 1123)
	// formats are supported

	string ToString (const string& Format) const;

    //conversion routine to a CDateTime from a string
    //String must be in the format "hh:mm:ss.uuu dd/mm/yyyy", where h=hours, m=minutes, s=seconds, u=milliseconds, d=day,m=month,y=year
    static CDateTime WTCall FromString ( const string& String );

	//returns the integer number of months which is the difference between the
	//CDateTime param and this instance. e.g. the result for CDateTime=March 3,2000 and
	//this instance of July 7,1999 in integral month is: 8

	int32 MonthsInterval ( const CDateTime& Other ) const;

	//returns the integer number of weeks which is the difference between the
	//CDateTime param and this instance.

	int32 WeeksInterval ( const CDateTime& Other ) const;

	//returns the first day of the month

	static DayOfWeek::Enum WTCall FirstDayOfMonth ( int32 Year , int32 Month );

	//returns the last day of the month

	static DayOfWeek::Enum WTCall LastDayOfMonth ( int32 Year , int32 Month );

	//return a specific weekday occurance (1st, 2nd, 3rd, 4th or last (5) of the month

	static int32 WTCall SpecificWeekDay ( int32 Year , int32 Month , int32 Count );

	//return a specific weekend day occurance (1st, 2nd, 3rd, 4th or last (5) of the month

	static int32 WTCall SpecificWeekendDay ( int32 Year , int32 Month , int32 Count );

	//return a specific day occurance (1st, 2nd, 3rd, 4th or last (5) of the month

	static int32 WTCall SpecificDay ( int32 Year , int32 Month , DayOfWeek::Enum Day , int32 Count );

	//operators:

	CDateTime  operator +  ( const CTimeSpan& Span ) const;
	CDateTime  operator -  ( const CTimeSpan& Span ) const;
	CTimeSpan  operator -  ( const CDateTime& Other ) const;
	bool   operator == ( const CDateTime& Other ) const;
	bool   operator != ( const CDateTime& Other ) const;
	bool   operator <  ( const CDateTime& Other ) const;
	bool   operator <= ( const CDateTime& Other ) const;
	bool   operator >  ( const CDateTime& Other ) const;
	bool   operator >= ( const CDateTime& Other ) const;
	CDateTime& operator += ( const CTimeSpan& Span );
	CDateTime& operator -= ( const CTimeSpan& Span );

private:

	// Number of 100ns ticks per time unit

	static const int64 TicksPerMillisecond = 10000;
	static const int64 TicksPerSecond = TicksPerMillisecond * 1000;
	static const int64 TicksPerMinute = TicksPerSecond * 60;
	static const int64 TicksPerHour = TicksPerMinute * 60;
	static const int64 TicksPerDay = TicksPerHour * 24;

	// Number of milliseconds per time unit

	static const int32 MillisPerSecond = 1000;
	static const int32 MillisPerMinute = MillisPerSecond * 60;
	static const int32 MillisPerHour = MillisPerMinute * 60;
	static const int32 MillisPerDay = MillisPerHour * 24;

	// Number of days in a non-leap year

	static const int32 DaysPerYear = 365;

	// Number of days in 4 years

	static const int32 DaysPer4Years = DaysPerYear * 4 + 1;

	// Number of days in 100 years

	static const int32 DaysPer100Years = DaysPer4Years * 25 - 1;

	// Number of days in 400 years

	static const int32 DaysPer400Years = DaysPer100Years * 4 + 1;

	// Number of days from 1/1/0001 to 12/31/1600

	static const int32 DaysTo1601 = DaysPer400Years * 4;

	// Number of days from 1/1/0001 to 12/30/1899

	static const int32 DaysTo1899 = DaysPer400Years * 4 + DaysPer100Years * 3 - 367;

	// Number of days from 1/1/0001 to 12/31/9999

	static const int32 DaysTo10000 = DaysPer400Years * 25 - 366;

	// offset to add to file time data.

	static const int64 FileTimeOffset = DaysTo1601 * TicksPerDay;

	static const int64 MinTicks = 0;
	static const int64 MaxTicks = DaysTo10000 * TicksPerDay - 1;
	static const int64 MaxMillis = (int64) DaysTo10000 * MillisPerDay;

	static const int32 DatePartYear = 0;
	static const int32 DatePartDayOfYear = 1;
	static const int32 DatePartMonth = 2;
	static const int32 DatePartDay = 3;
	static const int32 DaysToMonth365[13];
	static const int32 DaysToMonth366[13];

	int64 m_Ticks;

private:

	static int64 WTCall DateToTicks ( int32 Year , int32 Month , int32 Day );
	static int64 WTCall TimeToTicks ( int32 Hour, int32 Minute , int Second );
	static bool WTCall IsLeapYear ( int32 Year );
	CDateTime Add ( float64 Value , int32 Scale ) const;
	int32 GetDatePart ( int32 Part ) const;
};

}
}