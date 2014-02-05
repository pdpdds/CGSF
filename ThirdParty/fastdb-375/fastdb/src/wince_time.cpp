#define INSIDE_GIGABASE

#include <winbase.h>
#include "wince_time.h"

/* non-zero if daylight savings time is used */
_CRTIMP int _daylight;

/* offset for Daylight Saving Time */
_CRTIMP long _dstbias;

/* difference in seconds between GMT and local time */
_CRTIMP long _timezone;

/* standard/daylight savings time zone names */
_CRTIMP char * _tzname[2];


struct __lc_time_data {
        char *wday_abbr[7];
        char *wday[7];
        char *month_abbr[12];
        char *month[12];
        char *ampm[2];
        char *ww_sdatefmt;
        char *ww_ldatefmt;
        char *ww_timefmt;
};


static int _lpdays[] = {
        -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

static int _days[] = {
        -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
};

/*
 * Flag indicating that time zone information came from GetTimeZoneInformation
 * API call.
 */
static int tzapiused;

static TIME_ZONE_INFORMATION tzinfo;

/* Prototypes for local routines */
static void __cdecl _expandtime (char specifier, const struct tm *tmptr,
        char **out, size_t *count, struct __lc_time_data *lc_time);
static void __cdecl _store_str (char *in, char **out, size_t *count);
static void __cdecl _store_num (int num, int digits, char **out, size_t *count);
static void __cdecl _store_number (int num, char **out, size_t *count);
static void __cdecl _store_winword (const char *format, const struct tm *tmptr, char **out, size_t *count, struct __lc_time_data *lc_time);

time_t __cdecl __loctotime_t (
                int yr,         /* 0 based */
        int mo,         /* 1 based */
        int dy,         /* 1 based */
        int hr,
        int mn,
        int sc,
        int dstflag );
int __cdecl _isindst(struct tm *);
void __cdecl __tzset(void);

/*
 * DST start and end structs.
 */
static transitiondate dststart = { -1, 0, 0L };
static transitiondate dstend   = { -1, 0, 0L };

/*
 *  Code page.
 */
UINT __lc_codepage = _CLOCALECP;                /* CP_ACP */


/*
time_t __cdecl time(time_t *pTime)
{
        // what a mess for getting a simple time!
        SYSTEMTIME lTime;
        FILETIME lFileTime;

        ::GetLocalTime(&lTime);
        SystemTimeToFileTime(&lTime, &lFileTime);
        ULARGE_INTEGER lNTime = *(ULARGE_INTEGER*)&lFileTime;
        if (0l != pTime)
                *pTime = *(time_t*)&lNTime;
        return *(time_t*)&lNTime;
}
*/

/* LC_TIME data for local "C" */

__declspec(selectany) struct __lc_time_data __lc_time_c = {

        {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},

        {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
                "Friday", "Saturday", },

        {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
                "Sep", "Oct", "Nov", "Dec"},

        {"January", "February", "March", "April", "May", "June",
                "July", "August", "September", "October",
                "November", "December"},

        {"AM", "PM"}

        , { "M/d/yy" }
        , { "dddd, MMMM dd, yyyy" }
        , { "H:mm:ss" }
        };

/* Pointer to the current LC_TIME data structure. */

struct __lc_time_data *__lc_time_curr = &__lc_time_c;


/* Flags */
static unsigned __alternate_form;
static unsigned __no_lead_zeros;


time_t __cdecl mktime(struct tm * lTms)
{
        SYSTEMTIME lSTime;
        lSTime.wYear = lTms->tm_year + 1900;
        lSTime.wMonth = lTms->tm_mon + 1;
        lSTime.wDay = lTms->tm_mday;
        lSTime.wDayOfWeek = lTms->tm_wday;
        lSTime.wHour = lTms->tm_hour;
        lSTime.wMinute = lTms->tm_min;
        lSTime.wSecond = lTms->tm_sec;
        lSTime.wMilliseconds = 0;

        FILETIME lFileTime;

        SystemTimeToFileTime(&lSTime, &lFileTime);
        ULARGE_INTEGER lNTime = *(ULARGE_INTEGER*)&lFileTime;

        //TODO: update the values of wDay and yDay in the 
        //      original structure.
        return *(time_t*)&lNTime;
}

/*
struct tm * __cdecl localtime(const time_t* pTime)
{
        FILETIME lFileTime = *(FILETIME *)&pTime;
        FILETIME lLocalFileTime;

        ::FileTimeToLocalFileTime(&lFileTime, &lLocalFileTime);

        SYSTEMTIME lSTime;
        if (::FileTimeToSystemTime(&lLocalFimeTime, &lSTime) != 0)
        {
                struct tm* lTms = new struct tm;

                lTms->tm_year   =       lSTime.wYear - 1900;
                lTms->tm_mon    =       lSTime.wMonth - 1;
                lTms->tm_mday   =       lSTime.wDay;
                lTms->tm_wday   =       lSTime.wDayOfWeek;
                lTms->tm_hour   =       lSTime.wHour;
                lTms->tm_min    =       lSTime.wMinute;
                lTms->tm_sec    =       lSTime.wSecond;

                return lTms;
        }
        return 0l;
}

size_t __cdecl wcsftime(wchar_t *pDest, size_t maxsize, const wchar_t *pFormat,
        const struct tm *pTime)
{
        
}
*/

/***
*size_t _Strftime(string, maxsize, format,
*       timeptr, lc_time) - Format a time string for a given locale
*
*Purpose:
*       Place characters into the user's output buffer expanding time
*       format directives as described in the user's control string.
*       Use the supplied 'tm' structure for time data when expanding
*       the format directives. use the locale information at lc_time.
*       [ANSI]
*
*Entry:
*       char *string = pointer to output string
*       size_t maxsize = max length of string
*       const char *format = format control string
*       const struct tm *timeptr = pointer to tb data structure
*               struct __lc_time_data *lc_time = pointer to locale-specific info
*                       (passed as void * to avoid type mismatch with C++)
*
*Exit:
*       !0 = If the total number of resulting characters including the
*       terminating null is not more than 'maxsize', then return the
*       number of chars placed in the 'string' array (not including the
*       null terminator).
*
*       0 = Otherwise, return 0 and the contents of the string are
*       indeterminate.
*
*Exceptions:
*
*******************************************************************************/

size_t __cdecl _Strftime (
        char *string,
        size_t maxsize,
        const char *format,
        const struct tm *timeptr,
                void *lc_time_arg
        )
{
        struct __lc_time_data *lc_time;

        size_t left;                    /* space left in output string */

        /* Copy maxsize into temp. */
        left = maxsize;

        /* Defer initialization until the crtitical section has been entered */
        lc_time = lc_time_arg == 0 ? __lc_time_curr : (struct __lc_time_data *)lc_time_arg;

        /* Copy the input string to the output string expanding the format
        designations appropriately.  Stop copying when one of the following
        is true: (1) we hit a null char in the input stream, or (2) there's
        no room left in the output stream. */

        while (left > 0)
        {
                switch(*format)
                {

                case('\0'):

                        /* end of format input string */
                        goto done;

                case('%'):

                        /* Format directive.  Take appropriate action based
                        on format control character. */

                        format++;                       /* skip over % char */

                        /* process flags */
                        __alternate_form = 0;
                        if (*format == '#')
                        {
                                __alternate_form = 1;
                                format++;
                        }
                        _expandtime(*format, timeptr, &string, &left,
                                                        lc_time);
                        format++;                       /* skip format char */
                        break;


                default:

                        /* store character, bump pointers, dec the char count */
                        if (isleadbyte((int)(*format)) && left > 1)
                        {
                                *string++ = *format++;
                                left--;
                        }
                        *string++ = *format++;
                        left--;
                        break;
                }
        }


        /* All done.  See if we terminated because we hit a null char or because
        we ran out of space */

        done:

        if (left > 0) {

                /* Store a terminating null char and return the number of chars
                we stored in the output string. */

                *string = '\0';
                return(maxsize-left);
        }

        else
                return(0);

}


/***
*_expandtime() - Expand the conversion specifier
*
*Purpose:
*       Expand the given strftime conversion specifier using the time struct
*       and store it in the supplied buffer.
*
*       The expansion is locale-dependent.
*
*       *** For internal use with strftime() only ***
*
*Entry:
*       char specifier = strftime conversion specifier to expand
*       const struct tm *tmptr = pointer to time/date structure
*       char **string = address of pointer to output string
*       size_t *count = address of char count (space in output area)
*       struct __lc_time_data *lc_time = pointer to locale-specific info
*
*Exit:
*       none
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl _expandtime (
        char specifier,
        const struct tm *timeptr,
        char **string,
        size_t *left,
        struct __lc_time_data *lc_time
        )
{
        unsigned temp;                  /* temps */
        int wdaytemp;

        /* Use a copy of the appropriate __lc_time_data pointer.  This
        should prevent the necessity of locking/unlocking in mthread
        code (if we can guarantee that the various __lc_time data
        structures are always in the same segment). contents of time
        strings structure can now change, so thus we do use locking */

        switch(specifier) {             /* switch on specifier */

                case('a'):              /* abbreviated weekday name */
                        _store_str((char *)(lc_time->wday_abbr[timeptr->tm_wday]),
                                 string, left);
                        break;

                case('A'):              /* full weekday name */
                        _store_str((char *)(lc_time->wday[timeptr->tm_wday]),
                                 string, left);
                        break;

                case('b'):              /* abbreviated month name */
                        _store_str((char *)(lc_time->month_abbr[timeptr->tm_mon]),
                                 string, left);
                        break;

                case('B'):              /* full month name */
                        _store_str((char *)(lc_time->month[timeptr->tm_mon]),
                                 string, left);
                        break;

                case('c'):              /* date and time display */
                        if (__alternate_form)
                        {
                                __alternate_form = FALSE;
                                _store_winword(lc_time->ww_ldatefmt, timeptr, string, left, lc_time);
                                if (*left == 0)
                                        return;
                                *(*string)++=' ';
                                (*left)--;
                                _store_winword(lc_time->ww_timefmt, timeptr, string, left, lc_time);
                        }
                        else {
                                _store_winword(lc_time->ww_sdatefmt, timeptr, string, left, lc_time);
                                if (*left == 0)
                                        return;
                                *(*string)++=' ';
                                (*left)--;
                                _store_winword(lc_time->ww_timefmt, timeptr, string, left, lc_time);
                        }
                        break;

                case('d'):              /* mday in decimal (01-31) */
                        __no_lead_zeros = __alternate_form;
                        _store_num(timeptr->tm_mday, 2, string, left);
                        break;

                case('H'):              /* 24-hour decimal (00-23) */
                        __no_lead_zeros = __alternate_form;
                        _store_num(timeptr->tm_hour, 2, string, left);
                        break;

                case('I'):              /* 12-hour decimal (01-12) */
                        __no_lead_zeros = __alternate_form;
                        if (!(temp = timeptr->tm_hour%12))
                                temp=12;
                        _store_num(temp, 2, string, left);
                        break;

                case('j'):              /* yday in decimal (001-366) */
                        __no_lead_zeros = __alternate_form;
                        _store_num(timeptr->tm_yday+1, 3, string, left);
                        break;

                case('m'):              /* month in decimal (01-12) */
                        __no_lead_zeros = __alternate_form;
                        _store_num(timeptr->tm_mon+1, 2, string, left);
                        break;

                case('M'):              /* minute in decimal (00-59) */
                        __no_lead_zeros = __alternate_form;
                        _store_num(timeptr->tm_min, 2, string, left);
                        break;

                case('p'):              /* AM/PM designation */
                        if (timeptr->tm_hour <= 11)
                            _store_str((char *)(lc_time->ampm[0]), string, left);
                        else
                            _store_str((char *)(lc_time->ampm[1]), string, left);
                        break;

                case('S'):              /* secs in decimal (00-59) */
                        __no_lead_zeros = __alternate_form;
                        _store_num(timeptr->tm_sec, 2, string, left);
                        break;

                case('U'):              /* sunday week number (00-53) */
                        __no_lead_zeros = __alternate_form;
                        wdaytemp = timeptr->tm_wday;
                        goto weeknum;   /* join common code */

                case('w'):              /* week day in decimal (0-6) */
                        __no_lead_zeros = __alternate_form;
                        _store_num(timeptr->tm_wday, 1, string, left);
                        break;

                case('W'):              /* monday week number (00-53) */
                        __no_lead_zeros = __alternate_form;
                        if (timeptr->tm_wday == 0)  /* monday based */
                                wdaytemp = 6;
                        else
                                wdaytemp = timeptr->tm_wday-1;
                weeknum:
                        if (timeptr->tm_yday < wdaytemp)
                                temp=0;
                        else {
                                temp = timeptr->tm_yday/7;
                                if ((timeptr->tm_yday%7) >= wdaytemp)
                                        temp++;
                                }
                        _store_num(temp, 2, string, left);
                        break;

                case('x'):              /* date display */
                        if (__alternate_form)
                        {
                                __alternate_form = FALSE;
                                _store_winword(lc_time->ww_ldatefmt, timeptr, string, left, lc_time);
                        }
                        else
                        {
                                _store_winword(lc_time->ww_sdatefmt, timeptr, string, left, lc_time);
                        }
                        break;

                case('X'):              /* time display */
                        __alternate_form = FALSE;
                        _store_winword(lc_time->ww_timefmt, timeptr, string, left, lc_time);
                        break;

                case('y'):              /* year w/o century (00-99) */
                        __no_lead_zeros = __alternate_form;
                        temp = timeptr->tm_year%100;
                        _store_num(temp, 2, string, left);
                        break;

                case('Y'):              /* year w/ century */
                        __no_lead_zeros = __alternate_form;
                        temp = (((timeptr->tm_year/100)+19)*100) +
                               (timeptr->tm_year%100);
                        _store_num(temp, 4, string, left);
                        break;

                case('Z'):              /* time zone name, if any */
                case('z'):              /* time zone name, if any */
                        __tzset();      /* Set time zone info */
                        _store_str(_tzname[((timeptr->tm_isdst)?1:0)],
                                 string, left);
                        break;

                case('%'):              /* percent sign */
                        *(*string)++ = '%';
                        (*left)--;
                        break;

                default:                /* unknown format directive */
                        /* ignore the directive and continue */
                        /* [ANSI: Behavior is undefined.]    */
                        break;

        }       /* end % switch */
}


