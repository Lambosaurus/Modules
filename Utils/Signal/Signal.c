#include "Signal.h"

static const int16_t gSin16Table[65] = {
	0,      804,     1608,     2410,     3212,     4011,     4808,     5602,
	6393,     7179,     7962,     8739,     9512,    10278,    11039,    11793,
	12539,    13279,    14010,    14732,    15446,    16151,    16846,    17530,
	18204,    18868,    19519,    20159,    20787,    21403,    22005,    22594,
	23170,    23731,    24279,    24811,    25329,    25832,    26319,    26790,
	27245,    27683,    28105,    28510,    28898,    29268,    29621,    29956,
	30273,    30571,    30852,    31113,    31356,    31580,    31785,    31971,
	32137,    32285,    32412,    32521,    32609,    32678,    32728,    32757,
	32767, // Last entry to provide a valid interpolation target
};

static int16_t sin16_quadrant(uint16_t angle)
{
	int32_t index = angle >> 8;
	int32_t frac  = angle & 0xFF;
	int32_t a = gSin16Table[index];
	int32_t b = gSin16Table[index + 1];
	return a + (((b - a) * frac) >> 8);
}

int16_t sin16(uint16_t angle)
{
	switch (angle >> 14)
	{
	case 0:
		return sin16_quadrant(angle);
	case 1:
		return sin16_quadrant(PI16 - angle);
	case 2:
		return -sin16_quadrant(angle - PI16);
	case 3:
		return -sin16_quadrant((2*PI16) - angle);
	}
}

int16_t cos16(uint16_t angle)
{
	return sin16(angle + (PI16/2));
}

uint16_t sqrt16(uint32_t n)
{
	uint32_t result = 0;
	uint32_t one = 1uL << 30;

	// "one" starts at the highest power of four <= than the argument.
	while (one > n)
	{
		one >>= 2;
	}

	while (one != 0)
	{
		if (n >= result + one)
		{
			n -= (result + one);
			result += 2 * one;
		}
		result >>= 1;
		one >>= 2;
	}
	return result;
}

int16_t avg16(int16_t wave[], uint16_t count)
{
	int32_t sum = 0;
	for (uint16_t i = 0; i < count; i++)
	{
		sum += wave[i];
	}
	return sum / count;
}

uint16_t rms16(int16_t wave[], uint16_t count, int16_t mean)
{
	uint32_t sumsq = 0;
	for (uint16_t i = 0; i < count; i++)
	{
		int32_t v = wave[i] - mean;
		sumsq += v * v;
	}
	uint32_t meansq = sumsq / count;
	return sqrt16(meansq);
}

static uint16_t fft16_bitreverse(uint16_t x, uint8_t bits) {
	uint16_t y = 0;
	for (uint8_t i = 0; i < bits; i++) {
		y = (y << 1) | (x & 1);
		x >>= 1;
	}
	return y;
}

// fft16 using butterfly stages.
// length must be a power of two!
void fft16(int16_t real[], int16_t imag[], uint16_t length)
{
	uint8_t stages = 0;
	for (uint16_t m = length; m > 1; m >>= 1) stages++;

	// Bit reversal reordering
	for (uint16_t i = 0; i < length; i++) {
		uint16_t j = fft16_bitreverse(i, stages);
		if (j > i) {
			int16_t tmp_r = real[i];
			real[i] = real[j];
			real[j] = tmp_r;
			int16_t tmp_i = imag[i];
			imag[i] = imag[j];
			imag[j] = tmp_i;
		}
	}

	// FFT butterfly stages
	for (uint8_t stage = 1; stage <= stages; stage++) {
		uint16_t half_m = 1 << (stage - 1);
		uint16_t m = half_m << 1;
		uint16_t angle_step = (2 * PI16) / m;

		for (uint16_t k = 0; k < length; k += m) {
			for (uint16_t j = 0; j < half_m; j++) {
				uint16_t angle = j * angle_step;

				int16_t wr = cos16(angle);
				int16_t wi = -sin16(angle);  // Note: -sin for FFT

				int16_t r0 = real[k + j];
				int16_t i0 = imag[k + j];
				int16_t r1 = real[k + j + half_m];
				int16_t i1 = imag[k + j + half_m];

				// Complex multiply: (wr + j*wi) * (r1 + j*i1)
				int32_t t_real = ((int32_t)wr * r1 - (int32_t)wi * i1) >> 15;
				int32_t t_imag = ((int32_t)wr * i1 + (int32_t)wi * r1) >> 15;

				// Butterfly: even = a + t, odd = a - t
				real[k + j]         = ((int32_t)r0 + t_real) >> 1;
				imag[k + j]         = ((int32_t)i0 + t_imag) >> 1;
				real[k + j + half_m]= ((int32_t)r0 - t_real) >> 1;
				imag[k + j + half_m]= ((int32_t)i0 - t_imag) >> 1;
			}
		}
	}
}

uint32_t fft16_freq(uint32_t srate_hz, uint16_t bucket, uint16_t length)
{
	return ((uint64_t)bucket * srate_hz) / length;
}

