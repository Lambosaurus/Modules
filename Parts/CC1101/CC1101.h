#ifndef CC1101_H
#define CC1101_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#define CC1101_SPI_BITRATE		6000000 //6MHz
#define CC1101_PACKET_MAX		64

/*
 * PUBLIC TYPES
 */

typedef struct {
	uint8_t channel;
	uint8_t address;
	int8_t power; 		// in dBm. -30 to 10
} CC1101Config_t;

/*
 * PUBLIC FUNCTIONS
 */

bool CC1101_Init(CC1101Config_t * config);
void CC1101_Deinit(void);

// These can be updated on the fly
void CC1101_UpdateConfig(CC1101Config_t * config);

bool CC1101_ReadReady(void);
uint8_t CC1101_Read(uint8_t * data, uint8_t count);
void CC1101_Write(uint8_t dest, uint8_t * data, uint8_t count);
int16_t CC1101_GetRSSI(void);

/*
 * EXTERN DECLARATIONS
 */


#endif //CC1101_H
