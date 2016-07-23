#include "time.h"
#include "scheduler.h"
#include "systick.h"

static struct time_ current;
static struct {
	uint32_t last_systick;
	uint32_t systick_remainder;
	bool is_on;
} state;

static void update_time()
{
	uint32_t systick = get_systick();
	uint32_t subsecond_increment = 0;

	if (systick >= state.last_systick)
		state.systick_remainder += systick - state.last_systick;
	else
		state.systick_remainder += systick + (0xffffffff - state.last_systick) + 1;
	while (state.systick_remainder >= 100) {
		state.systick_remainder -= 100;
		subsecond_increment++;
	}
	state.last_systick = systick;

	current.subseconds += subsecond_increment;
	while (current.subseconds >= 10) {
		current.subseconds -= 10;
		current.seconds++;
	}
	while (current.seconds >= 60) {
		current.seconds -= 60;
		current.minutes++;
	}
	while (current.minutes >= 60) {
		current.minutes -= 60;
		current.hours++;
	}
	while (current.hours >= 24)
		current.hours -= 24;

	if (state.is_on)
		schedule_task(100 * ms, (task_handler) update_time, 0, 0, 0, 0);
}

/* public  api */
struct time_ get_time()
{
	return current;
}

void set_time(struct time_ t)
{
	current = t;
}

void start_time()
{
	if (!state.is_on) {
		state.last_systick = get_systick();
		state.is_on = true;
		schedule_task(now, (task_handler) update_time, 0, 0, 0, 0);
	}
}

void stop_time()
{
	state.is_on = false;
}
