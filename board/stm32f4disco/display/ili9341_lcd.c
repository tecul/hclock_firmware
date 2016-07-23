#include <string.h>

#include "ili9341_lcd.h"
#include "gpio.h"
#include "systick.h"
#include "rcc.h"

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

/* register mapping */
typedef struct
{
    uint32_t    reserved0[80];
    /*Type      Name                                Description                         Offset */
    uint32_t    sdcr[2];                            /* control register                 0x140-0x144 */
    uint32_t    sdtr[2];                            /* timing registers                 0x148-0x14c */
    uint32_t    sdcmr;                              /* command mode                     0x150 */
    uint32_t    sdrtr;                              /* refresh timer                    0x154 */
    uint32_t    sdsr;                               /* status                           0x158 */
} t_smc_sdram_register;

typedef struct
{
    uint32_t    reserved0;
    /*Type      Name                                Description                             Offset */
    uint32_t    lxcr;                               /* laxerx control                       0x004 */
    uint32_t    lxwhpcr;                            /* layerx window horizontal position    0x008 */
    uint32_t    lxwvpcr;                            /* layerx window vertical position      0x00c */
    uint32_t    lxckcr;                             /* layerx color keying configuration    0x010 */
    uint32_t    lxpfcr;                             /* layerx pixel format configuration    0x014 */
    uint32_t    lxcacr;                             /* layerx constant alpha configuration  0x018 */
    uint32_t    lxdccr;                             /* layerx default color configuration   0x01c */
    uint32_t    lxbfcr;                             /* layerx blending factors configuration 0x020 */
    uint32_t    reserved1[(0x02c - 0x024) >> 2];    /*                                      0x024-0x028 */
    uint32_t    lxcfbar;                            /* layerx color frame buffer address    0x02c */
    uint32_t    lxcfblr;                            /* layerx color frame buffer length     0x030 */
    uint32_t    lxcfblnr;                           /* layerx color frame buffer line number 0x034 */
    uint32_t    reserved2[(0x044 - 0x038) >> 2];    /*                                      0x038-0x040 */
    uint32_t    lxclutwr;                           /* layerx clut write                    0x044 */
    uint32_t    reserved3[(0x080 - 0x048) >> 2];    /*                                      0x048-0x7c */
} t_ltdc_layer_register;

typedef struct
{
    uint32_t    reserved0[2];
    /*Type      Name                                Description                         Offset */
    uint32_t    sscr;                               /* sync size configuration          0x008 */
    uint32_t    bpcr;                               /* back porch configuration         0x00c */
    uint32_t    awcr;                               /* active width configuration       0x010 */
    uint32_t    twcr;                               /* total width configuration        0x014 */
    uint32_t    gcr;                                /* global control                   0x018 */
    uint32_t    reserved1[(0x024 - 0x01c) >> 2];    /*                                  0x01c-0x020 */
    uint32_t    srcr;                               /* shadow reload configuration      0x024 */
    uint32_t    reserved4;                          /*                                  0x028 */
    uint32_t    bccr;                               /* background color configuration   0x02c */
    uint32_t    reserved2[(0x034 - 0x030) >> 2];    /*                                  0x030 */
    uint32_t    ier;                                /* interrupt enable                 0x034 */
    uint32_t    isr;                                /* interrupt status                 0x038 */
    uint32_t    icr;                                /* interrupt clear                  0x03c */
    uint32_t    lipcr;                              /* line interrupt position configuration    0x040 */
    uint32_t    cpsr;                               /* current position status          0x044 */
    uint32_t    cdsr;                               /* current display status           0x048 */
    uint32_t    reserved3[(0x080 - 0x04c) >> 2];    /*                                  0x04c-0x07c */
    t_ltdc_layer_register layer[2];
} t_ltdc_register;

static t_smc_sdram_register volatile *pSdramReg = (t_smc_sdram_register *) 0xa0000000;
static t_ltdc_register volatile *pLtdcReg = (t_ltdc_register *) 0x40016800;

static union gpio csx;
static union gpio d_cx;
static union gpio scl;
static union gpio sda;