/*
 * Cache holding the last time (GMT) for which the Daylight time status was
 * determined by an API call.
 */
static SYSTEMTIME gmt_cache;

/*
 * Three values of dstflag_cache and dstflag (local variable in code
 * below)
 */
#define DAYLIGHT_TIME   1
#define STANDARD_TIME   0
#define UNKNOWN_TIME    -1

static int dstflag_cache;

/***
*time_t time(timeptr) - Get current system time and convert to time_t value.
*
*Purpose:
*       Gets the current date and time and stores it in internal (time_t)
*       format. The time is returned and stored via the pointer passed in
*       timeptr. If timeptr == NULL, the time is only returned, not stored in
*       *timeptr. The internal (time_t) format is the number of seconds since
*       00:00:00, Jan 1 1970 (UTC).
*
*       Note: We cannot use GetSystemTime since its return is ambiguous. In
*       Windows NT, in return UTC. In Win32S, probably also Win32C, it
*       returns local time.
*
*Entry:
*       time_t *timeptr - pointer to long to store time in.
*
*Exit:
*       returns the current time.
*
*Exceptions:
*
*******************************************************************************/

time_t __cdecl time (
        time_t *timeptr
        )
{
        time_t tim;

        SYSTEMTIME loct, gmt;
        TIME_ZONE_INFORMATION tzinfo;
        DWORD tzstate;
        int dstflag;

        /*
         * Get local time from Win32
         */
        GetLocalTime( &loct );

        /*
         * Determine whether or not the local time is a Daylight Saving
         * Time. On Windows NT, the GetTimeZoneInformation API is *VERY*
         * expensive. The scheme below is intended to avoid this API call in
         * many important case by caching the GMT value and dstflag.In a
         * subsequent call to time(), the cached value of dstflag is used
         * unless the new GMT differs from the cached value at least in the
         * minutes place.
         */
        GetSystemTime( &gmt );

        if ( (gmt.wMinute == gmt_cache.wMinute) &&
             (gmt.wHour == gmt_cache.wHour) &&
             (gmt.wDay == gmt_cache.wDay) &&
             (gmt.wMonth == gmt_cache.wMonth) &&
             (gmt.wYear == gmt_cache.wYear) )
        {
            dstflag = dstflag_cache;
        }
        else
        {
            if ( (tzstate = GetTimeZoneInformation( &tzinfo )) != 0xFFFFFFFF )
            {
                /*
                 * Must be very careful in determining whether or not DST is
                 * really in effect.
                 */
                if ( (tzstate == TIME_ZONE_ID_DAYLIGHT) &&
                     (tzinfo.DaylightDate.wMonth != 0) &&
                     (tzinfo.DaylightBias != 0) )
                    dstflag = DAYLIGHT_TIME;
                else
                    /*
                     * When in doubt, assume standard time
                     */
                    dstflag = STANDARD_TIME;
            }
            else
                dstflag = UNKNOWN_TIME;

            dstflag_cache = dstflag;
            gmt_cache = gmt;
        }

        /* convert using our private routine */

        tim = __loctotime_t( (int)loct.wYear,
                             (int)loct.wMonth,
                             (int)loct.wDay,
                             (int)loct.wHour,
                             (int)loct.wMinute,
                             (int)loct.wSecond,
                             dstflag );

        if (timeptr)
                *timeptr = tim;         /* store time if requested */

        return tim;
}


