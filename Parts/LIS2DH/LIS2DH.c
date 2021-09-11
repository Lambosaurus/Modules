#include "LIS2DH.h"
#include "GPIO.h"
#include "SPI.h"

/*
 * PRIVATE DEFINITIONS
 */	

#define ADDR_WRITE		0x00
#define ADDR_READ		0x80
#define ADDR_BURST		0x40

#define REG_STAT_AUX	0x07
#define REG_OUT_TEMP_L	0x0C
#define REG_OUT_TEMP_H	0x0D
#define REG_INT_CNTR	0x0E
#define REG_WHOAMI		0x0F
#define REG_TEMP_CFG	0x1F
#define REG_CTRL1		0x20
#define REG_CTRL2		0x21
#define REG_CTRL3		0x22
#define REG_CTRL4		0x23
#define REG_CTRL5		0x24
#define REG_CTRL6		0x25
#define REG_REFERENCE	0x26
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
#define REG_INT1_THS	0x32
#define REG_INT1_DUR	0x33
#define REG_INT2_CFG	0x34
#define REG_INT2_SRC	0x35
#define REG_INT2_THS	0x36
#define REG_INT2_DUR	0x37
#define REG_CLK_CFG		0x38
#define REG_CLK_SRC		0x39
#define REG_CLK_THS		0x3A
#define REG_TIM_LIMIT	0x3B
#define REG_TIM_LATENCY	0x3C
#define REG_TIM_WINDOW	0x3D
#define REG_ACT_THS		0x3E
#define REG_ACT_DUR		0x3F

#define WHOAMI_VALUE	0x33

#define TEMP_CFG_EN		0xC0

#define CR1_X_EN		0x01
#define CR1_Y_EN		0x02
#define CR1_Z_EN		0x04
#define CR1_XYZ_EN		(CR1_X_EN | CR1_Y_EN | CR1_Z_EN)
#define CR1_LP_EN		0x08

#define CR1_ODR_POWERDOWN	0x00
#define CR1_ODR_1HZ			0x10
#define CR1_ODR_10HZ		0x20
#define CR1_ODR_25HZ		0x30
#define CR1_ODR_50HZ		0x40
#define CR1_ODR_100HZ		0x50
#define CR1_ODR_200HZ		0x60
#define CR1_ODR_400HZ		0x70
#define CR1_LPODR_1620HZ	0x80
#define CR1_LPODR_5376HZ	0x90
#define CR1_ODR_1334HZ		0x90

#define CR2_HPM_NORM_RST	0x00
#define CR2_HPM_REFERENCE	0x40
#define CR2_HPM_NORM		0x80
#define CR2_HPM_AUTO_RST	0xC0

#define CR2_HP_DATA			0x08
#define CR2_HP_CLICK		0x04
#define CR2_HP_INT2			0x02
#define CR2_HP_INT1			0x01
#define CR2_HPCF_ODR_50     0x00      // F cut = ODR Freq / 50
#define CR2_HPCF_ODR_100    0x10
#define CR2_HPCF_ODR_9      0x20
#define CR2_HPCF_ODR_400    0x30

#define CR3_I1_CLICK		0x80
#define CR3_I1_AOI1			0x40
#define CR3_I1_AOI2			0x20
#define CR3_I1_DRDY1		0x10
#define CR3_I1_DRDY2		0x08
#define CR3_I1_WTM			0x04
#define CR3_I1_OVERRUN		0x02

#define CR4_BDU_EN			0x80
#define CR4_HR_EN			0x08
#define CR4_FS_2G			0x00
#define CR4_FS_4G			0x10
#define CR4_FS_8G			0x20
#define CR4_FS_16G			0x30

#define CR5_BOOT			0x80
#define CR5_FIFO_EN			0x40
#define CR5_I1_LIR			0x08
#define CR5_I1_D4D			0x04
#define CR5_I2_LIR			0x02
#define CR5_I2_D4D			0x01

#define CR6_I2_CLICK		0x80
#define CR6_I1_INT1			0x40
#define CR6_I2_INT2			0x20
#define CR6_I2_BOOT			0x10
#define CR6_I2_ACT			0x08
#define CR6_L_ACTIVE		0x02

#define INT_CFG_OR			0x00
#define INT_CFG_AND			0x80
#define INT_CFG_6D_MOV		0x40
#define INT_CFG_6D_POS		(INT_CFG_6D_MOV | INT_CFG_AND)
#define INT_CFG_Z_H			0x20
#define INT_CFG_Z_L			0x10
#define INT_CFG_Y_H			0x08
#define INT_CFG_Y_L			0x04
#define INT_CFG_X_H			0x02
#define INT_CFG_X_L			0x01
#define INT_CFG_XYZ_H		(INT_CFG_Z_H | INT_CFG_Y_H | INT_CFG_X_H)

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static inline void LIS2_Select(void);
static inline void LIS2_Deselect(void);

static void LIS2_WriteRegs(uint8_t reg, const uint8_t * data, uint8_t count);
static void LIS2_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count);
static inline uint8_t LIS2_ReadReg(uint8_t reg);
static inline void LIS2_WriteReg(uint8_t reg, uint8_t data);

static void LIS2_INT_IRQHandler(void);

static uint8_t LIS2_CR1_GetODR(uint16_t f, bool lp);
static uint8_t LIS2_CR4_GetFS(uint8_t scale_g);

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

