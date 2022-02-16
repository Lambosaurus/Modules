
#include "VL6180.h"
#include "I2C.h"
#include "GPIO.h"

/*
 * PRIVATE DEFINITIONS
 */

#define VL6180_ADDR 				0x29

// Registers
#define VL6180_REG_MODEL_ID			0x000

// Config registers
#define VL6180_REG_INTR_CONFIG 		0x014
#define VL6180_REG_INTR_CLEAR 		0x015
#define VL6180_REG_INTR_STATUS 		0x04f

#define VL6180_REG_OUT_OF_RESET 	0x016
#define VL6180_REG_HOLD 			0x017

#define VL6180_REG_RANGE_START 		0x018
#define VL6180_REG_RANGE_STATUS 	0x04d
#define VL6180_REG_RANGE_VALUE 		0x062
#define VL6180_REG_RANGE_RATE 		0x066

#define VL6180_REG_RANGE_OFFSET		0x024

#define VL6180_REG_ALS_START		0x038
#define VL6180_REG_ALS_GAIN			0x03F
#define VL6180_REG_ALS_INTEGRATION_HI	0x040
#define VL6180_REG_ALS_INTEGRATION_LO	0x041
#define VL6180_REG_ALS_STATUS		0x050
#define VL6180_REG_ALS_VALUE		0x062


// VL6180_REG_MODEL_ID
#define VL6180_MODEL_ID 			0xb4

// VL6180_REG_HOLD
#define VL6180_HOLD_ON				0x01

// VL6180_REG_RANGE_START
#define VL6180_RANGE_START 			0x01

// VL6180_REG_RANGE_STATUS
#define VL6180_RANGE_STATUS_IDLE	0x01
#define VL6180_RANGE_STATUS_READY 	0x02
#define VL6180_RANGE_MAX_THS_INT 	0x04
#define VL6180_RANGE_MIN_THS_INT 	0x08
#define VL6180_RANGE_ERROR 			0xF0

// VL6180_REG_ALS_GAIN
#define VL6180X_ALS_GAIN_1 			0x06    ///< 1x gain
#define VL6180X_ALS_GAIN_1_25 		0x05 ///< 1.25x gain
#define VL6180X_ALS_GAIN_1_67 		0x04 ///< 1.67x gain
#define VL6180X_ALS_GAIN_2_5 		0x03  ///< 2.5x gain
#define VL6180X_ALS_GAIN_5 			0x02    ///< 5x gain
#define VL6180X_ALS_GAIN_10 		0x01   ///< 10x gain
#define VL6180X_ALS_GAIN_20 		0x00   ///< 20x gain
#define VL6180X_ALS_GAIN_40 		0x07   ///< 40x gain

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static bool VL6180_ReadByte(uint16_t reg, uint8_t * value);
static bool VL6180_WriteByte(uint16_t reg, uint8_t value);
static bool VL6180_Hold(bool hold);
static bool VL7180_WriteSettings(void);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool VL6180_Init(void)
{
	uint8_t b;
	if (VL6180_ReadByte(VL6180_REG_MODEL_ID, &b) && b == VL6180_MODEL_ID)
	{
		bool success = true;
		// It seems reading this register is not optional...
		success &= VL6180_ReadByte(VL6180_REG_OUT_OF_RESET, &b);

		success &= VL6180_Hold(true);

		success &= VL7180_WriteSettings();
		//int8_t offset = 0;
		//VL6180_ReadByte(VL6180_REG_RANGE_OFFSET, &offset);

		success &= VL6180_Hold(false);
		return success;
	}
	return false;
}

void VL6180_Deinit(void)
{
}

bool VL6180_Start(void)
{
	return VL6180_WriteByte(VL6180_REG_RANGE_START, VL6180_RANGE_START);
}

bool VL6180_IsReady(void)
{
	uint8_t status;
	if (VL6180_ReadByte(VL6180_REG_RANGE_STATUS, &status))
	{
		return (status & VL6180_RANGE_STATUS_READY);
	}
	return false;
}

