# STM32 Pushbutton Library

> An STM32-based pushbutton library with built-in debouncing.
> This project is a STM32-based library to help organize button actions and provide a simple software-based debouncing solution. When connecting buttons to microcontrollers a common problem engineers run into is _debouncing_: after pushing or releasing a button it will "bounce" between open and closed due to the electrical properties of the circuit. While it is possible to solve this problem in hardware, this library introduces a configurable, software-based debouncing solution that delays processing a button press until the bouncing has completed. This library also helps organize button actions buy introducing several higher-level concepts, such as long push, make organizing button actions easier.

### Main Features

- Completely written in C, making integration into RTOS and HAL projects easier.
- Built-in state machine ensures accurate button processing.
- Easy interface helps organize all button actions in a single location.
- Robustly unit tested

### Example

```c
  PushButton.init();
  PushButton.register_button(
    (struct button_config){
      .button_down_short = &hour_increment,
      .button_up = &hour_increment_stop,
      .gpio_pin = HR_SET_BTN_Pin,
      .gpio_port = HR_SET_BTN_GPIO_Port });
```

### Build Instructions

#### Install buildchain dependencies

```console
$ sudo apt-get install valgrind cppcheck clang-format clang ruby
```

#### Run Tests

```console
$ make clean && make test

-----------------------ANALYSIS AND TESTING SUMMARY-----------------------
0 tests ignored

0 tests failed

8 tests passed

0 memory leak(s) detected

0 code warnings

0 code errors

```