/***
*time_t __loctotime_t(yr, mo, dy, hr, mn, sc, dstflag) - converts OS local
*       time to internal time format (i.e., a time_t value)
*
*Purpose:
*       Converts a local time value, obtained in a broken down format from
*       the host OS, to time_t format (i.e., the number elapsed seconds since
*       01-01-70, 00:00:00, UTC).
*
*Entry:
*       int yr, mo, dy -    date
*       int hr, mn, sc -    time
*       int dstflag    -    1 if Daylight Time, 0 if Standard Time, -1 if
*                           not specified.
*
*Exit:
*       Returns calendar time value.
*
*Exceptions:
*
*******************************************************************************/

time_t __cdecl __loctotime_t (
        int yr,         /* 0 based */
        int mo,         /* 1 based */
        int dy,         /* 1 based */
        int hr,
        int mn,
        int sc,
        int dstflag )
{
        int tmpdays;
        long tmptim;
        struct tm tb;

        /*
         * Do a quick range check on the year and convert it to a delta
         * off of 1900.
         */
        if ( ((long)(yr -= 1900) < _BASE_YEAR) || ((long)yr > _MAX_YEAR) )
                return (time_t)(-1);

        /*
         * Compute the number of elapsed days in the current year. Note the
         * test for a leap year would fail in the year 2100, if this was in
         * range (which it isn't).
         */
        tmpdays = dy + _days[mo - 1];
        if ( !(yr & 3) && (mo > 2) )
                tmpdays++;

        /*
         * Compute the number of elapsed seconds since the Epoch. Note the
         * computation of elapsed leap years would break down after 2100
         * if such values were in range (fortunately, they aren't).
         */
        tmptim = /* 365 days for each year */
                 (((long)yr - _BASE_YEAR) * 365L

                 /* one day for each elapsed leap year */
                 + (long)((yr - 1) >> 2) - _LEAP_YEAR_ADJUST

                 /* number of elapsed days in yr */
                 + tmpdays)

                 /* convert to hours and add in hr */
                 * 24L + hr;

        tmptim = /* convert to minutes and add in mn */
                 (tmptim * 60L + mn)

                 /* convert to seconds and add in sec */
                 * 60L + sc;
        /*
         * Account for time zone.
         */
        __tzset();
        tmptim += _timezone;

        /*
         * Fill in enough fields of tb for _isindst(), then call it to
         * determine DST.
         */
        tb.tm_yday = tmpdays;
        tb.tm_year = yr;
        tb.tm_mon  = mo - 1;
        tb.tm_hour = hr;
        if ( (dstflag == 1) || ((dstflag == -1) && _daylight &&
                                _isindst(&tb)) )
                tmptim += _dstbias;
        return(tmptim);
}

