
/*
 * PRIVATE DEFINITIONS
 */

extern uint32_t __timer_now;

/*
 * INLINE FUNCTION DEFINITIONS
 */

static inline bool Timer_IsElapsed(Timer_t * t)
{
	return (__timer_now - t->last) > t->period;
}

static inline void Timer_Reload(Timer_t * t)
{
	t->last = __timer_now;
}

static inline bool Timer_Over(Timer_t * t, uint32_t ticks)
{
	return (__timer_now - t->last) > ticks;
}

static inline bool Timer_Under(Timer_t * t, uint32_t ticks)
{
	return (__timer_now - t->last) < ticks;
}
