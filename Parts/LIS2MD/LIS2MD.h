#ifndef LIS2MD_H
#define LIS2MD_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#define LIS2MD_SPI_BITRATE		10000000 // 10MHz

/*
 * PUBLIC TYPES
 */

// All axes in mG
typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} LIS2MD_Mag_t;

/*
 * PUBLIC FUNCTIONS
 */

bool LIS2MD_Init(uint16_t frequency, bool high_res);
void LIS2MD_Deinit(void);
void LIS2MD_Read(LIS2MD_Mag_t * mag);


#endif //LIS2MD_H
