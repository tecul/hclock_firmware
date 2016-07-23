#include "exceptions.h"
#include "nvic.h"
#include "systick.h"
#include "gpio.h"
#include "rcc.h"
//#include "ili9341_gpio.h"
#include "ili9341_lcd.h"
#include "stmpe811.h"
#include "app.h"
#include "scheduler.h"

int main(int argc, char const *argv[])
{
    construct_exceptions();
    construct_nvic();
    construct_systick();
    construct_scheduler();
    construct_app();
    construct_gpio();
    construct_rcc();
    //construct_ili9341_gpio();
    construct_ili9341_lcd();
    construct_stmpe811();

    start_exceptions();
    start_nvic();
    start_systick();
    start_rcc();
    start_gpio();
    //start_ili9341_gpio();
    start_ili9341_lcd();
    start_stmpe811();

    start_app();
    /* must be last */
    start_scheduler();
}
