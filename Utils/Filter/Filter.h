#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>
/*
 * PUBLIC DEFINITIONS
 */

#ifndef FILTER_MAX
#define FILTER_MAX		100
#endif

/*
 * PUBLIC TYPES
 */

typedef struct {
	uint16_t window[FILTER_MAX];
	uint32_t window_sum;
	uint32_t accumulator;
	uint16_t window_index;
	uint16_t window_max;
	uint16_t accum_index;
	uint16_t accum_max;
} Filter_t;

/*
 * PUBLIC FUNCTIONS
 */

void Filter_Init(Filter_t * flt, uint16_t filter_length);
uint16_t Filter_Read(Filter_t * flt);
void Filter_Push(Filter_t * flt, uint16_t sample);

/*
 * EXTERN DECLARATIONS
 */

#endif //FILTER_H
