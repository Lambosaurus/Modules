#include "ADS114S0x.h"

#include "Core.h"
#include "GPIO.h"
#include "SPI.h"

/*
 * PRIVATE DEFINITIONS
 */

#define ADS114S_CMD_NOP			0x00
#define ADS114S_CMD_WAKE		0x02
#define ADS114S_CMD_PDOWN		0x04
#define ADS114S_CMD_RESET		0x06
#define ADS114S_CMD_START		0x08
#define ADS114S_CMD_STOP		0x0A
#define ADS114S_CMD_SYOCAL		0x16
#define ADS114S_CMD_SYGCAL		0x17
#define ADS114S_CMD_SFOCAL		0x19
#define ADS114S_CMD_RDATA		0x12
#define ADS114S_CMD_RREG		0x20
#define ADS114S_CMD_WREG		0x40

#define ADS114S_REG_ID			0x00
#define ADS114S_REG_STATUS		0x01
#define ADS114S_REG_INPMUX		0x02
#define ADS114S_REG_PGA			0x03
#define ADS114S_REG_DATARATE	0x04
#define ADS114S_REG_REF			0x05
#define ADS114S_REG_IDACMAG		0x06
#define ADS114S_REG_IDACMUX		0x07
#define ADS114S_REG_VBIAS		0x08
#define ADS114S_REG_SYS			0x09
#define ADS114S_REG_OFCAL0		0x0B
#define ADS114S_REG_OFCAL1		0x0C
#define ADS114S_REG_FSCAL0		0x0E
#define ADS114S_REG_FSCAL1		0x0F
#define ADS114S_REG_GPIODAT		0x10
#define ADS114S_REG_GPIOCON		0x11

#define ADS114S_ID_MASK			0x07
#define ADS114S_ID_S06			0x05
#define ADS114S_ID_S08			0x04

#define ADS114S_DATARATE_EXTCLK		0x40
#define ADS114S_DATARATE_SINGLESHOT	0x20
#define ADS114S_DATARATE_RESERVED 	0x10
#define ADS114S_DATARATE_DR			0x0F
#define ADS114S_DATARATE_2p5		0x00
#define ADS114S_DATARATE_5			0x01
#define ADS114S_DATARATE_10			0x02
#define ADS114S_DATARATE_16p6		0x03
#define ADS114S_DATARATE_20			0x04
#define ADS114S_DATARATE_50			0x05
#define ADS114S_DATARATE_60			0x06
#define ADS114S_DATARATE_100		0x07
#define ADS114S_DATARATE_200		0x08
#define ADS114S_DATARATE_400		0x09
#define ADS114S_DATARATE_800		0x0A
#define ADS114S_DATARATE_1000		0x0B
#define ADS114S_DATARATE_2000		0x0C
#define ADS114S_DATARATE_4000		0x0D

#define ADS114S_REF_FL_REF_EN		0x80
#define ADS114S_REF_REFP_NBUFF		0x20
#define ADS114S_REF_REFN_NBUFF		0x10
#define ADS114S_REF_SEL_REFP0		0x00
#define ADS114S_REF_SEL_REFP1		0x04
#define ADS114S_REF_SEL_INT			0x08
#define ADS114S_REF_INT_OFF			0x00
#define ADS114S_REF_INT_STBY		0x01 // Intref is enabled when not in low power mode.
#define ADS114S_REF_INT_ON			0x02

#define ADS114S_GAIN_PGA_EN			0x08
#define ADS114S_GAIN_1				0x00
#define ADS114S_GAIN_2				0x01
#define ADS114S_GAIN_4				0x02
#define ADS114S_GAIN_8				0x03
#define ADS114S_GAIN_16				0x04
#define ADS114S_GAIN_32				0x05
#define ADS114S_GAIN_64				0x06
#define ADS114S_GAIN_128			0x07

#define ADS114S_AIN_COM				0x0C

#define ADS114S_DRDY_TIMEOUT		100 // Will need to be higher depending on datarate

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void ADS114S_Command(uint8_t cmd);
static int16_t ADS114S_ReadData(void);
static void ADS114S_WriteRegisters(uint8_t reg, const uint8_t * values, uint8_t count);
static void ADS114S_ReadRegisters(uint8_t reg, uint8_t * values, uint8_t count);
static void ADS114S_WriteRegister(uint8_t reg, uint8_t value);
static uint8_t ADS114S_ReadRegister(uint8_t reg);

