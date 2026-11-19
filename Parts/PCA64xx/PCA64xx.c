#include "PCA64xx.h"
#include "I2C.h"

/*
 * PRIVATE DEFINITIONS
 */

#ifndef PCA64XX_ADDR
#define PCA64XX_ADDR			0x20
#endif

#define PCA64XX_REG_INPUT		(0x00 * PCA64XX_BYTES)
#define PCA64XX_REG_OUTPUT		(0x01 * PCA64XX_BYTES)
#define PCA64XX_REG_POL			(0x02 * PCA64XX_BYTES)
#define PCA64XX_REG_CONFIG		(0x03 * PCA64XX_BYTES)

#define PCA64XX_SET_BITS(_val, _mask, _state)	((_val) = (_state) ? ((_val) | (_mask)) : ((_val) & ~(_mask)))

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

bool PCA64xx_ReadReg(uint8_t reg, uint8_t * value);
bool PCA64xx_WriteReg(uint8_t reg, uint8_t value);

static PCA64xx_Pin_t PCA64xx_ReadBlock(uint8_t reg, PCA64xx_Pin_t mask);
static bool PCA64xx_WriteBlock(uint8_t reg, PCA64xx_Pin_t mask, PCA64xx_Pin_t value);

/*
 * PRIVATE VARIABLES
 */

struct {
	PCA64xx_Pin_t out;
	PCA64xx_Pin_t dir;
} gPCA64xx;

/*
 * PUBLIC FUNCTIONS
 */

bool PCA64xx_Init(void)
{
	bzero(&gPCA64xx, sizeof(gPCA64xx));
	return 	   PCA64xx_WriteBlock(PCA64XX_REG_OUTPUT, ~0, 0)
			&& PCA64xx_WriteBlock(PCA64XX_REG_POL, ~0, 0)
			&& PCA64xx_WriteBlock(PCA64XX_REG_CONFIG, ~0, ~0);
}

bool PCA64xx_Read(PCA64xx_Pin_t pin)
{
	return PCA64xx_ReadPort(pin) != 0;
}

PCA64xx_Pin_t PCA64xx_ReadPort(PCA64xx_Pin_t pins)
{
	return PCA64xx_ReadBlock(PCA64XX_REG_INPUT, pins);
}

bool PCA64xx_Write(PCA64xx_Pin_t pins, bool state)
{
	PCA64XX_SET_BITS(gPCA64xx.out, pins, state);
	return PCA64xx_WriteBlock(PCA64XX_REG_OUTPUT, pins, gPCA64xx.out);
}

bool PCA64xx_Configure(PCA64xx_Pin_t pins, bool output)
{
	PCA64XX_SET_BITS(gPCA64xx.dir, pins, !output);
	return PCA64xx_WriteBlock(PCA64XX_REG_CONFIG, pins, gPCA64xx.dir);
}

/*
 * PRIVATE FUNCTIONS
 */

static PCA64xx_Pin_t PCA64xx_ReadBlock(uint8_t reg, PCA64xx_Pin_t mask)
{
	PCA64xx_Pin_t value = 0;
	for (uint32_t i = 0; i < PCA64XX_BITS; i += 8, reg++) {
		if ((mask >> i) & 0xFF) {
			uint8_t byte;
			PCA64xx_ReadReg(reg, &byte);
			value |= byte << i;
		}
	}
	return value & mask;
}

static bool PCA64xx_WriteBlock(uint8_t reg, PCA64xx_Pin_t mask, PCA64xx_Pin_t value)
{
	for (uint32_t i = 0; i < PCA64XX_BITS; i += 8, reg++) {
		if ((mask >> i) & 0xFF) {
			if (!PCA64xx_WriteReg(reg, (value >> i) & 0xFF))
				return false;
		}
	}
	return true;
}

bool PCA64xx_ReadReg(uint8_t reg, uint8_t * value)
{
	return I2C_Transfer(PCA64XX_I2C, PCA64XX_ADDR, &reg, 1, value, 1);
}

bool PCA64xx_WriteReg(uint8_t reg, uint8_t value)
{
	uint8_t tx[2] = { reg, value };
	return I2C_Write(PCA64XX_I2C, PCA64XX_ADDR, tx, sizeof(tx));
}

/*
 * INTERRUPT ROUTINES
 */

