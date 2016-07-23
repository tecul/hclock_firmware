#include <string.h>

#include "ili9341_gpio.h"
#include "gpio.h"
#include "systick.h"

static union gpio csx;
static union gpio d_cx;
static union gpio scl;
static union gpio sda;

#define ILI9341_RESET               0x01
#define ILI9341_SLEEP_OUT           0x11
#define ILI9341_GAMMA               0x26
#define ILI9341_DISPLAY_OFF         0x28
#define ILI9341_DISPLAY_ON          0x29
#define ILI9341_COLUMN_ADDR         0x2A
#define ILI9341_PAGE_ADDR           0x2B
#define ILI9341_GRAM                0x2C
#define ILI9341_MAC                 0x36
#define ILI9341_PIXEL_FORMAT        0x3A
#define ILI9341_WDB                 0x51
#define ILI9341_WCD                 0x53
#define ILI9341_RGB_INTERFACE       0xB0
#define ILI9341_FRC                 0xB1
#define ILI9341_BPC                 0xB5
#define ILI9341_DFC                 0xB6
#define ILI9341_POWER1              0xC0
#define ILI9341_POWER2              0xC1
#define ILI9341_VCOM1               0xC5
#define ILI9341_VCOM2               0xC7
#define ILI9341_POWERA              0xCB
#define ILI9341_POWERB              0xCF
#define ILI9341_PGAMMA              0xE0
#define ILI9341_NGAMMA              0xE1
#define ILI9341_DTCA                0xE8
#define ILI9341_DTCB                0xEA
#define ILI9341_POWER_SEQ           0xED
#define ILI9341_3GAMMA_EN           0xF2
#define ILI9341_INTERFACE           0xF6
#define ILI9341_PRC                 0xF7

static void delay(int in_ms)
{
    int start_time = get_systick();

    while (get_systick() - start_time < in_ms + 1) ;
}

static void write_bit(int bit)
{
    configure_gpio_output(&sda, GPIO_F, 9, GPIO_PUSH_PULL);
    if (bit)
        sda.out.set(&sda.out);
    else
        sda.out.clear(&sda.out);
    scl.out.clear(&scl.out);
    scl.out.set(&scl.out);
    scl.out.clear(&scl.out);
    configure_gpio_input(&sda, GPIO_F, 9, GPIO_PULL_UP);
}

static void send_command(int command)
{
    int i;

    d_cx.out.clear(&d_cx.out);
    csx.out.clear(&csx.out);
    for(i = 7; i >= 0; i--) {
        write_bit((command >> i) & 1);
    }
    csx.out.set(&csx.out);
}

static void send_data(int data)
{
    int i;

    d_cx.out.set(&d_cx.out);
    csx.out.clear(&csx.out);
    for(i = 7; i >= 0; i--) {
        write_bit((data >> i) & 1);
    }
    csx.out.set(&csx.out);
}

static void initlcd()
{
    send_command(ILI9341_MAC);
    send_data(0x48);
    send_command(ILI9341_PIXEL_FORMAT);
    send_data(0x55);
    send_command(ILI9341_SLEEP_OUT);
    delay(100);
    send_command(ILI9341_DISPLAY_ON);
}

static void set_cursor_position(int x1, int y1, int x2, int y2)
{
    send_command(ILI9341_COLUMN_ADDR);
    send_data(x1 >> 8);
    send_data(x1 & 0xff);
    send_data(x2 >> 8);
    send_data(x2 & 0xff);
    send_command(ILI9341_PAGE_ADDR);
    send_data(y1 >> 8);
    send_data(y1 & 0xff);
    send_data(y2 >> 8);
    send_data(y2 & 0xff);
}

static void swap_if_needed(int *p0, int *p1)
{
    if (*p0 > *p1) {
        int tmp = *p0;

        *p0 = *p1;
        *p1 = tmp;
    }
}

static int octant(int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int res = -1;

    if (dy >= 0) {
        if (dx >= 0) {
            if (dy >= dx)
                return 0;
            else
                return 1;
        } else {
            if (dy >= -dx)
                return 3;
            else
                return 2;
        }
    }

    return res;
}

static void draw_line_octant_0(int x0, int y0, int x1, int y1, int color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int x = x0;
    int y;
    int err = 0;

    for(y = y0; y <= y1; y++) {
        ili9341_gpio_draw_pixel(x, y, color);
        err += dx;
        if ((err << 1) >= dy) {
            x++;
            err -= dy;
        }
    }
}

