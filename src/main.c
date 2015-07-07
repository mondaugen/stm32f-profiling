#include <math.h> 
#include <stdint.h> 
#include <string.h> 
#include "stm32f4xx.h" 
#include "math_tests.h"
#include "sines.h" 
#include "mm_dsp_tests.h" 
#include "mm_common_calcs.h" 
#include "timer_example.h" 

#define TEST_TABLE_LENGTH 2048 

#define PROFILE_LINEAR_INTERP

#if defined PROFILE_DOT_PROD 
float32_t result_a, result_b;
double result_c, result_d;
q63_t result_e, result_f;

int __attribute__((optimize("O0"))) main (void)
{
    math_tests_setup();
    fast_sines_setup();
    result_b = math_tests_my_dot_f32();
    result_a = math_tests_arm_dot_f32();
    result_c = math_tests_arm_dot_q15();
    result_d = math_tests_arm_dot_q31();
    result_e = math_tests_arm_dot_q15_fixed();
    result_f = math_tests_arm_dot_q31_fixed();
    fast_sines_tick(srcA_buf_f32,table_length);
    faster_sines_tick(srcB_buf_f32,table_length);
    fast_sines_tick(srcA_buf_f32,table_length);
    faster_sines_tick(srcB_buf_f32,table_length);
    while (1);
}
#elif defined PROFILE_SAMPLE_PLAYER
int __attribute__((optimize("O0"))) main (void)
{
    float x;
    x = MMCC_MIDItoRate(60.);
    mm_dsp_tests_setup();
    mm_dsp_test_tick();
    while (1);
}
#elif defined PROFILE_LINEAR_INTERP
int __attribute__((optimize("O0"))) main (void)
{
    float32_t index = 0;
    int64_t index_q32_32 = 0;
    int32_t index_q24_8 = 0;
    uint32_t time_taken;
    uint32_t time_taken_2;
    math_tests_interp_tests_setup(TEST_TABLE_LENGTH);
    math_tests_li(&index);
    math_tests_li_ws(&index);
    math_tests_li_q_32_32(&index_q32_32);
    math_tests_li_fast_wrap(&index);
    timer_setup();
    ntimeouts = 0;
    math_tests_li_q_24_8(&index_q24_8);
    time_taken = ntimeouts;
    index_q24_8 = 0;
    TIM2->CNT = 0;
    ntimeouts = 0;
    math_tests_ci_q_24_8(&index_q24_8);
    time_taken_2 = ntimeouts;
    while (1);
}
#endif



void audio_callback(int16_t *tx, int16_t *rx,
        size_t frames, size_t channels)
{
}
