#include "NTC.h"
#include "ADC.h" // For ADC_MAX

/*
 * PRIVATE DEFINITIONS
 */

#define NTC_T_START		1250
#define NTC_T_STEP		-50
#define NTC_T_END		(NTC_T_START + (((int32_t)LENGTH(cNtcResistance) - 1) * NTC_T_STEP))

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

int32_t NTC_Interpolate(const uint32_t * table, uint32_t steps, int32_t start, int32_t step, uint32_t value);

/*
 * PRIVATE VARIABLES
 */

#ifdef NTC_CURVE_10K_3435K

#define NTC_BIAS_R		10000

static const uint32_t cNtcResistance[] = {
	531,
	596,
	672,
	758,
	858,
	974,
	1110,
	1268,
	1452,
	1669,
	1925,
	2228,
	2586,
	3014,
	3535,
	4161,
	4917,
	5834,
	6948,
	8315,
	10000,
	12081,
	14674,
	17926,
	22021,
	27219,
	33892,
	42506,
	53650,
	68237,
	87559,
	113347,
	148171,
	195652,
};

#else
#error "NTC Curve not specified"
#endif

/*
 * PUBLIC FUNCTIONS
 */

int16_t NTC_AinToTemp(uint16_t ain)
{
	if (ain >= ADC_MAX)
	{
		return NTC_T_END;
	}

	uint32_t r = NTC_BIAS_R * ain;
	r /= (ADC_MAX - ain);
	return NTC_Interpolate(cNtcResistance, LENGTH(cNtcResistance), NTC_T_START, NTC_T_STEP, r);
}

/*
 * PRIVATE FUNCTIONS
 */

int32_t NTC_Interpolate(const uint32_t * table, uint32_t steps, int32_t start, int32_t step, uint32_t value)
{
	if (value < table[0])
	{
		return start;
	}
	else if (value > table[steps-1])
	{
		return start + (step * ((int32_t)steps-1));
	}

	int8_t i = 0;
	while ( value > table[i+1] ) { i++; }
	uint32_t p0 = table[i];
	uint32_t p1 = table[i+1];

	int32_t alpha = ((value - p0) << 8) / (p1 - p0);
	int32_t result = start + (i * step) + ((alpha * step) >> 8);
	return result;
}

/*
 * INTERRUPT ROUTINES
 */
