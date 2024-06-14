
#include "M24LC04.h"
#include "I2C.h"
#include "Core.h"

/*
 * PRIVATE DEFINITIONS
 */

#define M24LC04_ADDR					0x50
#define M24LC04_BLOCK_ADDR(_addr)		(M24LC04_ADDR | ((_addr >> 8) & 0x01))
#define M24LC04_TIMEOUT					20

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static bool M24LC04_WaitForIdle(void);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool M24LC04_Init(void)
{
	return M24LC04_WaitForIdle();
}

bool M24LC04_Write(uint32_t pos, const uint8_t * bfr, uint32_t size)
{
	if (!M24LC04_WaitForIdle()) { return false; }

	// Load the position and data into a write buffer
	uint8_t tx[size + 1];
	tx[0] = pos;
	memcpy(tx + 1, bfr, size);

	return I2C_Write(M24LC04_I2C, M24LC04_BLOCK_ADDR(pos),
			tx, sizeof(tx)
			);
}

bool M24LC04_Read(uint32_t pos, uint8_t * bfr, uint32_t size)
{
	if (!M24LC04_WaitForIdle()) { return false; }

	// A read starts by writing the destination address
	uint8_t tx[1] = { pos };

	return I2C_Transfer(M24LC04_I2C, M24LC04_BLOCK_ADDR(pos),
			tx, sizeof(tx), // Write buffer
			bfr, size       // Read buffer
			);
}

/*
 * PRIVATE FUNCTIONS
 */

static bool M24LC04_WaitForIdle(void)
{
	uint32_t start = CORE_GetTick();
	while (CORE_GetTick() - start < M24LC04_TIMEOUT)
	{
		// IC will not ack if busy
		if (I2C_Scan(M24LC04_I2C, M24LC04_ADDR))
		{
			return true;
		}
		CORE_Idle();
	}
	return false;
}

/*
 * INTERRUPT ROUTINES
 */

