#include "exceptions.h"
#include "nvic.h"
#include "systick.h"
#include "gpio.h"
#include "rcc.h"
#include "timer.h"

#include "app.h"
#include "scheduler.h"

uint32_t SystemCoreClock = 8000000;

int main(int argc, char const *argv[])
{
    construct_exceptions();
    construct_nvic();
    construct_systick();
    construct_scheduler();
    construct_app();
    construct_gpio();
    construct_rcc();
    construct_tim14();

    start_exceptions();
    start_nvic();
    start_systick();
    start_gpio();
    start_rcc();
    start_tim14();

    start_app();
    /* must be last */
    start_scheduler();
}
