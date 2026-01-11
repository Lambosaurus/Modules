#include "BME280.h"

#include "I2C.h"
#include "Core.h"

/*
 * PRIVATE DEFINITIONS
 */

#ifndef BME280_ADDR
#define BME280_ADDR		0x76
#endif


#define BME280_REG_CALIB_00		0x88
#define BME280_REG_ID			0xD0
#define BME280_REG_RESET		0xE0
#define BME280_REG_CALIB_26		0xE1
#define BME280_REG_CTRLH		0xF2
#define BME280_REG_STATUS		0xF3
#define BME280_REG_CTRLM		0xF4
#define BME280_REG_CONFIG		0xF5
#define BME280_REG_PRES_MSB		0xF7
#define BME280_REG_PRES_LSB		0xF8
#define BME280_REG_PRES_XLSB	0xF9
#define BME280_REG_TEMP_MSB		0xFA
#define BME280_REG_TEMP_LSB		0xFB
#define BME280_REG_TEMP_XLSB	0xFC
#define BME280_REG_HUM_MSB		0xFD
#define BME280_REG_HUM_LSB		0xFE

// BME280_REG_ID
#define BME280_ID_ID				0x60

// BME280_REG_RESET
#define BME280_RESET_RESET			0xB6

// BME280_REG_CTRLH
#define BME280_CTRLH_HUM_X0			0x00
#define BME280_CTRLH_HUM_X1			0x01
#define BME280_CTRLH_HUM_X2			0x02
#define BME280_CTRLH_HUM_X4			0x03
#define BME280_CTRLH_HUM_X8			0x04
#define BME280_CTRLH_HUM_X16		0x05

// BME280_REG_STATUS
#define BME280_STATUS_MEASURING		0x08
#define BME280_STATUS_UPDATING		0x01

// BME280_REG_CTRLM
#define BME280_CTRLM_MODE_SLEEP		0x00
#define BME280_CTRLM_MODE_FORCED	0x01
#define BME280_CTRLM_MODE_NORMAL	0x03
#define BME280_CTRLM_MODE_MASK		0x03

#define BME280_CTRLM_PRES_X0		0x00
#define BME280_CTRLM_PRES_X1		0x04
#define BME280_CTRLM_PRES_X2		0x08
#define BME280_CTRLM_PRES_X4		0x0C
#define BME280_CTRLM_PRES_X8		0x10
#define BME280_CTRLM_PRES_X16		0x14

#define BME280_CTRLM_TEMP_X0		0x00
#define BME280_CTRLM_TEMP_X1		0x20
#define BME280_CTRLM_TEMP_X2		0x40
#define BME280_CTRLM_TEMP_X4		0x60
#define BME280_CTRLM_TEMP_X8		0x80
#define BME280_CTRLM_TEMP_X16		0xA0

// BME280_REG_CONFIG
#define BME280_CONFIG_SPI3W			0x01

#define BME280_CONFIG_FILTER_X1		0x00
#define BME280_CONFIG_FILTER_X2		0x04
#define BME280_CONFIG_FILTER_X4		0x08
#define BME280_CONFIG_FILTER_X8		0x0C
#define BME280_CONFIG_FILTER_X16	0x10

#define BME280_CONFIG_STANDBY_500US		0x00
#define BME280_CONFIG_STANDBY_62MS		0x20
#define BME280_CONFIG_STANDBY_125MS		0x40
#define BME280_CONFIG_STANDBY_250MS		0x60
#define BME280_CONFIG_STANDBY_500MS		0x80
#define BME280_CONFIG_STANDBY_1S		0xA0
#define BME280_CONFIG_STANDBY_10MS		0xC0
#define BME280_CONFIG_STANDBY_20MS		0xE0


#define BME280_MEAS_TIMEOUT				100

#define CLAMP(_value, _low, _high)			((_value) < (_low) ? (_low) : (_value) > (_high) ? (_high) : (_value))

/*
 * PRIVATE TYPES
 */

typedef struct {
	uint16_t T1;
	int16_t T2;
	int16_t T3;
	uint16_t P1;
	int16_t P2;
	int16_t P3;
	int16_t P4;
	int16_t P5;
	int16_t P6;
	int16_t P7;
	int16_t P8;
	int16_t P9;
	uint8_t H1;
	int16_t H2;
	uint8_t H3;
	int16_t H4;
	int16_t H5;
	int8_t H6;
} BME280_Cal_t;

/*
 * PRIVATE PROTOTYPES
 */

