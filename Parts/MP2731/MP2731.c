#include "MP2731.h"

#include "Core.h"
#include "I2C.h"

/*
 * PRIVATE DEFINITIONS
 */

#define MP2731_ADDR 0x4B

#define MP2731_REG_IINREG	0x00 // Input current limit
#define MP2731_REG_VINREG	0x01 // Input voltage regulation
#define MP2731_REG_TREG		0x02 // NTC configuration and thermal regulation
#define MP2731_REG_CFG3		0x03 // ADC control and OTG configuration
#define MP2731_REG_CFG4		0x04 // Charge control and VSYS configuration
#define MP2731_REG_IBATREG	0x05 // Charge current configuration
#define MP2731_REG_IBATTERM	0x06 // Pre-charge and termination current
#define MP2731_REG_VBATREG	0x07 // Charge voltage regulation
#define MP2731_REG_TIM		0x08 // Timer configuration
#define MP2731_REG_BGAP		0x09 // Bandgap
#define MP2731_REG_BFET		0x0A // BATFET configuration
#define MP2731_REG_CFGB		0x0B // INT MASK and USB detection
#define MP2731_REG_STAT		0x0C // Status
#define MP2731_REG_FAULT	0x0D // Fault
#define MP2731_REG_VBAT		0x0E // ADC of battery voltage
#define MP2731_REG_VSYS		0x0F // ADC of system voltage
#define MP2731_REG_VNTC		0x10 // ADC of NTC voltage
#define MP2731_REG_VIN		0x11 // ADC of input voltage
#define MP2731_REG_IBAT		0x12 // ADC of charge current
#define MP2731_REG_IIN		0x13 // ADC of input current
#define MP2731_REG_PSTAT	0x14 // Power management status
#define MP2731_REG_DPM		0x15 // DPM mask
#define MP2731_REG_JEITA	0x16 // JEITA configuration
#define MP2731_REG_17H		0x17 // Safety timer status and part number

// MP2731_REG_TIM
#define MP2731_TIM_EN_TERM				0x80
#define MP2731_TIM_WATCHDOG_DIS			0x00
#define MP2731_TIM_WATCHDOG_40S			0x10
#define MP2731_TIM_WATCHDOG_80S			0x20
#define MP2731_TIM_WATCHDOG_160S		0x30
#define MP2731_TIM_WATCHDOG_RESET		0x08
#define MP2731_TIM_CHG_TMR_5HR			0x00
#define MP2731_TIM_CHG_TMR_8HR			0x02
#define MP2731_TIM_CHG_TMR_12HR			0x04
#define MP2731_TIM_CHG_TMR_20HR			0x06
#define MP2731_TIM_SAFETY_TMR_EN		0x01

// MP2731_REG_IINREG
#define MP2731_IINREG_HIZ				0x80
#define MP2731_IINREG_LIM				0x40

// MP2731_REG_VINREG
#define MP2731_VINREG_RESET				0x80

// MP2731_REG_STAT
#define MP2731_STAT_VIN_OTG				0xE0
#define MP2731_STAT_VIN_FASTCHARGE		0xA0
#define MP2731_STAT_VIN_DCP				0x80
#define MP2731_STAT_VIN_CDP				0x60
#define MP2731_STAT_VIN_SDP				0x40
#define MP2731_STAT_VIN_NONSTANDARD		0x20
#define MP2731_STAT_VIN_NONE			0x00

#define MP2731_STAT_CHG_DONE			0x18
#define MP2731_STAT_CHG_CC				0x10
#define MP2731_STAT_CHG_TRICKLE			0x08
#define MP2731_STAT_CHG_IDLE			0x00

#define MP2731_STAT_NTC_FLOAT			0x04
#define MP2731_STAT_THERM				0x02
#define MP2731_STAT_VSYS				0x01

// MP2731_REG_FAULT
#define MP2731_FAULT_WATCHDOG			0x80
#define MP2731_FAULT_OTG				0x40
#define MP2731_FAULT_INPUT				0x20
#define MP2731_FAULT_THERM				0x10
#define MP2731_FAULT_BATT				0x08

#define MP2731_FAULT_NTC_HOT			0x06
#define MP2731_FAULT_NTC_COLD			0x05
#define MP2731_FAULT_NTC_COOL			0x03
#define MP2731_FAULT_NTC_WARM			0x02
#define MP2731_FAULT_NTC_NORMAL			0x00