bool LIS2_Init(const LIS2_Config_t * cfg)
{
	GPIO_EnableOutput(LIS2_CS_GPIO, LIS2_CS_PIN, GPIO_PIN_SET);
	gCfg.int_set = false;
	GPIO_EnableInput(LIS2_INT_GPIO, LIS2_INT_PIN, GPIO_Pull_None);
	GPIO_OnChange(LIS2_INT_GPIO, LIS2_INT_PIN, GPIO_IT_Falling, LIS2_INT_IRQHandler);

	bool success = LIS2_ReadReg(REG_WHOAMI) == WHOAMI_VALUE;
	if (success)
	{
		uint8_t ctrl[6] = {0};
		uint8_t i1cfg = 0;

		bool low_power = false;
		switch (cfg->resolution)
		{
		case LIS2_Res_8B:
			low_power = true;
			ctrl[0] |= CR1_LP_EN;
			break;
		case LIS2_Res_10B:
			break;
		case LIS2_Res_12B:
			ctrl[3] |= CR4_HR_EN;
			break;
		}

		int16_t threshold = cfg->threshold;
		gCfg.scale_g = cfg->scale_g;
		switch (gCfg.scale_g)
		{
		default:
			// Make sure a scale is set.
			gCfg.scale_g = 2;
			// fallthrough
		case 2:
			threshold /= 16;
			gCfg.data_scale = 1;
			break;
		case 4:
			threshold /= 32;
			gCfg.data_scale = 2;
			break;
		case 8:
			threshold /= 62;
			gCfg.data_scale = 4;
			break;
		case 16:
			threshold /= 186;
			gCfg.data_scale = 12;
			break;
		}

		switch (cfg->int_src & ~LIS2_IntSrc_XYZ)
		{
		case LIS2_IntSrc_None:
			break;
		case LIS2_IntSrc_DataReady:
			ctrl[2] |= CR3_I1_DRDY1;
			//ctrl[3] |= CR4_BDU_EN;
			break;
		case LIS2_IntSrc_Shock:
			ctrl[1] |= CR2_HP_INT1 | CR2_HPM_NORM | CR2_HPCF_ODR_50; // | CR2_HPCF_ODR_400;
			ctrl[2] |= CR3_I1_AOI1;
			i1cfg |= INT_CFG_OR;
			if (cfg->int_src & LIS2_IntSrc_X) { i1cfg |= INT_CFG_X_H; }
			if (cfg->int_src & LIS2_IntSrc_Y) { i1cfg |= INT_CFG_Y_H; }
			if (cfg->int_src & LIS2_IntSrc_Z) { i1cfg |= INT_CFG_Z_H; }
			break;
		}

		ctrl[0] |= CR1_XYZ_EN | LIS2_CR1_GetODR(cfg->frequency, low_power);
		ctrl[3] |= LIS2_CR4_GetFS(gCfg.scale_g); //| CR4_BDU_EN;
		ctrl[5] |= CR6_L_ACTIVE;

		LIS2_WriteRegs(REG_CTRL1, ctrl, sizeof(ctrl));
		LIS2_WriteReg(REG_INT1_THS, threshold );
		LIS2_WriteReg(REG_INT1_CFG, i1cfg );
		LIS2_WriteReg(REG_INT1_DUR, 0 );
	}

	return success;
}

void LIS2_Deinit(void)
{
	GPIO_Deinit(LIS2_INT_GPIO, LIS2_INT_PIN);

	uint8_t ctrl[6] = {0};
	LIS2_WriteRegs(REG_CTRL1, ctrl, sizeof(ctrl));
}

bool LIS2_IsIntSet(void)
{
	return gCfg.int_set || !GPIO_Read(LIS2_INT_GPIO, LIS2_INT_PIN);
}

void LIS2_Read(LIS2_Accel_t * acc)
{
	gCfg.int_set = false;

	uint8_t data[6];
	LIS2_ReadRegs(REG_OUT_X_L, data, sizeof(data));

	uint16_t scale = gCfg.data_scale;

	acc->x = ((int16_t)(data[0] | (data[1] << 8)) >> 4) * scale;
	acc->y = ((int16_t)(data[2] | (data[3] << 8)) >> 4) * scale;
	acc->z = ((int16_t)(data[4] | (data[5] << 8)) >> 4) * scale;
}

/*
 * PRIVATE FUNCTIONS
 */

static uint8_t LIS2_CR1_GetODR(uint16_t f, bool lp)
{
	if 			(f < 10) 	{ return CR1_ODR_1HZ;   }
	else if 	(f < 25) 	{ return CR1_ODR_10HZ;  }
	else if 	(f < 50) 	{ return CR1_ODR_25HZ;  }
	else if 	(f < 100)	{ return CR1_ODR_50HZ;  }
	else if 	(f < 200) 	{ return CR1_ODR_100HZ; }
	else if 	(f < 400) 	{ return CR1_ODR_200HZ; }
	else if (lp)
	{
		if 		(f < 1620) 	{ return CR1_ODR_400HZ;    }
		else if (f < 5376) 	{ return CR1_LPODR_1620HZ; }
		else 				{ return CR1_LPODR_5376HZ; }
	}
	else
	{
		if 		(f < 1334)	{ return CR1_ODR_400HZ;  }
		else 				{ return CR1_ODR_1334HZ; }
	}
}

static uint8_t LIS2_CR4_GetFS(uint8_t s)
{
	if 			(s < 4) 	{ return CR4_FS_2G;  }
	else if 	(s < 8) 	{ return CR4_FS_4G;  }
	else if 	(s < 16) 	{ return CR4_FS_8G;  }
	else 					{ return CR4_FS_16G; }
}

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

static inline void LIS2_Select(void)
{
	GPIO_Reset(LIS2_CS_GPIO, LIS2_CS_PIN);
}

static inline void LIS2_Deselect(void)
{
	GPIO_Set(LIS2_CS_GPIO, LIS2_CS_PIN);
}

/*
 * INTERRUPT ROUTINES
 */

static void LIS2_INT_IRQHandler(void)
{
	gCfg.int_set = true;
}
