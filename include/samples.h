#ifndef SAMPLES_H
#define SAMPLES_H

#include <stdint.h>

#define CLOCK_FREQ  8000000 // 8MHz timer clock
//#define SAMPL_FREQ    32000 // 32KHz audio
#define SAMPL_FREQ       0.5 // 0.5Hz visual
#define SAMPL_RESN     256 // 8 bit resolution
#define SAMPL_PSCL (CLOCK_FREQ/(SAMPL_RESN*SAMPL_FREQ))

typedef uint8_t sample_t;

#define NUM_SAMPLES (sizeof(samples)/sizeof(sample_t))
const sample_t samples[] = {
	(sample_t)(0.9*SAMPL_RESN),
	(sample_t)(0.1*SAMPL_RESN),
	(sample_t)(0.9*SAMPL_RESN),
	(sample_t)(0.1*SAMPL_RESN),
	(sample_t)(0.9*SAMPL_RESN),
	(sample_t)(0.1*SAMPL_RESN),
};
#endif
