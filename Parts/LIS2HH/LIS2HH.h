#ifndef LIS2HH_H
#define LIS2HH_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#define LIS2HH_SPI_BITRATE	10000000 // 10MHz

/*
 * PUBLIC TYPES
 */

// All axes in mG
typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} LIS2HH_Accel_t;

/*
 * PUBLIC FUNCTIONS
 */

bool LIS2HH_Init(uint8_t scale_g, uint16_t frequency, bool high_res);
void LIS2HH_Deinit(void);
void LIS2HH_Read(LIS2HH_Accel_t * acc);

void LIS2HH_EnableDataInt(void);
bool LIS2HH_IsIntSet(void);

#endif //LIS2HH_H
