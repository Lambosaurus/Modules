#include "SX1276.h"

#include "SPI.h"
#include "GPIO.h"
#include "Core.h"

/*
 * PRIVATE DEFINITIONS
 */

#define CLAMP(v, low, high)			((v) < (low) ? (low) : (v) > (high) ? (high) : (v))

#ifndef SX1276_SWITCH_TXPOL
#define SX1276_SWITCH_TXPOL			GPIO_PIN_RESET
#endif
#define SX1276_SWITCH_RXPOL			(!SX1276_SWITCH_TXPOL)

#ifndef SX1276_OVERCURRENT_LIMIT
#define SX1276_OVERCURRENT_LIMIT	240
#endif

#ifndef SX1276_OSC_FREQ
#define SX1276_OSC_FREQ				32000000
#endif

#if defined( SX1276_USE_PA_BOOST )
#elif defined( SX1276_USE_RFO )
#else
#error "Please select PA_BOOST of RFO"
#endif

#if defined(SX1276_USE_LF_PORT)
#elif defined(SX1276_USE_HF_PORT)
#else
#error "Please select LF or HF port"
#endif


// TODO: Maybe we should compute this....
#define SX1276_TX_TIMEOUT			   10000


#define SX1276_REG_WRITE				0x80

#define SX1276_REG_FIFO					0x00
#define SX1276_REG_OPMODE				0x01
#define SX1276_REG_FR_MSB				0x06
#define SX1276_REG_FR_MID				0x07
#define SX1276_REG_FR_LSB				0x08
#define SX1276_REG_PA_CFG				0x09
#define SX1276_REG_PA_RAMP				0x0A
#define SX1276_REG_PA_OCP				0x0B
#define SX1276_REG_LNA					0x0C
#define SX1276_REG_FIFO_ADDR			0x0D
#define SX1276_REG_FIFO_TX_BASE			0x0E
#define SX1276_REG_FIFO_RX_BASE			0x0F
#define SX1276_REG_FIFO_RX_HEAD			0x10
#define SX1276_REG_IRQ_MASK				0x11
#define SX1276_REG_IRQ_FLAGS			0x12
#define SX1276_REG_RX_PKT_SIZE			0x13
#define SX1276_REG_RX_HDR_COUNT_MSB		0x14
#define SX1276_REG_RX_HDR_COUNT_LSB		0x15
#define SX1276_REG_RX_PKT_COUNT_MSB		0x16
#define SX1276_REG_RX_PKT_COUNT_LSB		0x17
#define SX1276_REG_STAT					0x18
#define SX1276_REG_RX_PKT_SNR			0x19
#define SX1276_REG_RX_PKT_RSSI			0x1A
#define SX1276_REG_RSSI					0x1B
#define SX1276_REG_HOP_CH				0x1C
#define SX1276_REG_CFG1					0x1D
#define SX1276_REG_CFG2					0x1E
#define SX1276_REG_SYM_TIMEOUT_LSB		0x1F
#define SX1276_REG_PREAMBLE_MSB			0x20
#define SX1276_REG_PREAMBLE_LSB			0x21
#define SX1276_REG_PKT_SIZE				0x22
#define SX1276_REG_PKT_SIZE_MAX			0x23
#define SX1276_REG_HOP_PERIOD			0x24
#define SX1276_REG_FIFO_RX_TAIL			0x25
#define SX1276_REG_CFG3					0x26
#define SX1276_REG_PPM_CORRECTION		0x27
#define SX1276_REG_FEI_MSB				0x28
#define SX1276_REG_FEI_MID				0x29
#define SX1276_REG_FEI_LSB				0x2A
#define SX1276_REG_RSSI_WB				0x2C
#define SX1276_REG_DET_OPT				0x31
#define SX1276_REG_INV_IQ1				0x33
#define SX1276_REG_DET_THRS				0x37
#define SX1276_REG_SYNC_WORD			0x39
#define SX1276_REG_INV_IQ2				0x3B
#define SX1276_REG_DIO_MAP1				0x40
#define SX1276_REG_DIO_MAP2				0x41
#define SX1276_REG_VERSION				0x42
#define SX1276_REG_TCXO					0x4B
#define SX1276_REG_PA_DAC				0x4D
#define SX1276_REG_AGC_REF				0x61
#define SX1276_REG_AGC_THRS1			0x62
#define SX1276_REG_AGC_THRS2			0x63
#define SX1276_REG_AGC_THRS3			0x64
#define SX1276_REG_PLL					0x70