/***
*static void cvtdate( trantype, datetype, year, month, week, dayofweek,
*                     date, hour, min, second, millisec ) - convert
*       transition date format
*
*Purpose:
*       Convert the format of a transition date specification to a value of
*       a transitiondate structure.
*
*Entry:
*       int trantype    - 1, if it is the start of DST
*                         0, if is the end of DST (in which case the date is
*                            is a DST date)
*       int datetype    - 1, if a day-in-month format is specified.
*                         0, if an absolute date is specified.
*       int year        - year for which the date is being converted (70 ==
*                         1970)
*       int month       - month (0 == January)
*       int week        - week of month, if datetype == 1 (note that 5== last
*                         week of month),
*                         0, otherwise.
*       int dayofweek   - day of week (0 == Sunday), if datetype == 1.
*                         0, otherwise.
*       int date        - date of month (1 - 31)
*       int hour        - hours (0 - 23)
*       int min         - minutes (0 - 59)
*       int sec         - seconds (0 - 59)
*       int msec        - milliseconds (0 - 999)
*
*Exit:
*       dststart or dstend is filled in with the converted date.
*
*******************************************************************************/

static void __cdecl cvtdate (
        int trantype,
        int datetype,
        int year,
        int month,
        int week,
        int dayofweek,
        int date,
        int hour,
        int min,
        int sec,
        int msec
        )
{
        int yearday;
        int monthdow;

        if ( datetype == 1 ) {

            /*
             * Transition day specified in day-in-month format.
             */

            /*
             * Figure the year-day of the start of the month.
             */
            yearday = 1 + (IS_LEAP_YEAR(year) ? _lpdays[month - 1] :
                      _days[month - 1]);

            /*
             * Figure the day of the week of the start of the month.
             */
            monthdow = (yearday + ((year - 70) * 365) + ((year - 1) >> 2) -
                        _LEAP_YEAR_ADJUST + _BASE_DOW) % 7;

            /*
             * Figure the year-day of the transition date
             */
            if ( monthdow <= dayofweek )
                yearday += (dayofweek - monthdow) + (week - 1) * 7;
            else
                yearday += (dayofweek - monthdow) + week * 7;

            /*
             * May have to adjust the calculation above if week == 5 (meaning
             * the last instance of the day in the month). Check if year falls
             * beyond after month and adjust accordingly.
             */
            if ( (week == 5) &&
                 (yearday > (IS_LEAP_YEAR(year) ? _lpdays[month] :
                             _days[month])) )
            {
                yearday -= 7;
            }
        }
        else {
            /*
             * Transition day specified as an absolute day
             */
            yearday = IS_LEAP_YEAR(year) ? _lpdays[month - 1] :
                      _days[month - 1];

            yearday += date;
        }

        if ( trantype == 1 ) {
            /*
             * Converted date was for the start of DST
             */
            dststart.yd = yearday;
            dststart.ms = (long)msec +
                          (1000L * (sec + 60L * (min + 60L * hour)));
            /*
             * Set year field of dststart so that unnecessary calls to
             * cvtdate() may be avoided.
             */
            dststart.yr = year;
        }
        else {
            /*
             * Converted date was for the end of DST
             */
            dstend.yd = yearday;
            dstend.ms = (long)msec +
                              (1000L * (sec + 60L * (min + 60L * hour)));
            /*
             * The converted date is still a DST date. Must convert to a
             * standard (local) date while being careful the millisecond field
             * does not overflow or underflow.
             */
            if ( (dstend.ms += (_dstbias * 1000L)) < 0 ) {
                dstend.ms += DAY_MILLISEC;
                dstend.yd--;
            }
            else if ( dstend.ms >= DAY_MILLISEC ) {
                dstend.ms -= DAY_MILLISEC;
                dstend.yd++;
            }

            /*
             * Set year field of dstend so that unnecessary calls to cvtdate()
             * may be avoided.
             */
            dstend.yr = year;
        }

        return;
}

/***
*int _isindst(tb) - determine if broken-down time falls in DST
*
*Purpose:
*       Determine if the given broken-down time falls within daylight saving
*       time (DST). The DST rules are either obtained from Win32 (tzapiused !=
*       TRUE) or assumed to be USA rules, post 1986.
*
*       If the DST rules are obtained from Win32's GetTimeZoneInformation API,
*       the transition dates to/from DST can be specified in either of two
*       formats. First, a day-in-month format, similar to the way USA rules
*       are specified, can be used. The transition date is given as the n-th
*       occurence of a specified day of the week in a specified month. Second,
*       an absolute date can be specified. The two cases are distinguished by
*       the value of wYear field in the SYSTEMTIME structure (0 denotes a
*       day-in-month format).
*
*       USA rules for DST are that a time is in DST iff it is on or after
*       02:00 on the first Sunday in April, and before 01:00 on the last
*       Sunday in October.
*
*Entry:
*       struct tm *tb - structure holding broken-down time value
*
*Exit:
*       1, if time represented is in DST
*       0, otherwise
*
*******************************************************************************/

