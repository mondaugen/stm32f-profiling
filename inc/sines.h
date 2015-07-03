#ifndef SINES_H
#define SINES_H 
#include <stddef.h> 
#include <stdint.h> 
#include "mm_wavtab.h" 

#define NUM_SINS 4 
#define SINE_TABLE_SIZE 1024

extern MMWavTab sine_table;

void fast_sines_tick(float *buf, size_t length);
void faster_sines_tick(float *buf, uint32_t length);
void fast_sines_setup(void);
inline float sines_tick(void);
inline void sines_update_parameters(void);
#endif /* SINES_H */
