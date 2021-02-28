#include "gpio.h"

GPIO_PinState HAL_GPIO_ReadPin_Value = GPIO_PIN_RESET;

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) { return HAL_GPIO_ReadPin_Value; }