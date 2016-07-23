#ifndef __BUTTON__
#define __BUTTON__ 1

enum button {
	B_SET_TIME = 0,
	B_SET_ALARM,
	B_HOUR,
	B_MINUTE,
	B_ALARM_OFF,
	B_ALARM_ACTIVE, /* switch */
};

void button_configure();
bool is_pressed(enum button b);

#endif