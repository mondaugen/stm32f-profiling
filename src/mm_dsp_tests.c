#include "mm_dsp_tests.h" 
#include <stdlib.h> 
#include "i2s_setup.h" 
#include "sines.h" 
#include "mm_sigproc.h" 
#include "mm_sigchain.h"
#include "mm_sigconst.h" 
#include "mm_trapenvedsampleplayer.h" 

#define MM_DSP_TEST_HEAP_OVERFLOW 

#define BUS_BLOCK_SIZE (CODEC_DMA_BUF_LEN / CODEC_NUM_CHANNELS) 

MMBus *outBus;
MMSigChain sigChain;
MMSigConst sigConst;

#if defined MM_DSP_TEST_TWO_INST
 #define MM_DSP_NUM_INST 2 
 MMTrapEnvedSamplePlayer spsp[MM_DSP_NUM_INST];
#elif defined MM_DSP_TEST_HEAP_OVERFLOW
 #define MM_DSP_NUM_INST 10
 #undef BUS_BLOCK_SIZE
 #define BUS_BLOCK_SIZE 10000
 MMTrapEnvedSamplePlayer spsp[MM_DSP_NUM_INST];
#else
 MMTrapEnvedSamplePlayer spsp;
#endif

void mm_dsp_tests_setup(void)
{
#if defined MM_DSP_TEST_ONE_TABLE
    fast_sines_setup_one_table();
    outBus = MMBus_new(BUS_BLOCK_SIZE,1);
    MMSigChain_init(&sigChain);
    MMSigConst_init(&sigConst,outBus,0,MMSigConst_doSum_FALSE);
    MMSigProc_insertAfter(&sigChain.sigProcs,&sigConst);
    MMTrapEnvedSamplePlayerInitStruct init;
    ((MMEnvedSamplePlayerInitStruct*)&init)->outBus = outBus;
    ((MMEnvedSamplePlayerInitStruct*)&init)->internalBusSize = BUS_BLOCK_SIZE;
    ((MMEnvedSamplePlayerInitStruct*)&init)->tickType =
        MMEnvedSamplePlayerTickType_NOSUM;
    ((MMEnvedSamplePlayerInitStruct*)&init)->interp = MMInterpMethod_CUBIC;
    init.tickPeriod = 1. / (MMSample)CODEC_SAMPLE_RATE;
    MMTrapEnvedSamplePlayer_init(&spsp,&init);
    MMSigProc_insertAfter(&sigConst, &spsp);
#elif defined MM_DSP_TEST_TWO_INST
    fast_sines_setup_one_table();
    outBus = MMBus_new(BUS_BLOCK_SIZE,1);
    MMSigChain_init(&sigChain);
    MMSigConst_init(&sigConst,outBus,0,MMSigConst_doSum_FALSE);
    MMSigProc_insertAfter(&sigChain.sigProcs,&sigConst);
    MMTrapEnvedSamplePlayerInitStruct init;
    ((MMEnvedSamplePlayerInitStruct*)&init)->outBus = outBus;
    ((MMEnvedSamplePlayerInitStruct*)&init)->internalBusSize = BUS_BLOCK_SIZE;
    ((MMEnvedSamplePlayerInitStruct*)&init)->tickType =
        MMEnvedSamplePlayerTickType_NOSUM;
    ((MMEnvedSamplePlayerInitStruct*)&init)->interp = MMInterpMethod_CUBIC;
    init.tickPeriod = 1. / (MMSample)CODEC_SAMPLE_RATE;
    /* The first one doesn't sum into the bus but merely writes
     * straight to it. */
    MMTrapEnvedSamplePlayer_init(&spsp[0],&init);
    MMSigProc_insertAfter(&sigConst,&spsp[0]);
    /* The following ones sum into the bus. */
    ((MMEnvedSamplePlayerInitStruct*)&init)->tickType =
        MMEnvedSamplePlayerTickType_SUM;
    int n;
    for (n = 1; n < MM_DSP_NUM_INST; n++) {
        MMTrapEnvedSamplePlayer_init(&spsp[n],&init);
        MMSigProc_insertAfter(&spsp[n-1],&spsp[n]);
    }
#elif defined MM_DSP_TEST_HEAP_OVERFLOW
    fast_sines_setup_one_table();
    outBus = MMBus_new(BUS_BLOCK_SIZE,1);
    MMSigChain_init(&sigChain);
    MMSigConst_init(&sigConst,outBus,0,MMSigConst_doSum_FALSE);
    MMSigProc_insertAfter(&sigChain.sigProcs,&sigConst);
    MMTrapEnvedSamplePlayerInitStruct init;
    ((MMEnvedSamplePlayerInitStruct*)&init)->outBus = outBus;
    ((MMEnvedSamplePlayerInitStruct*)&init)->internalBusSize = BUS_BLOCK_SIZE;
    ((MMEnvedSamplePlayerInitStruct*)&init)->tickType =
        MMEnvedSamplePlayerTickType_NOSUM;
    ((MMEnvedSamplePlayerInitStruct*)&init)->interp = MMInterpMethod_CUBIC;
    init.tickPeriod = 1. / (MMSample)CODEC_SAMPLE_RATE;
    /* The first one doesn't sum into the bus but merely writes
     * straight to it. */
    MMTrapEnvedSamplePlayer_init(&spsp[0],&init);
    MMSigProc_insertAfter(&sigConst,&spsp[0]);
    /* The following ones sum into the bus. */
    ((MMEnvedSamplePlayerInitStruct*)&init)->tickType =
        MMEnvedSamplePlayerTickType_SUM;
    int n;
    for (n = 1; n < MM_DSP_NUM_INST; n++) {
        MMTrapEnvedSamplePlayer_init(&spsp[n],&init);
        MMSigProc_insertAfter(&spsp[n-1],&spsp[n]);
    }
#else
    fast_sines_setup();
    outBus = MMBus_new(BUS_BLOCK_SIZE,1);
    MMSigChain_init(&sigChain);
    MMSigConst_init(&sigConst,outBus,0,MMSigConst_doSum_FALSE);
    MMSigProc_insertAfter(&sigChain.sigProcs,&sigConst);
    MMTrapEnvedSamplePlayerInitStruct init;
    ((MMEnvedSamplePlayerInitStruct*)&init)->outBus = outBus;
    ((MMEnvedSamplePlayerInitStruct*)&init)->internalBusSize = BUS_BLOCK_SIZE;
    ((MMEnvedSamplePlayerInitStruct*)&init)->tickType =
        MMEnvedSamplePlayerTickType_NOSUM;
    ((MMEnvedSamplePlayerInitStruct*)&init)->interp = MMInterpMethod_CUBIC;
    init.tickPeriod = 1. / (MMSample)CODEC_SAMPLE_RATE;
    MMTrapEnvedSamplePlayer_init(&spsp,&init);
    MMSigProc_insertAfter(&sigConst, &spsp);
#endif /* defined MM_DSP_TEST_ONE_TABLE */
}

