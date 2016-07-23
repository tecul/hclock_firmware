#ifndef __INTERRUPT__
#define __INTERRUPT__

typedef void (*interrupt_handler)(void);

void construct_nvic(void);
void start_nvic(void);
void enable_interrupt(int interrupt_nb);
void disable_interrupt(int interrupt_nb);
void register_interrupt_handler(int interrupt_nb, interrupt_handler handler);

#endif