// MP2731_REG_CFG3
#define MP2731_CFG3_ADC_START			0x80
#define MP2731_CFG3_ADC_RATE			0x40

// MP2731_REG_IBATREG
#define MP2731_IBATREG_PRECHARGE_3V		0x80
#define MP2731_IBATREG_PRECHARGE_2V8	0x00

// MP2731_REG_VBATREG
#define MP2731_VBATREG_RECHARGE_200MV	0x80
#define MP2731_VBATREG_RECHARGE_100MV	0x00

// MP2731_REG_BFET
#define MP2731_BFET_SWFREQ				0x80
#define MP2731_BFET_TMR2X_EN			0x40
#define MP2731_BFET_BFET_DIS			0x20
#define MP2731_BFET_SYSRST_SW			0x10
#define MP2731_BFET_TDISC_H_0_5S		0x00
#define MP2731_BFET_TDISC_H_2S			0x04
#define MP2731_BFET_TDISC_H_4S			0x08
#define MP2731_BFET_TDISC_H_8S			0x0C
#define MP2731_BFET_TDISC_L_8S			0x00
#define MP2731_BFET_TDISC_L_10S			0x01
#define MP2731_BFET_TDISC_L_12S			0x02
#define MP2731_BFET_TDISC_L_16S			0x03

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static bool MP2731_ReadRegs(uint8_t reg, uint8_t * values, uint32_t count);
static bool MP2731_WriteRegs(uint8_t reg, const uint8_t * values, uint32_t count);
static uint8_t MP2731_ReadReg(uint8_t reg);
static bool MP2731_WriteReg(uint8_t reg, uint8_t value);

static inline uint32_t MP2731_FromQuanta(uint32_t value, uint8_t bits, uint32_t offset, uint32_t max);
static inline uint32_t MP2731_ToQuanta(uint32_t value, uint8_t bits, uint32_t offset, uint32_t max);

static bool MP2731_SetBattfet(bool set);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool MP2731_Init(void)
{
	if (I2C_Scan(MP2731_I2C, MP2731_ADDR)
			&& MP2731_SetBattfet(true)
			&& MP2731_WriteReg(MP2731_REG_TIM, MP2731_TIM_EN_TERM | MP2731_TIM_WATCHDOG_DIS | MP2731_TIM_CHG_TMR_8HR | MP2731_TIM_SAFETY_TMR_EN)
	)
	{
		return true;
	}
	return false;
}

void MP2731_Deinit(void)
{
	MP2731_SetBattfet(false);
}

MP2731_Status_t MP2731_GetStatus(void)
{
	uint8_t rx[2];
	MP2731_ReadRegs(MP2731_REG_STAT, rx, sizeof(rx));

	uint8_t chrg = rx[0] & MP2731_STAT_CHG_DONE;
	switch (chrg)
	{
	case MP2731_STAT_CHG_DONE:
		return MP2731_Status_ChargeDone;
	case MP2731_STAT_CHG_CC:
		return MP2731_Status_Charging;
	case MP2731_STAT_CHG_TRICKLE:
		return MP2731_Status_Trickle;
	//case MP2731_STAT_CHG_IDLE:
	//	break;
	}

	MP2731_Status_t status = MP2731_Status_None;

	if (rx[0] & MP2731_STAT_NTC_FLOAT) 	{ status |= MP2731_Status_NTCFloat; }
	if (rx[1] & MP2731_FAULT_INPUT) 	{ status |= MP2731_Status_InputFault; }
	if (rx[1] & MP2731_FAULT_BATT) 		{ status |= MP2731_Status_BatteryFault; }
	if (rx[1] & MP2731_FAULT_THERM) 	{ status |= MP2731_Status_ThermalFault; }
	if (rx[1] & MP2731_FAULT_WATCHDOG) 	{ status |= MP2731_Status_WatchdogFault; }
	if (rx[1] & MP2731_FAULT_OTG) 		{ status |= MP2731_Status_OTGFault; }

	return status;
}

