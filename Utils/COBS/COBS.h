#ifndef COBS_H
#define COBS_H

#include <stdint.h>

/*
 * PUBLIC DEFINITIONS
 */

 // These sizes do not include the delimiter (0x00)
#define COBS_ENCODE_MAX(_size)    (_size + ((_size + 253) / 254) + 1)
#define COBS_DECODE_MAX(_size)    (_size - 2)

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

uint32_t COBS_Encode(const uint8_t * src, uint32_t src_len, uint8_t * dst);
uint32_t COBS_Decode(const uint8_t * src, uint32_t src_len, uint8_t * dst);

/*
 * EXTERN DECLARATIONS
 */

#endif //COBS_H
