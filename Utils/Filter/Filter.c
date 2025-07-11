#include "Filter.h"
#include <string.h>

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

/*
 * PUBLIC FUNCTIONS
 */

void Filter_Init(Filter_t * flt, uint16_t filter_length)
{
	bzero(flt, sizeof(Filter_t));

	flt->accum_max = (filter_length + (FILTER_MAX - 1)) / FILTER_MAX;
	flt->window_max = filter_length / flt->accum_max;
}

uint16_t Filter_Read(Filter_t * flt)
{
	return flt->window_sum / flt->window_max;
}

void Filter_Push(Filter_t * flt, uint16_t sample)
{
	flt->accum_index += 1;
	flt->accumulator += sample;

	if (flt->accum_index >= flt->accum_max)
	{
		uint16_t new_sample = flt->accumulator / flt->accum_max;
		flt->accumulator = 0;
		flt->accum_index = 0;

		uint16_t old_sample = flt->window[flt->window_index];
		flt->window[flt->window_index] = new_sample;
		flt->window_sum += new_sample - old_sample;

		flt->window_index += 1;
		if (flt->window_index >= flt->window_max)
		{
			flt->window_index = 0;
		}
	}
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */
