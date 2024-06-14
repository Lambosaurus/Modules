#ifndef M24LC04_H
#define M24LC04_H

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

// This only checks for the presence of the EEPROM.
bool M24LC04_Init(void);

// Write data to the EEPROM
bool M24LC04_Write(uint32_t pos, const uint8_t * bfr, uint32_t size);

// Read data from the EEPROM
bool M24LC04_Read(uint32_t pos, uint8_t * bfr, uint32_t size);

/*
 * EXTERN DECLARATIONS
 */

#endif //M24LC04_H
