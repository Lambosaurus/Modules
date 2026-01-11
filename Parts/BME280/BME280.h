#ifndef BME280_H
#define BME280_H

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

bool BME280_Init(void);
void BME280_Deinit(void);

// Pressure in pA
// Temperature in 0.1 deg
// Humidity in % RH
bool BME280_Read(uint32_t * pressure, int16_t * temperature, uint8_t * humidity);

#endif //BME280_H
