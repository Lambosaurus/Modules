# Time
This module provides a monotonic millisecond timebase which can be derived from a low power timer.

It also provides helper for most standard conversions.

While Time_t is simply backed with a 64 bit number, helpers are provided so that caller can treat Time_t as an opaque type.

## Usage

```C
Time_Init();

Time_t deadline = Time_AddMillis(Time_Now(), 1000);

while(1)
{
    // Periodically manage timer wrapping
    Time_Update();

    // Time_t can be used as an opaque time.
    Time_t now = Time_Now();
    if (Time_IsGreaterThan(now, dealine))
    {
        deadline = Time_AddMillis(deadline, 1000);
        ...
    }

    // Timer is reliable in stop mode
    CORE_Stop();
}
```

## Update

`Time_Update()` must be called at least once per timer wrap to support timer rebasing. Ideally this is put in the main loop.

## Scheduling

`Time_ScheduleWakeup(t,cb)` and `Time_CancelWakeup()` are provided so that the timer can support a scheduler backend.

Only a single wakeup event is tracked at once. If the wakeup time is too far in the future for the underlying timer, it will be clamped to a safe value so that a wakeup is guaranteed.


# Board

The module is dependant on  definitions within `Board.h`

The following template can be used.

```C
// LPTIM Config
#define LPTIM1_ENABLE
#define LPTIM_USE_IRQS

// Time config
#define TIME_USE_LPTIM				LPTIM_1
//#define TIME_SUBSECOND_RES		256
```