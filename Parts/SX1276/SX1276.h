#ifndef SX1276_H
#define SX1276_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#ifndef SX1276_MAX_PACKET
#define SX1276_MAX_PACKET		255
#endif

#define SX1276_SPI_FREQ			10000000

/*
 * PUBLIC TYPES
 */

typedef enum {
	SX1276_State_Idle = 0,
	SX1276_State_Transmit,
	SX1276_State_Receive,
} SX1276_State_t;

typedef enum {
	SX1276_SpreadingFactor_6 	= 6,
	SX1276_SpreadingFactor_7 	= 7,
	SX1276_SpreadingFactor_8 	= 8,
	SX1276_SpreadingFactor_9 	= 9,
	SX1276_SpreadingFactor_10	= 10,
	SX1276_SpreadingFactor_11	= 11,
	SX1276_SpreadingFactor_12 	= 12,
} SX1276_SpreadingFactor_t;

typedef enum {
	SX1276_Bandwidth_7800Hz 	= 0,
	SX1276_Bandwidth_10400Hz 	= 1,
	SX1276_Bandwidth_15600Hz 	= 2,
	SX1276_Bandwidth_20800Hz 	= 3,
	SX1276_Bandwidth_31200Hz 	= 4,
	SX1276_Bandwidth_41700Hz 	= 5,
	SX1276_Bandwidth_62500Hz 	= 6,
	SX1276_Bandwidth_125kHz 	= 7,
	SX1276_Bandwidth_250kHz 	= 8,
	SX1276_Bandwidth_500kHz 	= 9,
} SX1276_Bandwidth_t;

typedef enum {
	SX1276_CodingRate_4_5 		= 1,
	SX1276_CodingRate_4_6 		= 2,
	SX1276_CodingRate_4_7 		= 3,
	SX1276_CodingRate_4_8 		= 4,
} SX1276_CodingRate_t;

typedef enum {
	SX1276_SyncWord_Private		= 0x12,
	SX1276_SyncWord_Public		= 0x34,
} SX1276_SyncWord_t;

typedef enum {
	SX1276_Flag_None			= 0x00,
	SX1276_Flag_ImplicitHeader	= 0x01,
	SX1276_Flag_CrcEnable		= 0x02,
	SX1276_Flag_InvertIQ		= 0x04,
} SX1276_Flag_t;

/*
 * PUBLIC FUNCTIONS
 */

bool SX1276_Init(void);
void SX1276_Deinit(void);

void SX1276_Update(void);
SX1276_State_t SX1276_GetState(void);

void SX1276_Configure(uint32_t freq, SX1276_Bandwidth_t bw, SX1276_SpreadingFactor_t sf, SX1276_CodingRate_t cr, SX1276_SyncWord_t sw, uint16_t preamble, SX1276_Flag_t flags, int8_t power);

void SX1276_Send(const uint8_t * data, uint8_t size);
void SX1276_StartReceive(void);
uint32_t SX1276_Receive(uint8_t * data, uint8_t size, int16_t * rssi);
void SX1276_Stop(void);

/*
 * EXTERN DECLARATIONS
 */

#endif // SX1276_H
