#ifndef __ILI9341_GPIO__
#define __ILI9341_GPIO__

#include "fonts.h"

void construct_ili9341_gpio(void);
void start_ili9341_gpio(void);
void ili9341_gpio_draw_pixel(int x, int y, int color);
void ili9341_gpio_draw_line(int x0, int y0, int x1, int y1, int color);
void ili9341_gpio_draw_rectangle(int x0, int y0, int x1, int y1, int color);
void ili9341_gpio_fill_area(int x0, int y0, int x1, int y1, int color);
void ili9341_gpio_draw_string(int x0, int y0, char *str, struct font *font, int color);

#endif 