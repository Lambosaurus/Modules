#include "LIS2DT.h"
#include "GPIO.h"
#include "Core.h"

#ifdef LIS2DT_SPI
#include "SPI.h"
#else //LIS2DT_I2C
#include "I2C.h"
#endif

/*
 * PRIVATE DEFINITIONS
 */	

// I2C only
#ifdef LIS2DT_SPI

#define ADDR_WRITE		0x00
#define ADDR_READ		0x80
#define ADDR_BURST		0x40

#else //LIS2DT_I2C

#ifndef LIS2DT_ADDR
#define LIS2DT_ADDR		0x19
#endif

#define ADDR_BURST		0x80

#endif // LIS2DT_I2C

#define REG_OUT_TEMP_L	0x0D
#define REG_OUT_TEMP_H	0x0E
#define REG_WHOAMI		0x0F
#define REG_CTRL1		0x20
#define REG_CTRL2		0x21
#define REG_CTRL3		0x22
#define REG_INT1_CFG	0x23
#define REG_INT2_CFG	0x24
#define REG_CTRL6		0x25
#define REG_STATUS		0x27
#define REG_OUT_X_L		0x28
#define REG_OUT_X_H		0x29
#define REG_OUT_Y_L		0x2A
#define REG_OUT_Y_H		0x2B
#define REG_OUT_Z_L		0x2C
#define REG_OUT_Z_H		0x2D
#define REG_FIFO_CTRL	0x2E
#define REG_FIFO_SMPLS	0x2F
#define REG_TAP_THS_X	0x30
#define REG_TAP_THS_Y	0x31
#define REG_TAP_THS_Z	0x32
#define REG_TAP_DUR		0x35
#define REG_WAKE_THS	0x34
#define REG_WAKE_DUR	0x35
#define REG_FREEFALL	0x36
#define REG_STATUS_DUP	0x37
#define REG_WAKE_SRC	0x38
#define REG_TAP_SRC		0x39
#define REG_SIXD_SRC	0x3A
#define REG_INT_SRC		0x3B
#define REG_OFS_X		0x3C
#define REG_OFS_Y		0x3D
#define REG_OFS_Z		0x3E
#define REG_CTRL7		0x3F

#define WHOAMI_VALUE	0x44


#define CR1_MODE_LP			0x00
#define CR1_MODE_HP			0x04
#define CR1_MODE_SINGLE		0x08 // I assume this also uses the LP mode bits.

#define CR1_LP_MODE1		0x00 // 12 bit
#define CR1_LP_MODE2		0x01 // 14 bit
#define CR1_LP_MODE3		0x02 // 14 bit
#define CR1_LP_MODE4		0x03 // 14 bit

#define CR1_ODR_POWERDOWN	0x00
#define CR1_ODR_1_6HZ		0x10 // LP only
#define CR1_ODR_12_5HZ		0x20
#define CR1_ODR_25HZ		0x30
#define CR1_ODR_50HZ		0x40
#define CR1_ODR_100HZ		0x50
#define CR1_ODR_200HZ		0x60
#define CR1_ODR_400HZ		0x70 // HP only
#define CR1_ODR_800HZ		0x80 // HP only
#define CR1_ODR_1600HZ		0x90 // HP only

#define CR2_BOOT			0x80
#define CR2_SOFT_RST		0x40
#define CR2_CS_PU_DISC		0x10 // Disables pullup on the CS pin
#define CR2_BDU				0x08 // 1 blocks data updates until MSB & LSB read
#define CR2_IF_ADDR_INCR	0x04 // Enables address auto increment
#define CR2_I2C_DISABLE		0x02
#define CR2_SPI_3W			0x01

#define CR3_INT_OD			0x20
#define CR3_INT_LATCH		0x10 // Set: lactched interrupt mode
#define CR3_INT_POL			0x08 // Set: active low
#define CR3_TRIG_MODE		0x02 // Set: Triggered by reg write, Clear: by gpio
#define CR3_TRIGGER			0x01 // Triggers a sample

