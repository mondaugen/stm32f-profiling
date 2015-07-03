#include "mm_dsp_tests.h" 
#include <stdlib.h> 
#include "i2s_setup.h" 
#include "sines.h" 
#include "mm_sigproc.h" 
#include "mm_sigchain.h"
#include "mm_sigconst.h" 
#include "mm_trapenvedsampleplayer.h" 

#define BUS_BLOCK_SIZE (CODEC_DMA_BUF_LEN / CODEC_NUM_CHANNELS) 

MMBus *outBus;
MMSigChain sigChain;
MMSigConst sigConst;
MMTrapEnvedSamplePlayer spsp;

void mm_dsp_tests_setup(void)
{
    fast_sines_setup();
    outBus = MMBus_new(BUS_BLOCK_SIZE,1);
    MMSigChain_init(&sigChain);
    MMSigConst_init(&sigConst,outBus,0,MMSigConst_doSum_FALSE);
    MMSigProc_insertAfter(&sigChain.sigProcs,&sigConst);
    MMTrapEnvedSamplePlayer_init(&spsp, outBus, BUS_BLOCK_SIZE, 
            1. / (MMSample)CODEC_SAMPLE_RATE);
    MMSigProc_insertAfter(&sigConst, &spsp);
}

void mm_dsp_test_tick(void)
{
    MMTrapEnvedSamplePlayer_noteOn(
            &spsp,
            60,
            0.9,
            MMInterpMethod_CUBIC,
            0,
            1.,
            1.,
            1.,
            &sine_table,
            1);
    MMSigProc_tick(&sigChain);
}
