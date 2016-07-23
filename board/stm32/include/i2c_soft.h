#ifndef __I2C_SOFT__
#define __I2C_SOFT__

#include "gpio.h"

struct i2c_soft {
    /* public */
    int (*write_byte)(struct i2c_soft *, int is_start, int is_stop, int data);
    int (*read_byte)(struct i2c_soft *, int nack, int is_stop);
    /* private */
    struct gpio_out scl;
    struct gpio_out sda;
};

void configure_i2c_soft(struct i2c_soft *i2c, enum gpio_bank scl_bank, int scl_pin_nb, enum gpio_bank sda_bank, int sda_pin_nb);

#endif