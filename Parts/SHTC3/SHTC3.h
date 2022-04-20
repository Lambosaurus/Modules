#ifndef SHTC3_H
#define SHTC3_H

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

bool SHTC3_Init(void);
void SHTC3_Deinit(void);

bool SHTC3_Read(int16_t * temp, uint8_t * humidity);

#endif //SHTC3_H
