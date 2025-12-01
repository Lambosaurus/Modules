#ifndef BI2C_H
#define BI2C_H

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

void BI2C_Init(uint32_t bitrate);
void BI2C_Deinit(void);

bool BI2C_Scan(uint8_t address);

bool BI2C_Write(uint8_t address, const uint8_t * data, uint32_t count);
bool BI2C_Read(uint8_t address, uint8_t * data, uint32_t count);
bool BI2C_Transfer(uint8_t address, const uint8_t * txdata, uint32_t txcount, uint8_t * rxdata, uint32_t rxcount);

/*
 * EXTERN DECLARATIONS
 */


#endif // BI2C_H
