#include "math_tests.h"
#include <stdlib.h>
#include "mm_wrap.h" 

uint32_t table_length = 1024;
float32_t *srcA_buf_f32 = NULL;
float32_t *srcB_buf_f32 = NULL;
q15_t *srcA_buf_q15 = NULL;
q15_t *srcB_buf_q15 = NULL;
q31_t *srcA_buf_q31 = NULL;
q31_t *srcB_buf_q31 = NULL;
float32_t *linear_interp_ws = NULL;

extern void HardFault_Handler(void);

void math_tests_setup(void)
{
    srcA_buf_f32 = (float32_t*)malloc(sizeof(float32_t)*table_length);
    if (!srcA_buf_f32) {
        HardFault_Handler();
    }
    srcB_buf_f32 = (float32_t*)malloc(sizeof(float32_t)*table_length);
    if (!srcB_buf_f32) {
        HardFault_Handler();
    }
    srcA_buf_q15 = (q15_t*)malloc(sizeof(q15_t)*table_length);
    if (!srcA_buf_q15) {
        HardFault_Handler();
    }
    srcB_buf_q15 = (q15_t*)malloc(sizeof(q15_t)*table_length);
    if (!srcB_buf_q15) {
        HardFault_Handler();
    }
    srcA_buf_q31 = (q31_t*)malloc(sizeof(q31_t)*table_length);
    if (!srcA_buf_q31) {
        HardFault_Handler();
    }
    srcB_buf_q31 = (q31_t*)malloc(sizeof(q31_t)*table_length);
    if (!srcB_buf_q31) {
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

void fwrap_v(float32_t *x, uint32_t len, float32_t a, float32_t b)
{
    if(a>b){
        /* swap entries */
        float32_t tmp = a;
        a = b;
        b = tmp;
    }
    int N;
    uint32_t n;
    for (n = 0; n < len; n++) {
        N = 0;
        if (*x < a) {
            N = (int) ((*x - a) / (b - a)) - 1;
        }
        *x -= N * (b - a);
        N = 0;
        if (*x >= b) {
            N = (int) ((*x - b) / (b - a)) + 1;
        }
        *x++ -= N * (b - a);
    }
}

/* No preallocated workspace */
void math_tests_my_linear_interp_f32(float32_t *y_,
                                     float32_t *y,
                                     uint32_t len,
                                     float32_t *idx0,
                                     float32_t rate)
{
    uint32_t n, idx0_, _idx0;
    float32_t y0, y1, frac;
    _idx0 = *idx0;
    for (n = 0; n < (len-1); n++) {
        idx0_ = (uint32_t)_idx0;
        frac = _idx0 - idx0_;
        y0 = y[idx0_];
        y1 = y[(idx0_ + 1) % len];
        y_[n] = y0 + frac*(y1 - y0);
        _idx0 = MM_fwrap(_idx0 + rate,0.,len);
    }
    idx0_ = (uint32_t)_idx0;
    frac = _idx0 - idx0_;
    y0 = y[idx0_];
    y1 = y[(idx0_ + 1) % len];
    y_[n] = y0 + frac * (y1 - y0);
    *idx0 = MM_fwrap(_idx0 + rate,0.,len);
}

/* No preallocated workspace. idx0 is updated to the new position but it is not
 * wrapped within table bounds. This is usually okay as the next call to this
 * function will wrap it within bounds. */
void math_tests_my_linear_interp_f32_fast_wrap(float32_t *y_,
                                     float32_t *y,
                                     int32_t len,
                                     float32_t *idx0,
                                     float32_t rate)
{
    uint32_t n, idx0_;
    float32_t y0, y1, frac, _idx0;
    _idx0 = *idx0;
    for (n = 0; n < len; n++) {
        _idx0 -= (((int32_t)_idx0)/len - (_idx0 < 0)) * len;
        idx0_ = (uint32_t)_idx0;
        frac = _idx0 - idx0_;
        y0 = y[idx0_];
        y1 = y[(idx0_ + 1) % len];
        y_[n] = y0 + frac*(y1 - y0);
        _idx0 += rate;
    }
    *idx0 = _idx0;
}

/* idx0 will  be updated but may not lie within the bounds of the table y. This
 * is okay because on successive calls to thi function, the index is always kept
 * within bounds. */
void math_tests_my_linear_interp_f32_q32_32_idx(float32_t *y_,
                                                float32_t *y,
                                                uint32_t len,
                                                int64_t *idx0,
                                                int64_t rate)
{
    uint32_t n, idx0_;
    int64_t _idx0 = *idx0, M;
    float32_t y0, y1, frac;
    /* len is only the integer part so shift it up by 32 bits */
    M = len;
    M <<= 32ll;
    for (n = 0; n < len; n++) {
        /* Ensure the index is within bounds */
        _idx0 -= (_idx0 / M - ((0x8000000000000000 & _idx0) >> 63ll)) * M;
        /* Compute the integer part of the index */
        idx0_ = (uint32_t)(_idx0 >> 32);
        /* Compute the fractional part of the index */
        frac = ((float32_t)(_idx0 & 0x00000000ffffffff)) / 4294967296.0f;
        y0 = y[idx0_];
        y1 = y[(idx0_ + 1) % len];
        y_[n] = y0 + frac*(y1 - y0);
        _idx0 += rate;
    }
    *idx0 = _idx0;
}

/* idx0 will  be updated but may not lie within the bounds of the table y. This
 * is okay because on successive calls to this function, the index is always kept
 * within bounds. */
void math_tests_my_linear_interp_f32_q24_8_idx(float32_t *y_,
                                               float32_t *y,
                                               uint32_t len,
                                               int32_t *idx0,
                                               int32_t rate)
{
    uint32_t n, idx0_;
    int32_t _idx0 = *idx0, M;
    float32_t y0, y1, frac;
    /* len is only the integer part so shift it up by 8 bits */
    M = len;
    M <<= 8l;
    for (n = 0; n < len; n++) {
        /* Ensure the index is within bounds */
//        _idx0 -= (_idx0 / M - ((0x80000000 & _idx0) >> 31l)) * M;
        _idx0 -= (_idx0 / M - (_idx0 < 0)) * M;
        /* Compute the integer part of the index */
        idx0_ = (uint32_t)(_idx0 >> 8l);
        /* Compute the fractional part of the index */
        frac = ((float32_t)(_idx0 & 0x000000ff)) / 256.0f;
        y0 = y[idx0_];
        y1 = y[(idx0_ + 1) % len];
        y_[n] = y0 + frac*(y1 - y0);
        _idx0 += rate;
    }
    *idx0 = _idx0;
}

/* idx0 will  be updated but may not lie within the bounds of the table y. This
 * is okay because on successive calls to this function, the index is always kept
 * within bounds. */
void math_tests_my_cubic_interp_f32_q24_8_idx(float32_t *y_,
                                              float32_t *y,
                                              uint32_t len,
                                              int32_t *idx0,
                                              int32_t rate)
{
    //uint32_t n, idx0_;
    //int32_t _idx0 = *idx0, M;
    //float32_t y_1, y0, y1, y2, frac, 
    //          y0_over_2, y1_over_2, y2_over_6, b;
    ///* len is only the integer part so shift it up by 8 bits */
    //M = len;
    //M <<= 8l;
    //for (n = 0; n < len; n++) {
    //    /* Ensure the index is within bounds */
//  //      _idx0 -= (_idx0 / M - ((0x80000000 & _idx0) >> 31l)) * M;
    //    _idx0 -= (_idx0 / M - (_idx0 < 0)) * M;
    //    /* Compute the integer part of the index */
    //    idx0_ = (uint32_t)(_idx0 >> 8l);
    //    /* Compute the fractional part of the index */
    //    frac = ((float32_t)(_idx0 & 0x000000ff)) / 256.0f;
    //    /* Compute value from interpolating polynomial using Horner's method */
    //    y0 = y[idx0_];
    //    y1 = y[(idx0_ + 1) % len];
    //    y2 = y[(idx0_ + 2) % len];
    //    /* wrap negative index to beginning */
    //    idx0_ -= 1;
    //    idx0_ += len * (idx0_ < 0);
    //    y_1 = y[idx0_];
    //    /* store terms that are used more than once */
    //    y0_over_2 = y0 * 0.5f;
    //    y1_over_2 = y1 * 0.5f;
    //    y2_over_6 = y2 * 0.166666667f;
    //    /* do computation */
    //    b = y0_over_2 - y_1*0.166666667f - y1_over_2 + y2_over_6;
    //    b = y_1*0.5f - y0 + y1_over_2 - b*frac;
    //    b = y1 - y0_over_2 - y_1*0.333333333f - y2_over_6 - b*frac;
    //    y_[n] = y0 - b*frac; 
    //    /* increment index */
    //    _idx0 += rate;
    //}
    //*idx0 = _idx0;
    uint32_t n, idx0_;
    int32_t _idx0 = *idx0, M;
    float32_t y_1, y0, y1, y2,
              frac1, frac0, frac_1, frac_2;
//              frac_2_over_2, frac_1_over_6;
    /* len is only the integer part so shift it up by 8 bits */
    M = len;
    M <<= 8l;
    for (n = 0; n < len; n++) {
        /* Ensure the index is within bounds */
//        _idx0 -= (_idx0 / M - ((0x80000000 & _idx0) >> 31l)) * M;
        _idx0 -= (_idx0 / M - (_idx0 < 0)) * M;
        /* Compute the integer part of the index */
        idx0_ = (uint32_t)(_idx0 >> 8l);
        /* Compute the fractional part of the index */
        frac0 = ((float32_t)(_idx0 & 0x000000ff)) / 256.0f;
        /* Compute value from interpolating polynomial using Horner's method */
        y0 = y[idx0_];
        y1 = y[(idx0_ + 1) % len];
        y2 = y[(idx0_ + 2) % len];
        /* wrap negative index to beginning */
        idx0_ -= 1;
        idx0_ += len * (idx0_ < 0);
        y_1 = y[idx0_];
        /* store terms that are used more than once */
        frac1 = frac0 + 1.;
        frac_1 = frac0 - 1.;
        frac_2 = frac0 - 2.;
//        frac_2_over_2 = frac_2 * 0.5f;
//        frac_1_over_6 = frac_1*0.166666667f;
        /* do computation */
        /*
        y_[n] = frac1*frac_1*frac_2_over_2*y0
                - frac0*frac_2*frac_1_over_6*y_1
                - frac1*frac0*frac_2_over_2*y1
                + frac1*frac0*frac_1_over_6*y2; 
                */
        y_[n] = frac1*frac_1*frac_2 * 0.5f*y0
                - frac0*frac_2*frac_1 * 0.166666667f*y_1
                - frac1*frac0*frac_2 * 0.5f*y1
                + frac1*frac0*frac_1 * 0.166666667f*y2; 
        /* increment index */
        _idx0 += rate;
    }
    *idx0 = _idx0;
}

/* With preallocated workspace. Workspace is an array of float32_t of size
 * 2*idx_len.  The index that should start the next call of this function is put
 * in idx0. y_ is an array of length idx_len. */
void math_tests_my_linear_interp_ws_f32(float32_t *ws,
                                        float32_t *y_,
                                        float32_t *y,
                                        uint32_t y_len,
                                        float32_t *idx0,
                                        uint32_t idx_len,
                                        float32_t rate)
{
    float32_t *idx = ws,
              *y1  = ws + idx_len;
    uint32_t n, idx_;
    *idx++ = *idx0;
    for (n = 1; n < idx_len; n++) {
        /* idx[n] = (idx[n-1] + rate) % y_len; */
        *idx = *(idx - 1) + rate;
        idx++;
    } 
    *idx0 = *(idx - 1) + rate;
    idx = ws;
    fwrap_v(idx,idx_len,0.,y_len);
    for (n = 0; n < idx_len; n++) {
        idx_ = (uint32_t)*idx;
        y_[n] = y[idx_];
        *y1++ = y[(idx_+1)%y_len];
        /* idx now holds the fractional part */
        *idx++ -= idx_;
    }
    idx = ws;
    y1 = ws + idx_len;
    for (n = 0; n < idx_len; n++) {
        *y_ += (*idx++)*(*y1++ - *y_);
        y_++;
    }
}

void math_tests_interp_tests_setup(uint32_t new_table_length)
{
    table_length = new_table_length;
    srcA_buf_f32 = (float32_t*)malloc(sizeof(float32_t)*table_length);
    if (!srcA_buf_f32) {
        HardFault_Handler();
    }
    srcB_buf_f32 = (float32_t*)malloc(sizeof(float32_t)*table_length);
    if (!srcB_buf_f32) {
        HardFault_Handler();
    }
    int n;
    float32_t phase = 0;
    for (n = 0; n < table_length; n++) {
        srcA_buf_f32[n] = sinf(phase);
        phase += 2*M_PI*1./((float32_t)table_length);
    }
    linear_interp_ws = (float32_t*)malloc(sizeof(float32_t)*table_length*2);
    if (!linear_interp_ws) {
        HardFault_Handler();
    }
}

void math_tests_li(float32_t *index)
{
    math_tests_my_linear_interp_f32(srcB_buf_f32,
                                     srcA_buf_f32,
                                     table_length,
                                     index,
                                     1.35);
}

void math_tests_li_fast_wrap(float32_t *index)
{
    math_tests_my_linear_interp_f32_fast_wrap(
                                     srcB_buf_f32,
                                     srcA_buf_f32,
                                     table_length,
                                     index,
                                     -4.35);
}

void math_tests_li_ws(float32_t *index)
{
    math_tests_my_linear_interp_ws_f32(linear_interp_ws,
                                       srcB_buf_f32,
                                       srcA_buf_f32,
                                       table_length,
                                       index,
                                       table_length,
                                       1.35);
}

void math_tests_li_q_32_32(int64_t *index)
{
    math_tests_my_linear_interp_f32_q32_32_idx(srcB_buf_f32,
                                               srcA_buf_f32,
                                               table_length,
                                               index,
                                               1.35*((float32_t)(1ll << 32)));
}

void math_tests_li_q_24_8(int32_t *index)
{
    math_tests_my_linear_interp_f32_q24_8_idx(srcB_buf_f32,
                                               srcA_buf_f32,
                                               table_length,
                                               index,
                                               -4.35*((float32_t)(1 << 8l)));
}

void math_tests_ci_q_24_8(int32_t *index)
{
    math_tests_my_cubic_interp_f32_q24_8_idx(srcB_buf_f32,
                                             srcA_buf_f32,
                                             table_length,
                                             index,
                                             -3.678*((float32_t)(1 << 8l)));
}
