#include <stdint.h>
#include <stddef.h>

#include "rcc.h"

uint32_t SystemCoreClock = 8000000;

#if defined(__STM32F4DISCO__)
typedef struct
{
    /*Type      Name                            Description                             Offset */
    uint32_t    cr;                                 /* clock control                    0x000 */
    uint32_t    pllcfgr;                            /* pll configuration                0x004 */
    uint32_t    cfgr;                               /* clock configuration              0x008 */
    uint32_t    cir;                                /* clock interrupt                  0x00c */
    uint32_t    ahb1rstr;                           /* ahb1 peripheral reset            0x010 */
    uint32_t    ahb2rstr;                           /* ahb2 peripheral reset            0x014 */
    uint32_t    ahb3rstr;                           /* ahb3 peripheral reset            0x018 */
    uint32_t    reserved_0;                         /*                                  0x01c */
    uint32_t    apb1rstr;                           /* apb1 peripheral reset            0x020 */
    uint32_t    apb2rstr;                           /* apb2 peripheral reset            0x024 */
    uint32_t    reserved_1[(0x030 - 0x028) >> 2];   /*                                  0x028 */
    uint32_t    ahb1enr;                            /* ahb1 peripheral clock            0x030 */
    uint32_t    ahb2enr;                            /* ahb2 peripheral clock            0x034 */
    uint32_t    ahb3enr;                            /* ahb3 peripheral clock            0x038 */
    uint32_t    reserved_2;                         /*                                  0x03c */
    uint32_t    apb1enr;                            /* apb1 peripheral clock            0x040 */
    uint32_t    apb2enr;                            /* apb2 peripheral clock            0x044 */
    uint32_t    reserved_3[(0x050 - 0x048) >> 2];   /*                                  0x048 */
    uint32_t    ahb1lpenr;                          /* ahb1 peripheral clock in low power mode  0x050 */
    uint32_t    ahb2lpenr;                          /* ahb2 peripheral clock in low power mode  0x054 */
    uint32_t    ahb3lpenr;                          /* ahb3 peripheral clock in low power mode  0x058 */
    uint32_t    reserved_4;                         /*                                  0x05c */
    uint32_t    apb1lpenr;                          /* apb1 peripheral clock in low power mode  0x060 */
    uint32_t    apb2lpenr;                          /* apb2 peripheral clock in low power mode  0x064 */
    uint32_t    reserved_5[(0x074 - 0x068) >> 2];   /* backup domain control            0x070-0x068 */
    uint32_t    csr;                                /* clock control and status         0x074 */
    uint32_t    reserved_6[(0x080 - 0x078) >> 2];   /*                                  0x078 */
    uint32_t    sscgr;                              /* spread spectrum clock generation 0x080 */
    uint32_t    plli2scfgr;                         /* plli2s configuration             0x084 */
    uint32_t    pllsaicfgr;                         /* pll configuration                0x088 */
    uint32_t    dkcfgr1;                            /* dedicated clocks configuration   0x08c */
    uint32_t    dckcfgr2;                           /* dedicated clocks configuration   0x090 */
} t_rcc_register;
#elif  defined(__STM32F0DISCO__) || defined(__STM32DEV__) || defined(__HCLOCK__)
typedef struct
{
    /*Type      Name                            Description                             Offset */
    uint32_t    cr;                                 /* clock control                    0x000 */
    uint32_t    cfgr;                               /* clock configuration              0x004 */
    uint32_t    cir;                                /* clock interrupt                  0x008 */
    uint32_t    apb2rstr;                           /* apb2 peripheral reset            0x00c */
    uint32_t    apb1rstr;                           /* apb1 peripheral reset            0x010 */
    uint32_t    ahbenr;                             /* ahb peripheral clock             0x014 */
    uint32_t    apb2enr;                            /* apb2 peripheral clock            0x018 */
    uint32_t    apb1enr;                            /* apb1 peripheral clock            0x01c */
    uint32_t    bdcr;                               /* domain control                   0x020 */
    uint32_t    csr;                                /* clock control and status         0x024 */
    uint32_t    ahbrstr;                            /* ahb2 peripheral reset            0x028 */
    uint32_t    cfgr2;                              /* clock configuration 2            0x02c */
    uint32_t    cfgr3;                              /* clock configuration 3            0x030 */
    uint32_t    cr2;                                /* clock control 2                  0x034 */
} t_rcc_register;
#else
#error "Unknown stm32 board"
#endif

#if defined(__STM32F4DISCO__)
    #define RCC_BASE_ADDRESS                                        0x40023800
#elif  defined(__STM32F0DISCO__) || defined(__STM32DEV__) || defined(__HCLOCK__)
    #define RCC_BASE_ADDRESS                                        0x40021000
#else
#error "Unknown stm32 board"
#endif

