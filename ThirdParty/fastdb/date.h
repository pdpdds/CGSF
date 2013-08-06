//-< DATE.H >--------------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     30-Apr-2000  K.A. Knizhnik  * / [] \ *
//                          Last update: 30-Apr-2000  K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Date field type
//-------------------------------------------------------------------*--------*

#ifndef __DATE_H__
#define __DATE_H__

#include "stdtp.h"
#include "class.h"

BEGIN_FASTDB_NAMESPACE

#if !defined(NO_PTHREADS) && !defined(_WIN32)
#define USE_REENTRANT_LIBRARY
#endif

class FASTDB_DLL_ENTRY dbDate { 
    int4 jday;
  public:
    bool operator == (dbDate const& dt) { 
        return jday == dt.jday;
    }
    bool operator != (dbDate const& dt) { 
        return jday != dt.jday;
    }
    bool operator > (dbDate const& dt) { 
        return jday > dt.jday;
    }
    bool operator >= (dbDate const& dt) { 
        return jday >= dt.jday;
    }
    bool operator < (dbDate const& dt) { 
        return jday < dt.jday;
    }
    bool operator <= (dbDate const& dt) { 
        return jday <= dt.jday;
    }
    int operator - (dbDate const& dt) { 
        return jday - dt.jday;
    }
    int operator + (int days) { 
        return jday + days;
    }
    dbDate& operator += (int days) { 
        jday += days;
        return *this;
    }
    dbDate& operator -= (int days) { 
        jday -= days;
        return *this;
    }
    static dbDate current() { 
        time_t now = time(NULL);
        struct tm* tp;
#ifdef USE_REENTRANT_LIBRARY
        struct tm t;
        tp = localtime_r(&now, &t);
#else 
        tp = localtime(&now);
#endif
        return dbDate(tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday);
    }

    dbDate() { 
        jday = -1;
    } 
    bool isValid()const  { 
        return jday != -1;
    }

    unsigned JulianDay() { return jday; }

    void clear() { jday = -1; }

    dbDate(int year, int month, int day)  {
    /*
      Convert Gregorian calendar date to the corresponding Julian day number
      j.  Algorithm 199 from Communications of the ACM, Volume 6, No. 8,
      (Aug. 1963), p. 444.  Gregorian calendar started on Sep. 14, 1752.
      This function not valid before that.
      */
        nat4 c, ya;
        if (month > 2)
            month -= 3;
        else {
            month += 9;
            year--;
        } /* else */
        c = year / 100;
        ya = year - 100*c;
        jday = ((146097*c)>>2) + ((1461*ya)>>2) + (153*month + 2)/5 + day + 1721119;
    } /* jday */

    void MDY(int& year, int& month, int& day) const { 
    /*
      Convert a Julian day number to its corresponding Gregorian calendar
      date.  Algorithm 199 from Communications of the ACM, Volume 6, No. 8,
      (Aug. 1963), p. 444.  Gregorian calendar started on Sep. 14, 1752.
      This function not valid before that.
     */
        nat4 j = jday - 1721119;
        int m, d, y;
        y = (((j<<2) - 1) / 146097);
        j = (j<<2) - 1 - 146097*y;
        d = (j>>2);
        j = ((d<<2) + 3) / 1461;
        d = ((d<<2) + 3 - 1461*j);
        d = (d + 4)>>2;
        m = (5*d - 3)/153;
        d = 5*d - 3 - 153*m;
        d = (d + 5)/5;
        y = (100*y + j);
        if (m < 10) { 
                m += 3;
        } else {
                m -= 9;
                y++;
        } /* else */
        month = m;
        day = d;
        year = y;
    } /* mdy */

    int day() {
        int month, day, year;
        MDY(year, month, day);
        return day;
    }

    int month() {
        int month, day, year;
        MDY(year, month, day);
        return month;
    }

    int year() {
        int month, day, year;
        MDY(year, month, day);
        return year;
    }

    int dayOfWeek() { 
        return (jday % 7) + 1;
    }

    char* asString(char* buf, char const* format = "%d-%M-%Y") const { 
        static const char* dayName[] = { "Mon", "Tue", "Wen", "Thu", "Fri", "Sat", "Sun" };
        static const char* monthName[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
                                           "Aug", "Sep", "Oct", "Nov", "Dec" };
        int month, day, year;
        MDY(year, month, day);
        char ch, *dst = buf;
        while ((ch = *format++) != '\0') { 
            if (ch == '%') {
                ch = *format++;
                switch (ch) { 
                  case 'd': dst += sprintf(dst, "%02u", day ); continue;
                  case 'D': dst += sprintf(dst, "%s",   dayName[jday % 7]); continue;
                  case 'm': dst += sprintf(dst, "%02u", month); continue;
                  case 'M': dst += sprintf(dst, "%s",   monthName[month - 1]); continue;
                  case 'y': dst += sprintf(dst, "%02u", year - 1900); continue;
                  case 'Y': dst += sprintf(dst, "%04u", year); continue;
                  default: *dst++ = ch;
                }
            } else { 
                *dst++ = ch;
            }
        }
        *dst = '\0';
        return buf;
    }


    CLASS_DESCRIPTOR(dbDate, 
                     (KEY(jday,INDEXED|HASHED), 
                      METHOD(year), METHOD(month), METHOD(day), METHOD(dayOfWeek)));

    dbQueryExpression operator == (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"jday"),"=",jday;
        return expr;
    }
    dbQueryExpression operator != (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"jday"),"<>",jday;
        return expr;
    }
    dbQueryExpression operator < (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"jday"),">",jday;
        return expr;
    }
    dbQueryExpression operator <= (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"jday"),">=",jday;
        return expr;
    }
    dbQueryExpression operator > (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"jday"),"<",jday;
        return expr;
    }
    dbQueryExpression operator >= (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"jday"),"<=",jday;
        return expr;
    }
    friend dbQueryExpression between(char const* field, dbDate& from,
                                     dbDate& till)
    { 
        dbQueryExpression expr;
        expr=dbComponent(field,"jday"),"between",from.jday,"and",till.jday;
        return expr;
    }

    static dbQueryExpression ascent(char const* field) { 
        dbQueryExpression expr;
        expr=dbComponent(field,"jday");
        return expr;
    }   
    static dbQueryExpression descent(char const* field) { 
        dbQueryExpression expr;
        expr=dbComponent(field,"jday"),"desc";
        return expr;
    }   
};

END_FASTDB_NAMESPACE

#endif
