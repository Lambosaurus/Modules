
#include "SHT3C.h"
#include "I2C.h"
#include "Core.h"
#include "US.h"

/*
 * PRIVATE DEFINITIONS
 */

#define SHT3C_ADDR			0x70

#define SHT3C_CMD_SLEEP 	0xB098
#define SHT3C_CMD_WAKE		0x3517

// These are the temperature first commands.
// Start with clock stretching enabled
#define SHT3C_CMD_START		0x7CA2
#define SHT3C_CMD_START_LP	0x6458

/*
// Start with clock stretching disabled
#define SHT3C_CMD_START		0x7866
#define SHT3C_CMD_START_LP	0x609C

#define SHT3C_MEASURE_TIME_LP	1
#define SHT3C_MEASURE_TIME		12
*/

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static bool SHT3C_Command(uint16_t cmd);
static bool SHT3C_CommandRead(uint16_t cmd, uint16_t * words, uint32_t count);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool SHT3C_Init(void)
{
	return SHT3C_Command(SHT3C_CMD_SLEEP);
}

void SHT3C_Deinit(void)
{
}

bool SHT3C_Read(int16_t * temp, uint8_t * humidity)
{
	bool success = false;
	if (SHT3C_Command(SHT3C_CMD_WAKE))
	{
		US_Delay(250);
		uint16_t words[2];
		if (SHT3C_CommandRead(SHT3C_CMD_START, words, LENGTH(words)))
		{
			*temp = (((uint32_t)words[0] * 1750) / 0x10000) - 450;
			*humidity = ((uint32_t)words[1] * 100) / 0x10000;
			success = true;
		}
		SHT3C_Command(SHT3C_CMD_SLEEP);
	}

	return success;
}

/*
 * PRIVATE FUNCTIONS
 */

static bool SHT3C_Command(uint16_t cmd)
{
	uint8_t data[] = {
		(uint8_t)(cmd >> 8),
		(uint8_t)(cmd)
	};
	return I2C_Write(SHT3C_I2C, SHT3C_ADDR, data, sizeof(data));
}

static bool SHT3C_CommandRead(uint16_t cmd, uint16_t * words, uint32_t count)
{
	uint8_t tx[] = {
		(uint8_t)(cmd >> 8),
		(uint8_t)(cmd)
	};

	uint8_t rx[count * 3];

	if (I2C_Transfer(SHT3C_I2C, SHT3C_ADDR, tx, sizeof(tx), rx, count * 3))
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
