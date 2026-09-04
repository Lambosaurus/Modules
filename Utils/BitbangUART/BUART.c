
#include "BUART.h"

#include "GPIO.h"
#include "TIM.h"
#include "Board.h"
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

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void BUART_WriteByte(uint8_t data);

static void BUART_TxIrq(void);
static void BUART_RxIrq(void);
static void BUART_RxStartIrq(void);

static void BUART_PrepareRx(void);

/*
 * PRIVATE VARIABLES
 */

static struct {
	uint32_t tx_data;
	uint32_t rx_data;
	volatile uint8_t tx_bits;
	volatile uint8_t rx_bits;

	FIFO_DECLARE(rx_fifo, BUART_RX_BFR_SIZE);
} gBUART;

/*
 * PUBLIC FUNCTIONS
 */

void BUART_Init(uint32_t baud)
{
	gBUART.tx_bits = 0;

	FIFO_Init(&gBUART.rx_fifo);

	GPIO_EnableOutput(BUART_TX_PIN, GPIO_PIN_SET);
	GPIO_EnableInput(BUART_RX_PIN, GPIO_Pull_Down);

	TIM_Init(BUART_TIM, baud * BUART_OSR, BUART_OSR - 1);
	TIM_Start(BUART_TIM);

	BUART_PrepareRx();
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
	while (count--)
	{
		BUART_WriteByte(*data++);
	}
}

void BUART_WriteStr(const char * str)
{
	BUART_Write((const uint8_t *)str, strlen(str));
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

static void BUART_PrepareRx(void)
{
	GPIO_OnChange(BUART_RX_PIN, GPIO_IT_Falling, BUART_RxStartIrq);
}

static void BUART_WriteByte(uint8_t data)
{
	// We write the stop bit twice, because the last bit marks the timer end point.
	gBUART.tx_bits = BUART_BITS + 1;
	gBUART.tx_data = 0x0
			| (data << BUART_START_BITS)
			| (BUART_MASK(BUART_STOP_BITS + 1) << (BUART_START_BITS + BUART_DATA_BITS));


	uint32_t t = BUART_OSR_WRAP(TIM_Read(BUART_TIM) + 1);
	TIM_SetPulse(BUART_TIM, TIM_CH1, t);
	TIM_OnPulse(BUART_TIM, TIM_CH1, BUART_TxIrq);

	while (gBUART.tx_bits);
}

/*
 * INTERRUPT ROUTINES
 */

static void BUART_TxIrq(void)
{
	uint32_t tx_bits = gBUART.tx_bits;
	if (tx_bits)
	{
		GPIO_Write(BUART_TX_PIN, gBUART.tx_data & 0x1);
		gBUART.tx_data >>= 1;
		gBUART.tx_bits = --tx_bits;
	}

	if (tx_bits == 0)
		TIM_StopPulse(BUART_TIM, TIM_CH1);
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

		BUART_PrepareRx();
	}
}

