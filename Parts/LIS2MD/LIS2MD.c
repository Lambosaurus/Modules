#include "LIS2MD.h"
#include "GPIO.h"
#include "Core.h"

#ifdef LIS2MD_SPI
#include "SPI.h"
#else //LIS2MD_I2C
#include "I2C.h"
#endif

/*
 * PRIVATE DEFINITIONS
 */	

// I2C only
#ifdef LIS2MD_SPI

#define ADDR_WRITE		0x00
#define ADDR_READ		0x80

#else //LIS2MD_I2C

#ifndef LIS2MD_ADDR
#define LIS2MD_ADDR		0x1E
#endif

#endif // LIS2MD_I2C


#define LIS2ML_REG_OFFSETX_L  	0x45
#define LIS2ML_REG_OFFSETX_H  	0x46
#define LIS2ML_REG_OFFSETY_L  	0x47
#define LIS2ML_REG_OFFSETY_H  	0x48
#define LIS2ML_REG_OFFSETZ_L  	0x49
#define LIS2ML_REG_OFFSETZ_H  	0x4A
#define LIS2ML_REG_WHOAMI  		0x4F
#define LIS2ML_REG_CFGA 		0x60
#define LIS2ML_REG_CFGB 		0x61
#define LIS2ML_REG_CFGC 		0x62
#define LIS2ML_REG_INT_CTRL 	0x63
#define LIS2ML_REG_INT_SRC 		0x64
#define LIS2ML_REG_INT_THS_L 	0x65
#define LIS2ML_REG_INT_THS_H 	0x66
#define LIS2ML_REG_STATUS 		0x67
#define LIS2ML_REG_OUTX_L 		0x68
#define LIS2ML_REG_OUTX_H 		0x69
#define LIS2ML_REG_OUTY_L 		0x6A
#define LIS2ML_REG_OUTY_H 		0x6B
#define LIS2ML_REG_OUTZ_L 		0x6C
#define LIS2ML_REG_OUTZ_H 		0x6D
#define LIS2ML_REG_OUTT_L 		0x6E
#define LIS2ML_REG_OUTT_H 		0x6F


// LIS2ML_REG_WHOAMI
#define LIS2ML_WHOAMI_WHOAMI	0x40

// LIS2ML_REG_CFGA
#define LIS2ML_CFGA_MD_CONT		0x00
#define LIS2ML_CFGA_MD_SINGLE	0x01
#define LIS2ML_CFGA_MD_IDLE		0x03

#define LIS2ML_CFGA_ODR_10HZ	0x00
#define LIS2ML_CFGA_ODR_20HZ	0x04
#define LIS2ML_CFGA_ODR_50HZ	0x08
#define LIS2ML_CFGA_ODR_100HZ	0x0C

#define LIS2ML_CFGA_LP			0x10
#define LIS2ML_CFGA_SOFT_RST	0x20
#define LIS2ML_CFGA_REBOOT		0x40
#define LIS2ML_CFGA_TEMP_COMP	0x80

// LIS2ML_REG_CFGB
#define LIS2ML_CFGB_LPF					0x01
#define LIS2ML_CFGB_OFF_CANC			0x02
#define LIS2ML_CFGB_SET_FREQ			0x04
#define LIS2ML_CFGB_INT_ON_DATAOFF		0x08
#define LIS2ML_CFGB_OFF_CANC_ONESHOT	0x10

// LIS2ML_REG_CFGC
#define LIS2ML_CFGC_PIN_DRDY			0x01 // Configures the INT pin for DRDY
#define LIS2ML_CFGC_SELFTEST			0x02
#define LIS2ML_CFGC_4WSPI				0x04
#define LIS2ML_CFGC_BLE					0x08
#define LIS2ML_CFGC_BDU					0x10
#define LIS2ML_CFGC_I2C_DIS				0x20
#define LIS2ML_CFGC_PIN_INT				0x40 // Configures the INT pin for the interrupt engine

// LIS2ML_REG_INT_CTRL
#define LIS2ML_INT_CTRL_IEN				0x01
#define LIS2ML_INT_CTRL_IEL				0x02
#define LIS2ML_INT_CTRL_IEA				0x04
#define LIS2ML_INT_CTRL_ZIEN			0x20
#define LIS2ML_INT_CTRL_YIEN			0x40
#define LIS2ML_INT_CTRL_XIEN			0x80

// LIS2ML_REG_INT_SRC
#define LIS2ML_INT_SRC_INT				0x01
#define LIS2ML_INT_SRC_MROI				0x02
#define LIS2ML_INT_SRC_THS_N_Z			0x04
#define LIS2ML_INT_SRC_THS_N_Y			0x08
#define LIS2ML_INT_SRC_THS_N_X			0x10
#define LIS2ML_INT_SRC_THS_P_Z			0x20
#define LIS2ML_INT_SRC_THS_P_Y			0x40
#define LIS2ML_INT_SRC_THS_P_X			0x80

// LIS2ML_REG_STATUS
#define LIS2ML_STATUS_XDA				0x01
#define LIS2ML_STATUS_YDA				0x02
#define LIS2ML_STATUS_ZDA				0x04
#define LIS2ML_STATUS_ZXYDA				0x08
#define LIS2ML_STATUS_XOR				0x10
#define LIS2ML_STATUS_YOR				0x20
#define LIS2ML_STATUS_ZOR				0x40
#define LIS2ML_STATUS_ZXYOR				0x80

#define LIS2ML_SENSITIVITY_MG			1500

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

#ifdef LIS2MD_SPI
static inline void LIS2MD_Select(void);
static inline void LIS2MD_Deselect(void);
#endif // LIS2MD_SPI

