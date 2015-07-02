#include <math.h> 
#include "sines.h" 
#include "i2s_setup.h"
#include "adc.h" 

/* Cubic interpolation as described by Miller Puckette in his "Theory and
* Technique of Electronic Music." Seems the best so far. */
#define MM_f_interp_cubic_msp_(y_1,y0,y1,y2,mu)\
(-(mu)*((mu)-1.)*((mu)-2.)/6.*(y_1) \
+((mu)+1.)*((mu)-1.)*((mu)-2.)/2.*(y0) \
-((mu)+1.)*(mu)*((mu)-2.)/2.*(y1) \
+((mu)+1.)*(mu)*((mu)-1.)/6.*(y2)); \

#define sine_table_lookup_cubic(pdest,phase)\
    int x_1, x0, x1, x2;\
    float mu;\
    x0 = (int)(phase*((float)SINE_TABLE_SIZE));\
    x_1 = x0-1;\
    if (x_1 < 0) {\
        x_1 += SINE_TABLE_SIZE;\
    }\
    x1 = x0+1;\
    if (x1 >= SINE_TABLE_SIZE) {\
        x1 -= SINE_TABLE_SIZE;\
    }\
    x2 = x0-2;\
    if (x2 >= SINE_TABLE_SIZE) {\
        x2 -= SINE_TABLE_SIZE;\
    }\
    mu = phase*((float)SINE_TABLE_SIZE)-x0;\
    *(pdest) = MM_f_interp_cubic_msp_(sine_table[x_1],\
            sine_table[x0],sine_table[x1],sine_table[x2],mu)

#define sine_table_lookup_none(pdest,phase)\
    int x0;\
    x0 = (int)(phase*((float)SINE_TABLE_SIZE));\
    *(pdest) = sine_table[x0]

//float freq[] = {((float)CODEC_SAMPLE_RATE)
//    /((float)(CODEC_DMA_BUF_LEN/CODEC_NUM_CHANNELS))/2. *1.1,
//    500., 540., 610.};
float freq[] = {440., 0., 0., 0.};
//float freq[] = {440., 500., 540., 610.};
float amp[]  = {.9,   0,  0, 0};
//float amp[]  = {1.,   0.5,  0.25, 0.125};
float sine_table[SINE_TABLE_SIZE];

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

void fast_sines_setup(void)
{
    int n;
    float phase = 0;
    for (n = 0; n < SINE_TABLE_SIZE; n++) {
        sine_table[n] = sinf(phase);
        phase += 2*M_PI*1./((float)SINE_TABLE_SIZE);
    }
}

float fast_sines_tick(void)
{
    int k;
    float x = 0;
    static float phase[] = {0,0,0,0};
    for (k = 0; k < NUM_SINS; k++) {
        float tmp;
//        sine_table_lookup_cubic(&tmp,phase[k]);
        sine_table_lookup_none(&tmp,phase[k]);
        x += tmp*amp[k];
        phase[k] += freq[k]/((float)CODEC_SAMPLE_RATE);
        phase[k] = fmodf(phase[k],1.);
//        phase[k] = phase[k] - (float)((int)phase[k]);
//        if (phase[k] < 0) {
//            phase[k] += 1.;
//        }
    }
    x /= (float)NUM_SINS;
    return x;
}
