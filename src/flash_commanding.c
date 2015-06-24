#include "flash_commanding.h" 
#include "stm32f4xx.h" 

volatile uint32_t flash_state = 0;
int flash_commanding_count = 0;

#define TOMMYS_SIZE (492*4) 
char tommy[TOMMYS_SIZE] = { [0 ... (TOMMYS_SIZE-1)] = 'c' };
#define FLASH_DO_WRITE 0 

void __attribute__((optimize("O0")))
flash_commanding_gpio_setup(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    /* Configure as input */
    GPIOD->MODER &= ~GPIO_MODER_MODER11;
    GPIOB->MODER &= ~GPIO_MODER_MODER0;
    /* Pull up */
    GPIOD->PUPDR &= ~GPIO_PUPDR_PUPDR11;
    GPIOD->PUPDR |= 0x1 << (11*2);
    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR0;
    GPIOB->PUPDR |= 0x1 << (0*2);
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB;
    EXTI->IMR |= EXTI_IMR_MR0;
    /* Trigger on falling edge */
    EXTI->FTSR |= EXTI_FTSR_TR0;
    NVIC_EnableIRQ(EXTI0_IRQn);
}

void __attribute__((optimize("O0"))) EXTI0_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    if (EXTI->PR & EXTI_PR_PR0) {
        /* Reset reqest bit (this is done by writing 1 to corresponding bit) */
        EXTI->PR |= EXTI_PR_PR0;
        if (flash_state & FLASH_CMD_BUSY) {
            /* A write or an erase is still in progress */
            return;
        }
        /* Request erasure which will then be followed by a write */
        flash_state |= FLASH_CMD_WRITE_REQUEST | FLASH_CMD_ERASE_REQUEST;
    }
}

static void __attribute__((optimize("O0"))) flash_commanding_erase_async(void)
{
    if ((flash_state & FLASH_CMD_WRITE_REQUEST) 
            && !(flash_state & FLASH_CMD_WRITE_IN_PROGRESS)) {
        flash_state |= FLASH_CMD_WRITE_IN_PROGRESS;
        /* Start writing ... */
//        flash_commanding_count++;
        /* wait for flash to be free */
        while (FLASH->SR & FLASH_SR_BSY);
        /* Unlock flash */
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xcdef89ab;
        /* Reset program size register */
        FLASH->CR &= ~FLASH_CR_PSIZE;
#if FLASH_ACCESS_SIZE == 4
        FLASH->CR = 0x2 << 8;
#else
#error "Bad value for FLASH_ACCESS_SIZE."
#endif 
        /* Set sector erase bit and select sector */
        FLASH->CR &= ~FLASH_CR_SNB;
#if FLASH_SECTOR >= 12
        FLASH->CR |= ((0x1 << 4) | (FLASH_SECTOR - 12)) << 3;
#else
        FLASH->CR |= FLASH_SECTOR << 3;
#endif  
        FLASH->CR |= FLASH_CR_SER;
        /* Set interrupt on done erasing */
        NVIC_EnableIRQ(FLASH_IRQn);
        FLASH->CR |= FLASH_CR_EOPIE;
        /* Start erasing */
        FLASH->CR |= FLASH_CR_STRT;
    }
}

void __attribute__((optimize("O0"))) flash_commanding_try_erase_then_write(void)
{
    if (flash_state 
            & (FLASH_CMD_WRITE_IN_PROGRESS | FLASH_CMD_ERASE_IN_PROGRESS)) {
        return; /* Already erasing or writing */
    }
    if ((flash_state & FLASH_CMD_WRITE_REQUEST)
            && (flash_state & FLASH_CMD_ERASE_REQUEST)) {
        /* Both of these are required to start the erase_then_write procedure */
        /* Start erasing */
        /* Unlock flash */
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xcdef89ab;
        /* Reset program size register */
        FLASH->CR &= ~FLASH_CR_PSIZE;
#if FLASH_ACCESS_SIZE == 4
        FLASH->CR = 0x2 << 8;
#else
#error "Bad value for FLASH_ACCESS_SIZE."
#endif 
        /* Set sector erase bit and select sector */
        FLASH->CR &= ~FLASH_CR_SNB;
#if FLASH_SECTOR >= 12
        FLASH->CR |= ((0x1 << 4) | (FLASH_SECTOR - 12)) << 3;
#else
        FLASH->CR |= FLASH_SECTOR << 3;
#endif  
        FLASH->CR |= FLASH_CR_SER;
        /* Set interrupt on done erasing */
        NVIC_EnableIRQ(FLASH_IRQn);
        FLASH->CR |= FLASH_CR_EOPIE;
        /* Set erase in progress bit */
        flash_state |= FLASH_CMD_ERASE_IN_PROGRESS;
        /* Request has been acknowledged to erase, clear this bit */
        flash_state &= ~FLASH_CMD_ERASE_REQUEST;
        /* Start erasing */
        FLASH->CR |= FLASH_CR_STRT;
    }
}