static uint8_t BME280_ReadReg(uint8_t reg);
static void BME280_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count);
static void BME280_WriteReg(uint8_t reg, uint8_t value);

static void BME280_SetMode(uint8_t mode);

static uint8_t BME280_GetMode(void);
static void BME280_Reset(void);
static void BME280_ReadCalibration(BME280_Cal_t * cal);

static uint32_t BME280_CompensatePres(const BME280_Cal_t * cal, uint32_t adc_p);
static int32_t BME280_CompensateTemp(const BME280_Cal_t * cal, uint32_t adc_t);
static uint32_t BME280_CompensateHum(const BME280_Cal_t * cal, uint32_t adc_h);

/*
 * PRIVATE VARIABLES
 */

static struct {
	uint8_t ctrlm;
	int32_t t_fine;
	BME280_Cal_t cal;
} gBME280;

/*
 * PUBLIC FUNCTIONS
 */

bool BME280_Init(void)
{
	if (!(BME280_ReadReg(BME280_REG_ID) == BME280_ID_ID))
	{
		return false;
	}

	BME280_Reset();
	CORE_Delay(2);

	gBME280.ctrlm = BME280_CTRLM_PRES_X1 | BME280_CTRLM_TEMP_X1;
	BME280_WriteReg(BME280_REG_CTRLH, BME280_CTRLH_HUM_X1);
	BME280_WriteReg(BME280_REG_CONFIG, BME280_CONFIG_FILTER_X1 | BME280_CONFIG_STANDBY_500US);
	BME280_SetMode(BME280_CTRLM_MODE_SLEEP); // Writes the CTRLM register.

	BME280_ReadCalibration(&gBME280.cal);

	return true;
}

void BME280_Deinit(void)
{
	BME280_Reset();
}

bool BME280_Read(uint32_t * pressure, int16_t * temperature, uint8_t * humidity)
{
	// Do a one-shot read
	BME280_SetMode(BME280_CTRLM_MODE_FORCED);

	uint32_t start = CORE_GetTick();
	while (BME280_GetMode() != BME280_CTRLM_MODE_SLEEP)
	{
		if (CORE_GetTick() - start >= BME280_MEAS_TIMEOUT)
		{
			return false;
		}
	}

	uint8_t values[8];
	BME280_ReadRegs(BME280_REG_PRES_MSB, values, sizeof(values));

	int32_t reg_p = (values[0] << 12) | (values[1] << 4) | (values[2] >> 4);
	int32_t reg_t = (values[3] << 12) | (values[4] << 4) | (values[5] >> 4);
	int32_t reg_h = (values[6] <<  8) | values[7];

	// Temperature must be done first.
	*temperature = BME280_CompensateTemp(&gBME280.cal, reg_t) / 10; // from 0.01 deg to 0.1 deg
	*pressure = BME280_CompensatePres(&gBME280.cal, reg_p);         // in pA
	*humidity = BME280_CompensateHum(&gBME280.cal, reg_h) / 1024;   // from 10 bit fractional RH to 1% RH

	return true;
}

/*
 * PRIVATE FUNCTIONS
 */

static uint32_t BME280_CompensatePres(const BME280_Cal_t * cal, uint32_t adc_p)
{
    int32_t var1, var2, var3, var4;
	uint32_t var5;
    var1 = (((int32_t)gBME280.t_fine) / 2) - (int32_t)64000;
    var2 = (((var1 / 4) * (var1 / 4)) / 2048) * ((int32_t)cal->P6);
    var2 = var2 + ((var1 * ((int32_t)cal->P5)) * 2);
    var2 = (var2 / 4) + (((int32_t)cal->P4) * 65536);
    var3 = (cal->P3 * (((var1 / 4) * (var1 / 4)) / 8192)) / 8;
    var4 = (((int32_t)cal->P2) * var1) / 2;
    var1 = (var3 + var4) / 262144;
    var1 = (((32768 + var1)) * ((int32_t)cal->P1)) / 32768;

    // avoid zero div
    if (var1 == 0)
    {
    	return 30000;
    }

	var5 = (uint32_t)((uint32_t)1048576) - adc_p;
	uint32_t pressure = ((uint32_t)(var5 - (uint32_t)(var2 / 4096))) * 3125;

	if (pressure < 0x80000000)
		pressure = (pressure << 1) / ((uint32_t)var1);
	else
		pressure = (pressure / (uint32_t)var1) * 2;

	var1 = (((int32_t)cal->P9) * ((int32_t)(((pressure / 8) * (pressure / 8)) / 8192))) / 4096;
	var2 = (((int32_t)(pressure / 4)) * ((int32_t)cal->P8)) / 8192;
	pressure = (uint32_t)((int32_t)pressure + ((var1 + var2 + cal->P7) / 16));

	return CLAMP(pressure, 30000, 110000);
}