// SX1276_REG_OPMODE
#define SX1276_OPMODE_MODE_SLEEP		0x00
#define SX1276_OPMODE_MODE_STBY			0x01
#define SX1276_OPMODE_MODE_FSTX			0x02
#define SX1276_OPMODE_MODE_TX			0x03
#define SX1276_OPMODE_MODE_FSRX			0x04
#define SX1276_OPMODE_MODE_RX			0x05
#define SX1276_OPMODE_MODE_RX_SINGLE	0x06
#define SX1276_OPMODE_MODE_CAD			0x07
#define SX1276_OPMODE_MODE_MASK			0x07
#define SX1276_OPMODE_LOWFREQ			0x08
#define SX1276_OPMODE_MOD_FSK			0x00
#define SX1276_OPMODE_MOD_OOK			0x20
#define SX1276_OPMODE_MOD_LORA			0x80

// SX1276_REG_PA_CFG
#define SX1276_PA_CFG_BOOST				0x80
#define SX1276_PA_CFG_RFO				0x00
#define SX1276_PA_CFG_MAX_15DB			(0x7 << 4) // The other values are valid, but I dont care....
#define SX1276_PA_CFG_MAX_12DB			(0x2 << 4)

// SX1276_REG_PA_RAMP
#define SX1276_PA_RAMP_3400US			0x00
#define SX1276_PA_RAMP_2000US			0x01
#define SX1276_PA_RAMP_1000US			0x02
#define SX1276_PA_RAMP_500US			0x03
#define SX1276_PA_RAMP_250US			0x04
#define SX1276_PA_RAMP_125US			0x05
#define SX1276_PA_RAMP_100US			0x06
#define SX1276_PA_RAMP_62US				0x07
#define SX1276_PA_RAMP_50US				0x08
#define SX1276_PA_RAMP_40US				0x09
#define SX1276_PA_RAMP_31US				0x0A
#define SX1276_PA_RAMP_25US				0x0B
#define SX1276_PA_RAMP_20US				0x0C
#define SX1276_PA_RAMP_15US				0x0D
#define SX1276_PA_RAMP_12US				0x0E
#define SX1276_PA_RAMP_10US				0x0F

// SX1276_REG_PA_OCP
#define SX1276_PA_OCP_ENABLE			0x20

// SX1276_REG_CFG1
#define SX1276_CFG1_IMPLICIT_HEADER		0x01
#define SX1276_CFG1_CR_POS				1
#define SX1276_CFG1_CR_MASK				(0x7 << 1)
#define SX1276_CFG1_BW_POS				4
#define SX1276_CFG1_BW_MASK				(0xF << 4)

// SX1276_REG_CFG2
#define SX1276_CFG2_SYM_TIMEOUT_MSB_POS		0
#define SX1276_CFG2_SYM_TIMEOUT_MSB_MASK	(0x03 << 0)
#define SX1276_CFG2_CRC_ENABLE				0x04
#define SX1276_CFG2_TX_CONTINOUS			0x08
#define SX1276_CFG2_SF_POS					4
#define SX1276_CFG2_SF_MASK					(0xF << 4)

// SX1276_REG_CFG3
#define SX1276_CFG3_AGC_ENABLE			0x04
#define SX1276_CFG3_LDRO				0x08