void __attribute__((optimize("O0"))) flash_commanding_try_writing_no_dma(void)
{
    if ((flash_state & FLASH_CMD_WRITE_REQUEST) 
            && !(flash_state & FLASH_CMD_WRITE_IN_PROGRESS)) {
        flash_state |= FLASH_CMD_WRITE_IN_PROGRESS;
        /* Start writing ... */
        flash_commanding_count++;
        /* wait for flash to be free */
        while (FLASH->SR & FLASH_SR_BSY);
        /* Unlock flash */
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xcdef89ab;
        /* Reset program size register */
        FLASH->CR &= ~FLASH_CR_PSIZE;
#if FLASH_ACCESS_SIZE == 4
        FLASH->CR = 0x2 << 8;
#else
#error "Bad value for FLASH_ACCESS_SIZE."
#endif 
        /* Set sector erase bit and select sector */
        FLASH->CR &= ~FLASH_CR_SNB;
#if FLASH_SECTOR >= 12
        FLASH->CR |= ((0x1 << 4) | (FLASH_SECTOR - 12)) << 3;
#else
        FLASH->CR |= FLASH_SECTOR << 3;
#endif  
        FLASH->CR |= FLASH_CR_SER;
        /* Start erasing */
        FLASH->CR |= FLASH_CR_STRT;
        /* Wait to be finished */
        while (FLASH->CR & FLASH_SR_BSY);
        /* Reset sector erase bit */
        FLASH->CR &= ~FLASH_CR_SER;
        /* Set program bit */
        FLASH->CR |= FLASH_CR_PG;
        /* wait for flash to be free */
        while (FLASH->SR & FLASH_SR_BSY);
        GPIOG->ODR |= (1 << 13);
        uint32_t *ptr, *val, *val_end;
        ptr = (uint32_t*)FLASH_START_ADDR;
        val = (uint32_t*)tommy;
        val_end = (uint32_t*)(tommy + TOMMYS_SIZE);
        while ((ptr < (uint32_t*)FLASH_END_ADDR)
                && (val < val_end)) {
            *ptr++ = *val++;
        }
        /* wait for flash to be free */
        while (FLASH->SR & FLASH_SR_BSY);
        /* Reset program bit */
        FLASH->CR &= ~FLASH_CR_PG;
        /* lock that flash */
        FLASH->CR |= FLASH_CR_LOCK;
        /* Reset flash writing flags */
        flash_state &= ~(FLASH_CMD_WRITE_IN_PROGRESS | FLASH_CMD_WRITE_REQUEST);
    }
}


void __attribute__((optimize("O0"))) FLASH_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(FLASH_IRQn);
    if (FLASH->SR & FLASH_SR_EOP) {
        /* Clear by writing one to this bit */
        FLASH->SR |= FLASH_SR_EOP;
        /* Turn off flash interrupt */
        FLASH->CR &= ~FLASH_CR_EOPIE;
        NVIC_DisableIRQ(FLASH_IRQn);
        if (flash_state & FLASH_CMD_ERASE_IN_PROGRESS) {
            /* Erasure just finished. */
            /* Reset sector erase bit */
            FLASH->CR &= ~FLASH_CR_SER;
            if (flash_state & FLASH_CMD_WRITE_REQUEST) {
                /* Start a write to flash */
                flash_state |= FLASH_CMD_WRITE_IN_PROGRESS;
                /* start writing */
                /* Set program bit */
                FLASH->CR |= FLASH_CR_PG;
                /* Set up DMA to write to flash */
                /* Turn on DMA2 clock */
                RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
                /* Reset control register */
                DMA2_Stream7->CR = 0x00000000;
                /* Set to channel 6, low priority, memory and peripheral datum
                 * size 32-bits, transfer complete interrupt enable, memory
                 * increment, peripheral (other memory) increment. */
                DMA2_Stream7->CR |= (6 << 25)
                    | (0x0 << 16)
                    | (0x2 << 13)
                    | (0x2 << 11)
                    | (0x1 << 4)
                    | (0x2 << 6) /* memory to memory transfer */
                    | DMA_SxCR_MINC
                    | DMA_SxCR_PINC;
                /* Set peripheral address to data we want to write */
                DMA2_Stream7->PAR = (uint32_t)tommy;
                /* Set memory address to some place */
                DMA2_Stream7->M0AR = (uint32_t)FLASH_START_ADDR;
                /* Set number of items to transfer */
                DMA2_Stream7->NDTR = TOMMYS_SIZE / 4; /* divided by 4 because
                                                         each datum 32 bits wide
                                                         */
    
                /* Enable DMA2_Stream0 interrupt */
                NVIC_EnableIRQ(DMA2_Stream7_IRQn);

                /* Enable DMA2 */
                DMA2_Stream7->CR |= DMA_SxCR_EN;

                /* request to write has been acknowledged, clear this bit */
                flash_state &= ~FLASH_CMD_WRITE_REQUEST;
            }
            /* Erase is done, clear that bit */
            flash_state &= ~FLASH_CMD_ERASE_IN_PROGRESS;
        }
    }
}

void __attribute__((optimize("O0"))) DMA2_Stream7_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(DMA2_Stream7_IRQn);
    if (DMA2->HISR & DMA_HISR_TCIF7) {
        /* Clear interrupt */
        DMA2->HIFCR |= DMA_HIFCR_CTCIF7;
        /* Disable DMA interrupt */
        NVIC_DisableIRQ(DMA2_Stream7_IRQn);
        if (flash_state & FLASH_CMD_WRITE_IN_PROGRESS) {
            /* wait for flash to be free */
            while (FLASH->SR & FLASH_SR_BSY);
            /* Reset program bit */
            FLASH->CR &= ~FLASH_CR_PG;
            /* lock that flash */
            FLASH->CR |= FLASH_CR_LOCK;
            /* Reset flash writing flag */
            flash_state &= ~FLASH_CMD_WRITE_IN_PROGRESS;
        }
    }
}
