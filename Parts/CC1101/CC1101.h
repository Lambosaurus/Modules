#ifndef CC1101_H
#define CC1101_H

#include "Board.h"

/*
 * PUBLIC DEFINITIONS
 */

#ifndef CC1101_BAUD
#define CC1101_BAUD				38400
#endif
#ifndef CC1101_FREQ_KHZ
#define CC1101_FREQ_KHZ			915000
#endif
#ifndef CC1101_DEV_KHZ
#define CC1101_DEV_KHZ			20
#endif
#ifndef CC1101_BANDWIDTH_KHZ
// The bandwidth can be estimated using BAUD + (DEVIATION * 2)
#define CC1101_BANDWIDTH_KHZ	80
#endif
#ifndef CC1101_CH_KHZ
#define CC1101_CH_KHZ			100
#endif
#ifndef CC1101_EN_FEC
#define CC1101_EN_FEC			1
#endif
#ifndef CC1101_OPTIMISE_SENS
#define CC1101_OPTIMISE_SENS 	1
#endif

/*
 * PUBLIC TYPES
 */

typedef struct {
	uint8_t channel;
	uint8_t address;
	int8_t power;
} CC1101Config_t;

/*
 * PUBLIC FUNCTIONS
 */

bool CC1101_Init(CC1101Config_t * config);
void CC1101_Deinit(void);

// These can be updated on the fly
void CC1101_UpdateConfig(CC1101Config_t * config);

bool CC1101_RxReady(void);
uint8_t CC1101_Rx(uint8_t * data, uint8_t count);
void CC1101_Tx(uint8_t dest, uint8_t * data, uint8_t count);
int16_t CC1101_GetRSSI(void);

/*
 * EXTERN DECLARATIONS
 */


#endif //CC1101_H
