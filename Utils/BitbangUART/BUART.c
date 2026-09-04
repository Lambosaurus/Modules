
#include "BUART.h"

#include "GPIO.h"
#include "TIM.h"
#include "Core.h"
#include "util/FIFO.h"

#include <string.h>

/*
 * PRIVATE DEFINITIONS
 */

#define BUART_OSR				16
#define BUART_OSR_WRAP(x)		((x) & (BUART_OSR - 1))

#define BUART_START_BITS		1
#define BUART_STOP_BITS			1
#define BUART_DATA_BITS			8
#define BUART_BITS				(BUART_START_BITS + BUART_DATA_BITS + BUART_STOP_BITS)

#define BUART_MASK(x)			((1 << (x)) - 1)

#ifndef BUART_RX_BFR_SIZE
#define BUART_RX_BFR_SIZE		64
#endif
#ifndef BUART_TX_BFR_SIZE
#define BUART_TX_BFR_SIZE		16
#endif

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void BUART_TxIrq(void);
static void BUART_RxIrq(void);
static void BUART_RxStartIrq(void);

static void BUART_StartRx(void);
static void BUART_StartTx(void);

/*
 * PRIVATE VARIABLES
 */

static struct {
	uint32_t tx_data;
	uint32_t rx_data;
	volatile uint8_t tx_bits;
	volatile uint8_t rx_bits;
	volatile bool tx_busy;
	FIFO_DECLARE(rx_fifo, BUART_RX_BFR_SIZE);
	FIFO_DECLARE(tx_fifo, BUART_TX_BFR_SIZE);
} gBUART;

/*
 * PUBLIC FUNCTIONS
 */

void BUART_Init(uint32_t baud)
{
	gBUART.tx_busy = false;
	gBUART.tx_bits = 0;

	FIFO_Init(&gBUART.rx_fifo);
	FIFO_Init(&gBUART.tx_fifo);

	GPIO_EnableOutput(BUART_TX_PIN, GPIO_PIN_SET);
	GPIO_EnableInput(BUART_RX_PIN, GPIO_Pull_Down);

	TIM_Init(BUART_TIM, baud * BUART_OSR, BUART_OSR - 1);
	TIM_Start(BUART_TIM);

	BUART_StartRx();
}

void BUART_Deinit(void)
{
	GPIO_OnChange(BUART_RX_PIN, GPIO_IT_None, NULL);
	TIM_Deinit(BUART_TIM);
	GPIO_Deinit(BUART_TX_PIN);
	GPIO_Deinit(BUART_RX_PIN);
}

void BUART_Write(const uint8_t * data, uint32_t count)
{
	while (count)
	{
		uint32_t written = FIFO_Write(&gBUART.tx_fifo, data, count);
		data += written;
		count -= written;

		if (!gBUART.tx_busy)
			BUART_StartTx();

		// Stall for a bit if the TX is busy
		if (count)
			CORE_Idle();
	}
}

void BUART_WriteStr(const char * str)
{
	BUART_Write((const uint8_t *)str, strlen(str));
}

void BUART_WriteFlush(void)
{
	while (gBUART.tx_busy)
		CORE_Idle();
}

uint32_t BUART_WriteCount(void)
{
	uint32_t count = FIFO_Count(&gBUART.tx_fifo);
	if (gBUART.tx_busy > 0)
		count++;
	return count;
}

uint32_t BUART_ReadCount(void)
{
	return FIFO_Count(&gBUART.rx_fifo);
}

uint32_t BUART_Read(uint8_t * data, uint32_t count)
{
	return FIFO_Read(&gBUART.rx_fifo, data, count);
}

uint8_t BUART_Pop(void)
{
	return FIFO_BlindPop(&gBUART.rx_fifo);
}

void BUART_ReadFlush(void)
{
	FIFO_Clear(&gBUART.rx_fifo);
}

/*
 * PRIVATE FUNCTIONS
 */

static void BUART_StartTx(void)
{
	gBUART.tx_busy = true;
	uint32_t t = BUART_OSR_WRAP(TIM_Read(BUART_TIM) + 1);
	TIM_SetPulse(BUART_TIM, TIM_CH1, t);
	TIM_OnPulse(BUART_TIM, TIM_CH1, BUART_TxIrq);
}

static void BUART_StartRx(void)
{
	GPIO_OnChange(BUART_RX_PIN, GPIO_IT_Falling, BUART_RxStartIrq);
}

/*
 * INTERRUPT ROUTINES
 */

static void BUART_TxIrq(void)
{
	uint32_t tx_bits = gBUART.tx_bits;
	if (tx_bits == 0)
	{
		uint8_t data;
		if (FIFO_Pop(&gBUART.tx_fifo, &data))
		{
			// Load the new byte
			tx_bits = BUART_BITS;
			gBUART.tx_data = 0x0
				| (data << BUART_START_BITS)
				| (BUART_MASK(BUART_STOP_BITS) << (BUART_START_BITS + BUART_DATA_BITS));
		}
		else
		{
			// No more bytes. Stop.
			TIM_StopPulse(BUART_TIM, TIM_CH1);
			gBUART.tx_busy = false;
			return;
		}
	}

	GPIO_Write(BUART_TX_PIN, gBUART.tx_data & 0x1);
	gBUART.tx_data >>= 1;
	gBUART.tx_bits = --tx_bits;
}

static void BUART_RxStartIrq(void)
{
	GPIO_OnChange(BUART_RX_PIN, GPIO_IT_None, NULL);

	gBUART.rx_bits = BUART_BITS;
	gBUART.rx_data = 0;

	uint32_t t = BUART_OSR_WRAP(TIM_Read(BUART_TIM) + (BUART_OSR / 2) - 1);
	TIM_SetPulse(BUART_TIM, TIM_CH2, t);
	TIM_OnPulse(BUART_TIM, TIM_CH2, BUART_RxIrq);
}

static void BUART_RxIrq(void)
{
	uint32_t rx_bits = gBUART.rx_bits;
	if (rx_bits)
	{
		gBUART.rx_data |= (uint32_t)GPIO_Read(BUART_RX_PIN) << (BUART_BITS - rx_bits);

		if (rx_bits == BUART_BITS && gBUART.rx_data != 0)
		{
			// We did not see the start bit.
			// Abort reception.
			// Because we didn't record the stop bit, the byte will be dropped.
			gBUART.rx_bits = 0;
		}
		else
		{
			gBUART.rx_bits = --rx_bits;
		}
	}
	if (rx_bits == 0)
	{
		TIM_StopPulse(BUART_TIM, TIM_CH2);

		uint32_t stop_bit_mask = BUART_MASK(BUART_STOP_BITS) << (BUART_START_BITS + BUART_DATA_BITS);

		// Confirm we saw the stop bits.
		if ((gBUART.rx_data & stop_bit_mask) == stop_bit_mask)
		{
			uint8_t data = (gBUART.rx_data >> BUART_START_BITS) & BUART_MASK(BUART_DATA_BITS);
			FIFO_Put(&gBUART.rx_fifo, data);
		}

		BUART_StartRx();
	}
}

