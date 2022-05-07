#ifndef __WAVETABLE__
#define __WAVETABLE__
#include <stdint.h>
// Sample's base frequency: 523.629906 Hz
// Sample's sample rate: 32000 Hz
#define WAVETABLE_LEN 2608
#define WAVETABLE_ATTACK_LEN 1998
#define WAVETABLE_LOOP_LEN 610

#ifdef __cplusplus
extern "C" {
#endif

extern const int16_t WaveTable[WAVETABLE_LEN];
extern const uint16_t WaveTable_Celesta_C5_Increment[];

#ifdef __cplusplus
} //end extern "C"
#endif

#endif
