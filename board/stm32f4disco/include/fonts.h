#ifndef __FONTS__
#define __FONTS__

#include <stdint.h>

struct font {
    int width;
    int height;
    const uint16_t *data;
};

extern struct font font_7x10;
extern struct font font_11x18;
extern struct font font_16x26;

#endif
