#include "stm32f4xx.h" 
#include "adc.h" 
#include "i2s_setup.h"
#include "sines.h" 
#include "gpio.h" 
#include "flash_commanding.h" 

#define PLAY_SINES 1

int main (void)
{
    /* Initialize i2s */
    if (i2s_dma_full_duplex_setup(CODEC_SAMPLE_RATE)) {
        /* Error occurred */
        while(1);
    }
    /* Initialize ADCs */
    adc_setup_dma_scan();
    /* Initialize GPIO (buttons) */
    gpio_setup();
//    gpio_setup_pull_down();
    gpio_blue_button();
//    gpio_my_button();
    flash_commanding_gpio_setup();
    while (1) {
        /* Wait for tx pointer to be non-null */
        while (!codecDmaTxPtr);
        int n;
        flash_commanding_try_erase_then_write();
//        sines_update_parameters(); 
        for (n = 0; n < CODEC_DMA_BUF_LEN; n += CODEC_NUM_CHANNELS) {
#if PLAY_SINES 
            codecDmaTxPtr[n] = FLOAT_TO_INT16(sines_tick());
#else
            codecDmaTxPtr[n] = 0;
#endif  
        }
        codecDmaTxPtr = NULL;
    }
}
