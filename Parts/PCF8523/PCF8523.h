#ifndef PCF8523_H
#define PCF8523_H

#include "STM32X.h"
#include "RTC.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool PCF8523_Init(void);
bool PCF8523_Read(DateTime_t * dt);
bool PCF8523_Write(const DateTime_t * dt);

/*
 * EXTERN DECLARATIONS
 */

#endif //PCF8523_H