static void LIS2MD_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count);
static void LIS2MD_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count);
static inline uint8_t LIS2MD_ReadReg(uint8_t reg);
static inline void LIS2MD_WriteReg(uint8_t reg, uint8_t data);

static uint8_t LIS2MD_CFGA_GetODR(uint16_t freq);

static void LIS2MD_Reset(void);


/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool LIS2MD_Init(uint16_t frequency, bool high_res)
{
#ifdef LIS2MD_SPI
	GPIO_EnableOutput(LIS2MD_CS_PIN, GPIO_PIN_SET);
#endif

	bool success = LIS2MD_ReadReg(LIS2ML_REG_WHOAMI) == LIS2ML_WHOAMI_WHOAMI;
	if (success)
	{
		LIS2MD_Reset();
		CORE_Delay(5);

		uint8_t cfga = LIS2ML_CFGA_MD_CONT | LIS2ML_CFGA_TEMP_COMP | LIS2MD_CFGA_GetODR(frequency);
		uint8_t cfgb = LIS2ML_CFGB_INT_ON_DATAOFF | LIS2ML_CFGB_OFF_CANC | LIS2ML_CFGB_OFF_CANC_ONESHOT;
		uint8_t cfgc = LIS2ML_CFGC_BDU;

#ifdef LIS2MD_SPI
		cfgc |= LIS2ML_CFGC_4WSPI | LIS2ML_CFGC_I2C_DIS;
#endif

		if (!high_res)
		{
			cfga |= LIS2ML_CFGA_LP;
		}

		if (frequency >= 20)
		{
			// Enable the low pass filter at higher data rates.
			cfgb |= LIS2ML_CFGB_LPF;
		}

		uint8_t regs[] = {
			cfga,
			cfgb,
			cfgc,
		};
		LIS2MD_WriteRegs(LIS2ML_REG_CFGA, regs, sizeof(regs));
	}

	return success;
}

void LIS2MD_Deinit(void)
{
	LIS2MD_Reset();
}

void LIS2MD_Read(LIS2MD_Mag_t * mag)
{
	uint8_t regs[6];
	LIS2MD_ReadRegs(LIS2ML_REG_OUTX_L, regs, sizeof(regs));

	int16_t x = regs[0] | (regs[1] << 8);
	int16_t y = regs[2] | (regs[3] << 8);
	int16_t z = regs[4] | (regs[5] << 8);

	mag->x = x * LIS2ML_SENSITIVITY_MG / 1000;
	mag->y = y * LIS2ML_SENSITIVITY_MG / 1000;
	mag->z = z * LIS2ML_SENSITIVITY_MG / 1000;
}

void LIS2MD_Calibrate(LIS2MD_Mag_t * offset)
{
	int16_t x = offset->x * 1000 / LIS2ML_SENSITIVITY_MG;
	int16_t y = offset->y * 1000 / LIS2ML_SENSITIVITY_MG;
	int16_t z = offset->z * 1000 / LIS2ML_SENSITIVITY_MG;

	uint8_t regs[] = {
		x & 0xFF, x >> 8,
		y & 0xFF, y >> 8,
		z & 0xFF, z >> 8,
	};

	LIS2MD_WriteRegs(LIS2ML_REG_OFFSETX_L, regs, sizeof(regs));
}

/*
 * PRIVATE FUNCTIONS
 */

static uint8_t LIS2MD_CFGA_GetODR(uint16_t f)
{
	if 			(f < 20) 	{ return LIS2ML_CFGA_ODR_10HZ;  }
	else if 	(f < 50) 	{ return LIS2ML_CFGA_ODR_20HZ; }
	else if 	(f < 100) 	{ return LIS2ML_CFGA_ODR_50HZ;   }
	else					{ return LIS2ML_CFGA_ODR_100HZ; }
}

static void LIS2MD_Reset(void)
{
	LIS2MD_WriteReg(LIS2ML_REG_CFGA, LIS2ML_CFGA_SOFT_RST);
}


static inline uint8_t LIS2MD_ReadReg(uint8_t reg)
{
	uint8_t v;
	LIS2MD_ReadRegs(reg, &v, 1);
	return v;
}

static inline void LIS2MD_WriteReg(uint8_t reg, uint8_t data)
{
	LIS2MD_WriteRegs(reg, &data, 1);
}

#ifdef LIS2MD_SPI
static void LIS2MD_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count)
{
	uint8_t header = reg | ADDR_WRITE;
	LIS2MD_Select();
	SPI_Write(LIS2MD_SPI, &header, 1);
	SPI_Write(LIS2MD_SPI, data, count);
	LIS2MD_Deselect();
}

static void LIS2MD_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count)
{
	uint8_t header = reg | ADDR_READ;
	LIS2MD_Select();
	SPI_Write(LIS2MD_SPI, &header, 1);
	SPI_Read(LIS2MD_SPI, data, count);
	LIS2MD_Deselect();
}

static inline void LIS2MD_Select(void)
{
	GPIO_Reset(LIS2MD_CS_PIN);
}

static inline void LIS2MD_Deselect(void)
{
	GPIO_Set(LIS2MD_CS_PIN);
}
#else // LIS2MD_I2C
static void LIS2MD_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count)
{
	// Ignore the error
	uint8_t tx[count + 1];
	tx[0] = reg;
	memcpy(tx+1, data, count);
	I2C_Write(LIS2MD_I2C, LIS2MD_ADDR, tx, count+1);
}

static void LIS2MD_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count)
{
	uint8_t tx = reg;
	if (!I2C_Transfer(LIS2MD_I2C, LIS2MD_ADDR, &tx, 1, data, count))
	{
		// If the I2C transfer failed - then zero everything out to at least make behavior well defined.
		bzero(data, count);
	}
}
#endif // LIS2MD_I2C

/*
 * INTERRUPT ROUTINES
 */

