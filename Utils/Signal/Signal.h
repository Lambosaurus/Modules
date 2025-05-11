#ifndef SIGNAL_H
#define SIGNAL_H

#include <stdint.h>

#define PI16        0x8000

int16_t cos16(uint16_t angle);
int16_t sin16(uint16_t angle);
uint16_t sqrt16(uint32_t n);

void fft16(int16_t real[], int16_t imag[], uint16_t length);

uint32_t fft16_freq(uint32_t samplerate, uint16_t index, uint16_t length);


#endif //SIGNAL_H
