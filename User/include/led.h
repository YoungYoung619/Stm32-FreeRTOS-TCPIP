#ifndef LED__H
#define LED__H

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/*stm32F4 hal lib includes*/
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal.h"


void led_init(void);
void led1_flash(void);
void led1_toggle(void);
#endif

