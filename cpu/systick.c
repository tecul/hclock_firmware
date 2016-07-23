#include <stdlib.h>
#include <stdint.h>

#include "systick.h"
#include "irq.h"
#include "exceptions.h"

extern uint32_t SystemCoreClock;

typedef struct
{
    /*Type      Name                            Description                             Offset */
    uint32_t    control;                            /* control and status               0x000 */
    uint32_t    reload;                             /* reload value                     0x004 */
    uint32_t    value;                              /* downcount register value         0x008 */
    uint32_t    calibration;                        /* calibration value                0x00c */
} t_systick_register;

static t_systick_register volatile *pSystickReg = (t_systick_register *) 0xe000e010;
static uint32_t systick_nb = 0;
static struct alarm *alarm_head = NULL;

extern "C" /*static */void systick_handler()
{
    struct alarm *alarm = alarm_head;

    systick_nb++;
    while(alarm) {
        if (alarm->date_alarm <= systick_nb) {
            alarm->handler(alarm);
            alarm_head = alarm_head->next;
            alarm = alarm_head;
        } else
            break;
    }
}

static uint32_t reload_value;

/* api */
void construct_systick()
{
    ;
}

void start_systick()
{
    /* register handler */
    register_exception_handler(15, systick_handler);

    reload_value = SystemCoreClock / 1000 - 1;
    pSystickReg->reload = reload_value;
    pSystickReg->value = reload_value;
    pSystickReg->control = 1/*enable*/ | 2/*interrupt enable*/ | 4/*core clock*/;
}

uint64_t get_us() __attribute__ ((noinline));
uint32_t get_systick()
{
    return systick_nb;
}

uint64_t get_us() __attribute__ ((noinline));
uint64_t get_us()
{
    uint32_t systick_nb_before;
    uint32_t value;
    uint32_t systick_nb_after;
    uint64_t cycles;

    do {
        systick_nb_before = get_systick();
        value = pSystickReg->value;
        systick_nb_after = get_systick();
    } while (systick_nb_before != systick_nb_after);
    cycles = systick_nb_before * (reload_value + 1) + (reload_value - value);

    return cycles / (SystemCoreClock / 1000000);
}

void delay_us(int delay)
{
    uint64_t start_time = get_us();

    while ((get_us() - start_time) < (uint64_t) delay) ;
}

/* alarm api */
void set_alarm(struct alarm *alarm)
{
    int irq_state;
    struct alarm *current;
    struct alarm *prev;

    prev = NULL;
    irq_state = disable_irq();
    current = alarm_head;
    while(current) {
        if (current->date_alarm > alarm->date_alarm)
            break;
        prev = current;
        current = current->next;
    }
    alarm->next = current;
    if (prev)
        prev->next = alarm;
    else
        alarm_head = alarm;
    restore_irq(irq_state);
}

void clear_alarm(struct alarm *alarm)
{
    int irq_state;
    struct alarm *current;
    struct alarm *prev;

    prev = NULL;
    irq_state = disable_irq();
    current = alarm_head;
    while(current) {
        if (current == alarm)
            break;
        prev = current;
        current = current->next;
    }
    if (current) {
        if (prev)
            prev->next = current->next;
        else
            alarm_head = current->next;
    }
    restore_irq(irq_state);
}
