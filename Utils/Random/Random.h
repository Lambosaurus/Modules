#ifndef RANDOM_H
#define RANDOM_H

#include "STM32X.h"


/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

void Random_Seed(void);

uint32_t Random_Read(void);

// Returns a random number between (and including) min and max
int32_t Random_RandInt(int32_t min, int32_t max);

/*
 * EXTERN DECLARATIONS
 */

#endif //RANDOM_H
