#include "math_tests.h"
#include <stdlib.h> 

const uint32_t table_length = 1024;
float32_t *srcA_buf_f32;
float32_t *srcB_buf_f32;
q15_t *srcA_buf_q15;
q15_t *srcB_buf_q15;
q31_t *srcA_buf_q31;
q31_t *srcB_buf_q31;

extern void HardFault_Handler(void);

void math_tests_setup(void)
{
    srcA_buf_f32 = (float32_t*)malloc(sizeof(float32_t)*table_length);
    if (!srcA_buf_f32) {
        HardFault_Handler();
    }
    srcB_buf_f32 = (float32_t*)malloc(sizeof(float32_t)*table_length);
    if (!srcA_buf_f32) {
        HardFault_Handler();
    }
    srcA_buf_q15 = (q15_t*)malloc(sizeof(q15_t)*table_length);
    if (!srcA_buf_q15) {
        HardFault_Handler();
    }
    srcB_buf_q15 = (q15_t*)malloc(sizeof(q15_t)*table_length);
    if (!srcA_buf_q15) {
        HardFault_Handler();
    }
    srcA_buf_q31 = (q31_t*)malloc(sizeof(q31_t)*table_length);
    if (!srcA_buf_q31) {
        HardFault_Handler();
    }
    srcB_buf_q31 = (q31_t*)malloc(sizeof(q31_t)*table_length);
    if (!srcA_buf_q31) {
        HardFault_Handler();
    }
    uint32_t n;
    for (n = 0; n < table_length; n++) {
        srcA_buf_f32[n] = drand48();
        srcB_buf_f32[n] = drand48();
    }
    arm_float_to_q15(srcA_buf_f32,srcA_buf_q15,table_length);
    arm_float_to_q15(srcB_buf_f32,srcB_buf_q15,table_length);
    arm_float_to_q31(srcA_buf_f32,srcA_buf_q31,table_length);
    arm_float_to_q31(srcB_buf_f32,srcB_buf_q31,table_length);
}

float32_t math_tests_arm_dot_f32(void)
{
    static float32_t tmp;
    arm_dot_prod_f32(srcA_buf_f32,srcB_buf_f32,table_length,&tmp);
    return tmp;
}

double math_tests_arm_dot_q15(void)
{
    q63_t tmp;
    double result;
    arm_dot_prod_q15(srcA_buf_q15,srcB_buf_q15,table_length,&tmp);
    result = (double)tmp / ((double)(1ll << 30));
    return result;
}

q63_t math_tests_arm_dot_q15_fixed(void)
{
    q63_t tmp;
    arm_dot_prod_q15(srcA_buf_q15,srcB_buf_q15,table_length,&tmp);
    return tmp;
}

q63_t math_tests_arm_dot_q31_fixed(void)
{
    q63_t tmp;
    arm_dot_prod_q31(srcA_buf_q31,srcB_buf_q31,table_length,&tmp);
    return tmp;
}

double math_tests_arm_dot_q31(void)
{
    q63_t tmp;
    double result;
    arm_dot_prod_q31(srcA_buf_q31,srcB_buf_q31,table_length,&tmp);
    result = (double)tmp / ((double)(1ll << 48));
    return result;
}

void my_dot_prod_f32(float32_t *a, float32_t *b, uint32_t len, float32_t *result)
{
    *result = 0;
    while (len--) {
        *result += a[len] * b[len];
    }
}

float32_t math_tests_my_dot_f32(void)
{
    static float32_t tmp;
    my_dot_prod_f32(srcA_buf_f32,srcB_buf_f32,table_length,&tmp);
    return tmp;
}
