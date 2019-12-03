/*
 * at_hal.h
 *
 *  Created on: Dec 5, 2018
 *      Author: sef
 */

#ifndef HAL_AT_HAL_H_
#define HAL_AT_HAL_H_

//#include "main.h"
//#include "tim.h"
#include "stm32l0xx_ll_spi.h"  // changed from f4 to l0
#include "stm32l0xx_hal.h"
/**
 * 	HAL_TickFreqTypeDef yoktu. STM32F4 kütüphanesinden bakıp enumu ekledim.
 *
 *
 */
typedef enum
{
  HAL_TICK_FREQ_10HZ         = 100U,
  HAL_TICK_FREQ_100HZ        = 10U,
  HAL_TICK_FREQ_1KHZ         = 1U,
  HAL_TICK_FREQ_DEFAULT      = HAL_TICK_FREQ_1KHZ
} HAL_TickFreqTypeDef;

struct at_hal_time
{
	uint64_t ms;
	uint32_t ns;
};

static const HAL_TickFreqTypeDef uwTickFreq = HAL_TICK_FREQ_DEFAULT;  /* 1KHz */


void at_hal_init ();

int32_t at_hal_millis(void);
uint64_t at_hal_micros(void);
uint64_t at_hal_nanos(void);
float at_hal_nanosf(void);
void at_hal_nanos2(struct at_hal_time *time);

#endif /* HAL_AT_HAL_H_ */