#define INT1_CFG_6D			0x80
#define INT1_CFG_TAP		0x40
#define INT1_CFG_WAKEUP		0x20
#define INT1_CFG_FREEFALL	0x10
#define INT1_CFG_DTAP		0x08
#define INT1_CFG_FIFO_FULL	0x04
#define INT1_CFG_FIFO_THS	0x02
#define INT1_CFG_DRDY		0x01

#define INT2_CFG_SLP_STATE	0x80
#define INT2_CFG_SLP_CHG	0x40
#define INT2_CFG_BOOT		0x20
#define INT2_CFG_DRDY_T		0x10
#define INT2_CFG_OVR		0x08
#define INT2_CFG_FIFO_FULL	0x04
#define INT2_CFG_FIFO_THS	0x02
#define INT2_CFG_DRDY		0x01

#define CR6_LOW_NOISE		0x04
#define CR6_FLTR_HP			0x08 // Set: High pass, Clear: Low pass
#define CR6_FS_2G			0x00
#define CR6_FS_4G			0x10
#define CR6_FS_8G			0x20
#define CR6_FS_16G			0x30
#define CR6_FS_MASK			0x30
#define CR6_FLTR_2			0x00 // Filter bandwidth is ODR/2
#define CR6_FLTR_4			0x40 // Filter bandwidth is ODR/4
#define CR6_FLTR_10			0x80 // Filter bandwidth is ODR/10
#define CR6_FLTR_20			0xC0 // Filter bandwidth is ODR/20

#define CR7_DRDY_PULSED		0x80
#define CR7_INT2_ON_INT1	0x40
#define CR7_INT_ENABLE		0x20
#define CR7_OFFS_ON_OUT		0x10
#define CR7_OFFS_ON_WU		0x08
#define CR7_OFFS_WEIGHT		0x04
#define CR7_HP_REF_MODE		0x02
#define CR7_LP_ON_6D		0x01

/*
#define WAKE_THS_TAP		0x80 // Enable single or double tap
#define WAKE_THS_SLEEP		0x40 // Enable sleep mode while waiting for threshold
#define WAKE_THS_MASK		0x3F // Threshold (in units of fs/64)

#define WAKE_DUR_FALL5		0x80
#define WAKE_DUR_MASK		0x60
#define WAKE_DIR_STATIONARY	0x10
#define WAKE_DUR_MASK		0x0F // Time to stay in wake mode (in 512 ODR)
*/

#define TAP_THS_MASK		0x1F

#define TAP_THS_Z_X_EN		0x80
#define TAP_THS_Z_Y_EN		0x40
#define TAP_THS_Z_Z_EN		0x20
#define TAP_THS_Z_XYZ_EN	(TAP_THS_Z_X_EN | TAP_THS_Z_Y_EN | TAP_THS_Z_Z_EN)

#define TAP_DUR_SHOCK_MAX		0x03
#define TAP_DUR_SHOCK_POS		0
#define TAP_DUR_QUIET_MAX		0x03
#define TAP_DUR_QUIET_POS		2
#define TAP_DUR_LATENCY_MAX		0x0F
#define TAP_DUR_LATENCY_POS		4


// Note the sign extention for the 16 bit number.
#define LIS2DT_ADC_TO_MG(b1, b2, fs)	(((int32_t)(int16_t)((b1) | ((b2) << 8)) * fs) >> 15)


#define PLACE_BITS(value, pos, max)	( (value & max) << pos )


/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

#ifdef LIS2DT_SPI
static inline void LIS2DT_Select(void);
static inline void LIS2DT_Deselect(void);
#endif // LIS2DT_SPI

static void LIS2DT_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count);
static void LIS2DT_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count);
static inline uint8_t LIS2DT_ReadReg(uint8_t reg);
static inline void LIS2DT_WriteReg(uint8_t reg, uint8_t data);

static void LIS2DT_INT_IRQHandler(void);

static uint8_t LIS2DT_CR1_GetODR(uint16_t f);
static uint8_t LIS2DT_CR6_GetFS(uint8_t s);
static uint8_t LIS2DT_CR6_GetFLTR(uint8_t div);

/*
 * PRIVATE VARIABLES
 */

static struct {
	bool int_set;
	uint8_t scale_g;
} gLis2dt;

/*
 * PUBLIC FUNCTIONS
 */

