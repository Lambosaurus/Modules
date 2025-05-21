#ifndef VEML3328_H
#define VEML3328_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef enum {
	VEML3328_Gain_Half 	= 0x0C00,
	VEML3328_Gain_1x 	= 0x0000,
	VEML3328_Gain_2x 	= 0x0400,
	VEML3328_Gain_4x 	= 0x0800,
	VEML3328_Gain_8x 	= 0x1800,
	VEML3328_Gain_16x 	= 0x2800,
} VEML3328_Gain_t;

typedef struct {
	uint16_t w;
	uint16_t r;
	uint16_t g;
	uint16_t b;
} VEML3328_Values_t;

/*
 * PUBLIC FUNCTIONS
 */

bool VEML3328_Init(VEML3328_Gain_t gain);
void VEML3328_Deinit(void);

void VEML3328_Read(VEML3328_Values_t * v);

/*
 * EXTERN DECLARATIONS
 */

#endif //VEML3328_H