static int32_t BME280_CompensateTemp(const BME280_Cal_t * cal, uint32_t adc_t)
{
    int32_t var1, var2;
    var1 = (int32_t)((adc_t/ 8) - ((int32_t)cal->T1 * 2));
    var1 = (var1 * ((int32_t)cal->T2)) / 2048;
    var2 = (int32_t)((adc_t / 16) - ((int32_t)cal->T1));
    var2 = (((var2 * var2) / 4096) * ((int32_t)cal->T3)) / 16384;

    gBME280.t_fine = var1 + var2;
    int32_t temperature = (gBME280.t_fine * 5 + 128) / 256;

    return CLAMP(temperature, -4000, 8500);
}

static uint32_t BME280_CompensateHum(const BME280_Cal_t * cal, uint32_t adc_h)
{
    int32_t var1, var2, var3, var4, var5;

    var1 = gBME280.t_fine - ((int32_t)76800);
    var2 = (int32_t)(adc_h * 16384);
    var3 = (int32_t)(((int32_t)cal->H4) * 1048576);
    var4 = ((int32_t)cal->H5) * var1;
    var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
    var2 = (var1 * ((int32_t)cal->H6)) / 1024;
    var3 = (var1 * ((int32_t)cal->H3)) / 2048;
    var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
    var2 = ((var4 * ((int32_t)cal->H2)) + 8192) / 16384;
    var3 = var5 * var2;
    var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
    var5 = var3 - ((var4 * ((int32_t)cal->H1)) / 16);
    var5 = (var5 < 0 ? 0 : var5);
    var5 = (var5 > 419430400 ? 419430400 : var5);
    uint32_t humidity = (uint32_t)(var5 / 4096);

    return CLAMP(humidity, 0, 102400);
}

static void BME280_ReadCalibration(BME280_Cal_t * cal)
{
	uint8_t params[33];
	BME280_ReadRegs(BME280_REG_CALIB_00, params +  0, 26);
	BME280_ReadRegs(BME280_REG_CALIB_26, params + 26,  7);

	cal->T1 = params[ 0] | (params[ 1] << 8);
	cal->T2 = params[ 2] | (params[ 3] << 8);
	cal->T3 = params[ 4] | (params[ 5] << 8);

	cal->P1 = params[ 6] | (params[ 7] << 8);
	cal->P2 = params[ 8] | (params[ 9] << 8);
	cal->P3 = params[10] | (params[11] << 8);
	cal->P4 = params[12] | (params[13] << 8);
	cal->P5 = params[14] | (params[15] << 8);
	cal->P6 = params[16] | (params[17] << 8);
	cal->P7 = params[18] | (params[19] << 8);
	cal->P8 = params[20] | (params[21] << 8);
	cal->P9 = params[22] | (params[23] << 8);

	cal->H1 = params[25];
	cal->H2 = params[26] | (params[27] << 8);
	cal->H3 = params[28];

	cal->H4 = (params[29] << 4) | (params[30] & 0xF);
	cal->H5 = (params[31] << 4) | (params[30 >> 4]);
	cal->H6 = params[32];
}

static void BME280_SetMode(uint8_t mode)
{
	BME280_WriteReg(BME280_REG_CTRLM, gBME280.ctrlm | mode);
}

static uint8_t BME280_GetMode(void)
{
	return BME280_ReadReg(BME280_REG_CTRLM) & BME280_CTRLM_MODE_MASK;
}

static void BME280_Reset(void)
{
	BME280_WriteReg(BME280_REG_RESET, BME280_RESET_RESET);
}

static void BME280_WriteReg(uint8_t reg, uint8_t value)
{
	uint8_t tx[] = { reg, value };
	I2C_Write(BME280_I2C, BME280_ADDR, tx, sizeof(tx));
}

static void BME280_ReadRegs(uint8_t reg, uint8_t * data, uint8_t count)
{
	if (!I2C_Transfer(BME280_I2C, BME280_ADDR, &reg, 1, data, count))
	{
		// If the I2C transfer failed - then zero everything out to at least make behavior well defined.
		bzero(data, count);
	}
}

static uint8_t BME280_ReadReg(uint8_t reg)
{
	uint8_t value;
	BME280_ReadRegs(reg, &value, 1);
	return value;
}

/*
 * INTERRUPT ROUTINES
 */

