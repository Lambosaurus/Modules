#include "Time.h"

/*
 * PRIVATE DEFINITIONS
 */

#if defined(TIME_USE_LPTIM)

#include "LPTIM.h"
#define TIME_LPTIM					TIME_USE_LPTIM
#define TIME_COUNTER_MAX			0xFFFF
#ifndef TIME_SUBSECOND_RES
#define TIME_SUBSECOND_RES			256
#endif

#else
#error "Please define a time source"
#endif

#define TIME_WAKEUP_TOLERANCE	3
#define TIME_WAKEUP_MIN			(TIME_WAKEUP_TOLERANCE * 1000 / TIME_SUBSECOND_RES)
#define TIME_WAKEUP_MAX			((TIME_COUNTER_MAX - TIME_WAKEUP_TOLERANCE) * 1000 / TIME_SUBSECOND_RES)


#define TIME_DAYS_4Y				((365*4) + 1)

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static inline void Time_Counter_Start(void);
static inline void Time_Counter_Stop(void);
static inline uint32_t Time_Counter_Read(void);
static inline void Time_Counter_SetWakeup(uint32_t ticks, VoidFunction_t callback);
static inline void Time_Counter_CancelWakeup(void);

/*
 * PRIVATE VARIABLES
 */

static struct {
	uint32_t counter_base;
	Time_t time_base;
} gTime;

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

void Time_Init(void)
{
	Time_Counter_Start();
	gTime.time_base = 0;
	gTime.counter_base = Time_Counter_Read();
}

void Time_Deinit(void)
{
	Time_Counter_Stop();
}

void Time_Update(void)
{
	uint32_t delta = (Time_Counter_Read() - gTime.counter_base) & TIME_COUNTER_MAX;
	if (delta >= TIME_SUBSECOND_RES)
	{
		uint32_t delta_s = delta / TIME_SUBSECOND_RES;
		CRITICAL_SECTION_BEGIN();
		gTime.counter_base += delta_s * TIME_SUBSECOND_RES;
		gTime.time_base += delta_s * 1000;
		CRITICAL_SECTION_END();
	}
}

Time_t Time_Now(void)
{
	// NOTE: if Time_Update can be called from an IRQ, we need to protect the gTime reads.
	uint32_t delta = (Time_Counter_Read() - gTime.counter_base) & TIME_COUNTER_MAX;

	// We dont check for overflow here. So long as the update happens, delta will stay small.
	return gTime.time_base + ((delta * 1000) / TIME_SUBSECOND_RES);
}

uint32_t Time_ToMillis(Time_t t)
{
	if (t < 0)
		return 0;
	if (t > UINT32_MAX)
		return UINT32_MAX;
	return t;
}

int32_t Time_DeltaMillis(Time_t a, Time_t b)
{
	int64_t delta = a - b;
	if (delta < INT32_MIN)
		return INT32_MIN;
	if (delta > INT32_MAX)
		return INT32_MAX;
	return delta;
}

int32_t Time_Compare(Time_t a, Time_t b)
{
	return a == b ? 0 : a > b ? 1 : -1;
}

Time_t Time_FromDateTime(const DateTime_t * dt)
{
    uint32_t year = dt->year - RTC_YEAR_MIN;
    uint32_t epoch =
    		  ((year/4) * TIME_DAYS_4Y)
			+ (gDays[year % 4][dt->month - 1])
			+ (dt->day - 1);

    epoch = (epoch * 24) + dt->hour;
    epoch = (epoch * 60) + dt->minute;
    epoch = (epoch * 60) + dt->second;

    return Time_Make(epoch, dt->millis);
}

void Time_ToDateTime(DateTime_t * dt, Time_t t)
{
	dt->millis = t % 1000;
	uint32_t epoch = t / 1000;

    dt->second = epoch % 60;
    epoch /= 60;
    dt->minute = epoch % 60;
    epoch /= 60;
    dt->hour = epoch % 24;
    epoch /= 24;

    uint32_t years = (epoch/TIME_DAYS_4Y) * 4;
    epoch %= TIME_DAYS_4Y;

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

void Time_ScheduleWakeup(Time_t t, VoidFunction_t callback)
{
	uint32_t counter = Time_Counter_Read();
	int64_t delta = (t - gTime.time_base) * TIME_SUBSECOND_RES / 1000;
	delta -= (counter - gTime.counter_base) & TIME_COUNTER_MAX;

	if (delta < TIME_WAKEUP_TOLERANCE)
		delta = TIME_WAKEUP_TOLERANCE;
	else if (delta > (TIME_COUNTER_MAX - TIME_WAKEUP_TOLERANCE))
		delta = TIME_COUNTER_MAX - TIME_WAKEUP_TOLERANCE;

	uint32_t tick = (counter + (uint32_t)delta) & TIME_COUNTER_MAX;
	Time_Counter_SetWakeup(tick, callback);
}

void Time_CancelWakeup(void)
{
	Time_Counter_CancelWakeup();
}

/*
 * PRIVATE FUNCTIONS
 */

#if defined(TIME_USE_LPTIM)

static inline void Time_Counter_Start(void)
{
	LPTIM_Init(TIME_LPTIM, TIME_SUBSECOND_RES, TIME_COUNTER_MAX);
	LPTIM_Start(TIME_LPTIM);
}

static inline void Time_Counter_Stop(void)
{
	LPTIM_Deinit(TIME_LPTIM);
}

static inline uint32_t Time_Counter_Read(void)
{
	return LPTIM_Read(TIME_LPTIM);
}

static inline void Time_Counter_SetWakeup(uint32_t tick, VoidFunction_t callback)
{
	LPTIM_OnPulse(TIME_LPTIM, tick, callback);
}

static inline void Time_Counter_CancelWakeup(void)
{
	LPTIM_StopPulse(TIME_LPTIM);
}

#endif

/*
 * INTERRUPT ROUTINES
 */
