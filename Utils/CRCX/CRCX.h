#ifndef CRCX_H
#define CRCX_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

uint8_t CRC8(uint8_t init, uint8_t poly, const uint8_t * data, uint32_t size);

// Note, the polynomial must be reflected.
uint8_t CRC8R(uint8_t init, uint8_t poly, const uint8_t * data, uint32_t size);

/*
 * PUBLIC FUNCTIONS
 */


#endif //CRCX_H
