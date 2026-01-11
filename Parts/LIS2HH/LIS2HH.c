#include "LIS2HH.h"
#include "GPIO.h"

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

#define CR4_BUS_CONFG	(CR4_IF_ADD_INC | CR4_I2C_DISABLE)

#else //LIS2_I2C

#define LIS2_ADDR		0x1D
#define CR4_BUS_CONFG	(CR4_IF_ADD_INC)

#endif



#define REG_OUT_TEMP_L	0x0B
#define REG_OUT_TEMP_H	0x0C
#define REG_WHOAMI		0x0F
#define REG_ACT_THS		0x1E
#define REG_ACT_DUR		0x1F
#define REG_CTRL1		0x20
#define REG_CTRL2		0x21
#define REG_CTRL3		0x22
#define REG_CTRL4		0x23
#define REG_CTRL5		0x24
#define REG_CTRL6		0x25
#define REG_CTRL7		0x26
#define REG_STATUS		0x27
#define REG_OUT_X_L		0x28
#define REG_OUT_X_H		0x29
#define REG_OUT_Y_L		0x2A
#define REG_OUT_Y_H		0x2B
#define REG_OUT_Z_L		0x2C
#define REG_OUT_Z_H		0x2D
#define REG_FIFO_CTRL	0x2E
#define REG_FIFO_SRC	0x2F
#define REG_INT1_CFG	0x30
#define REG_INT1_SRC	0x31
#define REG_INT1_THSX	0x32
#define REG_INT1_THSY	0x33
#define REG_INT1_THSZ	0x34
#define REG_INT1_DUR	0x35
#define REG_INT2_CFG	0x36
#define REG_INT2_SRC	0x37
#define REG_INT2_THS	0x38
#define REG_INT2_DUR	0x39
#define REG_REF_XL		0x3A
#define REG_REF_XH		0x3B
#define REG_REF_YL		0x3C
#define REG_REF_YH		0x3D
#define REG_REF_ZL		0x3E
#define REG_REF_ZH		0x3F


#define WHOAMI_VALUE	0x41


#define CR1_X_EN		0x01
#define CR1_Y_EN		0x02
#define CR1_Z_EN		0x04
#define CR1_XYZ_EN		(CR1_X_EN | CR1_Y_EN | CR1_Z_EN)
#define CR1_BDU_EN		0x08
#define CR1_ODR_POWERDOWN	0x00
#define CR1_ODR_10HZ		0x10
#define CR1_ODR_50HZ		0x20
#define CR1_ODR_100HZ		0x30
#define CR1_ODR_200HZ		0x40
#define CR1_ODR_400HZ		0x50
#define CR1_ODR_800HZ		0x60
#define CR1_HR_EN			0x80

#define CR2_HPIS1			0x01 // Enables high pass filter on int 1
#define CR2_HPIS2			0x02 // Enables high pass filter on int 2
#define CR2_FDS				0x04
#define CR2_HPM_NORMAL		0x00
#define CR2_HPM_REFERENCE	0x10
#define CR2_HPM_NA			0x30
#define CR2_DFC_D50			0x00 // High pass filter cutoff in ODR samples.
#define CR2_DFC_D100		0x40
#define CR2_DFC_D9			0x80
#define CR2_DFC_D400		0xC0

#define CR3_INT1_DRDY		0x01
#define CR3_INT1_FTH		0x02
#define CR3_INT1_OVR		0x04
#define CR3_INT1_IG1		0x08
#define CR3_INT1_IG2		0x10
#define CR3_INT1_INACT		0x20
#define CR3_STOP_FTH		0x40
#define CR3_FIFO_EN			0x80

#define CR4_SIM				0x01
#define CR4_I2C_DISABLE		0x02
#define CR4_IF_ADD_INC		0x04
#define CR4_BW_SCALE_ODR	0x08
#define CR4_FS_2G			0x00
#define CR4_FS_4G			0x20
#define CR4_FS_8G			0x30
#define CR4_BW_400HZ		0x00
#define CR4_BW_200HZ		0x40
#define CR4_BW_100HZ		0x80
#define CR4_BW_50HZ			0xC0

#define CR5_OPEN_DRAIN		0x01
#define CR5_ACTIVE_LOW		0x02
#define CR5_ST_OFF			0x00
#define CR5_ST_POS			0x04
#define CR5_ST_NET			0x08
#define CR5_DEC_NONE		0x00
#define CR5_DEC_2X			0x10
#define CR5_DEC_4X			0x20
#define CR5_DEC_8X			0x30
#define CR5_SRST			0x40
#define CR5_DEBUG			0x80

#define CR6_INT2_DRDY		0x01
#define CR6_INT2_FTH		0x02
#define CR6_INT2_EMPTY		0x04
#define CR6_INT2_IG1		0x08
#define CR6_INT2_IG2		0x10
#define CR6_INT2_BOOT		0x20
#define CR6_REBOOT			0x80

#define CR7_4D_IG1			0x01
#define CR7_4D_IG2			0x02
#define CR7_LIR1			0x04
#define CR7_LIR2			0x08
#define CR7_DCRM1			0x10
#define CR7_DCRM2			0x20


