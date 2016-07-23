#ifndef __DISPLAY__
#define __DISPLAY__ 1

#include "gpio.h"

void display_configure(void);
void display_on();
void display_off();
void display_set(uint8_t digit[4], bool is_led, bool is_double_dot);

#endif