static void ADS114S_SelectChannel(uint8_t pos_ch, uint8_t neg_ch);
static void ADS114S_WaitForDRDY(void);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool ADS114S_Init(void)
{
	GPIO_EnableOutput(ADS114S_CS_PIN, GPIO_PIN_SET);
	CORE_Delay(5);
	ADS114S_Command(ADS114S_CMD_WAKE);

	uint8_t id = ADS114S_ReadRegister(ADS114S_REG_ID) & ADS114S_ID_MASK;
	if (id == ADS114S_ID_S06 || id == ADS114S_ID_S08)
	{
		uint8_t regs[] = {
			// PGA Disabled
			ADS114S_GAIN_1,
			// Singleshot, 20Hz
			ADS114S_DATARATE_SINGLESHOT | ADS114S_DATARATE_RESERVED | ADS114S_DATARATE_20,
			// Internal 2.5V reference, buffering disabled
			ADS114S_REF_SEL_INT | ADS114S_REF_INT_STBY | ADS114S_REF_REFP_NBUFF | ADS114S_REF_REFN_NBUFF,
		};
		ADS114S_WriteRegisters(ADS114S_REG_PGA, regs, sizeof(regs));
		return true;
	}
	return false;
}

void ADS114S_Deinit(void)
{
	ADS114S_Command(ADS114S_CMD_PDOWN);
	GPIO_Deinit(ADS114S_CS_PIN);
}

int16_t ADS114S_Read(uint8_t channel)
{
	// Single ended measurement.
	ADS114S_SelectChannel(channel, ADS114S_AIN_COM);
	ADS114S_Command(ADS114S_CMD_START);
	ADS114S_WaitForDRDY();
	return ADS114S_ReadData();
}

/*
 * PRIVATE FUNCTIONS
 */

static void ADS114S_SelectChannel(uint8_t pos_ch, uint8_t neg_ch)
{
	ADS114S_WriteRegister(ADS114S_REG_INPMUX, (pos_ch << 4) | neg_ch);
}

static int16_t ADS114S_ReadData(void)
{
	GPIO_Reset(ADS114S_CS_PIN);

	uint8_t tx = ADS114S_CMD_RDATA;
	SPI_Write(ADS114S_SPI, &tx, sizeof(tx));
	uint8_t rx[2];
	SPI_Read(ADS114S_SPI, rx, sizeof(rx));

	GPIO_Set(ADS114S_CS_PIN);

	return (rx[0] << 8) | rx[1];
}

static void ADS114S_Command(uint8_t cmd)
{
	GPIO_Reset(ADS114S_CS_PIN);
	SPI_Write(ADS114S_SPI, &cmd, sizeof(cmd));
	GPIO_Set(ADS114S_CS_PIN);
}

static void ADS114S_WriteRegisters(uint8_t reg, const uint8_t * values, uint8_t count)
{
	GPIO_Reset(ADS114S_CS_PIN);

	uint8_t tx[] = {
		ADS114S_CMD_WREG | reg,
		count - 1,
	};

	SPI_Write(ADS114S_SPI, tx, sizeof(tx));
	SPI_Write(ADS114S_SPI, values, count);

	GPIO_Set(ADS114S_CS_PIN);
}

static void ADS114S_ReadRegisters(uint8_t reg, uint8_t * values, uint8_t count)
{
	GPIO_Reset(ADS114S_CS_PIN);

	uint8_t tx[] = {
		ADS114S_CMD_RREG | reg,
		count - 1,
	};

	SPI_Write(ADS114S_SPI, tx, sizeof(tx));
	SPI_Read(ADS114S_SPI, values, count);

	GPIO_Set(ADS114S_CS_PIN);
}

static void ADS114S_WriteRegister(uint8_t reg, uint8_t value)
{
	ADS114S_WriteRegisters(reg, &value, sizeof(value));
}

static uint8_t ADS114S_ReadRegister(uint8_t reg)
{
	uint8_t rx;
	ADS114S_ReadRegisters(reg, &rx, sizeof(rx));
	return rx;
}

static void ADS114S_WaitForDRDY(void)
{
	// Reading a register with a result ending in 1.
	// See 9.5.4 in the datasheet.
	ADS114S_ReadRegister(ADS114S_REG_IDACMUX);

	GPIO_Reset(ADS114S_CS_PIN);
	uint32_t start = CORE_GetTick();
	while (CORE_GetTick() - start < ADS114S_DRDY_TIMEOUT)
	{
		if (GPIO_Read(ADS114S_MISO_PIN) == 0)
		{
			break;
		}
		CORE_Idle();
	}
	GPIO_Set(ADS114S_CS_PIN);
}

/*
 * INTERRUPT ROUTINES
 */

