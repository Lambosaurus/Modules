#include "OneWire.h"
#include "GPIO.h"
#include "US.h"

#include "CRCX.h"

/*
 * PRIVATE DEFINITIONS
 */

#define D1W_CMD_READ_ROM		0x33
#define D1W_CMD_SKIP_ROM		0xCC
#define D1W_CMD_SKIP_ROM_OD		0x3C
#define D1W_CMD_MATCH_ROM		0x55
#define D1W_CMD_MATCH_ROM_OD	0x69
#define D1W_CMD_SEARCH_ROM		0xF0

#define D1W_RESET_US			500	// 480 min
#define D1W_PRESENSE_US			60  // 60 min?

#define D1W_MIN_US				5
#define D1W_SLOT_US				70
#define D1W_READ_US				5

#define DIW_NO_BRANCH			0xFF
#define D1W_ROM_BITS			64

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static bool D1W_Reset(void);
static void D1W_WriteBit(bool bit);
static bool D1W_ReadBit(void);

/*
 * PRIVATE VARIABLES
 */

static struct {
	GPIO_Pin_t pin;
	uint8_t branch;
} gD1W;

/*
 * PUBLIC FUNCTIONS
 */

void D1W_Init(GPIO_Pin_t pin)
{
	gD1W.pin = pin;
	gD1W.branch = D1W_ROM_BITS;
	GPIO_Init(gD1W.pin, GPIO_Mode_Output | GPIO_Flag_OpenDrain);
}

void D1W_Deinit(void)
{
	GPIO_Deinit(gD1W.pin);
}

bool D1W_Detect(void)
{
	return D1W_Reset();
}

bool D1W_SearchRom(uint8_t * rom)
{
	if (gD1W.branch == DIW_NO_BRANCH) { return false; }

	if (!D1W_Reset()) { return false; }

	D1W_WriteByte(D1W_CMD_SEARCH_ROM);

	uint8_t next_branch = DIW_NO_BRANCH;

	for (uint32_t i = 0; i < D1W_ROM_BITS; i++)
	{
		bool id_bit = D1W_ReadBit();
		bool inv_bit = D1W_ReadBit();

		bool next_bit;

		if (id_bit && inv_bit)
		{
			// No devices on the bus.
			return false;
		}
		else if (id_bit != inv_bit)
		{
			// No conflict. Continue.
			next_bit = id_bit;
		}
		else
		{
			// We have a conflict. Select direction.
			if (i < gD1W.branch) 		{ next_bit = rom[i >> 3] & (1 << (i & 0x07)); }
			else if (i == gD1W.branch)	{ next_bit = true; }
			else 						{ next_bit = 0; }

			// We took a low branch. Come back later.
			if (!next_bit) { next_branch = i; }
		}

		if (next_bit)	{ rom[i >> 3] |= 1 << (i & 0x07); }
		else 			{ rom[i >> 3] &= ~(1 << (i & 0x07)); }

		D1W_WriteBit(next_bit);
	}

	gD1W.branch = next_branch;
	return true;
}

bool D1W_ReadRom(uint8_t * rom)
{
	if (D1W_Reset())
	{
		D1W_WriteByte(D1W_CMD_READ_ROM);
		D1W_Read(rom,8);
		return true;
	}
	return false;
}

bool D1W_SelectRom(const uint8_t * rom)
{
	if (D1W_Reset())
	{
		if (rom == NULL)
		{
			D1W_WriteByte(D1W_CMD_SKIP_ROM);
		}
		else
		{
			D1W_WriteByte(D1W_CMD_MATCH_ROM);
			D1W_Write(rom, 8);
		}
		return true;
	}
	return false;
}

void D1W_Write(const uint8_t * tx, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		D1W_WriteByte(*tx++);
	}
}

void D1W_Read(uint8_t * rx, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		*rx++ = D1W_ReadByte();
	}
}

void D1W_WriteByte(uint8_t b)
{
	for (uint32_t i = 0; i < 8; i++)
	{
		D1W_WriteBit(b & 0x01);
		b >>= 1;
	}
}

uint8_t D1W_ReadByte(void)
{
	uint8_t b = 0;
	for (uint32_t i = 0; i < 8; i++)
	{
		b >>= 1;
		b |= D1W_ReadBit() ? 0x80 : 0;
	}
	return b;
}

uint8_t D1W_CRC(const uint8_t * bfr, uint32_t size)
{
	return CRC8R(0x00, 0x8C, bfr, size);
}

/*
 * PRIVATE FUNCTIONS
 */

static void D1W_PulseLow(uint32_t us)
{
	GPIO_Reset(gD1W.pin);
	US_Delay(us);
	GPIO_Set(gD1W.pin);
}

static void D1W_PulseHigh(uint32_t us)
{
	US_Delay(us);
}

static bool D1W_ReadPulse(uint32_t t_read, uint32_t t_slot)
{
	US_Delay(t_read);
	bool bit = GPIO_Read(gD1W.pin);
	US_Delay(t_slot - t_read);
	return bit;
}

static bool D1W_Reset(void)
{
	D1W_PulseLow(D1W_RESET_US);
	return !D1W_ReadPulse(D1W_PRESENSE_US, D1W_RESET_US);
}

static void D1W_WriteBit(bool bit)
{
	if (bit)
	{
		D1W_PulseLow(D1W_MIN_US);
		D1W_PulseHigh(D1W_SLOT_US);
	}
	else
	{
		D1W_PulseLow(D1W_SLOT_US);
		D1W_PulseHigh(D1W_MIN_US);
	}
}

static bool D1W_ReadBit(void)
{
	D1W_PulseLow(D1W_MIN_US);
	return D1W_ReadPulse(D1W_READ_US, D1W_SLOT_US);
}

/*
 * INTERRUPT ROUTINES
 */

