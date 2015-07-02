#include <stdint.h> 
#include <string.h> 
#include "stm32f4xx.h" 
#include "adc.h" 
#include "i2s_setup.h"
#include "sines.h" 
#include "gpio.h" 
#include "flash_commanding.h" 
#include "timer_example.h" 

#define AUDIO_ZEROS 0
#define AUDIO_SINES 1 
#define AUDIO_THROUGH 2
#define AUDIO_CHOICE AUDIO_SINES
#define GRACE_PERIOD 1000 

uint32_t timeout_agg = 0;
uint32_t timeout_add_size = 0;

void audio_callback(int16_t *tx, int16_t *rx, size_t frames, size_t channels)
{
#if AUDIO_CHOICE == AUDIO_THROUGH
    memcpy((void*)tx,(void*)rx,
            sizeof(int16_t)*frames*channels);
#elif AUDIO_CHOICE == AUDIO_ZEROS
   memset((void*)tx,0,sizeof(int16_t)*frames*channels); 
#elif AUDIO_CHOICE == AUDIO_SINES
    size_t n;
    static float buf[CODEC_DMA_BUF_LEN/CODEC_NUM_CHANNELS];
    fast_sines_tick(buf,frames);
    for (n = 0; n < frames; n++) {
        tx[n*channels] = FLOAT_TO_INT16(buf[n]);
    }
#endif /* AUDIO_CHOICE */
}

int main (void)
{
    fast_sines_setup();
    /* Initialize i2s */
//    if (i2s_dma_full_duplex_setup(CODEC_SAMPLE_RATE)) {
        /* Error occurred */
//        while(1);
//    }
    /* Initialize ADCs */
//    timer_setup();
//    adc_setup_dma_scan();
    /* Initialize GPIO (buttons) */
    flash_commanding_gpio_setup();
    while (1) {
        flash_commanding_try_erase_then_write();
    }
}
