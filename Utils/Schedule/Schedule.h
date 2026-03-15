#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "STM32X.h"
#include "Time.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef enum {
	Schedule_Flag_Repeat = (1 << 0),
	Schedule_Flag_Defer = (1 << 1),
} Schedule_Flag_t;

typedef struct Schedule_s Schedule_t;

typedef struct Schedule_s {
	Schedule_Flag_t flags;
	Time_t timeout;
	uint32_t period;
	VoidFunction_t callback;
	Schedule_t * running;
	Schedule_t * deferred;
} Schedule_t;

/*
 * PUBLIC FUNCTIONS
 */

void Schedule_Init(void);
void Schedule_Deinit(void);
void Schedule_Update(void);

void Schedule_New(Schedule_t * s, uint32_t period, VoidFunction_t callback, Schedule_Flag_t flags);
void Schedule_Start(Schedule_t * s);
void Schedule_Stop(Schedule_t * s);

/*
 * EXTERN DECLARATIONS
 */

#endif //SCHEDULE_H
