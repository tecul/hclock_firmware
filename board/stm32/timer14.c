#include <stdint.h>

#include "timer.h"
#include "rcc.h"

/* register mapping */
typedef struct
{
    /*Type      Name                                Description                         Offset */
    uint32_t    cr1;                                /* control 1                        0x000 */
    uint32_t    reserved_1[(0x00c - 0x004) >> 2];                       
    uint32_t    dier;                               /* interrupt enable                 0x00c */
    uint32_t    sr;                                 /* status                           0x010 */
    uint32_t    egr;                                /* event generation                 0x014 */
    uint32_t    ccmr1;                              /* capture/compare mode 1           0x018 */
    uint32_t    reserved_2[(0x020 - 0x01c) >> 2];
    uint32_t    ccer;                               /* capture/compare enable           0x020 */
    uint32_t    cnt;                                /* counter                          0x024 */
    uint32_t    psc;                                /* prescaler                        0x028 */
    uint32_t    arr;                                /* auto-reload                      0x02c */
    uint32_t    reserved_3[(0x034 - 0x030) >> 2];
    uint32_t    ccr1;                               /* capture/compare 1                0x034 */
    uint32_t    reserved_4[(0x050 - 0x038) >> 2];
    uint32_t    or_;                                 /* option                           0x050 */
} t_timer14_register;

#define TIM14_BASE_ADDRESS                                              0x40002000
static t_timer14_register volatile *pTim14Reg = (t_timer14_register *) TIM14_BASE_ADDRESS;

/* public api */
void construct_tim14()
{
    ;
}

void start_tim14()
{
#if defined(__STM32F4DISCO__)
    ;
#elif  defined(__STM32F0DISCO__) || defined(__STM32DEV__) || defined(__HCLOCK__)
    periph_clock_enable(TIM14);
#else
#error "Unknown stm32 board"
#endif
}

/* fixme : rework this */
struct timer_mode save_mode;

void tim14_enable()
{
    configure_gpio_alternate(save_mode.u.frequency_generator.bank,
                                         save_mode.u.frequency_generator.pin_nb,
                                         GPIO_PUSH_PULL,
                                         GPIO_NO_PULL,
                                         GPIO_MEDIUM_SPEED,
                                         save_mode.u.frequency_generator.alt);
    pTim14Reg->cr1 |= 1;
}

void tim14_disable()
{
    union gpio dummy;
    configure_gpio_input(&dummy, save_mode.u.frequency_generator.bank, save_mode.u.frequency_generator.pin_nb, GPIO_NO_PULL);
    pTim14Reg->cr1 &= ~1;
}

void tim14_set_mode(struct timer_mode *mode)
{
    save_mode = *mode;
    switch(mode->type) {
        case TIMER_FREQUENCY_GENERATOR:
            {
                pTim14Reg->ccmr1 = 0 << 0 | /* output mode */
                                   3 << 4; /* toggle mode */
                pTim14Reg->ccer = 1; /*output is enable */
                pTim14Reg->psc = 7;
                pTim14Reg->arr = 999;
                pTim14Reg->ccr1 = 0;
            }
            break;
        default:
            break;
    }
}

void tim14_set_freq(int freq)
{
    pTim14Reg->arr = 1000000 / freq -1;
}
