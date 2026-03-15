#include "Schedule.h"
#include "Core.h"

/*
 * PRIVATE DEFINITIONS
 */


#define QUEUE_END								((void*)0x00000001)

#ifndef SCHEDULE_COALESCING_WINDOW
#define SCHEDULE_COALESCING_WINDOW				5
#endif

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void Schedule_Insert(Schedule_t * s);
static void Schedule_OnWakeup(void);

/*
 * PRIVATE VARIABLES
 */

static struct {
	Schedule_t * running;
	Schedule_t * deferred;
} gSch;

/*
 * PUBLIC FUNCTIONS
 */

void Schedule_Init(void)
{
	gSch.running = QUEUE_END;
	gSch.deferred = QUEUE_END;
}

void Schedule_Deinit(void)
{
	Schedule_t * head;

	// We need to flag all running tasks as idle.
	head = gSch.running;
	while (head != QUEUE_END)
	{
		Schedule_t * s = head;
		head = s->running;
		s->running = NULL;
	}

	head = gSch.deferred;
	while (head != QUEUE_END)
	{
		Schedule_t * s = head;
		head = s->deferred;
		s->deferred = NULL;
	}
}

void Schedule_New(Schedule_t * s, uint32_t period, VoidFunction_t callback, Schedule_Flag_t flags)
{
	s->period = period;
	s->callback = callback;
	s->flags = flags;
	s->running = NULL;
	s->deferred = NULL;
}

void Schedule_Start(Schedule_t * s)
{
	CRITICAL_SECTION_BEGIN();
	if (s->running == NULL)
	{
		s->timeout = Time_AddMillis(Time_Now(), s->period);

		// Insert the new task
		Schedule_Insert(s);

		// Are we the next wakeup?
		if (gSch.running == s)
		{
			Time_ScheduleWakeup(s->timeout, Schedule_OnWakeup);
		}
	}
	CRITICAL_SECTION_END();
}

void Schedule_Stop(Schedule_t * s)
{
	CRITICAL_SECTION_BEGIN();
	if (s->running != NULL)
	{
		// Find the timer
		Schedule_t ** head = &gSch.running;
		while (*head != s)
		{
			head = &(*head)->running;
		}

		// Remove it
		*head = s->running;
		s->running = NULL;

		// Did we just remove the active timer?
		if (head == &gSch.running)
		{
			if (*head != QUEUE_END)
			{
				Time_ScheduleWakeup((*head)->timeout, Schedule_OnWakeup);
			}
			else
			{
				Time_CancelWakeup();
			}
		}
	}
	if (s->deferred != NULL)
	{
		// Find the timer
		Schedule_t ** head = &gSch.deferred;
		while (*head != s)
		{
			head = &(*head)->deferred;
		}

		// Remove it
		*head = s->deferred;
		s->deferred = NULL;
	}
	CRITICAL_SECTION_END();
}

void Schedule_Update(void)
{
	while (true)
	{
		CRITICAL_SECTION_BEGIN();

		if (gSch.deferred == QUEUE_END)
		{
			CRITICAL_SECTION_END();
			break;
		}

		Schedule_t * s = gSch.deferred;
		gSch.deferred = s->deferred;
		s->deferred = NULL;

		CRITICAL_SECTION_END();

		s->callback();
	}
}

/*
 * PRIVATE FUNCTIONS
 */

static void Schedule_Insert(Schedule_t * s)
{
    Schedule_t ** head = &gSch.running;

    while (*head != QUEUE_END)
    {
        if (Time_IsGreaterThan((*head)->timeout, s->timeout))
        {
            break;
        }
        head = &(*head)->running;
    }

    s->running = *head;
    *head = s;
}

static void Schedule_Defer(Schedule_t * s)
{
	Schedule_t ** head = &gSch.deferred;

	while (*head != QUEUE_END)
	{
		head = &(*head)->deferred;
	}

	s->deferred = *head;
	*head = s;
}

static void Schedule_OnWakeup(void)
{
	Time_t threshold = Time_AddMillis(Time_Now(), SCHEDULE_COALESCING_WINDOW);

	while (gSch.running != QUEUE_END)
	{
		Schedule_t * s = gSch.running;

		if (!Time_IsLessThan(s->timeout, threshold))
		{
			break;
		}

		gSch.running = s->running;
		s->running = NULL;

		if (s->flags & Schedule_Flag_Repeat)
		{
			s->timeout = Time_AddMillis(s->timeout, s->period);
			Schedule_Insert(s);
		}

		// Arrange execution
		if (s->flags & Schedule_Flag_Defer)
		{
			// Do not double defer
			if (s->deferred == NULL)
			{
				Schedule_Defer(s);
			}
		}
		else
		{
			s->callback();
		}
	}

	// This prevents an outer CORE_Stop, to guarantee the servicing of any main loop.
	CORE_Wake();

	// Restart the timer
	if (gSch.running != QUEUE_END)
	{
		Time_ScheduleWakeup(gSch.running->timeout, Schedule_OnWakeup);
	}
	else
	{
		Time_CancelWakeup();
	}
}

/*
 * TIMER IMPLEMENTATIONS
 */

