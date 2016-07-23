#ifndef __TIMER__
#define __TIMER__ 1

#include "gpio.h"

void construct_tim14(void);
void start_tim14(void);

enum timer_type {
	TIMER_FREQUENCY_GENERATOR,
};

struct timer_mode {
	enum timer_type type;
	union {
		struct {
			enum gpio_bank bank;
			int pin_nb;
			enum gpio_alternate alt;
		} frequency_generator;
	} u;
};

void tim14_enable(void);
void tim14_disable(void);
void tim14_set_mode(struct timer_mode *mode);
void tim14_set_freq(int freq);

#endif