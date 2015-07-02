#ifndef I2S_SETUP_H
#define I2S_SETUP_H 

#include <stdint.h> 
#include <stddef.h> 

#define CODEC_SAMPLE_RATE 44100//32000//

#define CODEC_NUM_CHANNELS 2

/* number of items in one half of the buffer (because we do our own double
 * buffering on a single array, addressing with no offset or an offset of one
 * half the total length of the array. */
#define CODEC_DMA_BUF_LEN 512

#define UINT16_TO_FLOAT(x) ((float)((int32_t)x - 0x8000)/((float)0x8000))
#define FLOAT_TO_UINT16(x) ((uint16_t)((x + 1.) * 0x8000))
#define INT16_TO_FLOAT(x) ((float)x/(float)32768)
#define FLOAT_TO_INT16(x) ((int16_t)(x * 32768))

extern int16_t * volatile codecDmaRxPtr;
extern int16_t * volatile codecDmaTxPtr;
extern int processing;
extern int grace_ticks;

void i2s_setup(void);
void i2s_full_duplex_setup(void);
int i2s_dma_full_duplex_setup(uint32_t sr);
/* This call back is provided with the input and output buffers, the number of
 * frames and the number of channels. Given N samples in a buffer and M channels
 * then there are N/M frames.
 */
extern void audio_callback(int16_t *tx, int16_t *rx,
        size_t frames, size_t channels);
#endif /* I2S_SETUP_H */
