/*
 * Copyright (c) 2026 Lutz Bichler
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
 
#include <linux/time.h>

/* Time Conversion Constants */
#define SECS_PER_DAY        86400LL
#define SECS_PER_HOUR       3600
#define SECS_PER_MIN        60
#define DAYS_EPOCH_SHIFT    719468LL  /* Days from 0000-03-01 to 1970-01-01 */

/* Gregorian Calendar Constants */
#define DAYS_PER_ERA        146097
#define LEAP_YEAR_MAX_ERA   146096
#define EPOCH_WEEKDAY_SHIFT 4         /* 1970-01-01 was Thursday (4) */
#define DAYS_PER_WEEK       7

/* March-based Calendar Algorithm Constants */
#define MP_MULTIPLIER       5
#define MP_SHIFT            2
#define MP_DIVISOR          153
#define MP_JAN_FEB_THRESHOLD 10
#define MONTHS_PER_YEAR     12

/* Year Offset Constants */
#define CALENDAR_YEAR_BASE  1900

/* Day-of-Year (yday) Baseline Offsets */
#define YDAY_MAR_DEC_OFFSET 58
#define YDAY_JAN_FEB_OFFSET 306

static inline int
lkpi_is_leap_year(long y) {
    return (y % 4 == 0) && ((y % 100 != 0) || (y % 400 == 0));
}

void
linux_time64_to_tm(time64_t total, int offset, struct tm *tm)
{
    time64_t days;
    long rem, era, doe, yoe, doy, mp;

    /* Apply offset and store it */
    total += (time64_t)offset;
    tm->tm_gmtoff = offset;

    days = total / SECS_PER_DAY;
    rem = (long)(total % SECS_PER_DAY);
    if (rem < 0) { rem += (long)SECS_PER_DAY; days -= 1; }

    tm->tm_hour = (int)(rem / SECS_PER_HOUR);
    rem %= SECS_PER_HOUR;
    tm->tm_min = (int)(rem / SECS_PER_MIN);
    tm->tm_sec = (int)(rem % SECS_PER_MIN);

    /* Weekday calculation */
    tm->tm_wday = (int)((days + EPOCH_WEEKDAY_SHIFT) % DAYS_PER_WEEK);
    if (tm->tm_wday < 0) tm->tm_wday += DAYS_PER_WEEK;

    /* Convert days since epoch to y/m/d (proleptic Gregorian)
       Shift epoch to 0000-03-01 by adding days */
    days += DAYS_EPOCH_SHIFT;
    era = (days >= 0 ? days : days - LEAP_YEAR_MAX_ERA) / DAYS_PER_ERA;
    doe = days - era * DAYS_PER_ERA;
    yoe = (doe - doe / 1460 + doe / 36524 - doe / LEAP_YEAR_MAX_ERA) / 365;
    
    /* Calculate tm_year (0000-based temporary, then adjusted to 1900-based) */
    tm->tm_year = (int)(yoe + era * 400); 
    doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    mp = (MP_MULTIPLIER * doy + MP_SHIFT) / MP_DIVISOR;
    
    tm->tm_mday = (int)(doy - (MP_DIVISOR * mp + MP_SHIFT) / MP_MULTIPLIER + 1);
    tm->tm_mon  = (int)(mp + (mp < MP_JAN_FEB_THRESHOLD ? 3 : -9)) - 1; /* Resulting in 0..11 */
    
    if (tm->tm_mon <= 1) {
        tm->tm_year--;
    }
    
    /* Calculate day of the year (tm_yday) using standard month offset rules */
    /* March 1st is the baseline (0) for the algorithm's internal calendar year */
    if (mp < MP_JAN_FEB_THRESHOLD) {
        /* March through December */
        tm->tm_yday = (int)(((MP_DIVISOR * mp + MP_SHIFT) / MP_MULTIPLIER) + tm->tm_mday + YDAY_MAR_DEC_OFFSET);
        if (lkpi_is_leap_year((long)tm->tm_year + CALENDAR_YEAR_BASE)) {
            tm->tm_yday++;
        }
    } else {
        /* January and February */
        tm->tm_yday = (int)(((MP_DIVISOR * mp + MP_SHIFT) / MP_MULTIPLIER) + tm->tm_mday - YDAY_JAN_FEB_OFFSET);
    }

    tm->tm_year -= CALENDAR_YEAR_BASE;
    tm->tm_isdst = 0;
    tm->tm_zone = NULL;
}