bool VL6180_Read(uint32_t * range)
{
	uint8_t status;
	uint8_t b;
	if (VL6180_ReadByte(VL6180_REG_RANGE_STATUS, &status) && VL6180_ReadByte(VL6180_REG_RANGE_VALUE, &b))
	{
		*range = b;
		return (status & VL6180_RANGE_ERROR) == 0;
	}
	return false;
}

/*
 * PRIVATE FUNCTIONS
 */

static bool VL7180_WriteSettings(void)
{
	struct {
		uint16_t reg;
		uint8_t value;
	} settings[] = {

		// private settings from page 24 of app note
		{ 0x0207, 0x01 },
		{ 0x0208, 0x01 },
		{ 0x0096, 0x00 },
		{ 0x0097, 0xfd },
		{ 0x00e3, 0x00 },
		{ 0x00e4, 0x04 },
		{ 0x00e5, 0x02 },
		{ 0x00e6, 0x01 },
		{ 0x00e7, 0x03 },
		{ 0x00f5, 0x02 },
		{ 0x00d9, 0x05 },
		{ 0x00db, 0xce },
		{ 0x00dc, 0x03 },
		{ 0x00dd, 0xf8 },
		{ 0x009f, 0x00 },
		{ 0x00a3, 0x3c },
		{ 0x00b7, 0x00 },
		{ 0x00bb, 0x3c },
		{ 0x00b2, 0x09 },
		{ 0x00ca, 0x09 },
		{ 0x0198, 0x01 },
		{ 0x01b0, 0x17 },
		{ 0x01ad, 0x00 },
		{ 0x00ff, 0x05 },
		{ 0x0100, 0x05 },
		{ 0x0199, 0x05 },
		{ 0x01a6, 0x1b },
		{ 0x01ac, 0x3e },
		{ 0x01a7, 0x1f },
		{ 0x0030, 0x00 },

		// Recommended : Public registers - See data sheet for more detail
		// Enables polling for 'New Sample ready' when measurement completes
		{ 0x0011, 0x10 },
		// Set the averaging sample period (compromise between lower noise and increased execution time)
		{ 0x010a, 0x30 },
		// Sets the light and dark gain (upper nibble). Dark gain should not be changed.
		{ 0x003f, 0x46 },
		// sets the # of range measurements after which auto calibration of system is performed
		{ 0x0031, 0xFF },
		// Set ALS integration time to 100ms
		{ 0x0041, 0x63 },
		// perform a single temperature calibration of the ranging sensor
		{ 0x002e, 0x01 },

		// Optional: Public registers - See data sheet for more detail
		// Set default ranging inter-measurement period to 100ms
		{ 0x001b, 0x09 },
		// Set default ALS inter-measurement period to 500ms
		{ 0x003e, 0x31 },
		// Configures interrupt on 'New Sample Ready threshold event'
		{ 0x0014, 0x24 }
	};

	for (uint32_t i = 0; i < LENGTH(settings); i++)
	{
		if (!VL6180_WriteByte(settings[i].reg, settings[i].value))
		{
			return false;
		}
	}
	return true;
}

// HOLD is needed before updating any config registers
static bool VL6180_Hold(bool hold)
{
	return VL6180_WriteByte(VL6180_REG_HOLD, hold ? VL6180_HOLD_ON : 0);
}

static bool VL6180_ReadByte(uint16_t reg, uint8_t * value)
{
	uint8_t tx[] = {
		(uint8_t)(reg >> 8),
		(uint8_t)reg,
	};
	return I2C_Transfer(VL6180_I2C, VL6180_ADDR, tx, sizeof(tx), value, 1);
}

static bool VL6180_WriteByte(uint16_t reg, uint8_t value)
{
	uint8_t tx[] = {
		(uint8_t)(reg >> 8),
		(uint8_t)reg,
		value,
	};
	return I2C_Write(VL6180_I2C, VL6180_ADDR, tx, sizeof(tx));
}

/*
 * INTERRUPT ROUTINES
 */
