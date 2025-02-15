#include "Random.h"

#ifdef RANDOM_SEED_UID
#include "Core.h"
#endif //RANDOM_SEED_UID
#ifdef RANDOM_SEED_TEMP
#include "ADC.h"
#endif //RANDOM_SEED_TEMP

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static inline void Random_Algorithm_Seed(uint32_t seed);
static inline uint32_t Random_Algorithm_Next(void);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

#ifdef RANDOM_SEED_MANUAL

void Random_Seed(uint32_t seed)
{
	Random_Algorithm_Seed(seed);
}

#else //RANDOM_SEED_MANUAL

void Random_Seed(void)
{
	uint32_t seed;
#if (defined(RANDOM_SEED_TEMP))
	ADC_Init();
	ADC_ReadTempNoise((uint8_t*)&seed, sizeof(seed));
	ADC_Deinit();
#elif (defined(RANDOM_SEED_UID))
	const uint32_t * uid = CORE_GetUID();
	seed = uid[0] ^ uid[1] ^ uid[2];
#endif
	Random_Algorithm_Seed(seed);
}

#endif //!RANDOM_SEED_MANUAL

uint32_t Random_Read(void)
{
	return Random_Algorithm_Next();
}

int32_t Random_RandInt(int32_t min, int32_t max)
{
	uint32_t range = max - min;
	return (int32_t)(Random_Read() % (range + 1)) + min;
}

/*
 * ALGORITHIM: LCG
 * The Linear Congruential Generator used in glibc
 */

#ifdef RANDOM_ALG_LCG

static uint32_t gRandom;

static inline void Random_Algorithm_Seed(uint32_t seed)
{
	gRandom = seed;
}

static inline uint32_t Random_Algorithm_Next(void)
{
	gRandom = 1103515245 * gRandom + 12345;
	return gRandom;
}

#endif //RANDOM_ALG_LCG

/*
 * ALGORITHIM: MUL
 * Mulberry32
 * https://gist.github.com/tommyettinger/46a874533244883189143505d203312c
 */

#ifdef RANDOM_ALG_MUL

static uint32_t gRandom;

static inline void Random_Algorithm_Seed(uint32_t seed)
{
	gRandom = seed;
}

static inline uint32_t Random_Algorithm_Next(void)
{
	uint32_t z = (gRandom += 0x6D2B79F5UL);
    z = (z ^ (z >> 15)) * (z | 1UL);
    z ^= z + (z ^ (z >> 7)) * (z | 61UL);
    return z ^ (z >> 14);
}

#endif //RANDOM_ALK_MUL

/*
 * ALGORITHIM: TMT
 * Tiny variant of the Mersenne Twister
 * https://github.com/MersenneTwister-Lab/TinyMT
 */

#ifdef RANDOM_ALG_TMT

#define TINYMT32_SH0 		1
#define TINYMT32_SH1 		10
#define TINYMT32_SH8 		8
#define TINYMT32_MASK 		0x7fffffff

// Note, these 3 magic numbers are parameters. These can be changed.
#define TINYMT32_MAT1		0x8f7011ee
#define TINYMT32_MAT2 		0xfc78ff1f
#define TINYMT32_TMAT 		0x3793fdff

#define TINYMT32_MIN_LOOP	8
#define TINYMT32_PRE_LOOP	8

static uint32_t gRandom[4];

static void Random_Algorithm_Permute(void)
{
	uint32_t x, y;
    y = gRandom[3];
    x = (gRandom[0] & TINYMT32_MASK) ^ gRandom[1] ^ gRandom[2];
    x ^= (x << TINYMT32_SH0);
    y ^= (y >> TINYMT32_SH0) ^ x;
    gRandom[0] = gRandom[1];
    gRandom[1] = gRandom[2];
    gRandom[2] = x ^ (y << TINYMT32_SH1);
    gRandom[3] = y;
    int32_t a = -((int32_t)(y & 1)) & TINYMT32_MAT1;
    int32_t b = -((int32_t)(y & 1)) & TINYMT32_MAT2;
    gRandom[1] ^= (uint32_t)a;
    gRandom[2] ^= (uint32_t)b;
}

static inline void Random_Algorithm_Seed(uint32_t seed)
{
	gRandom[0] = seed;
    gRandom[1] = TINYMT32_MAT1;
    gRandom[2] = TINYMT32_MAT2;
    gRandom[3] = TINYMT32_TMAT;
    for (uint32_t i = 1; i < TINYMT32_MIN_LOOP; i++) {
        gRandom[i & 3] ^= i + (uint32_t)1812433253
            * (gRandom[(i - 1) & 3] ^ (gRandom[(i - 1) & 3] >> 30));
    }
    for (uint32_t i = 0; i < TINYMT32_PRE_LOOP; i++) {
    	Random_Algorithm_Permute();
    }
}

static inline uint32_t Random_Algorithm_Next(void)
{
	Random_Algorithm_Permute();

	// Temper the value.
	uint32_t t0, t1;
    t0 = gRandom[3];
    t1 = gRandom[0] + (gRandom[2] >> TINYMT32_SH8);
    t0 ^= t1;
    if ((t1 & 1) != 0) {
        t0 ^= TINYMT32_TMAT;
    }
    return t0;
}

#endif //RANDOM_ALG_TMT
