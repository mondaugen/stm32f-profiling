#include "timer_example.h" 
#include "stm32f4xx.h" 

#define GPIOG_PIN 9

volatile uint32_t ntimeouts = 0;

void timer_setup(void)
{
    /* Enable timer peripheral clock */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    /* Set to upcounting mode */
    TIM2->CR1 &= ~TIM_CR1_DIR;
    /* Set overflow value
     * If the clock speed is 90MHz, this should overflow at 1MHz. To check this
     * is the case, find out what the AHB bus's clock speed is, then divide by
     * the APB1 prescalar. If the APB1 prescalar is not 1, then multiply this
     * value by 2. That will give you the frequency of the clock driving the
     * timer. */
    TIM2->ARR = 89; 
    /* Enable interrupt generation */
    TIM2->DIER |= TIM_DIER_UIE;
    /* Enable interrupts in NVIC */
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CNT = 0;
    /* Enable timer */
    TIM2->CR1 |= TIM_CR1_CEN;
}

void TIM2_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(TIM2_IRQn);
    if (TIM2->SR & TIM_SR_UIF) {
        /* Clear pending interrupt */
        TIM2->SR &= ~TIM_SR_UIF;
        ntimeouts++;
    }
}
