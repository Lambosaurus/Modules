#include "MCP425.h"

#include "SPI.h"
#include "GPIO.h"

/*
 * PRIVATE DEFINITIONS
 */

#define MCP425_SPI_BITRATE	10000000

#define MCP425_CMD_WRITE	0x00
#define MCP425_CMD_INCR		0x01
#define MCP425_CMD_DECR		0x02
#define MCP425_CMD_READ		0x03

#define MCP425_REG_WP0		0x00
#define MCP425_REG_WP1		0x01
#define MCP425_REG_TCON		0x04
#define MCP425_REG_STAT		0x05

#define MCP425_STAT_SHDN	0x02

#define MCP425_T_HW			0x08

#define MCP425_R_STEPS		256
#define MCP425_R_WIPER		75

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void MCP425_Select(void);
static void MCP425_Deselect(void);
static uint32_t MCP425_Read(uint32_t reg);
static void MCP425_Write(uint32_t reg, uint32_t value);

/*
 * PRIVATE VARIABLES
 */

static uint8_t gTCON = 0xFF;

/*
 * PUBLIC FUNCTIONS
 */

bool MCP425_Init(void)
{
	GPIO_EnableOutput(MCP425_CS_GPIO, MCP425_CS_PIN, GPIO_PIN_SET);

	uint32_t status = MCP425_Read(MCP425_REG_STAT);
	return (status & 0x02) == 0;
}

void MCP425_Deinit(void)
{
	GPIO_Deinit(MCP425_CS_GPIO, MCP425_CS_PIN);
}

void MCP425_SetTerminals(uint8_t ch, MCP425_Terminal_t tcon)
{
	// This bit disables hardware if not set
	tcon |= MCP425_T_HW;

	switch (ch)
	{
	case 0:
		gTCON = (gTCON & 0xF0) | tcon;
		break;
	case 1:
		gTCON = (gTCON & 0x0F) | (tcon << 4);
		break;
	}

	MCP425_Write(MCP425_REG_TCON, gTCON);
}

void MCP425_SetResistance(uint8_t ch, uint32_t resistance)
{
	uint32_t pos = (resistance - MCP425_R_WIPER) * MCP425_R_STEPS / MCP425_R_MAX;
	if (pos > MCP425_R_STEPS) { pos = MCP425_R_STEPS; }

	switch (ch)
	{
	case 0:
		MCP425_Write(MCP425_REG_WP0, pos);
		break;
	case 1:
		MCP425_Write(MCP425_REG_WP1, pos);
		break;
	}
}

/*
 * PRIVATE FUNCTIONS
 */

static uint32_t MCP425_Read(uint32_t reg)
{
	MCP425_Select();

	uint8_t tx[2] = {
			(reg << 4) | (MCP425_CMD_READ << 2),
			0
	};
	uint8_t rx[2];
	SPI_Transfer(MCP425_SPI, tx, rx, sizeof(tx));

	MCP425_Deselect();

	return ((rx[0] & 0x01) << 8) | rx[1];
}

static void MCP425_Write(uint32_t reg, uint32_t value)
{
	MCP425_Select();

	uint8_t tx[2] = {
		(reg << 4) | ((MCP425_CMD_WRITE << 2) | 0x02) | ((value >> 8) & 0x01),
		value
	};

	BSPI_Write(MCP425_SPI, tx, sizeof(tx));

	MCP425_Deselect();
}

static void MCP425_Select(void)
{
	// Might as well init & deinit the SPI for every transaction
	// As we do so few read/writes
	SPI_Init(MCP425_SPI, MCP425_SPI_BITRATE, SPI_Mode_0);
	GPIO_Reset(MCP425_CS_GPIO, MCP425_CS_PIN);
}

static void MCP425_Deselect(void)
{
	GPIO_Set(MCP425_CS_GPIO, MCP425_CS_PIN);
	SPI_Deinit(MCP425_SPI);
}

/*
 * INTERRUPT ROUTINES
 */