bool LIS2DT_Init(uint8_t scale_g, uint16_t frequency, bool high_res)
{
	gLis2dt.int_set = false;
	gLis2dt.scale_g = scale_g;

#ifdef LIS2DT_SPI
	GPIO_EnableOutput(LIS2DT_CS_PIN, GPIO_PIN_SET);
#endif
	GPIO_EnableInput(LIS2DT_INT_PIN, GPIO_Pull_Up);
	GPIO_OnChange(LIS2DT_INT_PIN, GPIO_IT_Falling, LIS2DT_INT_IRQHandler);

	bool success = LIS2DT_ReadReg(REG_WHOAMI) == WHOAMI_VALUE;
	if (success)
	{
		LIS2DT_WriteReg(REG_CTRL2, CR2_SOFT_RST);
		CORE_Delay(5);

		uint8_t cr1 = 0;
		uint8_t cr2 = 0;
		uint8_t cr3 = 0;
		uint8_t cr6 = 0;
		uint8_t cr7 = 0;

		// high_res -> 14 bit mode
		cr1 |= high_res ? CR1_MODE_HP : CR1_MODE_LP;
		cr1 |= high_res ? CR1_LP_MODE4 : CR1_LP_MODE1;
		cr1 |= LIS2DT_CR1_GetODR(frequency);

#ifdef LIS2DT_SPI
		cr2 = CR2_BDU | CR2_IF_ADDR_INCR | CR2_I2C_DISABLE;
#else //LIS2DT_I2C
		cr2 = CR2_BDU | CR2_IF_ADDR_INCR;
#endif
		// CR2 enables address incrementing. This must be written.
		LIS2DT_WriteReg(REG_CTRL2, cr2);

		// Leave interrupt as momentary, active low.
		cr3 = CR3_TRIG_MODE | CR3_INT_OD | CR3_INT_POL;
		cr6 = LIS2DT_CR6_GetFS(gLis2dt.scale_g);
		cr7 = CR7_DRDY_PULSED | CR7_INT_ENABLE;

		// Now write all the sequential regs in a single transaction
		uint8_t cr[] = { cr1, cr2, cr3, 0, 0, cr6 };
		LIS2DT_WriteRegs(REG_CTRL1, cr, sizeof(cr));
		LIS2DT_WriteReg(REG_CTRL7, cr7);
	}

	return success;
}

void LIS2DT_Deinit(void)
{
	GPIO_Deinit(LIS2DT_INT_PIN);
	GPIO_OnChange(LIS2DT_INT_PIN, GPIO_IT_None, NULL);
	LIS2DT_WriteReg(REG_CTRL2, CR2_SOFT_RST);
}

void LIS2DT_EnableDataInt(void)
{
	LIS2DT_WriteReg(REG_INT1_CFG, INT1_CFG_DRDY);
}

void LIS2DT_EnableThresholdInt(uint16_t threshold)
{
	uint16_t thrs = ((uint32_t)threshold * TAP_THS_MASK) / (gLis2dt.scale_g * 1000);
	thrs &= TAP_THS_MASK;

	// We are selecting the maximum on-time, and no quiet-time.
	uint8_t tap_dur = PLACE_BITS(TAP_DUR_SHOCK_MAX, TAP_DUR_SHOCK_POS, TAP_DUR_SHOCK_MAX);

	uint8_t tap[] = {
		thrs, // threshold x
		thrs, // threshold y
		thrs | TAP_THS_Z_XYZ_EN, // threshold z
		tap_dur,
	};

	LIS2DT_WriteRegs(REG_TAP_THS_X, tap, sizeof(tap));
	LIS2DT_WriteReg(REG_INT1_CFG, INT1_CFG_TAP);
}

void LIS2DT_EnableFilter(uint8_t div, bool high_pass)
{
	uint8_t cr6 = LIS2DT_CR6_GetFS(gLis2dt.scale_g);
	cr6 |= high_pass ? CR6_FLTR_HP : 0;
	cr6 |= LIS2DT_CR6_GetFLTR(div);
	LIS2DT_WriteReg(REG_CTRL6, cr6);
}

bool LIS2DT_IsIntSet(void)
{
	return gLis2dt.int_set;
}

