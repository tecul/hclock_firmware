#ifndef __GPIO__
#define __GPIO__

enum gpio_bank {
    GPIO_A = 0,
    GPIO_B,
    GPIO_C,
    GPIO_D,
    GPIO_E,
    GPIO_F,
    GPIO_G,
    GPIO_H,
    GPIO_I,
    GPIO_J,
    GPIO_K,
};

enum gpio_output_type {
    GPIO_PUSH_PULL = 0,
    GPIO_OPEN_DRAIN
};

enum gpio_input_pull_type {
    GPIO_NO_PULL = 0,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN
};

enum gpio_speed {
    GPIO_LOW_SPEED = 0,
    GPIO_MEDIUM_SPEED,
    GPIO_HIGH_SPEED,
    GPIO_VERY_HIGH_SPEED
};

enum gpio_alternate
{
    GPIO_AF0,
    GPIO_AF1,
    GPIO_AF2,
    GPIO_AF3,
    GPIO_AF4,
    GPIO_AF5,
    GPIO_AF6,
    GPIO_AF7,
    GPIO_AF8,
    GPIO_AF9,
    GPIO_AF10,
    GPIO_AF11,
    GPIO_AF12,
    GPIO_AF13,
    GPIO_AF14,
    GPIO_AF15,
};

struct gpio_out {
    /* use this */
    void (*set)(struct gpio_out *);
    void (*clear)(struct gpio_out *);
    int (*get)(struct gpio_out *);
    /* private field */
    enum gpio_bank bank;
    int pin_nb;
};

struct gpio_in {
    /* use this */
    int (*get)(struct gpio_in *);
    /* private field */
    enum gpio_bank bank;
    int pin_nb;
};

union gpio {
    struct gpio_out out;
    struct gpio_in in;
};

void construct_gpio(void);
void start_gpio(void);
void configure_gpio_output(union gpio *gpio, enum gpio_bank bank, int pin_nb, enum gpio_output_type type);
void configure_gpio_input(union gpio *gpio, enum gpio_bank bank, int pin_nb, enum gpio_input_pull_type type);
void configure_gpio_alternate(enum gpio_bank bank, int pin_nb, enum gpio_output_type out_type, enum gpio_input_pull_type in_type, enum gpio_speed speed, enum gpio_alternate alt);

#endif
