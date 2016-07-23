#ifndef __ALARM__
#define __ALARM__ 1

#include "time.h"

struct time_ get_alarm(void);
void set_alarm(struct time_ t);
void start_alarm(void);
void stop_alarm(void);

#endif