// SX1276_REG_DET_OPT
#define SX1276_DET_OPT_SF6				0xC5
#define SX1276_DET_OPT_SF7_SF12			0xC3

// SX1276_REG_INV_IQ1
#define SX1276_INV_IQ1_NORM				0x27
#define SX1276_INV_IQ1_INV				(0x41 | SX1276_INV_IQ1_NORM)

// SX1276_REG_INV_IQ2
#define SX1276_INV_IQ2_NORM				0x1D
#define SX1276_INV_IQ2_INV				0x19

// SX1276_REG_DET_THRS
#define SX1276_DET_THRS_SF6				0x0C
#define SX1276_DET_THRS_SF7_SF12		0x0A

// SX1276_REG_VERSION
#define SX1276_VERSION					0x12

//SX1276_REG_TCXO
#define SX1276_TCXO_XTAL				0x09
#define SX1276_TCXO_TCXO				(0x10 | SX1276_TCXO_XTAL)

// SX1276_REG_PA_DAC
#define SX1276_PA_DAC_NORMAL			0x84
#define SX1276_PA_DAC_BOOST				0x87

// SX1276_REG_IRQ_MASK
// SX1276_REG_IRQ_FLAGS
#define SX1276_IRQ_RX_TIMEOUT			0x80
#define SX1276_IRQ_RX_DONE				0x40
#define SX1276_IRQ_CRC_ERROR			0x20
#define SX1276_IRQ_VALID_HEADER			0x10
#define SX1276_IRQ_TX_DONE				0x08
#define SX1276_IRQ_CAD_DONE				0x04
#define SX1276_IRQ_FHSS_CC				0x02
#define SX1276_IRQ_CAD_DETECT			0x01
#define SX1276_IRQ_ALL					0xFF

// SX1276_REG_DIO_MAP1
#define SX1276_DIO_MAP1_DIO0_RX_DONE	(0x00 << 6)
#define SX1276_DIO_MAP1_DIO0_TX_DONE	(0x01 << 6)
#define SX1276_DIO_MAP1_DIO0_CAD_DONE	(0x02 << 6)
#define SX1276_DIO_MAP1_DIO1_RX_TIMEOUT	(0x00 << 4)
#define SX1276_DIO_MAP1_DIO1_FHSS_CC	(0x01 << 4)
#define SX1276_DIO_MAP1_DIO1_CAD_DETECT	(0x02 << 4)


/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void SX1276_ReadRegs(uint8_t addr, uint8_t * data, uint16_t count);
static void SX1276_WriteRegs(uint8_t addr, const uint8_t * data, uint16_t count);
static uint8_t SX1276_ReadReg(uint8_t addr);
static void SX1276_WriteReg(uint8_t addr, uint8_t data);

static uint8_t SX1276_ReadIrqs(void);
static void SX1276_ClearIrqs(void);

static void SX1276_EnterTransmit(void);
static void SX1276_EnterReceive(void);
static void SX1276_EnterSleep(void);
static void SX1276_EnterStandby(void);
static void SX1276_SetOpmode(uint8_t mode);

static void SX1276_PollTransmit(void);

static void SX1276_InitialConfig(void);

static inline uint8_t SX1276_GetPaConfig(int8_t power, uint8_t * padac);
static inline uint8_t SX1276_GetOcpConfig(uint8_t limit_ma);
static inline uint16_t SX1276_ComputeSymbolRate(SX1276_Bandwidth_t bw, SX1276_SpreadingFactor_t sf);
static inline int16_t SX1276_ReadPacketRssi(void);

static void SX1276_IRQHandler(void);

/*
 * PRIVATE VARIABLES
 */

static struct {
	SX1276_State_t state;
	uint32_t started;
	uint32_t timeout;
	volatile bool irq_ready;
} gSX1276;

/*
 * PUBLIC FUNCTIONS
 */

