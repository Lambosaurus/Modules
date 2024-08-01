#ifndef ONEWIRE_H
#define ONEWIRE_H

#include "STM32X.h"
#include "GPIO.h"

/*
 * PUBLIC DEFINITIONS
 */

#define D1W_ROM_SIZE	8

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

void D1W_Init(GPIO_Pin_t pin);
void D1W_Deinit(void);

bool D1W_Detect(void);

// Different start commands
// These all include the reset pulse
bool D1W_SearchRom(uint8_t * rom);
bool D1W_ReadRom(uint8_t * rom);

// rom may be NULL for broadcast
bool D1W_SelectRom(const uint8_t * rom);

// Read and writes must follow a start command.
void D1W_Write(const uint8_t * tx, uint32_t count);
void D1W_Read(uint8_t * rx, uint32_t count);
void D1W_WriteByte(uint8_t b);
uint8_t D1W_ReadByte(void);


uint8_t D1W_CRC(const uint8_t * bfr, uint32_t size);

/*
 * EXTERN DECLARATIONS
 */

#endif //ONEWIRE_H
