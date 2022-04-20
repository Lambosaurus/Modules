
#include "SHTC3.h"
#include "I2C.h"
#include "Core.h"
#include "US.h"

/*
 * PRIVATE DEFINITIONS
 */

#define SHTC3_ADDR			0x70

#define SHTC3_CMD_SLEEP 	0xB098
#define SHTC3_CMD_WAKE		0x3517

// These are the temperature first commands.
// Start with clock stretching enabled
#define SHTC3_CMD_START		0x7CA2
#define SHTC3_CMD_START_LP	0x6458

/*
// Start with clock stretching disabled
#define SHTC3_CMD_START		0x7866
#define SHTC3_CMD_START_LP	0x609C

#define SHTC3_MEASURE_TIME_LP	1
#define SHTC3_MEASURE_TIME		12
*/

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static bool SHTC3_Command(uint16_t cmd);
static bool SHTC3_CommandRead(uint16_t cmd, uint16_t * words, uint32_t count);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool SHTC3_Init(void)
{
	return SHTC3_Command(SHTC3_CMD_SLEEP);
}

void SHTC3_Deinit(void)
{
}

bool SHTC3_Read(int16_t * temp, uint8_t * humidity)
{
	bool success = false;
	if (SHTC3_Command(SHTC3_CMD_WAKE))
	{
		US_Delay(250);
		uint16_t words[2];
		if (SHTC3_CommandRead(SHTC3_CMD_START, words, LENGTH(words)))
		{
			*temp = (((uint32_t)words[0] * 1750) / 0x10000) - 450;
			*humidity = ((uint32_t)words[1] * 100) / 0x10000;
			success = true;
		}
		SHTC3_Command(SHTC3_CMD_SLEEP);
	}

	return success;
}

/*
 * PRIVATE FUNCTIONS
 */

static bool SHTC3_Command(uint16_t cmd)
{
	uint8_t data[] = {
		(uint8_t)(cmd >> 8),
		(uint8_t)(cmd)
	};
	return I2C_Write(SHTC3_I2C, SHTC3_ADDR, data, sizeof(data));
}

static bool SHTC3_CommandRead(uint16_t cmd, uint16_t * words, uint32_t count)
{
	uint8_t tx[] = {
		(uint8_t)(cmd >> 8),
		(uint8_t)(cmd)
	};

	uint8_t rx[count * 3];

	if (I2C_Transfer(SHTC3_I2C, SHTC3_ADDR, tx, sizeof(tx), rx, count * 3))
	{
		uint8_t * data = rx;
		while (count--)
		{
			*words++ = (data[0] << 8) | data[1];
			data += 3;
			// Third byte is a CRC.
		}
		return true;
	}
	return false;
}

/*
 * INTERRUPT ROUTINES
 */
