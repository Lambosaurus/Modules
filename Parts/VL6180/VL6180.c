
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


/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static bool VL6180_ReadByte(uint16_t reg, uint8_t * value);
static bool VL6180_WriteByte(uint16_t reg, uint8_t value);
static bool VL6180_Hold(bool hold);

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
