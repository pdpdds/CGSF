//-< DATEIME.H >-----------------------------------------------------*--------*
// FastDB                    Version 1.0         (c) 1999  GARRET    *     ?  *
// (Main Memory Database Management System)                          *   /\|  *
//                                                                   *  /  \  *
//                          Created:     20-Nov-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 10-Dec-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Date-time field type
//-------------------------------------------------------------------*--------*

#ifndef __DATETIME_H__
#define __DATETIME_H__

#include "stdtp.h"
#include "class.h"

BEGIN_FASTDB_NAMESPACE

class FASTDB_DLL_ENTRY dbDateTime { 
  protected:
    int4 stamp;
  public:
    bool operator == (dbDateTime const& dt) const { 
        return stamp == dt.stamp;
    }
    bool operator != (dbDateTime const& dt) const { 
        return stamp != dt.stamp;
    }
    bool operator > (dbDateTime const& dt) const { 
        return stamp > dt.stamp;
    }
    bool operator >= (dbDateTime const& dt) const { 
        return stamp >= dt.stamp;
    }
    bool operator < (dbDateTime const& dt) const { 
        return stamp < dt.stamp;
    }
    bool operator <= (dbDateTime const& dt) const { 
        return stamp <= dt.stamp;
    }
    int operator - (dbDateTime const& dt) const { 
        return stamp - dt.stamp;
    }
    int operator + (dbDateTime const& dt) const { 
        return stamp + dt.stamp;
    }
    static dbDateTime current() { 
        return dbDateTime(time(NULL));
    }
    dbDateTime(time_t tm) {
        stamp = (int4)tm;
    }
    dbDateTime() { 
        stamp = -1;
    } 
    bool isValid() const { 
        return stamp != -1;
    }

    time_t asTime_t() const { return stamp; }

    void clear() { stamp = -1; }

