#ifndef SI7006_H
#define SI7006_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool SI7006_Init(void);
void SI7006_Deinit(void);

// temperature in deci-degrees
// hum in percent
bool SI7006_Read(int16_t * temp, uint8_t * hum);

/*
 * EXTERN DECLARATIONS
 */

#endif // SI7006_H
