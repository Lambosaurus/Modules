# Timer
This is a helper module for doing timing management for simple millisecond tasks.
This module does very little work - just adds some syntactic sugar.

## Usage

The following showcases an action performed every 3 seconds.

```C

Timer_t t = {3000, 0};

while(1)
{
    Timer_Tick(CORE_GetTick());

    if (Timer_IsElapsed(&t))
    {
        Timer_Reload(&t);
        ...
    }
    ...
    CORE_Idle();
}
```

The timers are compared against a reference time set within `Timer_Tick()`.
This guarantees that synchronised timers will not fall out of sync when reloaded.