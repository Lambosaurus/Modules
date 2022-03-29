#include "LIS2DT.h"
#include "GPIO.h"
#include "Core.h"

#ifdef LIS2_SPI
#include "SPI.h"
#else //LIS2_I2C
#include "I2C.h"
#endif

/*
 * PRIVATE DEFINITIONS
 */	

// I2C only
#ifdef LIS2_SPI

#define ADDR_WRITE		0x00
#define ADDR_READ		0x80
#define ADDR_BURST		0x40

#else //LIS2_I2C

#define LIS2_ADDR		0x18
#define ADDR_BURST		0x80

#endif

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


#define CR1_MODE_LP		0x00
#define CR1_MODE_HP		0x04
#define CR1_MODE_SINGLE	0x08 // I assume this also uses the LP mode bits.

#define CR1_LP_MODE1	0x00 // 12 bit
#define CR1_LP_MODE2	0x01 // 14 bit
#define CR1_LP_MODE3	0x02 // 14 bit
#define CR1_LP_MODE4	0x03 // 14 bit


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
#define LIS2_ADC_TO_MG(b1, b2, fs)	(((int32_t)(int16_t)((b1) | ((b2) << 8)) * fs) >> 15)


#define PLACE_BITS(value, pos, max)	( (value & max) << pos )


/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

#ifdef LIS2_SPI
static inline void LIS2_Select(void);
static inline void LIS2_Deselect(void);
#endif // LIS2_SPI

static void LIS2_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count);
static void LIS2_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count);
static inline uint8_t LIS2_ReadReg(uint8_t reg);
static inline void LIS2_WriteReg(uint8_t reg, uint8_t data);

static void LIS2_INT_IRQHandler(void);

static uint8_t LIS2_CR1_GetODR(uint16_t f);
static uint8_t LIS2_CR6_GetFS(uint8_t s);

/*
 * PRIVATE VARIABLES
 */

static struct {
	bool int_set;
	bool one_shot;
	uint8_t scale_g;
} gLis2;

/*
 * PUBLIC FUNCTIONS
 */

bool LIS2_Init(const LIS2_Config_t * cfg)
{
#ifdef LIS2_SPI
	GPIO_EnableOutput(LIS2_CS_GPIO, LIS2_CS_PIN, GPIO_PIN_SET);
#endif
	gLis2.int_set = false;
	GPIO_EnableInput(LIS2_INT_GPIO, LIS2_INT_PIN, GPIO_Pull_Up);
	GPIO_OnChange(LIS2_INT_GPIO, LIS2_INT_PIN, GPIO_IT_Falling, LIS2_INT_IRQHandler);

	bool success = LIS2_ReadReg(REG_WHOAMI) == WHOAMI_VALUE;
	if (success)
	{
		LIS2_WriteReg(REG_CTRL2, CR2_SOFT_RST);
		CORE_Delay(5);


		uint8_t cr1 = 0;
		uint8_t cr2 = 0;
		uint8_t cr3 = 0;
		uint8_t int1 = 0;
		uint8_t int2 = 0;
		uint8_t cr6 = 0;
		uint8_t cr7 = 0;

		// We are in LP mode if the user has selected 12B.
		bool low_power = false;
		switch (cfg->resolution)
		{
		case LIS2_Res_14B:
			cr1 |= CR1_LP_MODE4; // Ignore the other modes for now.
			break;
		case LIS2_Res_12B:
			cr1 |= CR1_LP_MODE1;
			low_power = true;
			break;
		}

		// Triggered mode. Ie, the device has no need to be running continuously.
		gLis2.one_shot = cfg->int_src == LIS2_IntSrc_None;
		gLis2.scale_g = cfg->scale_g;

		// select the mode bit
		cr1 |= gLis2.one_shot  ? CR1_MODE_SINGLE : (low_power ? CR1_MODE_LP : CR1_MODE_HP);
		cr1 |= LIS2_CR1_GetODR(cfg->frequency);


#ifdef LIS2_SPI
		cr2 = CR2_BDU | CR2_IF_ADDR_INCR | CR2_I2C_DISABLE;
#else //LIS2_I2C
		cr2 = CR2_BDU | CR2_IF_ADDR_INCR;
#endif
		// Leave interrupt as momentary, active low.
		cr3 = CR3_TRIG_MODE | CR3_INT_OD | CR3_INT_POL;


		// CR2 enables address incrementing - so we need to write this first.
		LIS2_WriteReg(REG_CTRL2, cr2);

		// I'm just going to ignore CR6_LOW_NOISE
		if (cfg->int_src == LIS2_IntSrc_DataReady)
		{
			int1 = INT1_CFG_DRDY;

			// Low pass with cutoff at sample rate / 2
			cr6 |= CR6_FLTR_2;
		}
		else if (cfg->int_src == LIS2_IntSrc_Shock)
		{
			// This seems to be the only reasonable thing for a shock event.
			int1 = INT1_CFG_TAP;

			// High pass with cutoff at sample rate / 20
			cr6 |= CR6_FLTR_HP | CR6_FLTR_4;

			uint16_t threshold = ((uint32_t)cfg->threshold * TAP_THS_MASK) / (cfg->scale_g * 1000);
			threshold &= TAP_THS_MASK;

			uint8_t tap_x = threshold;
			uint8_t tap_y = threshold;
			uint8_t tap_z = threshold | TAP_THS_Z_XYZ_EN;

			// We are selecting the maximum on-time, and no quiet-time.
			uint8_t tap_dur = PLACE_BITS(TAP_DUR_SHOCK_MAX, TAP_DUR_SHOCK_POS, TAP_DUR_SHOCK_MAX);

			uint8_t tap[] = {
					tap_x,
					tap_y,
					tap_z,
					tap_dur,
			};

			LIS2_WriteRegs(REG_TAP_THS_X, tap, sizeof(tap));
		}

		cr6 |= LIS2_CR6_GetFS(cfg->scale_g);
		cr7 = CR7_DRDY_PULSED | CR7_INT_ENABLE;


		// Now write all the sequential regs in a single transaction
		uint8_t cr[] = { cr1, cr2, cr3, int1, int2, cr6 };
		LIS2_WriteRegs(REG_CTRL1, cr, sizeof(cr));

		// CR7 is not sequential
		LIS2_WriteReg(REG_CTRL7, cr7);
	}

	return success;
}

