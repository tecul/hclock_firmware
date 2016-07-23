#include "scheduler.h"
#include "gpio.h"

#define GPIO_NB		23

static int index_to_mapping[GPIO_NB][2] = {
	{GPIO_A, 0},{GPIO_A, 1},{GPIO_A, 2},{GPIO_A, 3},
	{GPIO_A, 4},{GPIO_A, 5},{GPIO_A, 6},{GPIO_A, 7},
	{GPIO_A, 8},{GPIO_A, 9},{GPIO_A, 10},{GPIO_A, 11},
	{GPIO_A, 12},{GPIO_A, 13},{GPIO_A, 14},{GPIO_A, 15},
	{GPIO_B, 0},{GPIO_B, 1},{GPIO_B, 3},
	{GPIO_B, 4},{GPIO_B, 5},{GPIO_B, 6},{GPIO_B, 7},
};
static union gpio gpio_to_toggle[GPIO_NB];

void toggle_gpio(int index)
{
	struct gpio_out *current = &gpio_to_toggle[index].out;
	struct gpio_out *next = &gpio_to_toggle[(index + 1) % GPIO_NB].out;

	current->clear(current);
	next->set(next);
	schedule_task(1 * ms, (task_handler) toggle_gpio, (index + 1) % GPIO_NB, 0, 0, 0);
}

void construct_test()
{
    ;
}

void start_test()
{
	int i;

	for (i = 0; i < GPIO_NB; ++i)
		configure_gpio_output(&gpio_to_toggle[i], (enum gpio_bank) index_to_mapping[i][0], index_to_mapping[i][1], GPIO_PUSH_PULL);

	schedule_task(now, (task_handler) toggle_gpio, 0, 0, 0, 0);
}
