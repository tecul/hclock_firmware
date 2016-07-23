#include <stdint.h>

#include "scheduler.h"
#include "display.h"

/* configuration */
#if defined(__STM32F0DISCO__)
static const int gpios_for_segments[8][2] = {
    {GPIO_B, 2},
    {GPIO_B, 3},
    {GPIO_B, 4},
    {GPIO_B, 5},
    {GPIO_B, 6},
    {GPIO_B, 7},
    {GPIO_B, 8},
    {GPIO_B, 9},
};
static const int gpios_for_digits[4][2] = {
    {GPIO_C, 10},
    {GPIO_C, 11},
    {GPIO_C, 12},
    {GPIO_D, 2},
};
#elif  defined(__STM32DEV__) || defined(__HCLOCK__)
static const int gpios_for_segments[8][2] = {
    {GPIO_A, 0},
    {GPIO_A, 1},
    {GPIO_A, 2},
    {GPIO_A, 3},
    {GPIO_A, 15},
    {GPIO_A, 5},
    {GPIO_A, 6},
    {GPIO_A, 7},
};
#if defined(__STM32DEV__)
static const int gpios_for_digits[4][2] = {
    {GPIO_A, 8},
    {GPIO_A, 9},
    {GPIO_A, 10},
    {GPIO_A, 11},
};
#else
static const int gpios_for_digits[4][2] = {
    {GPIO_A, 8},
    {GPIO_B, 6},
    {GPIO_B, 7},
    {GPIO_A, 11},
};
#endif
#else
#error "Unknown stm32 board"
#endif

/* private variables */
static const uint8_t digit_to_segment[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 
                                    0x6d, 0x7d, 0x07, 0x7f, 0x6f};
static union gpio segments[8];
static union gpio digits[4];
static struct gpio_out *segs[8];
static struct gpio_out *digs[4];
static bool is_on = false;
static uint8_t current_digit[4];
static bool is_multiplexer_running = false;
static bool current_is_led = false;
static bool current_is_double_dot = false;

/* private */
static void multiplex_digits(uint32_t idx)
{
    uint8_t segment = 0x79;
    uint8_t d = current_digit[idx];
    int i;

    /* clear all */
    for (i = 0; i < 8; ++i)
        segs[i]->set(segs[i]);

    /* turn off everything */
    for (i = 0; i < 4; ++i)
        digs[i]->set(digs[i]);

    if (d < sizeof(digit_to_segment) / sizeof(digit_to_segment[0]))
        segment = digit_to_segment[d];

    for (i = 0; i < 7; ++i)
        if ((segment >> i) & 1)
            segs[i]->clear(segs[i]);
    if (current_is_led && idx ==0)
        segs[7]->clear(segs[7]);
    if (current_is_double_dot && (idx == 1 || idx ==2))
        segs[7]->clear(segs[7]);

    if (is_on) {
        /* turn on index */
        digs[idx]->clear(digs[idx]);
        schedule_task(5 * ms, (task_handler) multiplex_digits, (idx + 1) % 4, 0, 0, 0);
    } else
        is_multiplexer_running = false;
}

/* public api */
void display_configure()
{
    int i;

    for (i = 0; i < 8; ++i) {
        configure_gpio_output(&segments[i], (enum gpio_bank) gpios_for_segments[i][0], gpios_for_segments[i][1], GPIO_OPEN_DRAIN);
        segs[i] = &segments[i].out;
        segs[i]->set(segs[i]);
    }
    for (i = 0; i < 4; ++i) {
        configure_gpio_output(&digits[i], (enum gpio_bank) gpios_for_digits[i][0], gpios_for_digits[i][1], GPIO_OPEN_DRAIN);
        digs[i] = &digits[i].out;
        digs[i]->set(digs[i]);
    }
}

void display_on()
{
    if (!is_on) {
        is_on = true;
        if (!is_multiplexer_running) {
            is_multiplexer_running = true;
            schedule_task(now, (task_handler) multiplex_digits, 0, 0, 0, 0);
        }
    }
}

void display_off()
{
    is_on = false;
}

void display_set(uint8_t digit[4], bool is_led, bool is_double_dot)
{
    int i;

    for (i = 0; i < 4; ++i)
        current_digit[i] = digit[i];
    current_is_led = is_led;
    current_is_double_dot = is_double_dot;
}