void LIS2_Deinit(void)
{
	GPIO_Deinit(LIS2_INT_GPIO, LIS2_INT_PIN);
	GPIO_OnChange(LIS2_INT_GPIO, LIS2_INT_PIN, GPIO_IT_Falling, LIS2_INT_IRQHandler);
	LIS2_WriteReg(REG_CTRL2, CR2_SOFT_RST);
}

bool LIS2_IsIntSet(void)
{
	return gLis2.int_set; // || !GPIO_Read(LIS2_INT_GPIO, LIS2_INT_PIN);
}

void LIS2_Read(LIS2_Accel_t * acc)
{
	gLis2.int_set = false;

	uint8_t data[6];
	LIS2_ReadRegs(REG_OUT_X_L, data, sizeof(data));

	int32_t full_scale = gLis2.scale_g * 1000;

	acc->x = LIS2_ADC_TO_MG(data[0], data[1], full_scale);
	acc->y = LIS2_ADC_TO_MG(data[2], data[3], full_scale);
	acc->z = LIS2_ADC_TO_MG(data[4], data[5], full_scale);
}

/*
 * PRIVATE FUNCTIONS
 */

static uint8_t LIS2_CR1_GetODR(uint16_t f)
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

static uint8_t LIS2_CR6_GetFS(uint8_t s)
{
	if 			(s < 4) 	{ return CR6_FS_2G;  }
	else if 	(s < 8) 	{ return CR6_FS_4G;  }
	else if 	(s < 16) 	{ return CR6_FS_8G;  }
	else 					{ return CR6_FS_16G; }
}

static inline uint8_t LIS2_ReadReg(uint8_t reg)
{
	uint8_t v;
	LIS2_ReadRegs(reg, &v, 1);
	return v;
}

static inline void LIS2_WriteReg(uint8_t reg, uint8_t data)
{
	LIS2_WriteRegs(reg, &data, 1);
}

#ifdef LIS2_SPI
static void LIS2_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count)
{
	uint8_t header = reg | ADDR_WRITE | ADDR_BURST;
	LIS2_Select();
	SPI_Write(LIS2_SPI, &header, 1);
	SPI_Write(LIS2_SPI, data, count);
	LIS2_Deselect();
}

static void LIS2_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count)
{
	uint8_t header = reg | ADDR_READ | ADDR_BURST;
	LIS2_Select();
	SPI_Write(LIS2_SPI, &header, 1);
	SPI_Read(LIS2_SPI, data, count);
	LIS2_Deselect();
}

static inline void LIS2_Select(void)
{
	GPIO_Reset(LIS2_CS_GPIO, LIS2_CS_PIN);
}

static inline void LIS2_Deselect(void)
{
	GPIO_Set(LIS2_CS_GPIO, LIS2_CS_PIN);
}
#else // LIS2_I2C

static void LIS2_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count)
{
	// Ignore the error

	uint8_t tx[count + 1];
	tx[0] = reg | ADDR_BURST;
	memcpy(tx+1, data, count);
	I2C_Write(LIS2_I2C, LIS2_ADDR, tx, count+1);
}

static void LIS2_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count)
{
	uint8_t tx = reg | ADDR_BURST;
	if (!I2C_Transfer(LIS2_I2C, LIS2_ADDR, &tx, 1, data, count))
	{
		// If the I2C transfer failed - then zero everything out to at least make behavior well defined.
		bzero(data, count);
	}
}

#endif

/*
 * INTERRUPT ROUTINES
 */

static void LIS2_INT_IRQHandler(void)
{
	gLis2.int_set = true;
}
