#include "exceptions.h"
#include "nvic.h"
#include "schedule.h"
#include "systick.h"
#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>

void blink_led(struct gpio_out *led, int is_turn_on, int on_duration, int off_duration)
{
    if (is_turn_on) {
        led->set(led);
        schedule_task(on_duration, (task_handler) blink_led, (uint32_t) led, 0, on_duration, off_duration);
    }
    else {
        led->clear(led);
        schedule_task(off_duration, (task_handler) blink_led, (uint32_t) led, 1, on_duration, off_duration);
    }
}

void poll_button(struct gpio_in *button, struct gpio_out *led)
{
    if (button->get(button))
        led->set(led);
    else
        led->clear(led);
    schedule_task(100 * ms, (task_handler) poll_button, (uint32_t) button, (uint32_t) led, 0, 0);
}

int main(int argc, char const *argv[])
{
    union gpio green;
    union gpio red;
    union gpio button;

    construct_schedule();
    start_exceptions();
    start_nvic();
    start_systick();
    start_gpio();

    configure_gpio_output(&green, GPIO_G, 13);
    configure_gpio_output(&red, GPIO_G, 14);
    configure_gpio_input(&button, GPIO_A, 0);

    schedule_task(now, (task_handler) blink_led, (uint32_t) &green.out, 1, 50 * ms, 1950 * ms);
    schedule_task(now, (task_handler) poll_button, (uint32_t) &button.in, (uint32_t) &red.out, 0, 0);
    /* must be last */
    start_schedule();

    return 0;
}
