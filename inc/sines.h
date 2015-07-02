#ifndef SINES_H
#define SINES_H 
#include <stddef.h> 
#define NUM_SINS 4 
#define SINE_TABLE_SIZE 1024
void fast_sines_tick(float *buf, size_t length);
void fast_sines_setup(void);
inline float sines_tick(void);
inline void sines_update_parameters(void);
#endif /* SINES_H */