void LIS2DT_Read(LIS2DT_Accel_t * acc)
{
	gLis2dt.int_set = false;

	uint8_t data[6];
	LIS2DT_ReadRegs(REG_OUT_X_L, data, sizeof(data));

	int32_t full_scale = gLis2dt.scale_g * 1000;

	acc->x = LIS2DT_ADC_TO_MG(data[0], data[1], full_scale);
	acc->y = LIS2DT_ADC_TO_MG(data[2], data[3], full_scale);
	acc->z = LIS2DT_ADC_TO_MG(data[4], data[5], full_scale);
}

/*
 * PRIVATE FUNCTIONS
 */

static uint8_t LIS2DT_CR1_GetODR(uint16_t f)
{
	if 			(f < 12) 	{ return CR1_ODR_1_6HZ;  }
	else if 	(f < 25) 	{ return CR1_ODR_12_5HZ; }
	else if 	(f < 50) 	{ return CR1_ODR_25HZ;   }
	else if 	(f < 100)	{ return CR1_ODR_50HZ;   }
	else if 	(f < 200) 	{ return CR1_ODR_100HZ;  }
	else if 	(f < 400) 	{ return CR1_ODR_200HZ;  }
	else if 	(f < 800) 	{ return CR1_ODR_400HZ;  }
	else if 	(f < 1600) 	{ return CR1_ODR_800HZ;  }
	else					{ return CR1_ODR_1600HZ; }

}

static uint8_t LIS2DT_CR6_GetFS(uint8_t s)
{
	if 			(s < 4) 	{ return CR6_FS_2G;  }
	else if 	(s < 8) 	{ return CR6_FS_4G;  }
	else if 	(s < 16) 	{ return CR6_FS_8G;  }
	else 					{ return CR6_FS_16G; }
}

static uint8_t LIS2DT_CR6_GetFLTR(uint8_t div)
{
	if 			(div < 4) 	{ return CR6_FLTR_2;  }
	else if 	(div < 10) 	{ return CR6_FLTR_4;  }
	else if 	(div < 20) 	{ return CR6_FLTR_10; }
	else 					{ return CR6_FLTR_20; }
}

static inline uint8_t LIS2DT_ReadReg(uint8_t reg)
{
	uint8_t v;
	LIS2DT_ReadRegs(reg, &v, 1);
	return v;
}

static inline void LIS2DT_WriteReg(uint8_t reg, uint8_t data)
{
	LIS2DT_WriteRegs(reg, &data, 1);
}

#ifdef LIS2DT_SPI
static void LIS2DT_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count)
{
	uint8_t header = reg | ADDR_WRITE | ADDR_BURST;
	LIS2DT_Select();
	SPI_Write(LIS2DT_SPI, &header, 1);
	SPI_Write(LIS2DT_SPI, data, count);
	LIS2DT_Deselect();
}

static void LIS2DT_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count)
{
	uint8_t header = reg | ADDR_READ | ADDR_BURST;
	LIS2DT_Select();
	SPI_Write(LIS2DT_SPI, &header, 1);
	SPI_Read(LIS2DT_SPI, data, count);
	LIS2DT_Deselect();
}

static inline void LIS2DT_Select(void)
{
	GPIO_Reset(LIS2DT_CS_PIN);
}

static inline void LIS2DT_Deselect(void)
{
	GPIO_Set(LIS2DT_CS_PIN);
}
#else // LIS2DT_I2C
static void LIS2DT_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count)
{
	// Ignore the error

	uint8_t tx[count + 1];
	tx[0] = reg | ADDR_BURST;
	memcpy(tx+1, data, count);
	I2C_Write(LIS2DT_I2C, LIS2DT_ADDR, tx, count+1);
}

static void LIS2DT_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count)
{
	uint8_t tx = reg | ADDR_BURST;
	if (!I2C_Transfer(LIS2DT_I2C, LIS2DT_ADDR, &tx, 1, data, count))
	{
		// If the I2C transfer failed - then zero everything out to at least make behavior well defined.
		bzero(data, count);
	}
}
#endif // LIS2_I2C

/*
 * INTERRUPT ROUTINES
 */

static void LIS2DT_INT_IRQHandler(void)
{
	gLis2dt.int_set = true;
}