int __cdecl _isindst (
        struct tm *tb
        )
{
        long ms;

        if ( _daylight == 0 )
            return 0;

        /*
         * Compute (recompute) the transition dates for daylight saving time
         * if necessary.The yr (year) fields of dststart and dstend is
         * compared to the year of interest to determine necessity.
         */
        if ( (tb->tm_year != dststart.yr) || (tb->tm_year != dstend.yr) ) {
            if ( tzapiused ) {
                /*
                 * Convert the start of daylight saving time to dststart.
                 */
                if ( tzinfo.DaylightDate.wYear == 0 )
                    cvtdate( 1,
                             1,             /* day-in-month format */
                             tb->tm_year,
                             tzinfo.DaylightDate.wMonth,
                             tzinfo.DaylightDate.wDay,
                             tzinfo.DaylightDate.wDayOfWeek,
                             0,
                             tzinfo.DaylightDate.wHour,
                             tzinfo.DaylightDate.wMinute,
                             tzinfo.DaylightDate.wSecond,
                             tzinfo.DaylightDate.wMilliseconds );
                else
                    cvtdate( 1,
                             0,             /* absolute date */
                             tb->tm_year,
                             tzinfo.DaylightDate.wMonth,
                             0,
                             0,
                             tzinfo.DaylightDate.wDay,
                             tzinfo.DaylightDate.wHour,
                             tzinfo.DaylightDate.wMinute,
                             tzinfo.DaylightDate.wSecond,
                             tzinfo.DaylightDate.wMilliseconds );
                /*
                 * Convert start of standard time to dstend.
                 */
                if ( tzinfo.StandardDate.wYear == 0 )
                    cvtdate( 0,
                             1,             /* day-in-month format */
                             tb->tm_year,
                             tzinfo.StandardDate.wMonth,
                             tzinfo.StandardDate.wDay,
                             tzinfo.StandardDate.wDayOfWeek,
                             0,
                             tzinfo.StandardDate.wHour,
                             tzinfo.StandardDate.wMinute,
                             tzinfo.StandardDate.wSecond,
                             tzinfo.StandardDate.wMilliseconds );
                else
                    cvtdate( 0,
                             0,             /* absolute date */
                             tb->tm_year,
                             tzinfo.StandardDate.wMonth,
                             0,
                             0,
                             tzinfo.StandardDate.wDay,
                             tzinfo.StandardDate.wHour,
                             tzinfo.StandardDate.wMinute,
                             tzinfo.StandardDate.wSecond,
                             tzinfo.StandardDate.wMilliseconds );

            }
            else {
                /*
                 * GetTimeZoneInformation API was NOT used, or failed. USA
                 * daylight saving time rules are assumed.
                 */
                cvtdate( 1,
                         1,
                         tb->tm_year,
                         4,                 /* April */
                         1,                 /* first... */
                         0,                 /* ...Sunday */
                         0,
                         2,                 /* 02:00 (2 AM) */
                         0,
                         0,
                         0 );

                cvtdate( 0,
                         1,
                         tb->tm_year,
                         10,                /* October */
                         5,                 /* last... */
                         0,                 /* ...Sunday */
                         0,
                         2,                 /* 02:00 (2 AM) */
                         0,
                         0,
                         0 );
            }
        }

        /*
         * Handle simple cases first.
         */
        if ( dststart.yd < dstend.yd ) {
            /*
             * Northern hemisphere ordering
             */
            if ( (tb->tm_yday < dststart.yd) || (tb->tm_yday > dstend.yd) )
                return 0;
            if ( (tb->tm_yday > dststart.yd) && (tb->tm_yday < dstend.yd) )
                return 1;
        }
        else {
            /*
             * Southern hemisphere ordering
             */
            if ( (tb->tm_yday < dstend.yd) || (tb->tm_yday > dststart.yd) )
                return 1;
            if ( (tb->tm_yday > dstend.yd) && (tb->tm_yday < dststart.yd) )
                return 0;
        }

        ms = 1000L * (tb->tm_sec + 60L * tb->tm_min + 3600L * tb->tm_hour);

        if ( tb->tm_yday == dststart.yd ) {
            if ( ms >= dststart.ms )
                return 1;
            else
                return 0;
        }
        else {
            /*
             * tb->tm_yday == dstend.yd
             */
            if ( ms < dstend.ms )
                return 1;
            else
                return 0;
        }

}

