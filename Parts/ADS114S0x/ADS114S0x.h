#ifndef ADS114S0X_H
#define ADS114S0X_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#define ADS114S_BITS	16 // 16 bits, signed
#define ADS114S_MAX		((1 << (ADS114S_BITS-1)) - 1)
#define ADS114S_REF		2500

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

bool ADS114S_Init(void);
void ADS114S_Deinit(void);

int16_t ADS114S_Read(uint8_t channel);

/*
 * EXTERN DECLARATIONS
 */

#endif //ADS114S0X_H
