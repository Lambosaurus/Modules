
#include "SDP8xx.h"

#include "I2C.h"
#include "Core.h"
#include "CRCX.h"

/*
 * PRIVATE DEFINITIONS
 */

#define SDP8XX_ADDR_A		0x25
#define SDP8XX_ADDR_B		0x26

#define SDP8XX_CMD_SLEEP				0x3677
#define SDP8XX_CMD_CONT_MEAS_STOP		0x3FF9
#define SDP8XX_CMD_READ_ID0				0x367C
#define SDP8XX_CMD_READ_ID1				0xE102


/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static uint8_t SDP8XX_Detect(void);
static void SDP8XX_Wake(void);
static bool SDP8XX_Command(uint16_t command);
static bool SDP8XX_ReadWords(uint16_t * words, uint32_t count);

/*
 * PRIVATE VARIABLES
 */

static struct {
	uint8_t address;
	int16_t scale;
} gSDP8XX;

/*
 * PUBLIC FUNCTIONS
 */

bool SDP8XX_Init(SDP8XX_Mode_t mode)
{
	SDP8XX_Wake();
	gSDP8XX.address = SDP8XX_Detect();
	gSDP8XX.scale = 0;

	if (gSDP8XX.address)
	{
		return SDP8XX_Command(mode);
	}
	return false;
}

void SDP8XX_Deinit(void)
{
	SDP8XX_Command(SDP8XX_CMD_CONT_MEAS_STOP);
	SDP8XX_Command(SDP8XX_CMD_SLEEP);
}

bool SDP8XX_Read(int32_t * pressure)
{
	bool need_scale = gSDP8XX.scale == 0;
	uint16_t words[3];
	if (!SDP8XX_ReadWords(words, need_scale ? 3 : 1))
	{
		return false;
	}

	int32_t dp = (int16_t)words[0];
	if (need_scale)
	{
		gSDP8XX.scale = (int16_t)words[2];
	}
	*pressure = (dp * 1000) / (int32_t)gSDP8XX.scale;
	return true;
}

/*
 * PRIVATE FUNCTIONS
 */

static uint8_t SDP8XX_Detect(void)
{
	if (I2C_Scan(SDP8XX_I2C, SDP8XX_ADDR_A))
	{
		return SDP8XX_ADDR_A;
	}
	if (I2C_Scan(SDP8XX_I2C, SDP8XX_ADDR_B))
	{
		return SDP8XX_ADDR_B;
	}
	return 0;
}

static void SDP8XX_Wake(void)
{
	// Issue a write to the device address wakes it u[.
	// It will nack us, but will wake up after 2ms.

	if (gSDP8XX.address)
	{
		I2C_Write(SDP8XX_I2C, gSDP8XX.address, NULL, 0);
	}
	else
	{
		// We dont know the address yet!
		I2C_Write(SDP8XX_I2C, SDP8XX_ADDR_A, NULL, 0);
		I2C_Write(SDP8XX_I2C, SDP8XX_ADDR_B, NULL, 0);
	}

	CORE_Delay(2);
}

static bool SDP8XX_Command(uint16_t command)
{
	uint8_t tx[] = {
		command >> 8,
		command
	};
	return I2C_Write(SDP8XX_I2C, gSDP8XX.address, tx, sizeof(tx));
}

static bool SDP8XX_ReadWords(uint16_t * words, uint32_t count)
{
	uint32_t rx_len = count*3;
	uint8_t rx[rx_len];
	if (I2C_Read(SDP8XX_I2C, gSDP8XX.address, rx, rx_len))
	{
		for (uint32_t i = 0; i < count; i++)
		{
			uint8_t * head = rx + (i * 3);

			if (CRC8(0xFF, 0x31, head, 3) != 0)
			{
				return false;
			}

			uint16_t word = (head[0] << 8) | head[1];
			// Ignore crc
			words[i] = word;
		}

		return true;
	}
	return false;
}

/*
 * INTERRUPT ROUTINES
 */

