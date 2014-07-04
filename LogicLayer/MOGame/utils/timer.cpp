/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
 *
 *  This file is part of The Mana Server.
 *
 *  The Mana Server is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana Server is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana Server.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "timer.h"

#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if (NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		/*converting file time to unix epoch*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tmpres /= 10;  /*convert into microseconds*/
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}

/**
 * Calls gettimeofday() and converts it into milliseconds.
 */
static uint64_t getTimeInMillisec()
{
    uint64_t timeInMillisec;
    timeval time;

    gettimeofday(&time, 0);
    timeInMillisec = (uint64_t)time.tv_sec * 1000 + time.tv_usec / 1000;
    return timeInMillisec;
}

namespace utils
{

Timer::Timer(unsigned ms)
{
    active = false;
    interval = ms;
    lastpulse = getTimeInMillisec();
}

void Timer::sleep()
{
    if (!active) return;
    uint64_t now = getTimeInMillisec();
    if (now - lastpulse >= interval) return;
#ifndef _WIN32
    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = (interval - (now - lastpulse)) * (1000 * 1000);
    nanosleep(&req, 0);
#else
    Sleep(interval - (now - lastpulse));
#endif
}

int Timer::poll()
{
    int elapsed = 0;
    if (active)
    {
        uint64_t now = getTimeInMillisec();
        if (now > lastpulse)
        {
            elapsed = (now - lastpulse) / interval;
            lastpulse += interval * elapsed;
        }
        else
        {
            // Time has made a jump to the past. This should be a rare
            // occurence, so just reset lastpulse to prevent problems.
            lastpulse = now;
        }
    };
    return elapsed;
}

void Timer::start()
{
    active = true;
    lastpulse = getTimeInMillisec();
}

void Timer::stop()
{
    active = false;
}

void Timer::changeInterval(unsigned newinterval)
{
    interval = newinterval;
}

} // ::utils
