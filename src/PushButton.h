#include "types_shortcuts.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#ifdef TEST
#include "gpio.h"
#include "hal.h"
#include <stdio.h>
#endif

enum state_codes { button_down,
    short_press,
    long_press,
    very_long_press,
    double_press,
    button_up,
    stop };
struct button_config {
    enum state_codes cur_state;
    u32 elapsed_time;
    u32 debounce_time;
    u16 gpio_pin;
    GPIO_TypeDef* gpio_port;
    GPIO_PinState state;
    enum state_codes last_state;
    u32 last_fsm_completion;
    void (*button_down_short)(void);
    void (*button_down_long)(void);
    void (*button_down_very_long)(void);
    void (*button_down_double)(void);
    void (*button_up)(void);
};
enum ret_codes { ok,
    fail,
    repeat,
    proceed,
    special };
static enum ret_codes button_down_handler(struct button_config* button);
static enum ret_codes short_press_handler(struct button_config* buttonid);
static enum ret_codes long_press_handler(struct button_config* button);
static enum ret_codes very_long_press_handler(struct button_config* button);
static enum ret_codes double_press_handler(struct button_config* button);
static enum ret_codes button_up_handler(struct button_config* button);
static enum ret_codes stop_handler(struct button_config* button);
int find_button(u16 gpio_pin);
#define MAX_BUTTONS 3
//Defines the number of ms before a value is considered legitimate
#define DBNC_COUNTER_MAX 100
#define LONG_PRESS_DELAY 1000
#define VERY_LONG_PRESS_DELAY LONG_PRESS_DELAY + 1000

extern enum ret_codes (*state[])(struct button_config* button);

struct transition {
    enum state_codes src_state;
    enum ret_codes ret_code;
    enum state_codes dst_state;
};
struct pushbutton {
    void (*init)(void);
    bool (*register_button)(struct button_config);
    void (*loop)(void);
    bool (*signal_state_change)(u16 gpio_pin, u32 dbnc_counter);
};

extern const struct pushbutton PushButton;