#include "BI2C.h"

#include "GPIO.h"
#include "US.h"

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static bool BI2C_Start(uint8_t address, bool read);
static bool BI2C_WriteByte(uint8_t byte);
static uint8_t BI2C_ReadByte(bool ack);
static void BI2C_Stop(void);


/*
 * PRIVATE VARIABLES
 */

static struct {
	uint32_t clock_low_us;
	uint32_t clock_high_us;
} gBi2c;

/*
 * PUBLIC FUNCTIONS
 */

void BI2C_Init(uint32_t bitrate)
{
	GPIO_Write(BI2C_SCL_PIN, true);
	GPIO_Init(BI2C_SCL_PIN, GPIO_Mode_Output | GPIO_Flag_OpenDrain | GPIO_Pull_Up);
	GPIO_Write(BI2C_SDA_PIN, true);
	GPIO_Init(BI2C_SDA_PIN, GPIO_Mode_Output | GPIO_Flag_OpenDrain | GPIO_Pull_Up);

	gBi2c.clock_low_us = (300000 + bitrate - 1) / bitrate;
	gBi2c.clock_high_us = (700000 + bitrate - 1) / bitrate;
}

void BI2C_Deinit(void)
{
	GPIO_Deinit(BI2C_SCL_PIN);
	GPIO_Deinit(BI2C_SDA_PIN);
}

bool BI2C_Scan(uint8_t address)
{
	bool ack = BI2C_Start(address, true);
	BI2C_Stop();
	return ack;
}

bool BI2C_Write(uint8_t address, const uint8_t * data, uint32_t count)
{
	bool ack = BI2C_Start(address, false);
	while (ack && count--)
	{
		ack = BI2C_WriteByte(*data++);
	}
	BI2C_Stop();
	return ack;
}

bool BI2C_Read(uint8_t address, uint8_t * data, uint32_t count)
{
	bool ack = BI2C_Start(address, true);
	if (ack)
	{
		while (count--)
		{
			*data++ = BI2C_ReadByte(count > 0);
		}
	}
	BI2C_Stop();
	return ack;
}

bool BI2C_Transfer(uint8_t address, const uint8_t * txdata, uint32_t txcount, uint8_t * rxdata, uint32_t rxcount)
{
	bool ack = BI2C_Start(address, false);
	while (ack && txcount--)
	{
		ack = BI2C_WriteByte(*txdata++);
	}

	if (ack)
		ack = BI2C_Start(address, true);

	while (ack && rxcount--)
	{
		*rxdata++ = BI2C_ReadByte(rxcount > 0);
	}

	BI2C_Stop();
	return ack;
}

/*
 * PRIVATE FUNCTIONS
 */

static bool BI2C_Start(uint8_t address, bool read)
{
	// Guarantee setup time.
	GPIO_Set(BI2C_SDA_PIN);
	US_Delay(gBi2c.clock_high_us);
	GPIO_Set(BI2C_SCL_PIN);
	US_Delay(gBi2c.clock_high_us);

	// Start condition.
	GPIO_Reset(BI2C_SDA_PIN);
	US_Delay(gBi2c.clock_low_us);
	GPIO_Reset(BI2C_SCL_PIN);
	US_Delay(gBi2c.clock_low_us);

	return BI2C_WriteByte((address << 1) | (read ? 0x01 : 0x00));
}

static void BI2C_WriteBit(bool bit)
{
	GPIO_Write(BI2C_SDA_PIN, bit);
	US_Delay(gBi2c.clock_low_us);
	GPIO_Set(BI2C_SCL_PIN);
	US_Delay(gBi2c.clock_high_us);
	GPIO_Reset(BI2C_SCL_PIN);
}

static bool BI2C_ReadBit(void)
{
	US_Delay(gBi2c.clock_low_us);
	GPIO_Set(BI2C_SCL_PIN);
	US_Delay(gBi2c.clock_high_us);
	bool bit = GPIO_Read(BI2C_SDA_PIN);
	GPIO_Reset(BI2C_SCL_PIN);
	return bit;
}

static bool BI2C_WriteByte(uint8_t byte)
{
	for (uint32_t mask = 0x80; mask > 0x00; mask >>= 1)
	{
		BI2C_WriteBit(byte & mask);
	}

	GPIO_Set(BI2C_SDA_PIN);
	return !BI2C_ReadBit();
}

static uint8_t BI2C_ReadByte(bool ack)
{
	uint8_t byte = 0;
	for (uint32_t mask = 0x80; mask > 0x00; mask >>= 1)
	{
		byte |= BI2C_ReadBit() ? mask : 0x00;
	}

	BI2C_WriteBit(!ack);
	GPIO_Set(BI2C_SDA_PIN);
	return byte;
}

static void BI2C_Stop(void)
{
	// Stop condition
	US_Delay(gBi2c.clock_low_us);
	GPIO_Reset(BI2C_SDA_PIN);
	US_Delay(gBi2c.clock_high_us);
	GPIO_Set(BI2C_SCL_PIN);
	US_Delay(gBi2c.clock_high_us);
	GPIO_Set(BI2C_SDA_PIN);
}

/*
 * INTERRUPT ROUTINES
 */
