#include "i2c_soft.h"
#include "systick.h"

static void start_bit(struct i2c_soft *i2c)
{
    i2c->sda.set(&i2c->sda);
    delay_us(10);
    i2c->scl.set(&i2c->scl);
    delay_us(10);
    i2c->sda.clear(&i2c->sda);
    delay_us(10);
    i2c->scl.clear(&i2c->scl);
    delay_us(10);
}

static void stop_bit(struct i2c_soft *i2c)
{
    i2c->sda.clear(&i2c->sda);
    delay_us(10);
    i2c->scl.set(&i2c->scl);
    delay_us(10);
    i2c->sda.set(&i2c->sda);
    delay_us(10);
}

static void write_bit(struct i2c_soft *i2c, int bit)
{
    if (bit)
        i2c->sda.set(&i2c->sda);
    else
        i2c->sda.clear(&i2c->sda);
    delay_us(10);
    i2c->scl.set(&i2c->scl);
    delay_us(10);
    /* should have clock stretching here */
    i2c->scl.clear(&i2c->scl);
    delay_us(10);
}

static int read_bit(struct i2c_soft *i2c)
{
    int bit;

    /* release sda (open drain) */
    i2c->sda.set(&i2c->sda);
    delay_us(10);
    i2c->scl.set(&i2c->scl);
    delay_us(10);
    /* should have clock stretching here */
    bit = i2c->sda.get(&i2c->sda);
    delay_us(10);
    i2c->scl.clear(&i2c->scl);
    delay_us(10);

    return bit;
}

static int write_byte(struct i2c_soft *i2c, int is_start, int is_stop, int data)
{
    int i;
    int nack;

    if (is_start)
        start_bit(i2c);

    for(i = 7; i >= 0; i--)
        write_bit(i2c, (data >> i) & 1);

    nack = read_bit(i2c);

    if (is_stop)
        stop_bit(i2c);

    return nack;
}

#include <stdio.h>
static int read_byte(struct i2c_soft *i2c, int nack, int is_stop)
{
    int i;
    int res = 0;

    for(i = 7; i >= 0; i--) {
        int bit = read_bit(i2c);
        //printf("bit = %d\n", bit);
        res = (res << 1) | bit;
    }

    write_bit(i2c, nack);

    if (is_stop)
        stop_bit(i2c);

    return res;
}

/* api */
void configure_i2c_soft(struct i2c_soft *i2c, enum gpio_bank scl_bank, int scl_pin_nb, enum gpio_bank sda_bank, int sda_pin_nb)
{
    configure_gpio_output((union gpio *) &i2c->scl, scl_bank, scl_pin_nb, GPIO_OPEN_DRAIN);
    configure_gpio_output((union gpio *) &i2c->sda, sda_bank, sda_pin_nb, GPIO_OPEN_DRAIN);
    i2c->write_byte = write_byte;
    i2c->read_byte = read_byte;
    i2c->sda.clear(&i2c->sda);
    delay_us(10);
    i2c->scl.clear(&i2c->scl);
    delay_us(10);
    i2c->scl.set(&i2c->scl);
    delay_us(10);
    i2c->sda.set(&i2c->sda);
    delay_us(10);
}
