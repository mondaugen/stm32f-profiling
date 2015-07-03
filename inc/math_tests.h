#ifndef MATH_TESTS_H
#define MATH_TESTS_H 
#include "stm32f4xx.h" 
#include "arm_math.h" 
extern const uint32_t table_length;
extern float32_t *srcA_buf_f32;
extern float32_t *srcB_buf_f32;
extern q15_t *srcA_buf_q15;
extern q15_t *srcB_buf_q15;
extern q31_t *srcA_buf_q31;
extern q31_t *srcB_buf_q31;
float32_t math_tests_arm_dot_f32(void);
float32_t math_tests_my_dot_f32(void);
double math_tests_arm_dot_q15(void);
double math_tests_arm_dot_q31(void);
q63_t math_tests_arm_dot_q15_fixed(void);
q63_t math_tests_arm_dot_q31_fixed(void);
void math_tests_setup(void);
#endif /* MATH_TESTS_H */
