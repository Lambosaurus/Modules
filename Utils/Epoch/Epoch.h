#ifndef EPOCH_H
#define EPOCH_H

#include "STM32X.h"
#include "RTC.h"

/*
 * PUBLIC DEFINITIONS
 */

#define SECONDS		1
#define MINUTES		(SECONDS * 60)
#define HOURS		(MINUTES * 60)
#define DAYS		(HOURS * 24)

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

uint32_t Epoch_FromDateTime(DateTime_t * dt);
void Epoch_ToDateTime(DateTime_t * dt, uint32_t epoch);
uint32_t Epoch_Read(void);

/*
 * EXTERN DECLARATIONS
 */

#endif //EPOCH
