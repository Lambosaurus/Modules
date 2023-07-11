#ifndef FPROM_H
#define FPROM_H

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

// The offset address must be a multiple of FLASH_WORD_SIZE
void FPROM_Read(uint32_t offset, void * data, uint32_t size);
void FPROM_Write(uint32_t offset, const void * data, uint32_t size);

/*
 * EXTERN DECLARATIONS
 */

#endif //FPROM_H