#if defined(__STM32F4DISCO__)
static t_rcc_register volatile *pRccReg = (t_rcc_register *) RCC_BASE_ADDRESS;
#elif defined(__STM32F0DISCO__) || defined(__STM32DEV__) || defined(__HCLOCK__)
static t_rcc_register volatile *pRccReg = (t_rcc_register *) RCC_BASE_ADDRESS;
#endif

#if defined(__STM32F4DISCO__)
static uint32_t perih_to_clock_info[][2] = {{RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahb1enr), 0}, /* GPIOA */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahb1enr), 1}, /* GPIOB */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahb1enr), 2}, /* GPIOC */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahb1enr), 3}, /* GPIOD */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahb1enr), 4}, /* GPIOE */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahb1enr), 5}, /* GPIOF */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahb1enr), 6}, /* GPIOG */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahb1enr), 7}, /* GPIOH */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahb1enr), 8}, /* GPIOI */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahb1enr), 9}, /* GPIOJ */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahb1enr), 10}, /* GPIOK */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahb3enr), 0}, /* FMC */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, apb2enr), 26}, /* LTDC */
                                            {0,0}, /* SYSCFG */
                                            {0,0}, /* TIM14 */
};
#elif defined(__STM32F0DISCO__)
static uint32_t perih_to_clock_info[][2] = {{RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahbenr), 17}, /* GPIOA */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahbenr), 18}, /* GPIOB */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahbenr), 19}, /* GPIOC */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahbenr), 20}, /* GPIOD */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahbenr), 21}, /* GPIOE */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahbenr), 22}, /* GPIOF */
                                            {0, 0}, /* GPIOG */
                                            {0, 0}, /* GPIOH */
                                            {0, 0}, /* GPIOI */
                                            {0, 0}, /* GPIOJ */
                                            {0, 0}, /* GPIOK */
                                            {0, 0}, /* FMC */
                                            {0, 0}, /* LDTC */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, apb2enr), 0}, /* SYSCFG */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, apb1enr), 8}, /* TIM14 */
};
#elif defined(__STM32DEV__)  || defined(__HCLOCK__)
static uint32_t perih_to_clock_info[][2] = {{RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahbenr), 17}, /* GPIOA */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, ahbenr), 18}, /* GPIOB */
                                            {0, 0}, /* GPIOC */
                                            {0, 0}, /* GPIOD */
                                            {0, 0}, /* GPIOE */
                                            {0, 0}, /* GPIOF */
                                            {0, 0}, /* GPIOG */
                                            {0, 0}, /* GPIOH */
                                            {0, 0}, /* GPIOI */
                                            {0, 0}, /* GPIOJ */
                                            {0, 0}, /* GPIOK */
                                            {0, 0}, /* FMC */
                                            {0, 0}, /* LDTC */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, apb2enr), 0}, /* SYSCFG */
                                            {RCC_BASE_ADDRESS + offsetof(t_rcc_register, apb1enr), 8}, /* TIM14 */
};
#else
#error "Unknown stm32 board"
#endif

/* api */
void construct_rcc()
{
    ;
}

void start_rcc()
{
    ;
}

void periph_clock_enable(enum peripheral periph)
{
    volatile uint32_t *reg = (volatile uint32_t *) perih_to_clock_info[periph][0];
    uint32_t bit_nb = perih_to_clock_info[periph][1];

    *reg |= (1 << bit_nb);
}

void periph_clock_disable(enum peripheral periph)
{
    volatile uint32_t *reg = (volatile uint32_t *) perih_to_clock_info[periph][0];
    uint32_t bit_nb = perih_to_clock_info[periph][1];

    *reg &= ~(1 << bit_nb);
}

/* FIXME : clock api need some work .... */
void switch_on_hse()
{
#if defined(__STM32DEV__) || defined(__HCLOCK__)
    /* turn on hse */
    pRccReg->cr |=  1 << 16;
    while((pRccReg->cr & (1 << 17)) == 0);
    /* now use it */
    pRccReg->cfgr &= ~3;
    pRccReg->cfgr |= 1;
    while((pRccReg->cfgr & (3 << 2)) == 1);
#else
    #error "Unknown stm32 board"
#endif
}

#if defined(__STM32F4DISCO__)
void setup_lcd_tft_clock()
{
    pRccReg->dkcfgr1 |= (1 << 16); /* 24 / 4 => 6Mhz for pixel clock */
    pRccReg->pllsaicfgr =   (168 << 6) | /* vco clock is 168Mhz */
                            (7 << 28) | /* r is 7 => 24 Mhz */
                            (2 << 24) | /* q is 2 => 86 Mhz */
                            (1 << 16);/* p is 4 => 43 Mhz */
    pRccReg->cr |=  (1 << 28); /* pllsai on */
    while ((pRccReg->cr & (1 << 29)) == 0) ;
}
#endif