static void swap_if_needed(int *p0, int *p1)
{
    if (*p0 > *p1) {
        int tmp = *p0;

        *p0 = *p1;
        *p1 = tmp;
    }
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

static void wait_sdram_ready_for_new_command()
{
    while(pSdramReg->sdsr & 0x20) ;
}

static void init_sdram()
{
    /* setup sdram parameters access */
    pSdramReg->sdcr[0] = 0 << 0 | /* 8bits column */
                         1 << 2 | /* 12 row bits */
                         1 << 4 | /* 16 bits data */
                         1 << 6 | /* 4 banks */
                         3 << 7 | /* cas3 */
                         0 << 9 | /* write protection disable */
                         2 << 10 | /* 2 * HCLK period */
                         0 << 12;  /* no read burst */
    pSdramReg->sdcr[1] = pSdramReg->sdcr[0];
    pSdramReg->sdtr[0] = 1 << 0 | /* 2 cycles for load to active */
                         6 << 4 | /* 7 cycles for exit self refresh */
                         3 << 8 | /* 4 cycles for self refresh time */
                         6 << 12 | /* 7 cycles for row cycle */
                         1 << 16 | /* 2 cycles between write and precharge */
                         1 << 20 | /* 2 cycles of row precharge delay */
                         1 << 24; /* 2 cycles between activate and read/write */
    pSdramReg->sdtr[1] = pSdramReg->sdtr[0];
    /* start init sequence */
    wait_sdram_ready_for_new_command();
    pSdramReg->sdcmr = 1 << 0 | /* clock configuration enable */
                       1 << 3 | /* bank1 */
                       0 << 5 | /* 1 cycle auto-refresh / valid when command = 3 */
                       0 << 9; /* mdr / valid when command = 4 */
    delay_us(1000);
    wait_sdram_ready_for_new_command();
    pSdramReg->sdcmr = 2 << 0 | /* pall */
                       1 << 3 | /* bank1 */
                       0 << 5 | /* 1 cycle auto-refresh / valid when command = 3 */
                       0 << 9; /* mdr / valid when command = 4 */
    wait_sdram_ready_for_new_command();
    pSdramReg->sdcmr = 3 << 0 | /* autorefresh */
                       1 << 3 | /* bank1 */
                       7 << 5 | /* 8 cycle auto-refresh / valid when command = 3 */
                       0 << 9; /* mdr / valid when command = 4 */
    wait_sdram_ready_for_new_command();
    pSdramReg->sdcmr = 4 << 0 | /* load mode register */
                       1 << 3 | /* bank1 */
                       1 << 5 | /* 1 cycle auto-refresh / valid when command = 3 */
                       0x0230 << 9; /* mdr / valid when command = 4 */
    wait_sdram_ready_for_new_command();
    pSdramReg->sdrtr = 680 << 1;
}

static void init_sdram_gpio()
{
    configure_gpio_alternate(GPIO_B, 5, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //sdcke1
    configure_gpio_alternate(GPIO_B, 6, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //sdne1
    configure_gpio_alternate(GPIO_C, 0, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //sdnwe
    configure_gpio_alternate(GPIO_D, 0, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d2
    configure_gpio_alternate(GPIO_D, 1, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d3
    configure_gpio_alternate(GPIO_D, 8, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d13
    configure_gpio_alternate(GPIO_D, 9, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d14
    configure_gpio_alternate(GPIO_D, 10, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d15
    configure_gpio_alternate(GPIO_D, 14, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d0
    configure_gpio_alternate(GPIO_D, 15, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d1
    configure_gpio_alternate(GPIO_E, 0, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //nbl0
    configure_gpio_alternate(GPIO_E, 1, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //nbl1
    configure_gpio_alternate(GPIO_E, 7, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d4
    configure_gpio_alternate(GPIO_E, 8, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d5
    configure_gpio_alternate(GPIO_E, 9, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d6
    configure_gpio_alternate(GPIO_E, 10, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d7
    configure_gpio_alternate(GPIO_E, 11, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d8
    configure_gpio_alternate(GPIO_E, 12, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d9
    configure_gpio_alternate(GPIO_E, 13, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d10
    configure_gpio_alternate(GPIO_E, 14, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d11
    configure_gpio_alternate(GPIO_E, 15, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //d12
    configure_gpio_alternate(GPIO_F, 0, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //a0
    configure_gpio_alternate(GPIO_F, 1, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //a1
    configure_gpio_alternate(GPIO_F, 2, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //a2
    configure_gpio_alternate(GPIO_F, 3, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //a3
    configure_gpio_alternate(GPIO_F, 4, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //a4
    configure_gpio_alternate(GPIO_F, 5, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //a5
    configure_gpio_alternate(GPIO_F, 11, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //sdnras
    configure_gpio_alternate(GPIO_F, 12, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //a6
    configure_gpio_alternate(GPIO_F, 13, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //a7
    configure_gpio_alternate(GPIO_F, 14, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //a8
    configure_gpio_alternate(GPIO_F, 15, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //a9
    configure_gpio_alternate(GPIO_G, 0, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //a10
    configure_gpio_alternate(GPIO_G, 1, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //a11
    configure_gpio_alternate(GPIO_G, 4, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //ba0
    configure_gpio_alternate(GPIO_G, 5, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //ba1
    configure_gpio_alternate(GPIO_G, 8, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //sdclk
    configure_gpio_alternate(GPIO_G, 15, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF12); //sdncas
}

static void init_ili9341_gpio()
{
    configure_gpio_alternate(GPIO_A, 3, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //b5
    configure_gpio_alternate(GPIO_A, 4, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //vsync
    configure_gpio_alternate(GPIO_A, 6, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //g2
    configure_gpio_alternate(GPIO_A, 11, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //r4
    configure_gpio_alternate(GPIO_A, 12, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //r5
    configure_gpio_alternate(GPIO_B, 0, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //r3
    configure_gpio_alternate(GPIO_B, 1, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //r6
    configure_gpio_alternate(GPIO_B, 8, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //b6
    configure_gpio_alternate(GPIO_B, 9, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //b7
    configure_gpio_alternate(GPIO_B, 10, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //g4
    configure_gpio_alternate(GPIO_B, 11, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //g5

    configure_gpio_alternate(GPIO_C, 6, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //hsync
    configure_gpio_alternate(GPIO_C, 7, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //g6
    configure_gpio_alternate(GPIO_C, 10, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //r2

    configure_gpio_alternate(GPIO_D, 3, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //g7
    configure_gpio_alternate(GPIO_D, 6, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //b2

    configure_gpio_alternate(GPIO_F, 10, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //de

    configure_gpio_alternate(GPIO_G, 6, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //r7
    configure_gpio_alternate(GPIO_G, 7, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //clk
    configure_gpio_alternate(GPIO_G, 10, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //g3
    configure_gpio_alternate(GPIO_G, 11, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //b3
    configure_gpio_alternate(GPIO_G, 12, GPIO_PUSH_PULL, GPIO_NO_PULL, GPIO_HIGH_SPEED, GPIO_AF14); //b4
}

static void init_ili9341_lcd()
{
    send_command(ILI9341_MAC);
    send_data(0x48);
    send_command(ILI9341_PIXEL_FORMAT);
    send_data(0x55);
    send_command(ILI9341_SLEEP_OUT);
    delay_us(100000);
    send_command(ILI9341_DISPLAY_ON);
    send_command(0xCA);


    send_data(0xC3);
    send_data(0x08);
    send_data(0x50);
    send_command(ILI9341_POWERB);
    send_data(0x00);
    send_data(0xC1);
    send_data(0x30);
    send_command(ILI9341_POWER_SEQ);
    send_data(0x64);
    send_data(0x03);
    send_data(0x12);
    send_data(0x81);
    send_command(ILI9341_DTCA);
    send_data(0x85);
    send_data(0x00);
    send_data(0x78);
    send_command(ILI9341_POWERA);
    send_data(0x39);
    send_data(0x2C);
    send_data(0x00);
    send_data(0x34);
    send_data(0x02);
    send_command(ILI9341_PRC);
    send_data(0x20);
    send_command(ILI9341_DTCB);
    send_data(0x00);
    send_data(0x00);
    send_command(ILI9341_FRC);
    send_data(0x00);
    send_data(0x1B);
    send_command(ILI9341_DFC);
    send_data(0x0A);
    send_data(0xA2);
    send_command(ILI9341_POWER1);
    send_data(0x10);
    send_command(ILI9341_POWER2);
    send_data(0x10);
    send_command(ILI9341_VCOM1);
    send_data(0x45);
    send_data(0x15);
    send_command(ILI9341_VCOM2);
    send_data(0x90);
    send_command(ILI9341_MAC);
    send_data(0xC8);
    send_command(ILI9341_3GAMMA_EN);
    send_data(0x00);
    send_command(ILI9341_RGB_INTERFACE);
    send_data(0xC2);
    send_command(ILI9341_DFC);
    send_data(0x0A);
    send_data(0xA7);
    send_data(0x27);
    send_data(0x04);

    send_command(ILI9341_COLUMN_ADDR);
    send_data(0x00);
    send_data(0x00);
    send_data(0x00);
    send_data(0xEF);

    send_command(ILI9341_PAGE_ADDR);
    send_data(0x00);
    send_data(0x00);
    send_data(0x01);
    send_data(0x3F);
    send_command(ILI9341_INTERFACE);
    send_data(0x01);
    send_data(0x00);
    send_data(0x06);

    send_command(ILI9341_GRAM);
    delay_us(100000);

    send_command(ILI9341_GAMMA);
    send_data(0x01);

    send_command(ILI9341_PGAMMA);
    send_data(0x0F);
    send_data(0x29);
    send_data(0x24);
    send_data(0x0C);
    send_data(0x0E);
    send_data(0x09);
    send_data(0x4E);
    send_data(0x78);
    send_data(0x3C);
    send_data(0x09);
    send_data(0x13);
    send_data(0x05);
    send_data(0x17);
    send_data(0x11);
    send_data(0x00);
    send_command(ILI9341_NGAMMA);
    send_data(0x00);
    send_data(0x16);
    send_data(0x1B);
    send_data(0x04);
    send_data(0x11);
    send_data(0x07);
    send_data(0x31);
    send_data(0x33);
    send_data(0x42);
    send_data(0x05);
    send_data(0x0C);
    send_data(0x0A);
    send_data(0x28);
    send_data(0x2F);
    send_data(0x0F);

    send_command(ILI9341_SLEEP_OUT);
    delay_us(100000);
    send_command(ILI9341_DISPLAY_ON);

    send_command(ILI9341_GRAM);
}

#define HSYNC           10
#define VSYNC           2
#define HBP             20
#define VBP             2
#define ACTIVE_WIDTH    240
#define ACTIVE_HEIGHT   320
#define HFP             10
#define VFP             4

extern void setup_lcd_tft_clock();
void init_lcd_controller(void)
{
    periph_clock_enable(LTDC);
    setup_lcd_tft_clock();
    /* panel timing */
    pLtdcReg->sscr = (HSYNC - 1) << 16 |
                     (VSYNC - 1);
    pLtdcReg->bpcr = (HSYNC + HBP - 1) << 16 |
                     (VSYNC + VBP - 1);
    pLtdcReg->awcr = (HSYNC + HBP + ACTIVE_WIDTH - 1) << 16 |
                     (VSYNC + VBP + ACTIVE_HEIGHT - 1);
    pLtdcReg->twcr = (HSYNC + HBP + ACTIVE_WIDTH + HFP - 1) << 16 |
                     (VSYNC + VBP + ACTIVE_HEIGHT + VFP - 1);

    /* hsync active low, vsync active low, data enable active low, pixel clock unchanged */
    pLtdcReg->gcr = 0;
    pLtdcReg->bccr = 0; /* black */

    /* layer 1 */
    pLtdcReg->layer[0].lxwhpcr = (HSYNC + HBP + ACTIVE_WIDTH - 1) << 16 | 
                                 (HSYNC + HBP);
    pLtdcReg->layer[0].lxwvpcr = (VSYNC + VBP + ACTIVE_HEIGHT - 1) << 16 |
                                 (VSYNC + VBP);
    pLtdcReg->layer[0].lxpfcr = 2; /* RGB565 */
    pLtdcReg->layer[0].lxcacr = 0xff; /* constant alpha is 1 */
    pLtdcReg->layer[0].lxbfcr = (4 << 8) | 4; /* use layer 1 only */
    pLtdcReg->layer[0].lxcfbar = 0xd0000000;
    pLtdcReg->layer[0].lxcfblr = ((ACTIVE_WIDTH * 2) << 16) |
                                 (ACTIVE_WIDTH * 2 + 3);
    pLtdcReg->layer[0].lxcfblnr = ACTIVE_HEIGHT;
    pLtdcReg->layer[0].lxcr = 1; /* enable layer */

    /* layer 2 not use */
    /* reload shadow registers */
    pLtdcReg->srcr |= 1;
    /* enable ltdc */
    pLtdcReg->gcr |= 1;
}

static void initlcd()
{
    init_sdram_gpio();
    periph_clock_enable(FMC);
    init_sdram();
    init_ili9341_gpio();
    init_ili9341_lcd();
    init_lcd_controller();
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
        ili9341_lcd_draw_pixel(x, y, color);
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
        ili9341_lcd_draw_pixel(x, y, color);
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
        ili9341_lcd_draw_pixel(x, y, color);
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
        ili9341_lcd_draw_pixel(x, y, color);
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
                ili9341_lcd_draw_pixel(x, y0, color);
            x++;
        }
        y0--;
    }
}

/* api */
void construct_ili9341_lcd()
{
    ;
}

void start_ili9341_lcd()
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
    ili9341_lcd_fill_area(0, 0, 319, 239, ~0);
}

void ili9341_lcd_draw_pixel(int x, int y, int color)
{
    uint16_t *p = (uint16_t *) 0xd0000000;

    if (y < 0 || y >= 240)
        return ;
    if (x < 0 || x >= 320)
        return ;

    p += (239 - y);
    p+= 240 * (319 - x);

    *p = color;
}

void ili9341_lcd_draw_line(int x0, int y0, int x1, int y1, int color)
{
    if (y1 < y0) {
        int tmp;

        tmp = y1;
        y1 = y0;
        y0 = tmp;
        tmp = x1;
        x1 = x0;
        x0 = tmp;
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

void ili9341_lcd_draw_rectangle(int x0, int y0, int x1, int y1, int color)
{
    ili9341_lcd_draw_line(x0, y0, x1, y0, color);
    ili9341_lcd_draw_line(x0, y0, x0, y1, color);
    ili9341_lcd_draw_line(x1, y0, x1, y1, color);
    ili9341_lcd_draw_line(x0, y1, x1, y1, color);
}

void ili9341_lcd_fill_area(int x0, int y0, int x1, int y1, int color)
{
    uint16_t *p = (uint16_t *) 0xd0000000;
    uint16_t *p_save;
    int x, y;

    swap_if_needed(&x0, &x1);
    swap_if_needed(&y0, &y1);
    if (y0 <= 0)
        y0 = 0;
    if (y1 <= 0)
        y1 = 0;
    if (y0 >= 240)
        y0 = 239;
    if (y1 >= 240)
        y1 = 239;
    if (x0 <= 0)
        x0 = 0;
    if (x1 <= 0)
        x1 = 0;
    if (x0 >= 320)
        x0 = 319;
    if (x1 >= 320)
        x1 = 319;

    p = (uint16_t *) 0xd0000000;
    p += (239 - y1);
    p+= 240 * (319 - x1);

    for(x = x1; x >= x0; x--) {
        p_save = p;
        for(y = y1; y >= y0; y--)
            *p++ = color;
        p = p_save + 240;
    }
}

void ili9341_lcd_draw_string(int x0, int y0, char *str, struct font *font, int color)
{
    int i;
    int len = strlen(str);

    for(i = 0; i < len; i++) {
        draw_char(x0, y0, ((unsigned int) str[i]) - 32, font, color);
        x0 += font->width;
    }
}
