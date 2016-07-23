#include <stdint.h>

#include "platform.h"
#include "exceptions.h"
#include "utils.h"
#include "rcc.h"

extern void *__boot_vector_start__;
exception_handler handlers[16 + IRQ_NB] __attribute__ ((section (".runtime_vectors")));

void loop_forever()
{
    while(1) ;
}

/* api */
void construct_exceptions()
{
    exception_handler *boot_handlers = (exception_handler *) &__boot_vector_start__;
    int i;

    handlers[0] = *boot_handlers++;
    handlers[1] = *boot_handlers++;
    for (i = 2; i < ARRAY_NB(handlers); ++i)
        handlers[i] = loop_forever;

#if defined(__STM32F4DISCO__)
    /* switch exceptions vector (vtor) */
    *((volatile uint32_t *)0xE000ED08) = (uint32_t) handlers;
#elif defined(__STM32F0DISCO__) || defined(__STM32DEV__) || defined(__HCLOCK__)
    /* switch exceptions vector (syscfg) */
    /* remap sram on 0 */
    periph_clock_enable(SYSCFG);
    *((volatile uint32_t *)0x40010000) |= 3;
#else
#error "Unknown stm32 board"
#endif

}

void start_exceptions()
{
    ;
}

exception_handler register_exception_handler(int exception_nb, exception_handler handler)
{
    exception_handler old_handler = handlers[exception_nb];

    handlers[exception_nb] = handler;

    return old_handler;
}
