
#include "Epoch.h"

/*
 * PRIVATE DEFINITIONS
 */

#define DAYS_4Y		((365*4) + 1)

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

const static uint16_t gDays[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};

/*
 * PUBLIC FUNCTIONS
 */

uint32_t Epoch_FromDateTime(DateTime_t * dt)
{
    uint32_t year = dt->year - RTC_YEAR_MIN;
    uint32_t epoch =
    		  ((year/4) * (365*4 + 1))
			+ (gDays[year % 4][dt->month - 1])
			+ (dt->day - 1);

    epoch = (epoch * 24) + dt->hour;
    epoch = (epoch * 60) + dt->minute;
    epoch = (epoch * 60) + dt->second;

    return epoch;
}

void Epoch_ToDateTime(DateTime_t * dt, uint32_t epoch)
{
    dt->second = epoch % 60;
    epoch /= 60;
    dt->minute = epoch % 60;
    epoch /= 60;
    dt->hour = epoch % 24;
    epoch /= 24;

    uint32_t years = (epoch/DAYS_4Y) * 4;
    epoch %= DAYS_4Y;

    uint8_t year;
    for (year = 3; year > 0; year--)
    {
        if (epoch >= gDays[year][0]) { break; }
    }

    uint8_t month;
    for (month = 11; month > 0; month--)
    {
        if (epoch >= gDays[year][month]) { break; }
    }

    dt->year  = years + year + RTC_YEAR_MIN;
    dt->month = month + 1;
    dt->day   = epoch - gDays[year][month] + 1;
}


uint32_t Epoch_Read(void)
{
	DateTime_t dt;
	RTC_Read(&dt);
	return Epoch_FromDateTime(&dt);
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