/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void LIS2_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count);
static void LIS2_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count);
static inline uint8_t LIS2_ReadReg(uint8_t reg);
static inline void LIS2_WriteReg(uint8_t reg, uint8_t data);

static void LIS2_INT_IRQHandler(void);

static uint8_t LIS2_CR1_GetODR(uint16_t f);
static uint8_t LIS2_CR4_GetFS(uint8_t s);

/*
 * PRIVATE VARIABLES
 */

static struct {
	uint8_t scale_g;
	uint8_t data_scale;
	bool int_set;
} gCfg;

/*
 * PUBLIC FUNCTIONS
 */

bool LIS2_Init(uint8_t scale_g, uint16_t frequency, bool high_res)
{
#ifdef LIS2_SPI
	GPIO_EnableOutput(LIS2_CS_PIN, GPIO_PIN_SET);
#endif
	gCfg.int_set = false;
	gCfg.data_scale = scale_g;

	bool success = LIS2_ReadReg(REG_WHOAMI) == WHOAMI_VALUE;
	if (success)
	{
		uint8_t ctrl[7] = {0};

		ctrl[0] = CR1_XYZ_EN | CR1_BDU_EN | LIS2_CR1_GetODR(frequency);
		if (high_res) { ctrl[0] |= CR1_HR_EN; }

		ctrl[3] = LIS2_CR4_GetFS(scale_g) | CR4_BW_SCALE_ODR | CR4_BUS_CONFG;

		ctrl[4] = CR5_DEC_NONE | CR5_ST_OFF | CR5_ACTIVE_LOW | CR5_OPEN_DRAIN;

		LIS2_WriteReg(REG_CTRL4, ctrl[3]); // Need to set the bus settings first.
		LIS2_WriteRegs(REG_CTRL1, ctrl, sizeof(ctrl));
	}

	return success;
}

void LIS2_EnableDataInt(void)
{
	uint8_t ctrl3 = CR3_INT1_DRDY;
	LIS2_WriteReg(REG_CTRL3, ctrl3);

	GPIO_EnableInput(LIS2_INT_PIN, GPIO_Pull_Up);
	GPIO_OnChange(LIS2_INT_PIN, GPIO_IT_Falling, LIS2_INT_IRQHandler);
}

void LIS2_Deinit(void)
{
	GPIO_Deinit(LIS2_INT_PIN);

	uint8_t ctrl[7] = {0};
	ctrl[3] = CR4_BUS_CONFG;
	LIS2_WriteRegs(REG_CTRL1, ctrl, sizeof(ctrl));
}

bool LIS2_IsIntSet(void)
{
	return gCfg.int_set || !GPIO_Read(LIS2_INT_PIN);
}

void LIS2_Read(LIS2_Accel_t * acc)
{
	gCfg.int_set = false;

	uint8_t data[6];
	LIS2_ReadRegs(REG_OUT_X_L, data, sizeof(data));

	int32_t scale = gCfg.data_scale * 1000;

	acc->x = (int16_t)(data[0] | (data[1] << 8)) * scale / 0x7FFF;
	acc->y = (int16_t)(data[2] | (data[3] << 8)) * scale / 0x7FFF;
	acc->z = (int16_t)(data[4] | (data[5] << 8)) * scale / 0x7FFF;
}

/*
 * PRIVATE FUNCTIONS
 */

static uint8_t LIS2_CR1_GetODR(uint16_t f)
{
	if 			(f < 50) 	{ return CR1_ODR_10HZ;   }
	else if 	(f < 100) 	{ return CR1_ODR_50HZ;   }
	else if 	(f < 200) 	{ return CR1_ODR_100HZ;  }
	else if 	(f < 400)	{ return CR1_ODR_200HZ;  }
	else if 	(f < 800) 	{ return CR1_ODR_400HZ;  }
	else                	{ return CR1_ODR_800HZ;  }
}

static uint8_t LIS2_CR4_GetFS(uint8_t s)
{
	if 			(s < 4) 	{ return CR4_FS_2G;  }
	else if 	(s < 8) 	{ return CR4_FS_4G;  }
	else 					{ return CR4_FS_8G;  }
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

static inline void LIS2_Select(void)
{
	GPIO_Reset(LIS2_CS_PIN);
}

static inline void LIS2_Deselect(void)
{
	GPIO_Set(LIS2_CS_PIN);
}

static void LIS2_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count)
{
	uint8_t header = reg | ADDR_WRITE;
	LIS2_Select();
	SPI_Write(LIS2_SPI, &header, 1);
	SPI_Write(LIS2_SPI, data, count);
	LIS2_Deselect();
}

static void LIS2_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count)
{
	uint8_t header = reg | ADDR_READ;
	LIS2_Select();
	SPI_Write(LIS2_SPI, &header, 1);
	SPI_Read(LIS2_SPI, data, count);
	LIS2_Deselect();
}

#else // LIS2_I2C

static void LIS2_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count)
{
	uint8_t tx[count + 1];
	tx[0] = reg;
	memcpy(tx+1, data, count);
	I2C_Write(LIS2_I2C, LIS2_ADDR, tx, count+1);
}

static void LIS2_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count)
{
	uint8_t tx = reg;
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
	gCfg.int_set = true;
}

