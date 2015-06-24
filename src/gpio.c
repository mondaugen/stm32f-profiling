#include "stm32f4xx.h"

void __attribute__((optimize("O0"))) gpio_setup(void)
{
    /* Configure IRQ on PG2 */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    /* Configure as input */
    GPIOG->MODER &= ~GPIO_MODER_MODER2;
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI2;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PG;
    EXTI->IMR |= EXTI_IMR_MR2;
    /* Trigger on rising edge */
    EXTI->RTSR |= EXTI_RTSR_TR2;
    NVIC_EnableIRQ(EXTI2_IRQn);
}

void __attribute__((optimize("O0"))) EXTI2_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(EXTI2_IRQn);
    if (EXTI->PR & EXTI_PR_PR2) {
        EXTI->PR |= EXTI_PR_PR2;
    }
}

