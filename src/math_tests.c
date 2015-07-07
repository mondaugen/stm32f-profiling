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
float32_t result_buf[RESULT_BUF_LEN];
const float32_t __attribute__((section(".rodata"))) et12_table[] = 
{
    0.0185813612f,
    0.0196862664f,
    0.0208568727f,
    0.0220970869f,
    0.0234110481f,
    0.0248031414f,
    0.0262780130f,
    0.0278405849f,
    0.0294960723f,
    0.0312500000f,
    0.0331082217f,
    0.0350769390f,
    0.0371627223f,
    0.0393725328f,
    0.0417137454f,
    0.0441941738f,
    0.0468220962f,
    0.0496062829f,
    0.0525560260f,
    0.0556811699f,
    0.0589921445f,
    0.0625000000f,
    0.0662164434f,
    0.0701538780f,
    0.0743254447f,
    0.0787450656f,
    0.0834274909f,
    0.0883883476f,
    0.0936441923f,
    0.0992125657f,
    0.1051120519f,
    0.1113623398f,
    0.1179842891f,
    0.1250000000f,
    0.1324328868f,
    0.1403077560f,
    0.1486508894f,
    0.1574901312f,
    0.1668549818f,
    0.1767766953f,
    0.1872883846f,
    0.1984251315f,
    0.2102241038f,
    0.2227246795f,
    0.2359685782f,
    0.2500000000f,
    0.2648657736f,
    0.2806155121f,
    0.2973017788f,
    0.3149802625f,
    0.3337099635f,
    0.3535533906f,
    0.3745767692f,
    0.3968502630f,
    0.4204482076f,
    0.4454493591f,
    0.4719371563f,
    0.5000000000f,
    0.5297315472f,
    0.5612310242f,
    0.5946035575f,
    0.6299605249f,
    0.6674199271f,
    0.7071067812f,
    0.7491535384f,
    0.7937005260f,
    0.8408964153f,
    0.8908987181f,
    0.9438743127f,
    1.0000000000f,
    1.0594630944f,
    1.1224620483f,
    1.1892071150f,
    1.2599210499f,
    1.3348398542f,
    1.4142135624f,
    1.4983070769f,
    1.5874010520f,
    1.6817928305f,
    1.7817974363f,
    1.8877486254f,
    2.0000000000f,
    2.1189261887f,
    2.2449240966f,
    2.3784142300f,
    2.5198420998f,
    2.6696797083f,
    2.8284271247f,
    2.9966141538f,
    3.1748021039f,
    3.3635856610f,
    3.5635948726f,
    3.7754972507f,
    4.0000000000f,
    4.2378523774f,
    4.4898481932f,
    4.7568284600f,
    5.0396841996f,
    5.3393594167f,
    5.6568542495f,
    5.9932283075f,
    6.3496042079f,
    6.7271713220f,
    7.1271897451f,
    7.5509945015f,
    8.0000000000f,
    8.4757047549f,
    8.9796963865f,
    9.5136569200f,
    10.0793683992f,
    10.6787188334f,
    11.3137084990f,
    11.9864566150f,
    12.6992084157f,
    13.4543426441f,
    14.2543794902f,
    15.1019890029f,
    16.0000000000f,
    16.9514095097f,
    17.9593927729f,
    19.0273138400f,
    20.1587367983f,
    21.3574376667f,
    22.6274169980f,
    23.9729132300f,
    25.3984168315f,
    26.9086852881f,
    28.5087589805f
};

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
 * within bounds. y_ is the table that will be written into and y is the
 * wavetable that contains data from which points are interpolated. */
void math_tests_my_cubic_interp_f32_q24_8_idx(float32_t *y_,
                                              uint32_t len_y_,
                                              float32_t *y,
                                              uint32_t len_y,
                                              int32_t *idx0,
                                              int32_t rate)
{
    uint32_t n, idx0_;
    int32_t _idx0 = *idx0, M;
    float32_t y_1, y0, y1, y2,
              frac1, frac0, frac_1, frac_2;
    /* len is only the integer part so shift it up by 8 bits */
    M = len_y;
    M <<= 8l;
    for (n = 0; n < len_y_; n++) {
        /* Ensure the index is within bounds */
        _idx0 -= (_idx0 / M - (_idx0 < 0)) * M;
        /* Compute the integer part of the index */
        idx0_ = (uint32_t)(_idx0 >> 8l);
        /* Compute the fractional part of the index */
        frac0 = ((float32_t)(_idx0 & 0x000000ff)) / 256.0f;
        /* There seems to be no speed gains by using the commented out code. */
//        y1 = y[(idx_0 + 1) * (idx0_ < (len_y-1))];
//        y2 = y[(idx_0 + 2) - len_y*(idx0_ == (len_y-2))];
        y0 = y[idx0_];
        y1 = y[(idx0_ + 1) % len_y];
        y2 = y[(idx0_ + 2) % len_y];
        /* wrap negative index to beginning */
        idx0_ += (len_y - 1) * (idx0_ == 0) - (idx0_ > 0);
        y_1 = y[idx0_];
        /* store terms that are used more than once */
        frac1 = frac0 + 1.;
        frac_1 = frac0 - 1.;
        frac_2 = frac0 - 2.;
        /* do computation */
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
        srcA_buf_f32[n] = sinf(phase) * 0.9;
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
                                             table_length,
                                             srcA_buf_f32,
                                             table_length,
                                             index,
                                             -3.678*((float32_t)(1 << 8l)));
}

void math_tests_ci_q_24_8_long(int32_t *index)
{
    /* The length of one period of the interpolated table in samples */
#define MTCQ248L_PERIOD_SAMPLES 97.65625f//125
    int32_t rate;
    /*
    rate = ((float)table_length)
        /((float)MTCQ248L_PERIOD_SAMPLES)
        *((float32_t)(1 << 8l));
        */
    rate = (5 << 8l) + 1;
    math_tests_my_cubic_interp_f32_q24_8_idx(result_buf,
                                             RESULT_BUF_LEN,
                                             srcA_buf_f32,
                                             table_length,
                                             index,
                                             -rate);
}

#define NUM_POW2_CALCS 1

float32_t math_tests_pow2_calc(void)
{
    int n = 0;
    float32_t result = 0;
    for (n = 0; n < NUM_POW2_CALCS; n++) {
        result += powf(2.,(127. * drand48() - 69.) / 12.);
    }
    return result;
}

/* pitch should be in [0,128) */
float32_t math_tests_pow2_lookup(float32_t pitch)
{
    if (pitch < 0) {
        return et12_table[0];
    }
    if (pitch >= (ET12_TABLE_LEN - 1)) {
        return et12_table[ET12_TABLE_LEN - 1];
    }
    float32_t y0, y1, mu;
    int idx_ = (int)pitch;
    y0 = et12_table[idx_];
    y1 = et12_table[idx_+1];
    mu = pitch - idx_;
    return y0 + mu*(y1 - y0);
}

float32_t math_tests_pow2_nolookup(float32_t pitch)
{
    return powf(2.,(pitch - 69.) / 12.);
}

float32_t math_tests_pow2_precalc(void)
{
    int n = 0;
    float32_t result = 0, pitch;
    for (n = 0; n < NUM_POW2_CALCS; n++) {
        pitch = 127. * drand48();
        result += math_tests_pow2_lookup(pitch);
    }
    return result;
}