bool MP2731_ReadAnalog(MP2731_Analog_t * values)
{
	MP2731_WriteReg(MP2731_REG_CFG3, MP2731_CFG3_ADC_START);

	// Poll for ADC completion. Normally happens on the first poll.
	uint32_t start = CORE_GetTick();
	while (MP2731_ReadReg(MP2731_REG_CFG3) & MP2731_CFG3_ADC_START)
	{
		if (CORE_GetTick() - start > 20)
		{
			return false;
		}
	}

	uint8_t regs[6];
	MP2731_ReadRegs(MP2731_REG_VBAT, regs, sizeof(regs));

	values->batt_mv 	= MP2731_FromQuanta(regs[0], 8, 0, 5100);
	values->sys_mv 		= MP2731_FromQuanta(regs[1], 8, 0, 5100);
	values->ntc 		= regs[2]; // No compensation, its ratiometric. Need to know the NTC curve.
	values->in_mv 		= MP2731_FromQuanta(regs[3], 8, 0, 15300);
	values->charge_ma 	= MP2731_FromQuanta(regs[4], 8, 0, 4462); // The datasheet is incorrect on this.
	values->in_ma 		= MP2731_FromQuanta(regs[5], 8, 0, 3390);
	return true;
}

void MP2731_SetInputLimit(uint32_t min_mv, uint32_t max_ma)
{
	if (min_mv > 15200) { min_mv = 15200; } // Apply limits.
	uint8_t ilim = MP2731_ToQuanta(max_ma, 6, 100, 3250) | MP2731_IINREG_LIM;
	uint8_t vlim = MP2731_ToQuanta(min_mv, 7, 3700, 16400); // TODO: limit to 15200

	uint8_t tx[] = {
		ilim,
		vlim,
	};
	MP2731_WriteRegs(MP2731_REG_IINREG, tx, sizeof(tx));
}

void MP2731_SetChargeLimit(uint32_t max_mv, uint32_t precharge_ma, uint32_t max_ma, uint32_t termination_ma)
{
	if (max_ma > 4520) { max_ma = 4520; } // Apply limits.
	uint8_t ireg = MP2731_ToQuanta(max_ma, 7, 320, 5400) | MP2731_IBATREG_PRECHARGE_3V;
	uint8_t iterm = (MP2731_ToQuanta(precharge_ma, 4, 150, 750) << 4) | MP2731_ToQuanta(termination_ma, 4, 120, 720);
	uint8_t vreg = (MP2731_ToQuanta(max_mv, 7, 3400, 4670) << 1) | MP2731_VBATREG_RECHARGE_100MV;

	uint8_t tx[] = {
		ireg,
		iterm,
		vreg
	};

	MP2731_WriteRegs(MP2731_REG_IBATREG, tx, sizeof(tx));
}

/*
 * PRIVATE FUNCTIONS
 */

static bool MP2731_SetBattfet(bool set)
{
	uint8_t bfet = MP2731_BFET_TMR2X_EN | MP2731_BFET_SYSRST_SW | MP2731_BFET_TDISC_H_4S | MP2731_BFET_TDISC_L_8S;
	if (!set) { bfet |= MP2731_BFET_BFET_DIS; }

	return MP2731_WriteReg(MP2731_REG_BFET, bfet);
}

static inline uint32_t MP2731_FromQuanta(uint32_t value, uint8_t bits, uint32_t offset, uint32_t max)
{
	const uint32_t mask = (1 << bits) - 1;
	return (value * (max - offset) / mask) + offset;
}

static inline uint32_t MP2731_ToQuanta(uint32_t value, uint8_t bits, uint32_t offset, uint32_t max)
{
	const uint32_t mask = (1 << bits) - 1;
	if (value < offset) { return 0; }
	if (value > max) { return mask; }
	return (value - offset) * mask / (max - offset);
}

static bool MP2731_ReadRegs(uint8_t reg, uint8_t * values, uint32_t count)
{
	return I2C_Transfer(MP2731_I2C, MP2731_ADDR, &reg, 1, values, count);
}

static bool MP2731_WriteRegs(uint8_t reg, const uint8_t * values, uint32_t count)
{
	uint8_t tx[count + 1];
	tx[0] = reg;
	memcpy(tx + 1, values, count);
	return I2C_Write(MP2731_I2C, MP2731_ADDR, tx, count + 1);
}

static uint8_t MP2731_ReadReg(uint8_t reg)
{
	uint8_t value = 0;
	I2C_Transfer(MP2731_I2C, MP2731_ADDR, &reg, 1, &value, 1);
	return value;
}

static bool MP2731_WriteReg(uint8_t reg, uint8_t value)
{
	uint8_t tx[] = {
		reg,
		value,
	};
	return I2C_Write(MP2731_I2C, MP2731_ADDR, tx, sizeof(tx));
}


/*
 * INTERRUPT ROUTINES
 */
