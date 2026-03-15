#ifndef TIME_H
#define TIME_H

#include "RTC.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef int64_t Time_t;

/*
 * PUBLIC FUNCTIONS
 */

void Time_Init(void);
void Time_Deinit(void);
void Time_Update(void);

Time_t Time_Now(void);
static inline Time_t Time_Make(uint32_t s, uint32_t ms);

static inline Time_t Time_Add(Time_t a, Time_t b);
static inline Time_t Time_Sub(Time_t a, Time_t b);

static inline Time_t Time_AddMillis(Time_t t, uint32_t ms);
static inline Time_t Time_SubMillis(Time_t t, uint32_t ms);
static inline Time_t Time_FromMillis(uint32_t ms);
uint32_t Time_ToMillis(Time_t t);
int32_t Time_DeltaMillis(Time_t a, Time_t b);

Time_t Time_FromDateTime(const DateTime_t * dt);
void Time_ToDateTime(DateTime_t * dt, Time_t t);

int32_t Time_Compare(Time_t a, Time_t b);
static inline bool Time_IsGreaterThan(Time_t a, Time_t b);
static inline bool Time_IsLessThan(Time_t a, Time_t b);

static inline bool Time_IsElapsed(Time_t t);
static inline bool Time_IsDurationElapsed(Time_t t, uint32_t ms);

// API to expose the underlying timers wakeup capability. Used to support a scheduler.
void Time_ScheduleWakeup(Time_t t, VoidFunction_t callback);
void Time_CancelWakeup(void);

/*
 * EXTERN DECLARATIONS
 */

#include "Time.inl.h"

#endif // TIME_H
