#ifndef __SYSTICK__
#define __SYSTICK__

#include <stdint.h>

struct alarm;
typedef void (*alarm_handler)(struct alarm *);
struct alarm {
    struct alarm *next;
    alarm_handler handler;
    uint32_t date_alarm;
};

void construct_systick(void);
void start_systick(void);
uint32_t get_systick(void);
void set_alarm(struct alarm *alarm);
void clear_alarm(struct alarm *alarm);
uint64_t get_us(void);
void delay_us(int delay);

#endif