/***
*_store_num() - Convert a number to ascii and copy it
*
*Purpose:
*       Convert the supplied number to decimal and store
*       in the output buffer.  Update both the count and
*       buffer pointers.
*
*       *** For internal use with strftime() only ***
*
*Entry:
*       int num = pointer to integer value
*       int digits = # of ascii digits to put into string
*       char **out = address of pointer to output string
*       size_t *count = address of char count (space in output area)
*
*Exit:
*       none
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_num (
        int num,
        int digits,
        char **out,
        size_t *count
        )
{
int temp=0;

        if (__no_lead_zeros) {
                _store_number (num, out, count);
                return;
        }

        if ((size_t)digits < *count)  {
                for (digits--; (digits+1); digits--) {
                        (*out)[digits] = (char)('0' + num % 10);
                        num /= 10;
                        temp++;
                }
                *out += temp;
                *count -= temp;
        }
else
        *count = 0;
}

/***
*_store_str() - Copy a time string
*
*Purpose:
*       Copy the supplied time string into the output string until
*       (1) we hit a null in the time string, or (2) the given count
*       goes to 0.
*
*       *** For internal use with strftime() only ***
*
*Entry:
*       char *in = pointer to null terminated time string
*       char **out = address of pointer to output string
*       size_t *count = address of char count (space in output area)
*
*Exit:
*       none
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_str (
        char *in,
        char **out,
        size_t *count
        )
{

        while ((*count != 0) && (*in != '\0')) {
                *(*out)++ = *in++;
                (*count)--;
        }
}

/***
*_store_winword() - Store date/time in WinWord format
*
*Purpose:
*       Format the date/time in the supplied WinWord format
*       and store it in the supplied buffer.
*
*       *** For internal use with strftime() only ***
*
*       The WinWord format is converted token by token to
*       strftime conversion specifiers.  _expandtime is then called to
*       do the work.  The WinWord format is expected to be a
*       character string (not wide-chars).
*
*Entry:
*       const char **format = address of pointer to WinWord format
*       const struct tm *tmptr = pointer to time/date structure
*       char **out = address of pointer to output string
*       size_t *count = address of char count (space in output area)
*       struct __lc_time_data *lc_time = pointer to locale-specific info
*
*Exit:
*       none
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl _store_winword (
        const char *format,
        const struct tm *tmptr,
        char **out,
        size_t *count,
        struct __lc_time_data *lc_time
        )
{
        char specifier;
        const char *p;
        int repeat;
        char *ampmstr;

        while (*format && *count != 0)
        {
                specifier = 0;          /* indicate no match */
                __no_lead_zeros = 0;    /* default is print leading zeros */

                /* count the number of repetitions of this character */
                for (repeat=0, p=format; *p++ == *format; repeat++);
                /* leave p pointing to the beginning of the next token */
                p--;

                /* switch on ascii format character and determine specifier */
                switch (*format)
                {
                        case 'M':
                                switch (repeat)
                                {
                                        case 1: __no_lead_zeros = 1; /* fall thru */
                                        case 2: specifier = 'm'; break;
                                        case 3: specifier = 'b'; break;
                                        case 4: specifier = 'B'; break;
                                } break;
                        case 'd':
                                switch (repeat)
                                {
                                        case 1: __no_lead_zeros = 1; /* fall thru */
                                        case 2: specifier = 'd'; break;
                                        case 3: specifier = 'a'; break;
                                        case 4: specifier = 'A'; break;
                                } break;
                        case 'y':
                                switch (repeat)
                                {
                                        case 2: specifier = 'y'; break;
                                        case 4: specifier = 'Y'; break;
                                } break;
                        case 'h':
                                switch (repeat)
                                {
                                        case 1: __no_lead_zeros = 1; /* fall thru */
                                        case 2: specifier = 'I'; break;
                                } break;
                        case 'H':
                                switch (repeat)
                                {
                                        case 1: __no_lead_zeros = 1; /* fall thru */
                                        case 2: specifier = 'H'; break;
                                } break;
                        case 'm':
                                switch (repeat)
                                {
                                        case 1: __no_lead_zeros = 1; /* fall thru */
                                        case 2: specifier = 'M'; break;
                                } break;
                        case 's': /* for compatibility; not strictly WinWord */
                                switch (repeat)
                                {
                                        case 1: __no_lead_zeros = 1; /* fall thru */
                                        case 2: specifier = 'S'; break;
                                } break;
                        case 'A':
                        case 'a':
                                if (!_stricmp(format, "am/pm"))
                                        p = format + 5;
                                else if (!_stricmp(format, "a/p"))
                                        p = format + 3;
                                specifier = 'p';
                                break;
                        case 't': /* t or tt time marker suffix */
                                if ( tmptr->tm_hour <= 11 )
                                        ampmstr = lc_time->ampm[0];
                                else
                                        ampmstr = lc_time->ampm[1];

                                while ( (repeat > 0) && (*count > 0) )
                                {
                                        if ( isleadbyte((int)*ampmstr) &&
                                             (*count > 1) )
                                        {
                                                *(*out)++ = *ampmstr++;
                                                (*count)--;
                                        }
                                        *(*out)++ = *ampmstr++;
                                        (*count)--;
                                        repeat--;
                                }
                                format = p;
                                continue;

                        case '\'': /* literal string */
                                if (repeat & 1) /* odd number */
                                {
                                        format += repeat;
                                        while (*format && *count != 0)
                                        {
                                                if (*format == '\'')
                                                {
                                                        format++;
                                                        break;
                                                }
                                                if ( isleadbyte((int)*format) &&
                                                     (*count > 1) )
                                                {
                                                        *(*out)++ = *format++;
                                                        (*count)--;
                                                }
                                                *(*out)++ = *format++;
                                                (*count)--;
                                        }
                                }
                                else { /* even number */
                                        format += repeat;
                                }
                                continue;

                        default: /* non-control char, print it */
                                break;
                } /* switch */

                /* expand specifier, or copy literal if specifier not found */
                if (specifier)
                {
                        _expandtime(specifier, tmptr, out, count, lc_time);
                        format = p; /* bump format up to the next token */
                } else {
                        if (isleadbyte((int)*format))
                        {
                                *(*out)++ = *format++;
                                (*count)--;
                        }
                        *(*out)++ = *format++;
                        (*count)--;
                }
        } /* while */
}

/***
*_store_number() - Convert positive integer to string
*
*Purpose:
*       Convert positive integer to a string and store it in the output
*       buffer with no null terminator.  Update both the count and
*       buffer pointers.
*
*       Differs from _store_num in that the precision is not specified,
*       and no leading zeros are added.
*
*       *** For internal use with strftime() only ***
*
*       Created from xtoi.c
*
*Entry:
*       int num = pointer to integer value
*       char **out = address of pointer to output string
*       size_t *count = address of char count (space in output area)
*
*Exit:
*       none
*
*Exceptions:
*       The buffer is filled until it is out of space.  There is no
*       way to tell beforehand (as in _store_num) if the buffer will
*       run out of space.
*
*******************************************************************************/

static void __cdecl _store_number (
        int num,
        char **out,
        size_t *count
        )
{
        char *p;                /* pointer to traverse string */
        char *firstdig;         /* pointer to first digit */
        char temp;              /* temp char */

        p = *out;

        /* put the digits in the buffer in reverse order */
        if (*count > 1)
        {
                do {
                        *p++ = (char) (num % 10 + '0');
                        (*count)--;
                } while ((num/=10) > 0 && *count > 1);
        }

        firstdig = *out;                /* firstdig points to first digit */
        *out = p;                       /* return pointer to next space */
        p--;                            /* p points to last digit */

        /* reverse the buffer */
        do {
                temp = *p;
                *p-- = *firstdig;
                *firstdig++ = temp;     /* swap *p and *firstdig */
        } while (firstdig < p);         /* repeat until halfway */
}

