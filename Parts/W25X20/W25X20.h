#ifndef W25X20_H
#define W25X20_H

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

void W25X20_Init(void);
void W25X20_Deinit(void);

void W25X20_Read(uint32_t addr, uint8_t * data, uint32_t size);

// Page writes that need to be page sligned (256 bytes)
void W25X20_Write(uint32_t addr, const uint8_t * data, uint32_t size);

// Erase needs to be sector aligned (4K bytes)
// Erases will be faster if aligned to 32K or 64K
void W25X20_Erase(uint32_t pos, uint32_t size);

// UID is 8 bytes long
void W25X20_GetUID(uint8_t * uid);
uint32_t W25X20_ReadSize(void);

void W25X20_EraseChip(void);

/*
 * EXTERN DECLARATIONS
 */

#endif //W25X20_H
