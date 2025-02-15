#ifndef RANDOM_H
#define RANDOM_H

#include "STM32X.h"


/*
 * PUBLIC DEFINITIONS
 */

// Select defaults for seed strategy and algorithim.
#if !(defined(RANDOM_SEED_UID) || defined(RANDOM_SEED_TEMP))
#define RANDOM_SEED_MANUAL
#endif

#if !(defined(RANDOM_ALG_TMT) || defined(RANDOM_ALG_MUL))
#define RANDOM_ALG_LCG
#endif

/*
 * PUBLIC TYPES
 */

/*
 * PUBLIC FUNCTIONS
 */

#ifdef RANDOM_SEED_MANUAL
void Random_Seed(uint32_t seed);
#else
void Random_Seed(void);
#endif

uint32_t Random_Read(void);

// Returns a random number between (and including) min and max
int32_t Random_RandInt(int32_t min, int32_t max);

/*
 * EXTERN DECLARATIONS
 */

#endif //RANDOM_H
