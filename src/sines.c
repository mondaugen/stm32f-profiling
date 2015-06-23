#include <math.h> 
#include "sines.h" 
#include "i2s_setup.h"
#include "adc.h" 

float freq[] = {440., 500., 540., 610.};
float amp[]  = {1.,   0.5,  0.25, 0.125};

inline void sines_update_parameters(void)
{
    int k;
    for (k = 0; k < NUM_SINS; k++) {
        freq[k] = adc_values[k];
        amp[k]  = adc_values[k + NUM_SINS]/((float)(1 << 12));
    }
}

inline float sines_tick(void)
{
    int k;
    float x = 0;
    static float phase[] = {0,0,0,0};
    for (k = 0; k < NUM_SINS; k++) {
        x += sinf(phase[k])*amp[k];
        phase[k] += 2.*M_PI*freq[k]/((float)CODEC_SAMPLE_RATE);
        while (phase[k] >= 2.*M_PI) {
            phase[k] -= 2.*M_PI;
        }
    }
    x /= (float)NUM_SINS;
    return x;
}
