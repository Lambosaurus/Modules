
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
			codep = dst;
			if (!byte || length)
				dst++;
		}
	}
	*codep = code;
	*dst++ = 0;
	return (uint32_t)(dst - dst_start);
}

uint32_t COBS_Decode(const uint8_t * src, uint32_t length, uint8_t * dst)
{
    const uint8_t * src_end = src + length;
    uint8_t * dst_start = dst;
    
    uint8_t code = 0xFF;
    uint8_t block = 0;

	while (src < src_end)
	{
		if (block--)
			*dst++ = *src++;
		else
		{
		    // Fetch a new code byte
			block = *src++;
			if (!block)
			    // We hit a literal delimiter. This should not happen.
			    break;
			else if (code != 0xFF)
			    // Only insert 0 if the previous block was not an extention.
				*dst++ = 0;
			code = block--;
		}
	}

	if (*src != 0)
	{
		// Uh oh. No delmiter!
		return 0;
	}

	return (uint32_t)(dst - dst_start);
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