bool SX1276_Init(void)
{
	// Module does not enable the SPI. This is a responsibility of the caller.
	GPIO_EnableOutput(SX1276_CS_PIN, true);
	GPIO_EnableInput(SX1276_DIO0_PIN, GPIO_Pull_None);
	GPIO_OnChange(SX1276_DIO0_PIN, GPIO_IT_Rising, SX1276_IRQHandler);

#ifdef SX1276_SWITCH_PIN
	GPIO_EnableOutput(SX1276_SWITCH_PIN, SX1276_SWITCH_RXPOL);
#endif
#ifdef SX1276_RST_PIN
	GPIO_EnableOutput(SX1276_RST_PIN, true);
#endif

	if (SX1276_ReadReg(SX1276_REG_VERSION) == SX1276_VERSION)
	{
		SX1276_InitialConfig();
		return true;
	}
	return false;
}

void SX1276_Deinit(void)
{
	SX1276_EnterSleep();

	GPIO_Deinit(SX1276_CS_PIN);
	GPIO_OnChange(SX1276_DIO0_PIN, GPIO_IT_None, NULL);
	GPIO_Deinit(SX1276_DIO0_PIN);

#ifdef SX1276_RST_PIN
	GPIO_Deinit(SX1276_RST_PIN);
#endif
#ifdef SX1276_SWITCH_PIN
	GPIO_Deinit(SX1276_SWITCH_PIN);
#endif

	CORE_Delay(5);
}

void SX1276_Update(void)
{
	switch (gSX1276.state)
	{
	case SX1276_State_Idle:
		break;

	case SX1276_State_Transmit:
		SX1276_PollTransmit();
		break;

	case SX1276_State_Receive:
		break;
	}
}

SX1276_State_t SX1276_GetState(void)
{
	return gSX1276.state;
}

void SX1276_Configure(uint32_t freq, SX1276_Bandwidth_t bw, SX1276_SpreadingFactor_t sf, SX1276_CodingRate_t cr, SX1276_SyncWord_t sw, uint16_t preamble, SX1276_Flag_t flags, int8_t power)
{
	SX1276_EnterSleep();

	uint8_t regs[6];

	// TODO: There might be a u32 way of achieving this.
	uint32_t frf = ((((uint64_t)freq << 19) + (1 << 18)) / SX1276_OSC_FREQ);
	uint8_t padac;
	uint8_t pacfg = SX1276_GetPaConfig(power, &padac);
	uint8_t paocp = SX1276_GetOcpConfig(SX1276_OVERCURRENT_LIMIT);

	regs[0] = frf >> 16;		// SX1276_REG_FR_MSB
	regs[1] = frf >> 8;			// SX1276_REG_FR_MID
	regs[2] = frf;				// SX1276_REG_FR_LSB
	regs[3] = pacfg; 			// SX1276_REG_PA_CFG
	regs[4] = SX1276_PA_RAMP_40US; 	// SX1276_REG_PA_RAMP.
	regs[5] = paocp;			// SX1276_REG_PA_OCP
	SX1276_WriteRegs(SX1276_REG_FR_MSB, regs, 6);
	SX1276_WriteReg(SX1276_REG_PA_DAC, padac);

	uint16_t sym_timeout = 64; // We are leaving this as default - as we may not even use it.
	uint8_t cfg1 = (bw << SX1276_CFG1_BW_POS)
		         | (cr << SX1276_CFG1_CR_POS)
				 | (flags & SX1276_Flag_ImplicitHeader ? SX1276_CFG1_IMPLICIT_HEADER : 0);

	uint8_t cfg2 = (sf << SX1276_CFG2_SF_POS)
			     | (flags & SX1276_Flag_CrcEnable ? SX1276_CFG2_CRC_ENABLE : 0)
				 | ((sym_timeout >> 8) << SX1276_CFG2_SYM_TIMEOUT_MSB_POS);

	regs[0] = cfg1; 			// SX1276_REG_CFG1
	regs[1] = cfg2;				// SX1276_REG_CFG2
	regs[2] = sym_timeout;		// SX1276_REG_SYM_TIMEOUT_LSB
	regs[3] = preamble >> 8;	// SX1276_REG_PREAMBLE_MSB
	regs[4] = preamble;			// SX1276_REG_PREAMBLE_LSB
	SX1276_WriteRegs(SX1276_REG_CFG1, regs, 5);

	bool sf6 = sf <= SX1276_SpreadingFactor_6;
	uint16_t sym_rate = SX1276_ComputeSymbolRate(bw, sf);

	// Enable LDRO if symbol time over 16ms (ie, 62.5Hz)
	SX1276_WriteReg(SX1276_REG_CFG3, SX1276_CFG3_AGC_ENABLE | (sym_rate <= 62 ? SX1276_CFG3_LDRO : 0));
	SX1276_WriteReg(SX1276_REG_DET_OPT, sf6 ? SX1276_DET_OPT_SF6 : SX1276_DET_OPT_SF7_SF12);
	SX1276_WriteReg(SX1276_REG_INV_IQ1, (flags & SX1276_Flag_InvertIQ ? SX1276_INV_IQ1_INV : SX1276_INV_IQ1_NORM));
	SX1276_WriteReg(SX1276_REG_INV_IQ2, (flags & SX1276_Flag_InvertIQ ? SX1276_INV_IQ2_INV : SX1276_INV_IQ2_NORM));
	SX1276_WriteReg(SX1276_REG_DET_THRS, sf6 ? SX1276_DET_THRS_SF6 : SX1276_DET_THRS_SF7_SF12);
	SX1276_WriteReg(SX1276_REG_SYNC_WORD, sw);

	// TODO:
	// Implement the spurious fixes and 500kHz BW detect optimization stuff from the erratasheet.
}

