#include "stm32f4xx.h"

int numClicks = 0;

void __attribute__((optimize("O0"))) gpio_setup(void)
{
    /* Configure IRQ on PG2 */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    /* Configure as input */
    GPIOG->MODER &= ~GPIO_MODER_MODER2;
    /* Pull up */
    GPIOG->PUPDR &= ~GPIO_PUPDR_PUPDR2;
    GPIOG->PUPDR |= 0x1 << (2*2);
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI2;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PG;
    EXTI->IMR |= EXTI_IMR_MR2;
    /* Trigger on falling edge */
    EXTI->FTSR |= EXTI_FTSR_TR2;
    NVIC_EnableIRQ(EXTI2_IRQn);
}

void __attribute__((optimize("O0"))) gpio_setup_pull_down(void)
{
    /* Configure IRQ on PG2 */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    /* Configure as input */
    GPIOG->MODER &= ~GPIO_MODER_MODER2;
    /* Pull down */
    GPIOG->PUPDR &= ~GPIO_PUPDR_PUPDR2;
    GPIOG->PUPDR |= 0x2 << (2*2);
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI2;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PG;
    EXTI->IMR |= EXTI_IMR_MR2;
    /* Trigger on rising edge */
    EXTI->RTSR |= EXTI_RTSR_TR2;
    NVIC_EnableIRQ(EXTI2_IRQn);
}

void __attribute__((optimize("O0"))) gpio_blue_button(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    /* Configure as input */
    GPIOA->MODER &= ~GPIO_MODER_MODER0;
}

void __attribute__((optimize("O0"))) gpio_my_button(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    /* Configure as input */
    GPIOG->MODER &= ~GPIO_MODER_MODER2;
    GPIOG->MODER &= ~GPIO_MODER_MODER3;
    /* Pull up */
    GPIOG->PUPDR &= ~GPIO_PUPDR_PUPDR2;
    GPIOG->PUPDR |= 0x1 << (2*2);
    GPIOG->PUPDR &= ~GPIO_PUPDR_PUPDR3;
    GPIOG->PUPDR |= 0x1 << (3*2);
}

void __attribute__((optimize("O0"))) EXTI2_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(EXTI2_IRQn);
    if (EXTI->PR & EXTI_PR_PR2) {
        EXTI->PR |= EXTI_PR_PR2;
        if (GPIOG->IDR & GPIO_IDR_IDR_9) {
            numClicks++;
        }
    }
}