    dbDateTime(int year, int month, int day, 
               int hour=0, int min=0, int sec = 0) 
    { 
        struct tm t;
        t.tm_year = year > 1900 ? year - 1900 : year;
        t.tm_mon = month-1;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = min;
        t.tm_sec = sec;
        t.tm_isdst = -1;
        stamp = (int4)mktime(&t);
    }
    dbDateTime(int hour, int min) { 
        stamp = (hour*60+min)*60;
    }

#if defined(HAVE_LOCALTIME_R) && !defined(NO_PTHREADS)
    int year() { 
        struct tm t;
        time_t tt = (nat4)stamp;
        return localtime_r(&tt, &t)->tm_year + 1900;
    }
    int month() { // 1..12
        struct tm t;
        time_t tt = (nat4)stamp;
        return localtime_r(&tt, &t)->tm_mon + 1;
    }
    int day() { // 1..31
        struct tm t;
        time_t tt = (nat4)stamp;
        return localtime_r(&tt, &t)->tm_mday;
    }
    int dayOfYear() { // 1..366
        struct tm t;
        time_t tt = (nat4)stamp;
        return localtime_r(&tt, &t)->tm_yday+1;
    }
    int dayOfWeek() { // 1..7
        struct tm t;
        time_t tt = (nat4)stamp;
        return localtime_r(&tt, &t)->tm_wday+1;
    }
    int hour() { // 0..24
        struct tm t;
        time_t tt = (nat4)stamp;
        return localtime_r(&tt, &t)->tm_hour;
    }
    int minute() { // 0..59
        struct tm t;
        time_t tt = (nat4)stamp;
        return localtime_r(&tt, &t)->tm_min;
    }
    int second() { // 0..59
        struct tm t;
        time_t tt = (nat4)stamp;
        return localtime_r(&tt, &t)->tm_sec;
    }
    char* asString(char* buf, int buf_size, char const* format = "%c") const { 
        struct tm t;
        time_t tt = (nat4)stamp;
        strftime(buf, buf_size, format, localtime_r(&tt, &t));
        return buf;
    }
    static dbDateTime currentDate() { 
        struct tm t;
        time_t curr = time(NULL);
        localtime_r(&curr, &t);
        t.tm_hour = 0;
        t.tm_min = 0;
        t.tm_sec = 0;
        return dbDateTime(mktime(&t));
    }
#else
    int year() { 
        time_t tt = (nat4)stamp;
        return localtime(&tt)->tm_year + 1900;
    }
    int month() { // 1..12
        time_t tt = (nat4)stamp;
        return localtime(&tt)->tm_mon + 1;
    }
    int day() { // 1..31
        time_t tt = (nat4)stamp;
        return localtime(&tt)->tm_mday;
    }
    int dayOfYear() { // 1..366
        time_t tt = (nat4)stamp;
        return localtime(&tt)->tm_yday+1;
    }
    int dayOfWeek() { // 1..7
        time_t tt = (nat4)stamp;
        return localtime(&tt)->tm_wday+1;
    }
    int hour() { // 0..24
        time_t tt = (nat4)stamp;
        return localtime(&tt)->tm_hour;
    }
    int minute() { // 0..59
        time_t tt = (nat4)stamp;
        return localtime(&tt)->tm_min;
    }
    int second() { // 0..59
        time_t tt = (nat4)stamp;
        return localtime(&tt)->tm_sec;
    }
    char* asString(char* buf, int buf_size, char const* format = "%c") const { 
        time_t tt = (nat4)stamp;
#ifdef _WINCE
        struct tm* t = localtime(&tt);
        char cnvBuf[CNV_BUF_SIZE];
        sprintf(cnvBuf, "%02d/02d/%40d %02d:%02d:%02d", t->tm_mon, t->tm_yday+1, t->tm_year + 1900,
                t->tm_hour, t->tm_min, t->tm_sec);
        strncpy(buf, cnvBuf, buf_size);
#else
        strftime(buf, buf_size, format, localtime(&tt));
#endif
        return buf;
    }
    static dbDateTime currentDate() { 
        time_t curr = time(NULL);
        struct tm* tp = localtime(&curr);;
        tp->tm_hour = 0;
        tp->tm_min = 0;
        tp->tm_sec = 0;
        return dbDateTime(mktime(tp));
    }
#endif    

    CLASS_DESCRIPTOR(dbDateTime, 
                     (KEY(stamp,INDEXED|HASHED), 
                      METHOD(year), METHOD(month), METHOD(day),
                      METHOD(dayOfYear), METHOD(dayOfWeek),
                      METHOD(hour), METHOD(minute), METHOD(second)));

    dbQueryExpression operator == (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"stamp"),"=",stamp;
        return expr;
    }
    dbQueryExpression operator != (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"stamp"),"<>",stamp;
        return expr;
    }
    dbQueryExpression operator < (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"stamp"),">",stamp;
        return expr;
    }
    dbQueryExpression operator <= (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"stamp"),">=",stamp;
        return expr;
    }
    dbQueryExpression operator > (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"stamp"),"<",stamp;
        return expr;
    }
    dbQueryExpression operator >= (char const* field) { 
        dbQueryExpression expr;
        expr = dbComponent(field,"stamp"),"<=",stamp;
        return expr;
    }
    friend dbQueryExpression between(char const* field, dbDateTime& from,
                                     dbDateTime& till)
    { 
        dbQueryExpression expr;
        expr=dbComponent(field,"stamp"),"between",from.stamp,"and",till.stamp;
        return expr;
    }

    static dbQueryExpression ascent(char const* field) { 
        dbQueryExpression expr;
        expr=dbComponent(field,"stamp");
        return expr;
    }   
    static dbQueryExpression descent(char const* field) { 
        dbQueryExpression expr;
        expr=dbComponent(field,"stamp"),"desc";
        return expr;
    }   
};

END_FASTDB_NAMESPACE

#endif
