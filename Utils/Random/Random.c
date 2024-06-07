#include "Random.h"
#include "Core.h"

#ifdef RANDOM_TEMP_SEED
#include "ADC.h"
#endif //RANDOM_TEMP_SEED

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

static uint32_t gRandom;

/*
 * PUBLIC FUNCTIONS
 */

void Random_Seed(void)
{
#ifdef RANDOM_TEMP_SEED
	ADC_Init();
	gRandom = ADC_ReadTempNoise();
	ADC_Deinit();
#else //RANDOM_TEMP_SEED
	const uint32_t * uid = CORE_GetUID();
	gRandom = uid[0] ^ uid[1] ^ uid[2];
#endif
}

uint32_t Random_Read(void)
{
	gRandom = 1103515245 * gRandom + 12345;
	return gRandom;
}

int32_t Random_RandInt(int32_t min, int32_t max)
{
	uint32_t range = max - min;
	return (int32_t)(Random_Read() % (range + 1)) + min;
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