void mm_dsp_test_tick(void)
{
    MMTrapEnvedSamplePlayer_noteOnStruct init;
#if defined MM_DSP_TEST_TWO_INST
    /* Test a couple blocks */
    int m;
    for (m = 0; m < 2; m++) {
        int n;
        init.note = 60;
        init.amplitude = 1.;
        init.index = 0;
        init.samples = &sine_table;
        init.rate = 0;
        for (n = 0; n < MM_DSP_NUM_INST; n++) {
            init.attackTime = (64./(n+1))/(MMSample)CODEC_SAMPLE_RATE;
            init.releaseTime = (64./(n+1))/(MMSample)CODEC_SAMPLE_RATE;
            init.sustainTime = (64./(n+1))/(MMSample)CODEC_SAMPLE_RATE;
            MMTrapEnvedSamplePlayer_noteOn(&spsp[n],&init);
        }
        MMSigProc_tick(&sigChain);
    }
#else
    init.note = 60;
    init.amplitude = 1.;
    init.index = 0;
    init.attackTime = 64./(MMSample)CODEC_SAMPLE_RATE;
    init.releaseTime = 64./(MMSample)CODEC_SAMPLE_RATE;
    init.sustainTime = 64./(MMSample)CODEC_SAMPLE_RATE;
    init.samples = &sine_table;
    init.rate = 0;
    MMTrapEnvedSamplePlayer_noteOn(&spsp,&init);
#endif /* defined MM_DSP_TEST_TWO_INST */
}
