#include "alarm.h"
#include "scheduler.h"
#include "time.h"
#include "hclock.h"
#include "music.h"
#include "button.h"

#define POLLING_ALARM_PERIOD		1000 * ms
#define POLLING_BUTTON_PERIOD		100 * ms

static struct time_ alarm;
bool is_active = false;

/* forward decl */
static void check_alarm(void);
static void wait_alarm(void);

static bool is_alarm_occur()
{
	struct time_ t = get_time();

	return alarm.hours == t.hours && alarm.minutes == t.minutes;
}

static void check_alarm()
{
	if (is_alarm_on() && is_alarm_occur()) {
		music_start();
		schedule_task(POLLING_BUTTON_PERIOD, (task_handler) wait_alarm, 0, 0, 0, 0);
	} else
		schedule_task(POLLING_ALARM_PERIOD, (task_handler) check_alarm, 0, 0, 0, 0);
} 

static void wait_alarm()
{
	if (is_pressed(B_ALARM_OFF))
		music_stop();
	if (!is_alarm_occur()) {
		music_stop();
		schedule_task(POLLING_ALARM_PERIOD, (task_handler) check_alarm, 0, 0, 0, 0);
	} else
		schedule_task(POLLING_BUTTON_PERIOD, (task_handler) wait_alarm, 0, 0, 0, 0);
}

/* public api */
struct time_ get_alarm()
{
	return alarm;
}

void set_alarm(struct time_ t)
{
	alarm = t;
}

void start_alarm()
{
	if (!is_active) {
		is_active = true;
		schedule_task(now, (task_handler) check_alarm, 0, 0, 0, 0);
	}
}

void stop_alarm()
{
	is_active = false;
}
