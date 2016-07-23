#include "scheduler.h"
#include "display.h"
#include "time.h"
#include "alarm.h"
#include "button.h"
#include "hclock.h"

#define POLLING_PERIOD		100 * ms

bool is_alarm_allow = false;

/* forward decl */
static void power_on_state(int index);
static void display_time_state(void);
static void set_time_state(void);
static void set_alarm_state(void);

static void display_time(struct time_ t)
{
	uint8_t digit[4];

	digit[0] = t.hours / 10;
	digit[1] = t.hours % 10;
	digit[2] = t.minutes / 10;
	digit[3] = t.minutes % 10;
	display_set(digit, is_alarm_on(), true);
	display_on();
}

static struct time_ add_one_hour(struct time_ t)
{
	if (++t.hours == 24)
		t.hours = 0;
	return t;
}

static struct time_ add_one_minute(struct time_ t)
{
	if (++t.minutes == 60)
		t.minutes = 0;
	return t;
}

/* this is the power on state */
/* we blink zero hour time */
static void power_on_state(int index)
{
	/* display blinking time */
	if ((index / 5) % 2)
		display_off();
	else
		display_time(get_time());

	/* detect when we go in set time mode */
	if (is_pressed(B_SET_TIME)) {
		display_on();
		schedule_task(POLLING_PERIOD, (task_handler) set_time_state, 0, 0, 0, 0);
	} else
		schedule_task(POLLING_PERIOD, (task_handler) power_on_state, index + 1, 0, 0, 0);
}

static void display_time_state()
{
	is_alarm_allow = true;
	display_time(get_time());

	if (is_pressed(B_SET_TIME)) {
		stop_time();
		schedule_task(POLLING_PERIOD, (task_handler) set_time_state, 0, 0, 0, 0);
	}
	else if (is_pressed(B_SET_ALARM)) 
		schedule_task(POLLING_PERIOD, (task_handler) set_alarm_state, 0, 0, 0, 0);
	else
		schedule_task(POLLING_PERIOD, (task_handler) display_time_state, 0, 0, 0, 0);
}

static void set_time_state()
{
	is_alarm_allow = false;
	display_time(get_time());

	if (is_pressed(B_SET_TIME)) {
		if (is_pressed(B_HOUR)) {
			set_time(add_one_hour(get_time()));
		} else if (is_pressed(B_MINUTE)) {
			set_time(add_one_minute(get_time()));
		}
		schedule_task(POLLING_PERIOD, (task_handler) set_time_state, 0, 0, 0, 0);
	} else {
		start_time();
		schedule_task(POLLING_PERIOD, (task_handler) display_time_state, 0, 0, 0, 0);
	}
}

static void set_alarm_state()
{
	is_alarm_allow = false;
	display_time(get_alarm());

	if (is_pressed(B_SET_ALARM)) {
		if (is_pressed(B_HOUR)) {
			set_alarm(add_one_hour(get_alarm()));
		} else if (is_pressed(B_MINUTE)) {
			set_alarm(add_one_minute(get_alarm()));
		}
		schedule_task(POLLING_PERIOD, (task_handler) set_alarm_state, 0, 0, 0, 0);
	} else
		schedule_task(POLLING_PERIOD, (task_handler) display_time_state, 0, 0, 0, 0);
}

/* public */
void hclock_start()
{
	struct time_ t = {0, 0, 0, 0};

	button_configure();
	display_configure();
	display_on();
	stop_time();
	set_time(t);
	start_alarm();

	schedule_task(now, (task_handler) power_on_state, 0, 0, 0, 0);
}

bool is_alarm_on()
{
	return is_alarm_allow && is_pressed(B_ALARM_ACTIVE);
}
