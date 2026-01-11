#ifndef LIS2HH_H
#define LIS2HH_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#define LIS2_SPI_BITRATE	10000000 // 10MHz

/*
 * PUBLIC TYPES
 */

// All axes in mG
typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} LIS2_Accel_t;

/*
 * PUBLIC FUNCTIONS
 */

bool LIS2_Init(uint8_t scale_g, uint16_t frequency, bool high_res);
void LIS2_Deinit(void);
void LIS2_Read(LIS2_Accel_t * acc);

void LIS2_EnableDataInt(void);
bool LIS2_IsIntSet(void);

#endif //LIS2HH_H
