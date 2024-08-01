#ifndef DS18B20_H
#define DS18B20_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#define BS18B20_TEMP_SCALAR		1000

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

// Returns temperature in milli-degrees.
bool DS18B20_ReadTemperature(const uint8_t * rom, int32_t * temp);

/*
 * EXTERN DECLARATIONS
 */

#endif //DS18B20_H