void SX1276_Send(const uint8_t * data, uint8_t size)
{
	SX1276_EnterStandby();

#ifdef SX1276_SWITCH_PIN
	GPIO_Write(SX1276_SWITCH_PIN, SX1276_SWITCH_TXPOL);
#endif

	// Write the packet
	SX1276_WriteReg(SX1276_REG_FIFO_ADDR, 0);
	SX1276_WriteRegs(SX1276_REG_FIFO, data, size);
	SX1276_WriteReg(SX1276_REG_PKT_SIZE, size);

	// Configure the IRQs
	SX1276_WriteReg(SX1276_REG_DIO_MAP1, SX1276_DIO_MAP1_DIO0_TX_DONE);
	SX1276_ClearIrqs();

	gSX1276.timeout = SX1276_TX_TIMEOUT;
	SX1276_EnterTransmit();
}

void SX1276_StartReceive(void)
{
	gSX1276.timeout = 0;

	// Configure the IRQs
	SX1276_WriteReg(SX1276_REG_DIO_MAP1, SX1276_DIO_MAP1_DIO0_RX_DONE);
	SX1276_ClearIrqs();

	SX1276_EnterReceive();
}

uint32_t SX1276_Receive(uint8_t * data, uint8_t size, int16_t * rssi)
{
	if (gSX1276.state == SX1276_State_Receive && gSX1276.irq_ready)
	{
		uint8_t rx_size = SX1276_ReadReg(SX1276_REG_RX_PKT_SIZE);

		if (rx_size > size)
		{
			// Drop the packet. What else can we do?
			SX1276_ClearIrqs();
		}
		else
		{
			uint8_t addr = SX1276_ReadReg(SX1276_REG_FIFO_RX_HEAD);
			*rssi = SX1276_ReadPacketRssi();

			SX1276_WriteReg(SX1276_REG_FIFO_ADDR, addr);
			SX1276_ReadRegs(SX1276_REG_FIFO, data, rx_size);
			SX1276_ClearIrqs();
			return rx_size;
		}
	}

	return 0;
}

void SX1276_Stop(void)
{
	SX1276_EnterSleep();
}

