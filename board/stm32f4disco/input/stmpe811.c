#include "stmpe811.h"

#include "systick.h"
#include "gpio.h"
#include "i2c_soft.h"

#define STMPE811_CHIP_ID                0x00    //STMPE811 Device identification
#define STMPE811_ID_VER                 0x02    //STMPE811 Revision number; 0x01 for engineering sample; 0x03 for final silicon
#define STMPE811_SYS_CTRL1              0x03    //Reset control
#define STMPE811_SYS_CTRL2              0x04    //Clock control
#define STMPE811_SPI_CFG                0x08    //SPI interface configuration
#define STMPE811_INT_CTRL               0x09    //Interrupt control register
#define STMPE811_INT_EN                 0x0A    //Interrupt enable register
#define STMPE811_INT_STA                0x0B    //Interrupt status register
#define STMPE811_GPIO_EN                0x0C    //GPIO interrupt enable register
#define STMPE811_GPIO_INT_STA           0x0D    //GPIO interrupt status register
#define STMPE811_ADC_INT_EN             0x0E    //ADC interrupt enable register
#define STMPE811_ADC_INT_STA            0x0F    //ADC interface status register
#define STMPE811_GPIO_SET_PIN           0x10    //GPIO set pin register
#define STMPE811_GPIO_CLR_PIN           0x11    //GPIO clear pin register
#define STMPE811_MP_STA                 0x12    //GPIO monitor pin state register
#define STMPE811_GPIO_DIR               0x13    //GPIO direction register
#define STMPE811_GPIO_ED                0x14    //GPIO edge detect register
#define STMPE811_GPIO_RE                0x15    //GPIO rising edge register
#define STMPE811_GPIO_FE                0x16    //GPIO falling edge register
#define STMPE811_GPIO_AF                0x17    //alternate function register
#define STMPE811_ADC_CTRL1              0x20    //ADC control
#define STMPE811_ADC_CTRL2              0x21    //ADC control
#define STMPE811_ADC_CAPT               0x22    //To initiate ADC data acquisition
#define STMPE811_ADC_DATA_CHO           0x30    //ADC channel 0
#define STMPE811_ADC_DATA_CH1           0x32    //ADC channel 1
#define STMPE811_ADC_DATA_CH2           0x34    //ADC channel 2
#define STMPE811_ADC_DATA_CH3           0x36    //ADC channel 3
#define STMPE811_ADC_DATA_CH4           0x38    //ADC channel 4
#define STMPE811_ADC_DATA_CH5           0x3A    //ADC channel 5
#define STMPE811_ADC_DATA_CH6           0x3C    //ADC channel 6
#define STMPE811_ADC_DATA_CH7           0x3E    //ADC channel 7
#define STMPE811_TSC_CTRL               0x40    //4-wire touchscreen controller setup
#define STMPE811_TSC_CFG                0x41    //Touchscreen controller configuration
#define STMPE811_WDW_TR_X               0x42    //Window setup for top right X
#define STMPE811_WDW_TR_Y               0x44    //Window setup for top right Y
#define STMPE811_WDW_BL_X               0x46    //Window setup for bottom left X
#define STMPE811_WDW_BL_Y               0x48    //Window setup for bottom left Y
#define STMPE811_FIFO_TH                0x4A    //FIFO level to generate interrupt
#define STMPE811_FIFO_STA               0x4B    //Current status of FIFO
#define STMPE811_FIFO_SIZE              0x4C    //Current filled level of FIFO
#define STMPE811_TSC_DATA_X             0x4D    //Data port for touchscreen controller data access
#define STMPE811_TSC_DATA_Y             0x4F    //Data port for touchscreen controller data access
#define STMPE811_TSC_DATA_Z             0x51    //Data port for touchscreen controller data access
#define STMPE811_TSC_DATA_XYZ           0x52    //Data port for touchscreen controller data access
#define STMPE811_TSC_FRACTION_Z         0x56    //Touchscreen controller FRACTION_Z
#define STMPE811_TSC_DATA               0x57    //Data port for touchscreen controller data access
#define STMPE811_TSC_I_DRIVE            0x58    //Touchscreen controller drivel
#define STMPE811_TSC_SHIELD             0x59    //Touchscreen controller shield
#define STMPE811_TEMP_CTRL              0x60    //Temperature sensor setup
#define STMPE811_TEMP_DATA              0x61    //Temperature data access port
#define STMPE811_TEMP_TH                0x62    //Threshold for temperature controlled interrupt

static struct i2c_soft i2c;
static struct {
    int ax;
    int bx;
    int ay;
    int by;
    int width;
    int height;
} calib_info;

static int stmpe811_read_bytes(struct i2c_soft *i2c, int address, int byte_nb, uint8_t *data)
{
    int i;
    int nack = 0;

    nack |= i2c->write_byte(i2c, 1, 0, 0x82);
    nack |= i2c->write_byte(i2c, 0, 0, address);
    nack |= i2c->write_byte(i2c, 1, 0, 0x82 | 1);

    for(i = 0; i < byte_nb - 1; i++)
        data[i] = i2c->read_byte(i2c, 0, 0);
    data[i] = i2c->read_byte(i2c, 1, 1);

    return nack;
}

