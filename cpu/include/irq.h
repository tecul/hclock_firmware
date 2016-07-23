#ifndef __IRQ__
#define __IRQ__

int disable_irq(void);
void enable_irq(void);
void restore_irq(int state);

#endif