/***
*void tzset() - sets timezone information and calc if in daylight time
*
*Purpose:
*       Sets the timezone information from the TZ environment variable
*       and then sets _timezone, _daylight, and _tzname. If we're in daylight
*       time is automatically calculated.
*
*Entry:
*       None, reads TZ environment variable.
*
*Exit:
*       sets _daylight, _timezone, and _tzname global vars, no return value
*
*Exceptions:
*
*******************************************************************************/


void __cdecl __tzset(void)
{
        int defused;
        int negdiff = 0;

        /*
         * Clear the flag indicated whether GetTimeZoneInformation was used.
         */
        tzapiused = 0;

        /*
         * Set year fields of dststart and dstend structures to -1 to ensure
         * they are recomputed as after this
         */
        dststart.yr = dstend.yr = -1;

        /*
         * Fetch the value of the TZ environment variable.
         */
        if ( 1 /*(TZ = _getenv_lk("TZ")) == NULL */) {

            /*
             * There is no TZ environment variable, try to use the time zone
             * information from the system.
             */

            if ( GetTimeZoneInformation( &tzinfo ) != 0xFFFFFFFF ) {
                /*
                 * Note that the API was used.
                 */
                tzapiused = 1;

                /*
                 * Derive _timezone value from Bias and StandardBias fields.
                 */
                _timezone = tzinfo.Bias * 60L;

                if ( tzinfo.StandardDate.wMonth != 0 )
                    _timezone += (tzinfo.StandardBias * 60L);

                /*
                 * Check to see if there is a daylight time bias. Since the
                 * StandardBias has been added into _timezone, it must be
                 * compensated for in the value computed for _dstbias.
                 */
                if ( (tzinfo.DaylightDate.wMonth != 0) &&
                     (tzinfo.DaylightBias != 0) )
                {
                    _daylight = 1;
                    _dstbias = (tzinfo.DaylightBias - tzinfo.StandardBias) *
                               60L;
                }
                else {
                        _daylight = 0;

                    /*
                     * Set daylight bias to 0 because GetTimeZoneInformation
                     * may return TIME_ZONE_ID_DAYLIGHT even though there is
                     * no DST (in NT 3.51, just turn off the automatic DST
                     * adjust in the control panel)!
                     */
                    _dstbias = 0;
                }

                /*
                 * Try to grab the name strings for both the time zone and the
                 * daylight zone. Note the wide character strings in tzinfo
                 * must be converted to multibyte characters strings. The
                 * locale codepage, __lc_codepage, is used for this. Note that
                 * if setlocale() with LC_ALL or LC_CTYPE has not been called,
                 * then __lc_codepage will be 0 (_CLOCALECP), which is CP_ACP
                 * (which means use the host's default ANSI codepage).
                 */
                if ( (WideCharToMultiByte( __lc_codepage,
                                           WC_COMPOSITECHECK |
                                            WC_SEPCHARS,
                                           tzinfo.StandardName,
                                           -1,
                                           _tzname[0],
                                           63,
                                           NULL,
                                           &defused ) != 0) &&
                     (!defused) )
                    _tzname[0][63] = '\0';
                else
                    _tzname[0][0] = '\0';

                if ( (WideCharToMultiByte( __lc_codepage,
                                           WC_COMPOSITECHECK |
                                            WC_SEPCHARS,
                                           tzinfo.DaylightName,
                                           -1,
                                           _tzname[1],
                                           63,
                                           NULL,
                                           &defused ) != 0) &&
                     (!defused) )
                    _tzname[1][63] = '\0';
                else
                    _tzname[1][0] = '\0';

            }

            /*
             * Time zone information is unavailable, just return.
             */
            return;
        }
}

