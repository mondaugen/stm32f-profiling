#include "flash_commanding.h" 
#include "stm32f4xx.h" 

//#define FLASH_DEBUG 

volatile uint32_t flash_state = 0;
int flash_commanding_count = 0;

#define TOMMYS_SIZE 4//100000//16000//8192//(492*4) 

/* Align on 1K boundary so DMA bursts work */
char tommy[TOMMYS_SIZE] __attribute__((aligned(1024)))
    = { [0 ... (TOMMYS_SIZE-1)] = 'c' };

#define GPIOG_PIN 13 

#define FLASH_DMA_BASE DMA2_Stream6 
#define FLASH_DMA_BASE_IRQHandler DMA2_Stream6_IRQHandler
#define FLASH_DMA_BASE_IRQn DMA2_Stream6_IRQn
#define DMA_HISR_TCIF() DMA_HISR_TCIF ## 6 
#define DMA_HIFCR_CTCIF() DMA_HIFCR_CTCIF ## 6 
#define DMA_HISR_TEIF() DMA_HISR_TEIF ## 6 
#define DMA_HIFCR_CTEIF() DMA_HIFCR_CTEIF ## 6 

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

    /* For debugging */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    GPIOG->MODER |= (1 << (GPIOG_PIN*2));
    int n;
    for (n = 0; n < TOMMYS_SIZE/4; n++) {
        ((int*)tommy)[n] = n;
    }
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
    if (FLASH_DMA_BASE->CR & DMA_SxCR_EN) {
        return; /* DMA is not done transferring. */
    }
    if (FLASH->SR & FLASH_SR_BSY) {
        return; /* FLASH busy */
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

#ifdef FLASH_DEBUG
void __attribute__((optimize("O0"))) FLASH_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(FLASH_IRQn);
    if (FLASH->SR & FLASH_SR_EOP) {
        /* Clear by writing one to this bit */
        FLASH->SR |= FLASH_SR_EOP;
        /* Turn off flash interrupt */
        FLASH->CR &= ~FLASH_CR_EOPIE;
        NVIC_DisableIRQ(FLASH_IRQn);
        /* Reset sector erase bit */
        FLASH->CR &= ~FLASH_CR_SER;
        /* lock that flash */
        FLASH->CR |= FLASH_CR_LOCK;
        flash_state = 0x00000000;
    }
}
#else
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
                FLASH_DMA_BASE->CR = 0x00000000;
                /* Set to channel 0, low priority, memory and peripheral datum
                 * size 32-bits, transfer complete interrupt enable, memory
                 * increment, peripheral (other memory) increment. */
                FLASH_DMA_BASE->CR |= (0 << 25)
                    | (0x0 << 16)
                    | (0x2 << 13)
                    | (0x2 << 11)
                    | (0x1 << 4)
                    | (0x2 << 6) /* memory to memory transfer */
                    | DMA_SxCR_MINC
                    | DMA_SxCR_PINC
                    | (0x0 << 23) /* no burst */
                    | (0x0 << 21) /* no burst */
                    | DMA_SxCR_TEIE; /* Transfer error interrupt enable */
                FLASH_DMA_BASE->FCR &= ~DMA_SxFCR_FTH;
                /* Set FIFO threshold to 1/4 */
                FLASH_DMA_BASE->FCR |= (0x0 << 0);
                /* Set peripheral address to data we want to write */
                FLASH_DMA_BASE->PAR = (uint32_t)tommy;
                /* Set memory address to some place */
                FLASH_DMA_BASE->M0AR = (uint32_t)FLASH_START_ADDR;
                /* Set number of items to transfer */
                FLASH_DMA_BASE->NDTR = TOMMYS_SIZE / 4; /* divided by 4 because
                                                         each datum 32 bits wide
                                                         */
    
                /* Enable DMA2_Stream0 interrupt */
                NVIC_EnableIRQ(FLASH_DMA_BASE_IRQn);

                /* Enable DMA2 */
                FLASH_DMA_BASE->CR |= DMA_SxCR_EN;

                /* request to write has been acknowledged, clear this bit */
                flash_state &= ~FLASH_CMD_WRITE_REQUEST;
            }
            /* Erase is done, clear that bit */
            flash_state &= ~FLASH_CMD_ERASE_IN_PROGRESS;
        }
    }
}
#endif /* FLASH_DEBUG */

void __attribute__((optimize("O0"))) FLASH_DMA_BASE_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(FLASH_DMA_BASE_IRQn);
    if (DMA2->HISR & DMA_HISR_TCIF()) {
        /* Clear interrupt */
        DMA2->HIFCR |= DMA_HIFCR_CTCIF();
        /* Disable DMA interrupt */
        NVIC_DisableIRQ(FLASH_DMA_BASE_IRQn);
        /* Disable DMA. */
        FLASH_DMA_BASE->CR &= ~DMA_SxCR_EN;
        if (flash_state & FLASH_CMD_WRITE_IN_PROGRESS) {
            /* Toggle GPIO */
            GPIOG->ODR ^= (1 << GPIOG_PIN);
            /* wait for flash to be free */
//            while (FLASH->SR & FLASH_SR_BSY);
            /* Reset program bit */
            FLASH->CR &= ~FLASH_CR_PG;
            /* lock that flash */
            FLASH->CR |= FLASH_CR_LOCK;
            /* Reset flash writing flag */
            flash_state &= ~FLASH_CMD_WRITE_IN_PROGRESS;
        }
    }
    if (DMA2->HISR & DMA_HISR_TEIF()) {
        /* Transfer error occurred */
        /* Clear interrupt */
        DMA2->HIFCR |= DMA_HIFCR_CTEIF();
    }
}
