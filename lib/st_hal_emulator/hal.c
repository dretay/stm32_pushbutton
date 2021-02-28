#include "hal.h"

uint32_t HAL_TICK = 10000;
uint32_t HAL_GetTick(void)
{
    return HAL_TICK;
}