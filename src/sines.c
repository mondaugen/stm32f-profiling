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
//float freq[] = {440., 0., 0., 0.};
float freq[] = {440., 500., 540., 610.};
//float amp[]  = {.9,   0,  0, 0};
float amp[]  = {1.,   0.5,  0.25, 0.125};
float sine_table_data[SINE_TABLE_SIZE];
MMWavTab sine_table;

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
        sine_table_data[n] = sinf(phase);
        phase += 2*M_PI*1./((float)SINE_TABLE_SIZE);
    }
    sine_table.samplerate = CODEC_SAMPLE_RATE;
    ((MMArray*)(&sine_table))->length = SINE_TABLE_SIZE;
    ((MMArray*)(&sine_table))->data = (void*)&sine_table_data;
}

/* This clobbers whatever is in buf */
void fast_sines_tick(float *buf, size_t length)
{
    size_t n, k;
    static float phase[] = {0,0,0,0};
    for (n = 0; n < length; n++) {
        buf[n] = 0;
        for (k = 0; k < NUM_SINS; k++) {
//            float tmp;
            MMWavTab_get_interpLinear_flt_sum_(&sine_table,&buf[n],phase[k]);
//            buf[n] += amp[k] * tmp;
            phase[k] += freq[k]/MMWavTab_get_freq(&sine_table);
//            while (phase[k] > SINE_TABLE_SIZE) {
//                phase[k] -= SINE_TABLE_SIZE;
//            }
            phase[k] = MM_fwrap(phase[k],0.,SINE_TABLE_SIZE);
        }
        buf[n] /= (float)NUM_SINS;
    }
}

void faster_sines_tick(float *buf, uint32_t length)
{
    uint32_t n, k;
    static float phase[NUM_SINS];
    float phase_inc[NUM_SINS], wav_tab_freq;
    wav_tab_freq = MMWavTab_get_freq(&sine_table);
    phase_inc[0] = freq[0]/wav_tab_freq;
    for (n = 0; n < length; n++) {
        MMWavTab_get_interpLinear_flt_(&sine_table,&buf[n],phase[0]);
        phase[0] += phase_inc[0];
        phase[0] = MM_fwrap(phase[0],0.,SINE_TABLE_SIZE);
    }
    for (k = 1; k < NUM_SINS; k++) {
        phase_inc[k] = freq[k]/wav_tab_freq;
        for (n = 0; n < length; n++) {
            MMWavTab_get_interpLinear_flt_sum_(&sine_table,&buf[n],phase[k]);
            phase[k] += phase_inc[k];
            phase[k] = MM_fwrap(phase[k],0.,SINE_TABLE_SIZE);
        }
    }
    for (n = 0; n < length; n++) {
        buf[n] /= (float)NUM_SINS;
    }
}
