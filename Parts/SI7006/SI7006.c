#include "SI7006.h"

#include "Core.h"
#include "I2C.h"

/*
 * PRIVATE DEFINITIONS
 */

#define SI7006_ADDR 	0x40

#define SI7006_CMD_MEAS_RH_HOLD		0xE5
#define SI7006_CMD_MEAS_RH			0xF5
#define SI7006_CMD_MEAS_T_HOLD		0xE3
#define SI7006_CMD_MEAS_T			0xF3
#define SI7006_CMD_READ_T			0xE0
#define SI7006_CMD_RESET			0xFE
#define SI7006_CMD_WRITE_RHTR		0xE6
#define SI7006_CMD_READ_RHTR		0xE7
#define SI7006_CMD_WRITE_HCR		0x51
#define SI7006_CMD_READ_HCR			0x11
#define SI7006_CMD_READ_EID1		0xFA0F
#define SI7006_CMD_READ_EID2		0xFCC9
#define SI7006_CMD_READ_FWR			0x84B8

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static bool SI7006_ReadWord(uint8_t cmd, uint16_t * value);
static bool SI7006_Measure(uint8_t cmd, uint16_t * value, uint32_t delay);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool SI7006_Init(void)
{
	return I2C_Scan(SI7006_I2C, SI7006_ADDR);
}

void SI7006_Deinit(void)
{

}

bool SI7006_Read(int16_t * temp, uint8_t * hum)
{
	uint16_t rh_reg;
	uint16_t t_reg;

	if (   SI7006_Measure(SI7006_CMD_MEAS_RH, &rh_reg, 25)
		&& SI7006_ReadWord(SI7006_CMD_READ_T, &t_reg))
	{
		*hum = (int32_t)(rh_reg * 125 / 65536) - 6;
		*temp = (int32_t)(t_reg * 1757 / 65536) - 468;
		return true;
	}
	return false;
}

/*
 * PRIVATE FUNCTIONS
 */

static bool SI7006_ReadWord(uint8_t cmd, uint16_t * value)
{
	uint8_t rx[2];
	if (I2C_Transfer(SI7006_I2C, SI7006_ADDR, &cmd, sizeof(cmd), rx, sizeof(rx)))
	{
		*value = (rx[0] << 8) | rx[1];
		return true;
	}
	return false;
}

static bool SI7006_Measure(uint8_t cmd, uint16_t * value, uint32_t delay)
{
	if (!I2C_Write(SI7006_I2C, SI7006_ADDR, &cmd, sizeof(cmd)))
		return false;

	CORE_Delay(delay);

	uint8_t rx[2];
	if (I2C_Read(SI7006_I2C, SI7006_ADDR, rx, sizeof(rx)))
	{
		*value = (rx[0] << 8) | rx[1];
		return true;
	}
	return false;
}

/*
 * INTERRUPT ROUTINES
 */