/*
 * PRIVATE FUNCTIONS
 */

static void SX1276_InitialConfig(void)
{
	// Send all the config that doesnt require the lora settings.

	// Writes to the SX1276_OPMODE_MOD_LORA are ignored until the part is put in sleep mode.
	// So writing the opmode twice on init is required.
	SX1276_EnterSleep();
	SX1276_EnterSleep();

#ifdef SX1276_OSC_TCXO
	SX1276_WriteReg(SX1276_REG_TCXO, SX1276_TCXO_TCXO);
#endif // XTAL is default, so we can leave this reg in this case.

	uint8_t regs[2] = {
			0,	// SX1276_REG_FIFO_TX_BASE
			0,	// SX1276_REG_FIFO_RX_BASE
	};
	SX1276_WriteRegs(SX1276_REG_FIFO_TX_BASE, regs, 2);

	// Note, the mask is inverted. 0 = enable.
	SX1276_WriteReg(SX1276_REG_IRQ_MASK, ~(SX1276_IRQ_TX_DONE | SX1276_IRQ_RX_DONE));
}

__attribute((unused))
static uint8_t SX1276_ReadIrqs(void)
{
	return SX1276_ReadReg(SX1276_REG_IRQ_FLAGS);
}

static void SX1276_ClearIrqs(void)
{
	SX1276_WriteReg(SX1276_REG_IRQ_FLAGS, SX1276_IRQ_ALL);
	gSX1276.irq_ready = false;
}

static inline uint32_t SX1276_GetTimestamp(void)
{
	return CORE_GetTick();
}

static void SX1276_PollTransmit(void)
{
	if (gSX1276.irq_ready || (SX1276_GetTimestamp() - gSX1276.started > gSX1276.timeout))
	{
#ifdef SX1276_SWITCH_PIN
		GPIO_Write(SX1276_SWITCH_PIN, SX1276_SWITCH_RXPOL);
#endif
		// Nothing to do. Mission accomplished.
		SX1276_EnterSleep();
	}
}

static uint32_t SX1276_GetBandwidthHz(SX1276_Bandwidth_t bw)
{
	switch (bw)
	{
	default:
	case SX1276_Bandwidth_7800Hz:
		return 7800;
	case SX1276_Bandwidth_10400Hz:
		return 10400;
	case SX1276_Bandwidth_15600Hz:
		return 15600;
	case SX1276_Bandwidth_20800Hz:
		return 20800;
	case SX1276_Bandwidth_31200Hz:
		return 31200;
	case SX1276_Bandwidth_41700Hz:
		return 41700;
	case SX1276_Bandwidth_62500Hz:
		return 62500;
	case SX1276_Bandwidth_125kHz:
		return 125000;
	case SX1276_Bandwidth_250kHz:
		return 250000;
	case SX1276_Bandwidth_500kHz:
		return 500000;
	}
}

static inline uint16_t SX1276_ComputeSymbolRate(SX1276_Bandwidth_t bw, SX1276_SpreadingFactor_t sf)
{
	uint32_t bw_hz = SX1276_GetBandwidthHz(bw);
	return bw_hz >> sf;
}

static inline uint8_t SX1276_GetPaConfig(int8_t power, uint8_t * padac)
{
	// We could varying the max power - but its to much of a pain.
#ifdef SX1276_USE_PA_BOOST
	power = CLAMP(power, 2, 20);
	if (power > 17)
	{
		*padac = SX1276_PA_DAC_BOOST;
		return SX1276_PA_CFG_BOOST | SX1276_PA_CFG_MAX_15DB | (power - 5);
	}
	else
	{
		*padac = SX1276_PA_DAC_NORMAL;
		return SX1276_PA_CFG_BOOST | SX1276_PA_CFG_MAX_15DB | (power - 2);
	}
#else // !SX1276_USE_PA_BOOST
	power = CLAMP(power, -3, 15);
	*padac = SX1276_PA_DAC_NORMAL;
	if (power > 0)
	{
		return SX1276_PA_CFG_RFO | SX1276_PA_CFG_MAX_15DB | (power + 0);
	}
	else
	{
		return SX1276_PA_CFG_RFO | SX1276_PA_CFG_MAX_12DB | (power + 3);
	}
#endif
}

