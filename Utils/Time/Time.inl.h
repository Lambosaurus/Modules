/*
 * PRIVATE DEFINITIONS
 */

/*
 * INLINE FUNCTION DEFINITIONS
 */

static inline Time_t Time_Make(uint32_t s, uint32_t ms)
{
	return ((int64_t)s * 1000) + ms;
}

static inline Time_t Time_Add(Time_t a, Time_t b)
{
	return a + b;
}

static inline Time_t Time_Sub(Time_t a, Time_t b)
{
	return a - b;
}

static inline Time_t Time_AddMillis(Time_t t, uint32_t ms)
{
	return t + ms;
}

static inline Time_t Time_SubMillis(Time_t t, uint32_t ms)
{
	return t - ms;
}

static inline Time_t Time_FromMillis(uint32_t ms)
{
	return ms;
}

static inline bool Time_IsGreaterThan(Time_t a, Time_t b)
{
	return a > b;
}

static inline bool Time_IsLessThan(Time_t a, Time_t b)
{
	return a < b;
}

static inline bool Time_IsElapsed(Time_t t)
{
	return Time_IsLessThan(t, Time_Now());
}

static inline bool Time_IsDurationElapsed(Time_t t, uint32_t ms)
{
	return Time_IsElapsed(Time_AddMillis(t, ms));
}