/***
*struct tm *localtime(ptime) - convert time_t value to tm structure
*
*Purpose:
*       Convert a value in internal (time_t) format to a tm struct
*       containing the corresponding local time.
*
* NOTES:
*       (1) gmtime must be called before _isindst to ensure that the tb time
*           structure is initialized.
*       (2) gmtime and localtime use a single statically allocated buffer.
*           Each call to one of these routines destroys the contents of the
*           previous call.
*       (3) It is assumed that time_t is a 32-bit long integer representing
*           the number of seconds since 00:00:00, 01-01-70 (UTC) (i.e., the
*           Posix/Unix Epoch. Only non-negative values are supported.
*       (4) It is assumed that the maximum adjustment for local time is
*           less than three days (include Daylight Savings Time adjustment).
*           This only a concern in Posix where the specification of the TZ
*           environment restricts the combined offset for time zone and
*           Daylight Savings Time to 2 * (24:59:59), just under 50 hours.
*
*Entry:
*       time_t *ptime - pointer to a long time value
*
*Exit:
*       If *ptime is non-negative, returns a pointer to the tm structure.
*       Otherwise, returns NULL.
*
*Exceptions:
*       See items (3) and (4) in the NOTES above. If these assumptions are
*       violated, behavior is undefined.
*
*******************************************************************************/
struct tm * __cdecl localtime (
        const time_t *ptime
        )
{
        struct tm *ptm;
        long ltime;

        /*
         * Check for illegal time_t value
         */
        if ( (long)*ptime < 0L )
                return( NULL );

#ifdef _WIN32
        __tzset();
#else  /* _WIN32 */
#if defined (_M_MPPC) || defined (_M_M68K)
        _tzset();
#endif  /* defined (_M_MPPC) || defined (_M_M68K) */
#endif  /* _WIN32 */

        if ( (*ptime > 3 * _DAY_SEC) && (*ptime < LONG_MAX - 3 * _DAY_SEC) ) {
                /*
                 * The date does not fall within the first three, or last
                 * three, representable days of the Epoch. Therefore, there
                 * is no possibility of overflowing or underflowing the
                 * time_t representation as we compensate for timezone and
                 * Daylight Savings Time.
                 */

                ltime = (long)*ptime - _timezone;
                ptm = gmtime( (time_t *)&ltime );

                /*
                 * Check and adjust for Daylight Saving Time.
                 */
                if ( _daylight && _isindst( ptm ) ) {
                        ltime -= _dstbias;
                        ptm = gmtime( (time_t *)&ltime );
                        ptm->tm_isdst = 1;
                }
        }
        else {
                ptm = gmtime( ptime );

                /*
                 * The date falls with the first three, or last three days
                 * of the Epoch. It is possible the time_t representation
                 * would overflow or underflow while compensating for
                 * timezone and Daylight Savings Time. Therefore, make the
                 * timezone and Daylight Savings Time adjustments directly
                 * in the tm structure. The beginning of the Epoch is
                 * 00:00:00, 01-01-70 (UTC) and the last representable second
                 * in the Epoch is 03:14:07, 01-19-2038 (UTC). This will be
                 * used in the calculations below.
                 *
                 * First, adjust for the timezone.
                 */
                if ( _isindst(ptm) )
                        ltime = (long)ptm->tm_sec - (_timezone + _dstbias);
                else
                        ltime = (long)ptm->tm_sec - _timezone;
                ptm->tm_sec = (int)(ltime % 60);
                if ( ptm->tm_sec < 0 ) {
                        ptm->tm_sec += 60;
                        ltime -= 60;
                }

                ltime = (long)ptm->tm_min + ltime/60;
                ptm->tm_min = (int)(ltime % 60);
                if ( ptm->tm_min < 0 ) {
                        ptm->tm_min += 60;
                        ltime -= 60;
                }

                ltime = (long)ptm->tm_hour + ltime/60;
                ptm->tm_hour = (int)(ltime % 24);
                if ( ptm->tm_hour < 0 ) {
                        ptm->tm_hour += 24;
                        ltime -=24;
                }

                ltime /= 24;

                if ( ltime > 0L ) {
                        /*
                         * There is no possibility of overflowing the tm_mday
                         * and tm_yday fields since the date can be no later
                         * than January 19.
                         */
                        ptm->tm_wday = (ptm->tm_wday + ltime) % 7;
                        ptm->tm_mday += ltime;
                        ptm->tm_yday += ltime;
                }
                else if ( ltime < 0L ) {
                        /*
                         * It is possible to underflow the tm_mday and tm_yday
                         * fields. If this happens, then adjusted date must
                         * lie in December 1969.
                         */
                        ptm->tm_wday = (ptm->tm_wday + 7 + ltime) % 7;
                        if ( (ptm->tm_mday += ltime) <= 0 ) {
                                ptm->tm_mday += 31;
                                ptm->tm_yday = 364;
                                ptm->tm_mon = 11;
                                ptm->tm_year--;
                        }
                        else {
                                ptm->tm_yday += ltime;
                        }
                }


        }


        return(ptm);
}

static struct tm tb = { 0 };    /* time block */

/***
*struct tm *gmtime(timp) - convert *timp to a structure (UTC)
*
*Purpose:
*       Converts the calendar time value, in internal format (time_t), to
*       broken-down time (tm structure) with the corresponding UTC time.
*
*Entry:
*       const time_t *timp - pointer to time_t value to convert
*
*Exit:
*       returns pointer to filled-in tm structure.
*       returns NULL if *timp < 0L
*
*Exceptions:
*
*******************************************************************************/

struct tm * __cdecl gmtime (
        const time_t *timp
        )
{

        long caltim = *timp;            /* calendar time to convert */
        int islpyr = 0;                 /* is-current-year-a-leap-year flag */
        int tmptim;
        int *mdays;                /* pointer to days or lpdays */

        struct tm *ptb = &tb;

        if ( caltim < 0L )
                return(NULL);

        /*
         * Determine years since 1970. First, identify the four-year interval
         * since this makes handling leap-years easy (note that 2000 IS a
         * leap year and 2100 is out-of-range).
         */
        tmptim = (int)(caltim / _FOUR_YEAR_SEC);
        caltim -= ((long)tmptim * _FOUR_YEAR_SEC);

        /*
         * Determine which year of the interval
         */
        tmptim = (tmptim * 4) + 70;         /* 1970, 1974, 1978,...,etc. */

        if ( caltim >= _YEAR_SEC ) {

            tmptim++;                       /* 1971, 1975, 1979,...,etc. */
            caltim -= _YEAR_SEC;

            if ( caltim >= _YEAR_SEC ) {

                tmptim++;                   /* 1972, 1976, 1980,...,etc. */
                caltim -= _YEAR_SEC;

                /*
                 * Note, it takes 366 days-worth of seconds to get past a leap
                 * year.
                 */
                if ( caltim >= (_YEAR_SEC + _DAY_SEC) ) {

                        tmptim++;           /* 1973, 1977, 1981,...,etc. */
                        caltim -= (_YEAR_SEC + _DAY_SEC);
                }
                else {
                        /*
                         * In a leap year after all, set the flag.
                         */
                        islpyr++;
                }
            }
        }

        /*
         * tmptim now holds the value for tm_year. caltim now holds the
         * number of elapsed seconds since the beginning of that year.
         */
        ptb->tm_year = tmptim;

        /*
         * Determine days since January 1 (0 - 365). This is the tm_yday value.
         * Leave caltim with number of elapsed seconds in that day.
         */
        ptb->tm_yday = (int)(caltim / _DAY_SEC);
        caltim -= (long)(ptb->tm_yday) * _DAY_SEC;

        /*
         * Determine months since January (0 - 11) and day of month (1 - 31)
         */
        if ( islpyr )
            mdays = _lpdays;
        else
            mdays = _days;


        for ( tmptim = 1 ; mdays[tmptim] < ptb->tm_yday ; tmptim++ ) ;

        ptb->tm_mon = --tmptim;

        ptb->tm_mday = ptb->tm_yday - mdays[tmptim];

        /*
         * Determine days since Sunday (0 - 6)
         */
        ptb->tm_wday = ((int)(*timp / _DAY_SEC) + _BASE_DOW) % 7;

        /*
         *  Determine hours since midnight (0 - 23), minutes after the hour
         *  (0 - 59), and seconds after the minute (0 - 59).
         */
        ptb->tm_hour = (int)(caltim / 3600);
        caltim -= (long)ptb->tm_hour * 3600L;

        ptb->tm_min = (int)(caltim / 60);
        ptb->tm_sec = (int)(caltim - (ptb->tm_min) * 60);

        ptb->tm_isdst = 0;
        return( (struct tm *)ptb );

}

int abort() { return 0; }
