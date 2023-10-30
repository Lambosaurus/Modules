
#include "PCF8523.h"
#include "I2C.h"

/*
 * PRIVATE DEFINITIONS
 */

#define PCF8523_ADDR			0x51

#define PCF8523_REG_CTL1		0x00
#define PCF8523_REG_CTL2		0x01
#define PCF8523_REG_OFFSET		0x02
#define PCF8523_REG_RAM			0x03
#define PCF8523_REG_SEC			0x04
#define PCF8523_REG_MIN			0x05
#define PCF8523_REG_HR			0x06
#define PCF8523_REG_DAY			0x07
#define PCF8523_REG_WDAY		0x08
#define PCF8523_REG_MON			0x09
#define PCF8523_REG_YR			0x0A
#define PCF8523_REG_ALARM_SEC	0x0B
#define PCF8523_REG_ALARM_MIN	0x0C
#define PCF8523_REG_ALARM_HR	0x0D
#define PCF8523_REG_ALARM_DAY	0x0E
#define PCF8523_REG_ALARM_WDAY	0x0F
#define PCF8523_REG_TIMER_VAL	0x10
#define PCF8523_REG_TIMER_MODE	0x11

#define PCF8523_CTL1_EXT_TEST	0x80
#define PCF8523_CTL1_STOP		0x20
#define PCF8523_CTL1_SR			0x10
#define PCF8523_CTL1_CIE		0x04
#define PCF8523_CTL1_12H		0x02
#define PCF8523_CTL1_CAL_SEL	0x01

#define PCF8523_CTL2_AIE		0x80
#define PCF8523_CTL2_AF			0x40
#define PCF8523_CTL2_MI			0x20
#define PCF8523_CTL2_HMI		0x10
#define PCF8523_CTL2_TF			0x08
#define PCF8523_CTL2_COF_MASK	0x07

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool PCF8523_Init(void)
{
	uint8_t tx[] = {
		PCF8523_REG_CTL1,
		0
	};
	return I2C_Write(PCF8523_I2C, PCF8523_ADDR, tx, sizeof(tx));
}

bool PCF8523_Read(DateTime_t * dt)
{
	uint8_t tx[] = {
		PCF8523_REG_SEC
	};
	uint8_t rx[7];
	if (I2C_Transfer(PCF8523_I2C, PCF8523_ADDR, tx, sizeof(tx), rx, sizeof(rx)))
	{
		//bool osc_stopped = rx[0] & 0x80;
		dt->second = rx[0] & 0x7F;
		dt->minute = rx[1];
		dt->hour = rx[2];
		dt->day = rx[3];
		dt->month = rx[5];
		dt->year = rx[6] + 2000;
		dt->millis = 0;
		return true;
	}
	return false;
}

bool PCF8523_Write(const DateTime_t * dt)
{
	uint8_t tx[] = {
		PCF8523_REG_SEC,
		dt->second,
		dt->minute,
		dt->hour,
		dt->day,
		0, // Weekday
		dt->month,
		dt->year - 2000,
	};

	return I2C_Write(PCF8523_I2C, PCF8523_ADDR, tx, sizeof(tx));
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

