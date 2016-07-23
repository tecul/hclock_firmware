#include <stdint.h>

#include "exceptions.h"
#include "irq.h"
#include "nvic.h"

/* register mapping */
typedef struct
{
    /*Type      Name                            Description                             Offset */
    uint32_t    reserved_1;
    uint32_t    type;                               /* IRQ type                         0x004 */
    uint32_t    reserved_2[(0x100 - 0x08) >> 2];
    uint32_t    enableSet[16];                      /* enable irq                       0x100-0x13c */
    uint32_t    reserved_3[(0x180 - 0x140) >> 2];
    uint32_t    enableClear[16];                    /* disable irq                      0x180-0x1bc */
    uint32_t    reserved_4[(0x200 - 0x1c0) >> 2];
    uint32_t    pendingSet[16];                     /* set pending state                0x200-0x23c */
    uint32_t    reserved_5[(0x280 - 0x240) >> 2];
    uint32_t    pendingClear[16];                   /* clear pending state              0x280-0x2bc */
    uint32_t    reserved_6[(0x300 - 0x2c0) >> 2];
    uint32_t    status[16];                         /* interrupt status                 0x300-0x33c */
    uint32_t    reserved_7[(0x400 - 0x340) >> 2];
    uint32_t    priority[124];                      /* priority level                    0x400-0x7ec */
} t_nvic_register;

static t_nvic_register volatile *pNvicReg = (t_nvic_register *) 0xe000e000;

/* api */
void construct_nvic()
{
    int i;

    for(i = 0 ; i < ((pNvicReg->type & 0xf) + 1); i++) {
        pNvicReg->enableClear[i] = ~0;
        pNvicReg->pendingClear[i] = ~0;
    }
    enable_irq();
}

void start_nvic()
{
    ;
}

void enable_interrupt(int interrupt_nb)
{
    pNvicReg->enableSet[interrupt_nb / 32] = (1 << (interrupt_nb % 32));
}

void disable_interrupt(int interrupt_nb)
{
    pNvicReg->enableClear[interrupt_nb / 32] = (1 << (interrupt_nb % 32));
}

void register_interrupt_handler(int interrupt_nb, interrupt_handler handler)
{
    register_exception_handler(interrupt_nb + 16, (exception_handler) handler);
}