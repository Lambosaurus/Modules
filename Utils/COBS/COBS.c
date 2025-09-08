
#include "COBS.h"

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

uint32_t COBS_Encode(const uint8_t * src, uint32_t length, uint8_t * dst)
{
	const uint8_t * src_end = src + length;
	uint8_t * dst_start = dst;
	uint8_t * codep = dst++;
	uint8_t code = 1;

	while (src < src_end)
	{
		uint8_t byte = *src++;
		if (byte)
		{
			*dst++ = byte;
			code++;
		}

		if (!byte || code == 0xFF)
		{
			// Time to write a code byte
			*codep = code;
			code = 1;
			codep = dst++;
		}
	}
	*codep = code;
	return (uint32_t)(dst - dst_start);
}

uint32_t COBS_Decode(const uint8_t * src, uint32_t length, uint8_t * dst)
{
    const uint8_t * src_end = src + length;
    uint8_t * dst_start = dst;

    while (src < src_end)
    {
        uint8_t code = *src++;

        uint32_t block = code - 1;
        if (src + block > src_end) {
            return 0; // overrun
        }
        memcpy(dst, src, block);
        src += block;
        dst += block;

        if (code < 0xFF && src < src_end)
            *dst++ = 0;
    }

    return (uint32_t)(dst - dst_start);
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

