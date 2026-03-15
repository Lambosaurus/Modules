# Schedule
This module provides a scheduler.

It is dependant on the Time module as a backend.

## Usage

```C
Time_Init();
Schedule_Init();

Schedule_t blink_on;
Schedule_t blink_off;

// User_LedOn can start the blink off schedule. Ie, Schedule_Start(&blink_off);
Schedule_New(&blink_on, 1000, User_LedOn, Schedule_Flag_Repeat | Schedule_Flag_Defer);
Schedule_New(&blink_off, 100, User_LedOff, 0);
Schedule_Start(&blink_on);

while (1)
{
    Time_Update();

    // Application code here

    Schedule_Update();
    CORE_Stop();
}
```

## Deferral
By default, scheduled functions will be called in the schedulers wakup ISR. They can instead be deferred using `Schedule_Flag_Defer`. This will cause them to be scheduled in the next `Schedule_Update()` call. This will guarantee they are called outside from a non IRQ context, so can be synchronous and blocking.

## Coalescence
The scheduler will perform early execution & scheduling of events which are less than `SCHEDULE_COALESCING_WINDOW` ms in the future.
This minimizes the number of scheduled wakeup events. This should never be set less than 2 ms.

## Wakeups

To guarantee the mainloop is executed each cycle, its expected to use WFE signaling.
`#define CORE_USE_WFE` will enable this. This may require `CORE_Wake()` on other IRQ's to exit stop mode.

# Board

The module is dependant on  definitions within `Board.h`

The following template can be used.

```C
// CORE config
#define CORE_USE_WFE

// Schedule config
//#define SCHEDULE_COALESCING_WINDOW 5
```