#include "PushButton.h"

/* array and enum below must be in sync! */

enum ret_codes (*state[])(struct button_config*) = { button_down_handler, short_press_handler, long_press_handler, very_long_press_handler, double_press_handler, button_up_handler, stop_handler };
struct transition state_transitions[] = {

    { button_down, ok, short_press },

    { short_press, ok, button_up },
    { short_press, repeat, short_press },
    { short_press, proceed, long_press },
    { short_press, special, double_press },

    { double_press, ok, button_up },

    { long_press, ok, button_up },
    { long_press, repeat, long_press },
    { long_press, proceed, very_long_press },

    { very_long_press, ok, button_up },
    { very_long_press, repeat, very_long_press },

    { button_up, ok, stop },

    { stop, repeat, stop },

};

static u8 registered_buttons = 0;
static struct button_config button_configs[MAX_BUTTONS];

static void init()
{
    memset(button_configs, 0, sizeof(button_configs));
    registered_buttons = 0;
}
static enum state_codes lookup_transitions(enum state_codes cur_state,
    enum ret_codes rc)
{
    int i;
    int max_items;
    enum state_codes next_state;

    i = 0;
    max_items = sizeof(state_transitions) / sizeof(state_transitions[0]);
    next_state = stop;

    for (i = 0; i < max_items; i++) {
        if ((state_transitions[i].src_state == cur_state) && (state_transitions[i].ret_code == rc)) {
            next_state = state_transitions[i].dst_state;
            break;
        }
    }

    return (next_state);
}

static enum ret_codes button_down_handler(struct button_config* button)
{
    button->elapsed_time = HAL_GetTick();
    return ok;
}
static enum ret_codes short_press_handler(struct button_config* button)
{
    if (button->state == GPIO_PIN_RESET) {
        return ok;
    } else {
        // if held for more than 1s transition to long
        if (HAL_GetTick() - button->elapsed_time > LONG_PRESS_DELAY) {
            return proceed;
        } else if (HAL_GetTick() - button->last_fsm_completion < LONG_PRESS_DELAY) {

            return special;
        } else if (button->last_state != short_press) {

            if (button->button_down_short != NULL) {
                button->button_down_short();
            }
        }
    }
    // fall through to repeat this state
    return repeat;
}
static enum ret_codes long_press_handler(struct button_config* button)
{
    if (button->state == GPIO_PIN_RESET) {
        return ok;
    } else {
        // if held for an additional 1s transition to very_long
        if (HAL_GetTick() - button->elapsed_time > VERY_LONG_PRESS_DELAY) {
            return proceed;
        } else if (button->last_state != long_press) {
            if (button->button_down_long != NULL) {
                button->button_down_long();
            }
        }
    }
    // fall through to repeat this state
    return repeat;
}
static enum ret_codes very_long_press_handler(struct button_config* button)
{
    if (button->state == GPIO_PIN_RESET) {
        return ok;
    } else {
        // just sit here till they get off the button
        if (button->last_state != very_long_press) {
            if (button->button_down_very_long != NULL) {
                button->button_down_very_long();
            }
        }
    }
    // fall through to repeat this state
    return repeat;
}

static enum ret_codes double_press_handler(struct button_config* button)
{
    if (button->button_down_double != NULL) {
        button->button_down_double();
    }
    return ok;
}
static enum ret_codes button_up_handler(struct button_config* button)
{
    button->last_fsm_completion = HAL_GetTick();
    if (button->button_up != NULL) {
        button->button_up();
    }
    return ok;
}
static enum ret_codes stop_handler(struct button_config* button)
{
    return repeat;
}
static void loop()
{
    u32 i = 0;
    for (i = 0; i < registered_buttons; i++) {
        struct button_config* button = &button_configs[i];
        // is debounce over?
        if (button->debounce_time != 0 && HAL_GetTick() > button->debounce_time) {

            button->debounce_time = 0;
            GPIO_PinState state = HAL_GPIO_ReadPin(button->gpio_port, button->gpio_pin);

            // did state change
            if (state != button->state) {
                // update state
                button->state = state;
                //app_log_trace("button %d state changed: %d", button->gpio_pin, state);
                // if state is stop and button is down, start fsm
                if (state == GPIO_PIN_SET && button->cur_state == stop) {
                    button->state = state;
                    button->cur_state = button_down;
                }
            }
        }
        // if debounce has elapsed and we're stopped continue
        else if (button->cur_state != stop) {
            enum ret_codes (*state_fun)(struct button_config*) = state[button->cur_state];
            enum ret_codes return_code = state_fun(button);
            button->last_state = button->cur_state;
            button->cur_state = lookup_transitions(button->cur_state, return_code);
        }
    }
}

static bool register_button(struct button_config config_in)
{
    if (registered_buttons < MAX_BUTTONS) {
        struct button_config* config = &button_configs[registered_buttons++];
        config->button_down_short = config_in.button_down_short;
        config->button_down_long = config_in.button_down_long;
        config->button_down_very_long = config_in.button_down_very_long;
        config->button_down_double = config_in.button_down_double;
        config->button_up = config_in.button_up;
        config->cur_state = stop;
        config->last_fsm_completion = 0;
        config->gpio_pin = config_in.gpio_pin;
        config->gpio_port = config_in.gpio_port;
        config->state = HAL_GPIO_ReadPin(config_in.gpio_port, config_in.gpio_pin);
        return true;
    }

    return false;
}
int find_button(u16 gpio_pin)
{
    u8 i = 0;
    for (i = 0; i < registered_buttons; i++) {
        if (button_configs[i].gpio_pin == gpio_pin) {
            return i;
        }
    }
    return -1;
}
static bool signal_state_change(u16 gpio_pin, u32 dbnc_counter)
{
    int button_index = find_button(gpio_pin);
    if (button_index != -1) {
        struct button_config* signaled_button = &button_configs[button_index];
        if (dbnc_counter > 0) {
            signaled_button->debounce_time = HAL_GetTick() + dbnc_counter;
        } else {
            signaled_button->debounce_time = HAL_GetTick() + DBNC_COUNTER_MAX;
        }
        return true;
    }
    return false;
}

const struct pushbutton PushButton = {
    .init = init,
    .loop = loop,
    .register_button = register_button,
    .signal_state_change = signal_state_change
};
