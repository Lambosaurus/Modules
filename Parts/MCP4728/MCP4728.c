#include "MCP4728.h"
#include "I2C.h"
#include "Core.h"

#ifdef MCP4728_SUPPORT_ADDRESSING
#include "US.h"
#endif

/*
 * PRIVATE DEFINITIONS
 */

#define MCP4728_ADDR_BASE					0x60
#ifdef MCP4728_SUPPORT_ADDRESSING
#define MCP4728_ADDR						(gMCP4728.addr | MCP4728_ADDR_BASE)
#else
#define MCP4728_ADDR						MCP4728_ADDR_BASE
#endif

#define MCP4728_CMD_UDAC					0x01
#define MCP4728_CMD_CH(n)					(n << 1)

#define MCP4728_CMD_WRITE_MULTI				0x40
#define MCP4728_CMD_PROG_SEQUENTIAL			0x50
#define MCP4728_CMD_PROG_SINGLE				0x58
#define MCP4728_CMD_PROG_ADDR				0x60
#define MCP4728_CMD_WRITE_VREF				0x80
#define MCP4728_CMD_WRITE_GAIN				0xC0
#define MCP4728_CMD_WRITE_PDOWN				0xA0

#define MCP4728_SR_RDY						0x80
#define MCP4728_SR_POR						0x40

#define MCP4728_CFG_PD_POS					5	 // Offset for PD settings

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void MCP4728_WaitForProg(void);
static bool MCP4728_PowerdownAll(MCP4728_PD_t pd);

/*
 * PRIVATE VARIABLES
 */

#ifdef MCP4728_SUPPORT_ADDRESSING
static struct {
	uint8_t addr; // This is just the LSB
} gMCP4728;
#endif

/*
 * PUBLIC FUNCTIONS
 */

bool MCP4728_Init(void)
{
	return I2C_Scan(MCP4728_I2C, MCP4728_ADDR) && MCP4728_PowerdownAll(MCP4728_PD_500K);
}

bool MCP4728_Deinit(void)
{
	return MCP4728_PowerdownAll(MCP4728_PD_500K);
}

bool MCP4728_Powerdown(uint8_t channel, MCP4728_PD_t pd)
{
	// Its the write command, but we just set the PD bits instead.
	return MCP4728_Write(channel, pd << MCP4728_CFG_PD_POS, 0);
}

bool MCP4728_Write(uint8_t channel, MCP4728_Ref_t ref, uint16_t value)
{
	// Enable gain gives us 1 mV resolution.
	uint8_t tx[] = {
		MCP4728_CMD_WRITE_MULTI | (channel << 1),
		ref | ((value >> 8) & 0xF),
		value,
	};
	return I2C_Write(MCP4728_I2C, MCP4728_ADDR, tx, sizeof(tx));
}

#ifdef MCP4728_SUPPORT_ADDRESSING

void MCP4728_Select(uint8_t addr)
{
	gMCP4728.addr = addr;
}

bool MCP4728_SetAddress(uint8_t new_addr, GPIO_Pin_t ldac_pin)
{
	// We need to toggle LDAC during the a specific bit of the transfer.
	// We do a software i2c transfer, taking over the I2C pins.

	GPIO_Write(MCP4728_SDA_PIN | MCP4728_SCL_PIN, GPIO_PIN_SET);
	GPIO_Init(MCP4728_SDA_PIN | MCP4728_SCL_PIN, GPIO_Mode_Output | GPIO_Speed_High | GPIO_Flag_OpenDrain);

	US_Delay(10);

	uint8_t src_addr = gMCP4728.addr;
	uint8_t tx[4] = {
		(MCP4728_ADDR << 1),
		MCP4728_CMD_PROG_ADDR | (src_addr << 2) | 0x01,
		MCP4728_CMD_PROG_ADDR | (new_addr << 2) | 0x02,
		MCP4728_CMD_PROG_ADDR | (new_addr << 2) | 0x03,
	};

	// Start condition
	GPIO_Reset(MCP4728_SDA_PIN);
	US_Delay(2);
	GPIO_Reset(MCP4728_SCL_PIN);
	US_Delay(2);

	bool ack = true;

	for (uint32_t i = 0; i < sizeof(tx) && ack; i++)
	{
		// Clock out data
		uint8_t byte = tx[i];
		for (uint32_t mask = 0x80; mask > 0x00; mask >>= 1)
		{
			GPIO_Write(MCP4728_SDA_PIN, byte & mask);
			US_Delay(1);
			GPIO_Set(MCP4728_SCL_PIN);
			US_Delay(2);
			GPIO_Reset(MCP4728_SCL_PIN);
		}

		// bit 8 to 9 transition of byte 1. Set the LDAC pin.
		if(i == 1) GPIO_Reset(ldac_pin);

		// Ready for ACK. We should read this, but do not.
		GPIO_Set(MCP4728_SDA_PIN);
		US_Delay(1);
		GPIO_Set(MCP4728_SCL_PIN);
		US_Delay(2);
		ack = !GPIO_Read(MCP4728_SDA_PIN);
		GPIO_Reset(MCP4728_SCL_PIN);
		US_Delay(2);
	}

	GPIO_Set(ldac_pin);

	// Stop condition
	GPIO_Reset(MCP4728_SDA_PIN | MCP4728_SCL_PIN);
	US_Delay(2);
	GPIO_Set(MCP4728_SCL_PIN);
	US_Delay(2);
	GPIO_Set(MCP4728_SDA_PIN);

	// Return the pins to their alternate function
	GPIO_Init(MCP4728_SDA_PIN | MCP4728_SCL_PIN, GPIO_Mode_Alternate | GPIO_Speed_High | GPIO_Flag_OpenDrain);

	if (ack)
	{
		gMCP4728.addr = new_addr;
		MCP4728_WaitForProg();
	}
	return ack;
}
#endif // MCP4728_SUPPORT_ADDRESSING

/*
 * PRIVATE FUNCTIONS
 */

static bool MCP4728_PowerdownAll(MCP4728_PD_t pd)
{
	// Just set all channels to the selected powerdown.
	uint8_t pair = (pd << 2) | pd;
	uint8_t tx[] = {
		MCP4728_CMD_WRITE_PDOWN | pair,
		pair << 4,
	};
	return I2C_Write(MCP4728_I2C, MCP4728_ADDR, tx, sizeof(tx));
}

#ifdef MCP4728_SUPPORT_ADDRESSING
static void MCP4728_WaitForProg(void)
{
	uint32_t start = CORE_GetTick();
	while (CORE_GetTick() - start < 50)
	{
		CORE_Idle();

		// First byte of the read command has some status info in it.
		uint8_t sr;
		if (I2C_Read(MCP4728_I2C, MCP4728_ADDR, &sr, sizeof(sr)))
		{
			if (sr & MCP4728_SR_RDY)
			{
				break;
			}
		}
	}
}
#endif //MCP4728_SUPPORT_ADDRESSING

/*
 * INTERRUPT ROUTINES
 */

