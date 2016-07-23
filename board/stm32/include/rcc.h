#ifndef __RCC__
#define __RCC__

enum peripheral {
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE,
    GPIOF,
    GPIOG,
    GPIOH,
    GPIOI,
    GPIOJ,
    GPIOK,
    FMC,
    LTDC,
    SYSCFG,
    TIM14,
};

void construct_rcc(void);
void start_rcc(void);
void periph_clock_enable(enum peripheral periph);
void periph_clock_disable(enum peripheral periph);
void switch_on_hse(void);

#endif