#ifndef VL6180_H
#define VL6180_H

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

bool VL6180_Init(void);
void VL6180_Deinit(void);

bool VL6180_Start(void);
bool VL6180_IsReady(void);
bool VL6180_Read(uint32_t * range);

/*
 * PUBLIC FUNCTIONS
 */


#endif //VL6180_H
