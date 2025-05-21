
#include "VEML3328.h"
#include "I2C.h"

/*
 * PRIVATE DEFINITIONS
 */

#define VEML3328_ADDR	0x10

#define VEML3328_ID		0x28

#define VEML_REG_CR		0x00
#define VEML_REG_CLEAR	0x04
#define VEML_REG_RED	0x05
#define VEML_REG_GREEN	0x06
#define VEML_REG_BLUE	0x07
#define VEML_REG_IR		0x08
#define VEML_REG_ID		0x0C

#define VEML_CR_SD_ON		0x0000
#define VEML_CR_SD_OFF		0x8001
#define VEML_CR_SD_ALS		0x4000

#define VEML_CR_DG_1X		0x0000
#define VEML_CR_DG_2X		0x1000
#define VEML_CR_DG_4X		0x2000

#define VEML_CR_GAIN_2DIV	0x0C00
#define VEML_CR_GAIN_1X		0x0000
#define VEML_CR_GAIN_2X		0x0400
#define VEML_CR_GAIN_4X		0x0800

#define VEML_CR_SENS		0x0040 // Applies a 1/3 gain

#define VEML_CR_IT_50MS		0x0000
#define VEML_CR_IT_100MS	0x0010
#define VEML_CR_IT_200MS	0x0020
#define VEML_CR_IT_400MS	0x0030

#define VEML_CR_AF			0x0008
#define VEML_CR_TRIG		0x0004

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static uint16_t VEML3328_ReadWord(uint8_t cmd);
static void VEML3328_WriteWord(uint8_t cmd, uint16_t data);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool VEML3328_Init(VEML3328_Gain_t gain)
{
	uint16_t id = VEML3328_ReadWord(VEML_REG_ID);
	if ((id & 0x00FF) == VEML3328_ID)
	{
		uint16_t cr = VEML_CR_SD_ON
					| gain
					| VEML_CR_IT_50MS;

		VEML3328_WriteWord(VEML_REG_CR, cr);
		return true;
	}
	return false;
}

void VEML3328_Deinit(void)
{
	VEML3328_WriteWord(VEML_REG_CR, VEML_CR_SD_OFF);
}

void VEML3328_Read(VEML3328_Values_t * v)
{
	v->w = VEML3328_ReadWord(VEML_REG_CLEAR);
	v->r = VEML3328_ReadWord(VEML_REG_RED);
	v->g = VEML3328_ReadWord(VEML_REG_GREEN);
	v->b = VEML3328_ReadWord(VEML_REG_BLUE);

	// Ok. Lets confirm that we havent crossed a sampling boundary.
	// We really want to make sure we have coherant data.
	uint16_t w2 = VEML3328_ReadWord(VEML_REG_CLEAR);
	if (v->w != w2)
	{
		v->w = w2;
		v->r = VEML3328_ReadWord(VEML_REG_RED);
		v->g = VEML3328_ReadWord(VEML_REG_GREEN);
		v->b = VEML3328_ReadWord(VEML_REG_BLUE);
	}
}

/*
 * PRIVATE FUNCTIONS
 */

static uint16_t VEML3328_ReadWord(uint8_t reg)
{
	uint8_t rx[2];
	I2C_Transfer(VEML3328_I2C, VEML3328_ADDR, &reg, sizeof(reg), rx, sizeof(rx));
	return rx[0] | (rx[1] << 8);
}

static void VEML3328_WriteWord(uint8_t reg, uint16_t word)
{
	uint8_t tx[] = {
		reg,
		word & 0xFF,
		word >> 8
	};
	I2C_Write(VEML3328_I2C, VEML3328_ADDR, tx, sizeof(tx));
}

/*
 * INTERRUPT ROUTINES
 */

