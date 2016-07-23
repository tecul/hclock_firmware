#include "irq.h"

int disable_irq()
{
    int res;

    asm volatile("mrs %0, primask\n\t"
                 "mov r0, #1\n\t"
                 "and %0, %0, r0\n\t"
                 "cpsid i\n\t"
                : "=r" (res)
                :
                : "r0");

    return res;
}

void enable_irq()
{
    asm volatile("cpsie i");
}

void restore_irq(int state)
{
    if (!state)
        enable_irq();
}
