#include <stdint.h>

#include "gpio.h"
#include "rcc.h"

/* register mapping */
typedef struct
{
    /*Type      Name                                Description                         Offset */
    uint32_t    moder;                              /* port mode                        0x000 */
    uint32_t    otyper;                             /* port output type                 0x004 */
    uint32_t    ospeedr;                            /* port output speed                0x008 */
    uint32_t    pupdr;                              /* port pull-up/pull-down           0x00c */
    uint32_t    idr;                                /* port input data                  0x010 */
    uint32_t    odr;                                /* port output data                 0x014 */
    uint32_t    bsrr;                               /* port bit set/reset               0x018 */
    uint32_t    lckr;                               /* port configuration lock          0x01c */
    uint32_t    afrl;                               /* alternate function low           0x020 */
    uint32_t    afrh;                               /* alternate function high          0x024 */
} t_gpio_bank_register;

#if defined(__STM32F4DISCO__)
static t_gpio_bank_register volatile *pGpioBankReg[] = {(t_gpio_bank_register *) 0x40020000, (t_gpio_bank_register *) 0x40020400,
                                                        (t_gpio_bank_register *) 0x40020800, (t_gpio_bank_register *) 0x40020C00,
                                                        (t_gpio_bank_register *) 0x40021000, (t_gpio_bank_register *) 0x40021400,
                                                        (t_gpio_bank_register *) 0x40021800, (t_gpio_bank_register *) 0x40021C00,
                                                        (t_gpio_bank_register *) 0x40022000, (t_gpio_bank_register *) 0x40022400,
                                                        (t_gpio_bank_register *) 0x40022800};
#elif  defined(__STM32F0DISCO__)
static t_gpio_bank_register volatile *pGpioBankReg[] = {(t_gpio_bank_register *) 0x48000000, (t_gpio_bank_register *) 0x48000400,
                                                        (t_gpio_bank_register *) 0x48000800, (t_gpio_bank_register *) 0x48000C00,
                                                        (t_gpio_bank_register *) 0x48001000, (t_gpio_bank_register *) 0x48001400};
#elif  defined(__STM32DEV__) || defined(__HCLOCK__)
static t_gpio_bank_register volatile *pGpioBankReg[] = {(t_gpio_bank_register *) 0x48000000, (t_gpio_bank_register *) 0x48000400};
#else
#error "Unknown stm32 board"
#endif

static void out_set(struct gpio_out *gpio) {
    pGpioBankReg[gpio->bank]->bsrr = 1 << gpio->pin_nb;
}

static void out_clear(struct gpio_out *gpio) {
    pGpioBankReg[gpio->bank]->bsrr = 1 << (gpio->pin_nb + 16);
}

static int out_get(struct gpio_out *gpio) {
    return (pGpioBankReg[gpio->bank]->idr >> gpio->pin_nb) & 1;
}

static int in_get(struct gpio_in *gpio) {
    return (pGpioBankReg[gpio->bank]->idr >> gpio->pin_nb) & 1;
}

/* public api */
void construct_gpio()
{
    ;
}

void start_gpio()
{
#if defined(__STM32F4DISCO__)
    periph_clock_enable(GPIOA);
    periph_clock_enable(GPIOB);
    periph_clock_enable(GPIOC);
    periph_clock_enable(GPIOD);
    periph_clock_enable(GPIOE);
    periph_clock_enable(GPIOF);
    periph_clock_enable(GPIOG);
    periph_clock_enable(GPIOH);
    periph_clock_enable(GPIOI);
    periph_clock_enable(GPIOJ);
    periph_clock_enable(GPIOK);
#elif  defined(__STM32F0DISCO__)
    periph_clock_enable(GPIOA);
    periph_clock_enable(GPIOB);
    periph_clock_enable(GPIOC);
    periph_clock_enable(GPIOD);
    periph_clock_enable(GPIOE);
    periph_clock_enable(GPIOF);
#elif  defined(__STM32DEV__) || defined(__HCLOCK__)
    periph_clock_enable(GPIOA);
    periph_clock_enable(GPIOB);
#else
#error "Unknown stm32 board"
#endif
}

void configure_gpio_output(union gpio *gpio, enum gpio_bank bank, int pin_nb, enum gpio_output_type type)
{
    /* set output */
    pGpioBankReg[bank]->moder &= ~(3 << (pin_nb * 2));
    pGpioBankReg[bank]->moder |= 1 << (pin_nb * 2);

    /* output type */
    if (type)
        pGpioBankReg[bank]->otyper |= (1 << pin_nb);
    else
        pGpioBankReg[bank]->otyper &= ~(1 << pin_nb);

    /* no pull-up/pull-down */
    pGpioBankReg[bank]->pupdr &= ~(3 << (pin_nb * 2));

    /* set to low level */
    pGpioBankReg[bank]->odr &= ~(1 << pin_nb);

    /* set method */
    gpio->out.set = out_set;
    gpio->out.clear = out_clear;
    gpio->out.get = out_get;
    gpio->out.bank = bank;
    gpio->out.pin_nb = pin_nb;
}

void configure_gpio_input(union gpio *gpio, enum gpio_bank bank, int pin_nb, enum gpio_input_pull_type type)
{
    /* set input */
    pGpioBankReg[bank]->moder &= ~(3 << (pin_nb * 2));

    /* pull-up / pull-down configuration */
    pGpioBankReg[bank]->pupdr &= ~(3 << (pin_nb * 2));
    if (type)
        pGpioBankReg[bank]->pupdr |= (type << (pin_nb * 2));

    /* set method */
    gpio->in.get = in_get;
    gpio->in.bank = bank;
    gpio->in.pin_nb = pin_nb;
}

void configure_gpio_alternate(enum gpio_bank bank, int pin_nb, enum gpio_output_type out_type, enum gpio_input_pull_type in_type, enum gpio_speed speed, enum gpio_alternate alt)
{
    /* set alternate function mode */
    if (pin_nb >= 8) {
        pGpioBankReg[bank]->afrh &= ~(0xf << ((pin_nb - 8) * 4));
        pGpioBankReg[bank]->afrh |= alt << ((pin_nb - 8) * 4);
    } else {
        pGpioBankReg[bank]->afrl &= ~(0xf << (pin_nb * 4));
        pGpioBankReg[bank]->afrl |= alt << (pin_nb * 4);
    }
    /* out driver type */
    if (out_type)
        pGpioBankReg[bank]->otyper |= (1 << pin_nb);
    else
        pGpioBankReg[bank]->otyper &= ~(1 << pin_nb);
    /* and speed */
    pGpioBankReg[bank]->ospeedr &= ~(3 << (pin_nb * 2));
    pGpioBankReg[bank]->ospeedr |= (speed << (pin_nb * 2));

    /* input pull-up/down */
    pGpioBankReg[bank]->pupdr &= ~(3 << (pin_nb * 2));
    if (in_type)
        pGpioBankReg[bank]->pupdr |= (in_type << (pin_nb * 2));

    /* set pin in alternate mode */
    pGpioBankReg[bank]->moder &= ~(3 << (pin_nb * 2));
    pGpioBankReg[bank]->moder |= 2 << (pin_nb * 2);
}
