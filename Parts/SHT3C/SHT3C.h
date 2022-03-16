#ifndef SHT3C_H
#define SHT3C_H

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

/*
 * PUBLIC FUNCTIONS
 */

bool SHT3C_Init(void);
void SHT3C_Deinit(void);

bool SHT3C_Read(int16_t * temp, uint8_t * humidity);

#endif //SHT3C_H
