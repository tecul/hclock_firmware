#ifndef __TIME___
#define __TIME___ 1

#include <stdint.h>

struct time_ {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t subseconds;
};

struct time_ get_time();
void set_time(struct time_ t);
void start_time(void);
void stop_time(void);

#endif