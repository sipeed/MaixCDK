#ifndef __STFT_H
#define __STFT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "agc.h"
// #include "fft.h"


#define FLOAT_MODE 0
#if !FLOAT_MODE
	typedef uint8_t ftr_t;
#else
	typedef float ftr_t;
#endif

void enable_agc(int agc);
void mel_compute(int16_t *input_data, ftr_t *output_data, int16_t d0);
int agcProcess(int16_t *buffer, uint32_t sampleRate, size_t samplesCount, int16_t agcMode);

#endif