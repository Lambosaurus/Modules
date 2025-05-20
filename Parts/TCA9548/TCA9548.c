
#include "TCA9548.h"
#include "I2C.h"

/*
 * PRIVATE DEFINITIONS
 */

#define TCA9548_ADDR	0x70

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

bool TCA9548_Select(uint8_t port)
{
	return I2C_Write(TCA9548_I2C, TCA9548_ADDR, &port, sizeof(port));
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

