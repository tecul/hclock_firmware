#include "button.h"
#include "gpio.h"

/* configuration */
#if defined(__STM32F0DISCO__)
static const int gpios_for_buttons[8][2] = {
    {GPIO_A, 1},
    {GPIO_A, 5},
    {GPIO_A, 8},
    {GPIO_A, 9},
    {GPIO_A, 10},
    {GPIO_A, 15},
};
#elif  defined(__STM32DEV__) || defined(__HCLOCK__)
static const int gpios_for_buttons[8][2] = {
    {GPIO_B, 0},
    {GPIO_B, 1},
    {GPIO_B, 3},
    {GPIO_B, 4},
    {GPIO_B, 5},
    {GPIO_A, 12},
};
#else
#error "Unknown stm32 board"
#endif

static union gpio buttons[6];

void button_configure()
{
	int i;

	for (i = 0; i < 6; ++i)
		configure_gpio_input(&buttons[i], (enum gpio_bank) gpios_for_buttons[i][0], gpios_for_buttons[i][1], GPIO_PULL_UP);
}

bool is_pressed(enum button idx)
{ 
	struct gpio_in *b = &buttons[idx].in;

	return !b->get(b);
}
