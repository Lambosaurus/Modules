#ifndef CRCX_H
#define CRCX_H

#include <stdint.h>

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */


// NOTE: For reflected inputs, the polynomial shall also be reflected.
uint8_t CRC8(uint8_t init, uint8_t poly, const uint8_t * data, uint32_t size);
uint8_t CRC8R(uint8_t init, uint8_t poly, const uint8_t * data, uint32_t size);
uint16_t CRC16(uint16_t init, uint16_t poly, const uint8_t * data, uint32_t size);
uint16_t CRC16R(uint16_t init, uint16_t poly, const uint8_t * data, uint32_t size);

// Optimized implementations
uint16_t CRC16_CCITT(uint16_t init, const uint8_t * data, uint32_t size);

/*
 * PUBLIC FUNCTIONS
 */


#endif //CRCX_H