static inline uint8_t SX1276_GetOcpConfig(uint8_t limit_ma)
{
	if (limit_ma <= 45)
		return SX1276_PA_OCP_ENABLE | 0;
	if (limit_ma <= 120)
		return SX1276_PA_OCP_ENABLE | ((limit_ma - 45) / 5);
	if (limit_ma <= 240)
		return SX1276_PA_OCP_ENABLE | ((limit_ma + 30) / 10);
	return 27; // Leave OCP disabled.
}

static inline int16_t SX1276_ReadPacketRssi(void)
{
	uint8_t regs[2];
	SX1276_ReadRegs(SX1276_REG_RX_PKT_SNR, regs, sizeof(regs));

	int32_t snr = ((int8_t)regs[0] + 2) >> 2; // SNR in dbm
	int32_t rssi = regs[1];

	return
#ifdef SX1276_USE_LF_PORT
		-164
#else // SX1276_USE_HF_PORT
		-157
#endif
		+ (rssi + (rssi >> 4)) // Slope compensation. Equiv to *16/15
		+ (snr < 0 ? snr : 0); // The SNR may be negative. This gives a better result for packets below noise floor.
}

static void SX1276_EnterTransmit(void)
{
	SX1276_SetOpmode(SX1276_OPMODE_MODE_TX);
	gSX1276.state = SX1276_State_Transmit;
	gSX1276.started = SX1276_GetTimestamp();
}

static void SX1276_EnterReceive(void)
{
	SX1276_SetOpmode(SX1276_OPMODE_MODE_RX);
	gSX1276.state = SX1276_State_Receive;
	gSX1276.started = SX1276_GetTimestamp();
}

static void SX1276_EnterSleep(void)
{
	SX1276_SetOpmode(SX1276_OPMODE_MODE_SLEEP);
	gSX1276.state = SX1276_State_Idle;
}

static void SX1276_EnterStandby(void)
{
	SX1276_SetOpmode(SX1276_OPMODE_MODE_STBY);
	gSX1276.state = SX1276_State_Idle;
}

static void SX1276_SetOpmode(uint8_t mode)
{
	SX1276_WriteReg(SX1276_REG_OPMODE,
			mode
			| SX1276_OPMODE_MOD_LORA
#ifdef SX1276_USE_LF_PORT
			| SX1276_OPMODE_LOWFREQ
#endif
	);
}

static void SX1276_ReadRegs(uint8_t addr, uint8_t * data, uint16_t count)
{
	GPIO_Reset(SX1276_CS_PIN);
	SPI_Write(SX1276_SPI, &addr, sizeof(addr));
	SPI_Read(SX1276_SPI, data, count);
	GPIO_Set(SX1276_CS_PIN);
}

static void SX1276_WriteRegs(uint8_t addr, const uint8_t * data, uint16_t count)
{
	addr |= SX1276_REG_WRITE;
	GPIO_Reset(SX1276_CS_PIN);
	SPI_Write(SX1276_SPI, &addr, sizeof(addr));
	SPI_Write(SX1276_SPI, data, count);
	GPIO_Set(SX1276_CS_PIN);
}

static uint8_t SX1276_ReadReg(uint8_t addr)
{
	uint8_t value;
	SX1276_ReadRegs(addr, &value, sizeof(addr));
	return value;
}

static void SX1276_WriteReg(uint8_t addr, uint8_t data)
{
	SX1276_WriteRegs(addr, &data, sizeof(addr));
}

/*
 * INTERRUPT ROUTINES
 */

static void SX1276_IRQHandler(void)
{
	gSX1276.irq_ready = true;
}

