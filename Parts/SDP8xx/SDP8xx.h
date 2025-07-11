#ifndef SDP8XX_H
#define MODULE_H

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

bool SDP8XX_Init(void);
void SDP8XX_Deinit(void);
bool SDP8XX_Read(int32_t * pressure); // in millipascals

/*
 * EXTERN DECLARATIONS
 */

#endif //MODULE_H
