#ifndef __RANDOMNUM__H
#define __RANDOMNUM__H

#include "stm32f4xx_hal_rng.h"
#include "stm32f4xx_hal_rcc.h"


void RNG_init(void);
uint32_t getRandomNum(void);
void getRandomNumTo(uint32_t * num);
#endif