static int stmpe811_write_bytes(struct i2c_soft *i2c, int address, int byte_nb, uint8_t *data)
{
    int i;
    int nack = 0;

    nack |= i2c->write_byte(i2c, 1, 0, 0x82);
    nack |= i2c->write_byte(i2c, 0, 0, address);

    for(i = 0; i < byte_nb - 1; i++) {
        nack |= i2c->write_byte(i2c, 0, 0, data[i]);
    }
    nack |= i2c->write_byte(i2c, 0, 1, data[i]);

    return nack;
}

static uint8_t stmpe811_read_byte(struct i2c_soft *i2c, int address)
{
    uint8_t data;

    stmpe811_read_bytes(i2c, address, 1, &data);

    return data;
}

static int stmpe811_write_byte(struct i2c_soft *i2c, int address, uint8_t data)
{
    return stmpe811_write_bytes(i2c, address, 1, &data);
}

static int stmpe811_clear_bits(struct i2c_soft *i2c, int address, uint8_t bits_to_clear)
{
    return stmpe811_write_byte(i2c, address, stmpe811_read_byte(i2c, address) & ~bits_to_clear);
}

#if 0
static int stmpe811_set_bits(struct i2c_soft *i2c, int address, uint8_t bits_to_set)
{
    return stmpe811_write_byte(i2c, address, stmpe811_read_byte(i2c, address) | bits_to_set);
}
#endif

/* public api */
void construct_stmpe811()
{
    ;
}

void start_stmpe811()
{
    uint8_t data[2];

    configure_i2c_soft(&i2c, GPIO_A, 8, GPIO_C, 9);

    stmpe811_write_byte(&i2c, STMPE811_SYS_CTRL1, 2);
    delay_us(10000);
    stmpe811_write_byte(&i2c, STMPE811_SYS_CTRL1, 0);
    delay_us(10000);

    stmpe811_read_bytes(&i2c, STMPE811_CHIP_ID, 2, data);

    /* enable touch screeen and adc clocks */
    stmpe811_clear_bits(&i2c, STMPE811_SYS_CTRL2, 3);

    /* set tsc pins in alternate function mode */
    stmpe811_clear_bits(&i2c, STMPE811_GPIO_AF, 0xf0);

    /* tsc cfg : 4 samples avg / 500us delay / 500us panel*/
    stmpe811_write_byte(&i2c, STMPE811_TSC_CFG, 0x9a);

    /* fifo threshold set to one */
    stmpe811_write_byte(&i2c, STMPE811_FIFO_TH, 1);
    /* fifo reset */
    stmpe811_write_byte(&i2c, STMPE811_FIFO_STA, 1);
    stmpe811_write_byte(&i2c, STMPE811_FIFO_STA, 0);

    /* z accuracy */
    stmpe811_write_byte(&i2c, STMPE811_TSC_FRACTION_Z, 1);

    /* 50 ma */
    stmpe811_write_byte(&i2c, STMPE811_TSC_I_DRIVE, 1);

    /* start with now indow in xy mode */
    stmpe811_write_byte(&i2c, STMPE811_TSC_CTRL, 3);
}

void smtpe811_set_calibration_settings(int ax, int bx, int ay, int by, int width, int height)
{
    calib_info.ax = ax;
    calib_info.bx = bx;
    calib_info.ay = ay;
    calib_info.by = by;
    calib_info.width = width;
    calib_info.height = height;
}

int smtpe811_get_touch_info(int *x, int *y)
{
    uint8_t x_raw[2];
    uint8_t y_raw[2];
    int fifo_level = stmpe811_read_byte(&i2c, STMPE811_FIFO_SIZE);

    if (fifo_level >= 1) {
        stmpe811_read_bytes(&i2c, STMPE811_TSC_DATA_X, 2, x_raw);
        stmpe811_read_bytes(&i2c, STMPE811_TSC_DATA_Y, 2, y_raw);
        stmpe811_write_byte(&i2c, STMPE811_FIFO_STA, 1);
        stmpe811_write_byte(&i2c, STMPE811_FIFO_STA, 0);

        *x = x_raw[0] * 256 + x_raw[1];
        *y = y_raw[0] * 256 + y_raw[1];
    } else {
        *x = -1;
        *y = -1;
    }

    return fifo_level >= 1;
}

void smtpe811_cook_touch_info(int x_raw, int y_raw, int *x_cook, int *y_cook)
{
    int ax = calib_info.ax;
    int bx = calib_info.bx;
    int ay = calib_info.ay;
    int by = calib_info.by;
    int width = calib_info.width;
    int height = calib_info.height;

    *y_cook = (ax * x_raw + bx) / 32768;
    *x_cook = (ay * y_raw + by) / 32768;

    if (*y_cook < 0)
        *y_cook = 0;
    if (*y_cook >= height)
        *y_cook = height - 1;
    if (*x_cook < 0)
        *x_cook = 0;
    if (*x_cook >= width)
        *x_cook = width - 1;
}

int smtpe811_get_touch_info_cooked(int *x, int *y)
{
    int x_raw;
    int y_raw;
    int res = smtpe811_get_touch_info(&x_raw, &y_raw);

    if (res) {
        smtpe811_cook_touch_info(x_raw, y_raw, x, y);
    } else {
        *x = -1;
        *y = -1;
    }

    return res;
}
