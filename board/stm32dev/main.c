#include "exceptions.h"
#include "nvic.h"
#include "systick.h"
#include "rcc.h"
#include "gpio.h"
#include "timer.h"

#include "app.h"
#include "scheduler.h"

extern void switch_on_hse(void);

int main(int argc, char const *argv[])
{
    switch_on_hse();
    construct_exceptions();
    construct_nvic();
    construct_systick();
    construct_scheduler();
    construct_app();
    construct_rcc();
    construct_gpio();
    construct_tim14();

    start_exceptions();
    start_nvic();
    start_systick();
    start_rcc();
    start_gpio();
    start_tim14();

    start_app();
    /* must be last */
    start_scheduler();
}
