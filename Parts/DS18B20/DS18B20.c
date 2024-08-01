
#include "DS18B20.h"
#include "OneWire.h"
#include "Core.h"

/*
 * PRIVATE DEFINITIONS
 */

#define DS18B20_CONVERT_TEMP		0x44
#define DS18B20_READ_SCRATCHPAD		0xBE

#define DS18B20_CONV_TIME_9BIT		100
#define DS18B20_CONV_TIME_10BIT		200
#define DS18B20_CONV_TIME_11BIT		400
#define DS18B20_CONV_TIME_12BIT		800

#define DS18B20_CONF_RES_9BIT		0x00
#define DS18B20_CONF_RES_10BIT		0x20
#define DS18B20_CONF_RES_11BIT		0x40
#define DS18B20_CONF_RES_12BIT		0x60
#define DS18B20_CONF_RESERVED		0x1F


/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static bool DS18B20_StartConversion(const uint8_t * rom);
static bool DS18B20_ReadScratchpad(const uint8_t * rom, uint8_t scratchpad[9]);
static int32_t DS18B20_GetTemperature(const uint8_t * scratchpad);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool DS18B20_ReadTemperature(const uint8_t * rom, int32_t * temp)
{
	if (DS18B20_StartConversion(rom))
	{
		// Just assume we are in 12 bit mode I guess
		CORE_Delay(DS18B20_CONV_TIME_12BIT);
		uint8_t scratchpad[9];
		if (DS18B20_ReadScratchpad(rom, scratchpad))
		{
			*temp = DS18B20_GetTemperature(scratchpad);
			return true;
		}
	}
	return false;
}

/*
 * PRIVATE FUNCTIONS
 */

static bool DS18B20_StartConversion(const uint8_t * rom)
{
	if (D1W_SelectRom(rom))
	{
		D1W_WriteByte(DS18B20_CONVERT_TEMP);
		return true;
	}
	return false;
}

static bool DS18B20_ReadScratchpad(const uint8_t * rom, uint8_t scratchpad[9])
{
	if (D1W_SelectRom(rom))
	{
		D1W_WriteByte(DS18B20_READ_SCRATCHPAD);
		D1W_Read(scratchpad, 9);
		uint8_t crc = D1W_CRC(scratchpad, 8);
		return crc == scratchpad[8];
	}
	return false;
}

static int32_t DS18B20_GetTemperature(const uint8_t * scratchpad)
{
	int32_t treg = (int16_t)((scratchpad[1] << 8) | scratchpad[0]);
	return (treg * BS18B20_TEMP_SCALAR) >> 4;
}

/*
 * INTERRUPT ROUTINES
 */