static void draw_line_octant_3(int x0, int y0, int x1, int y1, int color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int x = x0;
    int y;
    int err = 0;

    for(y = y0; y <= y1; y++) {
        ili9341_gpio_draw_pixel(x, y, color);
        err += dx;
        if ((err << 1) <= -dy) {
            x--;
            err += dy;
        }
    }
}

static void draw_line_octant_1(int x0, int y0, int x1, int y1, int color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int y = y0;
    int x;
    int err = 0;

    for(x = x0; x <= x1; x++) {
        ili9341_gpio_draw_pixel(x, y, color);
        err += dy;
        if ((err << 1) >= dx) {
            y++;
            err -= dx;
        }
    }
}

static void draw_line_octant_2(int x0, int y0, int x1, int y1, int color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int y = y0;
    int x;
    int err = 0;

    for(x = x0; x >= x1; x--) {
        ili9341_gpio_draw_pixel(x, y, color);
        err += dy;
        if ((err << 1) >= -dx) {
            y++;
            err += dx;
        }
    }
}

static void draw_char(int x0, int y0, int c, struct font *font, int color)
{
    int width = font->width;
    int height = font->height;
    int w, h;
    const uint16_t *data = &font->data[c * height];

    y0 += height;
    for(h = 0; h < height; h++) {
        uint16_t d = *data++;
        uint16_t x = x0;
        for(w = 15; w > 15 - width; w--) {
            if ((d >> w) & 1)
                ili9341_gpio_draw_pixel(x, y0, color);
            x++;
        }
        y0--;
    }

}

/* api */
void construct_ili9341_gpio()
{
    ;
}

void start_ili9341_gpio()
{
    /* configure gpio */
    configure_gpio_output(&csx, GPIO_C, 2, GPIO_PUSH_PULL);
    csx.out.set(&csx.out);
    configure_gpio_output(&d_cx, GPIO_D, 13, GPIO_PUSH_PULL);
    configure_gpio_output(&scl, GPIO_F, 7, GPIO_PUSH_PULL);
    configure_gpio_input(&sda, GPIO_F, 9, GPIO_PULL_UP);

    /* init screen */
    initlcd();

    /* blank screen */
    ili9341_gpio_fill_area(0, 0, 319, 239, ~0);
}

void ili9341_gpio_draw_pixel(int x, int y, int color)
{
    set_cursor_position(y, x, y, x);
    send_command(ILI9341_GRAM);
    send_data(color >> 8);
    send_data(color & 0xff);
}

void ili9341_gpio_draw_line(int x0, int y0, int x1, int y1, int color)
{
    if (x1 < x0) {
        int tmp;

        tmp = x1;
        x1 = x0;
        x0 = tmp;
        tmp = y1;
        y1 = y0;
        y0 = tmp;
    }

    switch(octant(x0, y0, x1, y1)) {
        case 0:
            draw_line_octant_0(x0, y0, x1, y1, color);
            break;
        case 1:
            draw_line_octant_1(x0, y0, x1, y1, color);
            break;
        case 2:
            draw_line_octant_2(x0, y0, x1, y1, color);
            break;
        case 3:
            draw_line_octant_3(x0, y0, x1, y1, color);
            break;
        default:
            break;
    }
}

void ili9341_gpio_draw_rectangle(int x0, int y0, int x1, int y1, int color)
{
    ili9341_gpio_draw_line(x0, y0, x1, y0, color);
    ili9341_gpio_draw_line(x0, y0, x0, y1, color);
    ili9341_gpio_draw_line(x1, y0, x1, y1, color);
    ili9341_gpio_draw_line(x0, y1, x1, y1, color);
}

void ili9341_gpio_fill_area(int x0, int y0, int x1, int y1, int color)
{
    swap_if_needed(&x0, &x1);
    swap_if_needed(&y0, &y1);

    {
        int i = (x1 - x0 + 1) * (y1 - y0 + 1);

        set_cursor_position(y0, x0, y1, x1);
        send_command(ILI9341_GRAM);
        while(i--) {
            send_data(color >> 8);
            send_data(color & 0xff);
        }
    }
}

void ili9341_gpio_draw_string(int x0, int y0, char *str, struct font *font, int color)
{
    int i;
    int len = strlen(str);

    for(i = 0; i < len; i++) {
        draw_char(x0, y0, ((unsigned int) str[i]) - 32, font, color);
        x0 += font->width;
    }
}
