#ifndef __STMPE811__
#define __STMPE811__

void construct_stmpe811(void);
void start_stmpe811(void);

void smtpe811_set_calibration_settings(int ax, int bx, int ay, int by, int width, int height);
int smtpe811_get_touch_info(int *x, int *y);
void smtpe811_cook_touch_info(int x_raw, int y_raw, int *x_cook, int *y_cook);
int smtpe811_get_touch_info_cooked(int *x, int *y);

#endif
