#include "../src/PushButton.h"
#include "gpio.h"
#include "unity.h"
#include <stdbool.h>
#include <string.h>

//HAL simulated values
extern GPIO_PinState HAL_GPIO_ReadPin_Value;
extern u32 HAL_TICK;

//testing fixtures
bool SHORT_PRESSED = false;
static u16 HR_SET_BTN_Pin = 1;
static GPIO_TypeDef HR_SET_BTN_GPIO_Port;
void short_press_fn(void)
{
    printf("short_press_fn");
    SHORT_PRESSED = true;
}
bool LONG_PRESSED = false;
void long_press_fn(void)
{
    printf("long_press_fn");
    LONG_PRESSED = true;
}
bool VERY_LONG_PRESSED = false;
void very_long_press_fn(void)
{
    printf("very_long_press_fn");
    VERY_LONG_PRESSED = true;
}
bool DOUBLE_PRESSED = false;
void double_press_fn(void)
{
    printf("double_press_fn");
    DOUBLE_PRESSED = true;
}
bool BUTTON_UP = false;
void button_up_fn(void)
{
    printf("button up");
    BUTTON_UP = true;
}

//test helper fns
void TEST_Advance_Systick(u32 loop_ms)
{
    for (int i = 0; i < loop_ms; i++) {
        HAL_TICK++;
        PushButton.loop();
    }
}
bool TEST_button_down(u16 pin)
{
    HAL_GPIO_ReadPin_Value = GPIO_PIN_SET;
    bool result = PushButton.signal_state_change(pin, false);
    TEST_Advance_Systick(DBNC_COUNTER_MAX + 3);
    return result;
}
bool TEST_button_up(u16 pin)
{
    HAL_GPIO_ReadPin_Value = GPIO_PIN_RESET;
    bool result = PushButton.signal_state_change(pin, false);
    TEST_Advance_Systick(DBNC_COUNTER_MAX + 3);
    return result;
}

void setUp(void)
{
    HAL_TICK = 10000;
    SHORT_PRESSED = false;
    LONG_PRESSED = false;
    VERY_LONG_PRESSED = false;
    DOUBLE_PRESSED = false;
    BUTTON_UP = false;

    PushButton.init();
    HAL_GPIO_ReadPin_Value = GPIO_PIN_RESET;

    PushButton.register_button(
        (struct button_config) {
            .button_down_short = &short_press_fn,
            .button_down_long = &long_press_fn,
            .button_down_very_long = &very_long_press_fn,
            .button_down_double = &double_press_fn,
            .button_up = &button_up_fn,
            .gpio_pin = HR_SET_BTN_Pin,
            .gpio_port = &HR_SET_BTN_GPIO_Port });
}

void tearDown(void)
{
}

void test_button_instantiate(void)
{

    bool result = PushButton.register_button(
        (struct button_config) {
            .gpio_pin = HR_SET_BTN_Pin,
            .gpio_port = &HR_SET_BTN_GPIO_Port });
    TEST_ASSERT_EQUAL(result, true);
}
void test_button_cant_instantiate(void)
{

    for (int i = 0; i < MAX_BUTTONS; i++) {
        PushButton.register_button(
            (struct button_config) {
                .gpio_pin = HR_SET_BTN_Pin,
                .gpio_port = &HR_SET_BTN_GPIO_Port });
    }

    bool result = PushButton.register_button(
        (struct button_config) {
            .gpio_pin = HR_SET_BTN_Pin,
            .gpio_port = &HR_SET_BTN_GPIO_Port });
    TEST_ASSERT_EQUAL(result, false);
}

void test_trigger_button_state_changed(void)
{
    bool result = TEST_button_down(HR_SET_BTN_Pin);
    TEST_ASSERT_EQUAL(result, true);
}
void test_trigger_invalid_button_state_changed(void)
{
    bool result = TEST_button_down(HR_SET_BTN_Pin + 1);
    TEST_ASSERT_EQUAL(result, false);

    TEST_ASSERT_EQUAL(SHORT_PRESSED, false);
    TEST_ASSERT_EQUAL(LONG_PRESSED, false);
    TEST_ASSERT_EQUAL(VERY_LONG_PRESSED, false);
    TEST_ASSERT_EQUAL(DOUBLE_PRESSED, false);
    TEST_ASSERT_EQUAL(BUTTON_UP, false);
}
void test_short_press_handler(void)
{
    bool result = TEST_button_down(HR_SET_BTN_Pin);
    TEST_ASSERT_EQUAL(result, true);
    result = TEST_button_up(HR_SET_BTN_Pin);
    TEST_ASSERT_EQUAL(result, true);

    TEST_ASSERT_EQUAL(SHORT_PRESSED, true);
    TEST_ASSERT_EQUAL(LONG_PRESSED, false);
    TEST_ASSERT_EQUAL(VERY_LONG_PRESSED, false);
    TEST_ASSERT_EQUAL(DOUBLE_PRESSED, false);
    TEST_ASSERT_EQUAL(BUTTON_UP, true);
}
void test_long_press_handler(void)
{
    bool result = TEST_button_down(HR_SET_BTN_Pin);
    TEST_ASSERT_EQUAL(result, true);
    TEST_Advance_Systick(LONG_PRESS_DELAY + 1);
    result = TEST_button_up(HR_SET_BTN_Pin);
    TEST_ASSERT_EQUAL(result, true);

    TEST_ASSERT_EQUAL(SHORT_PRESSED, true);
    TEST_ASSERT_EQUAL(LONG_PRESSED, true);
    TEST_ASSERT_EQUAL(VERY_LONG_PRESSED, false);
    TEST_ASSERT_EQUAL(DOUBLE_PRESSED, false);
    TEST_ASSERT_EQUAL(BUTTON_UP, true);
}
void test_very_long_press_handler(void)
{
    bool result = TEST_button_down(HR_SET_BTN_Pin);
    TEST_Advance_Systick(VERY_LONG_PRESS_DELAY + 4);
    result = TEST_button_up(HR_SET_BTN_Pin);
    TEST_ASSERT_EQUAL(result, true);

    TEST_ASSERT_EQUAL(SHORT_PRESSED, true);
    TEST_ASSERT_EQUAL(LONG_PRESSED, true);
    TEST_ASSERT_EQUAL(VERY_LONG_PRESSED, true);
    TEST_ASSERT_EQUAL(DOUBLE_PRESSED, false);
    TEST_ASSERT_EQUAL(BUTTON_UP, true);
}
void test_double_press_handler(void)
{
    bool result = TEST_button_down(HR_SET_BTN_Pin);
    result = TEST_button_up(HR_SET_BTN_Pin);
    TEST_ASSERT_EQUAL(result, true);
    result = TEST_button_down(HR_SET_BTN_Pin);
    TEST_ASSERT_EQUAL(result, true);
    result = TEST_button_up(HR_SET_BTN_Pin);
    TEST_ASSERT_EQUAL(result, true);

    TEST_ASSERT_EQUAL(SHORT_PRESSED, true);
    TEST_ASSERT_EQUAL(LONG_PRESSED, false);
    TEST_ASSERT_EQUAL(VERY_LONG_PRESSED, false);
    TEST_ASSERT_EQUAL(DOUBLE_PRESSED, true);
    TEST_ASSERT_EQUAL(BUTTON_UP, true);
}