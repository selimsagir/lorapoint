/*
 * at_hal.c
 *
 *  Created on: Dec 20, 2018
 *      Author: sefa unal
 */

#include "AT_hal.h"

#define CPU_FREQUENCY 		(32000000)  //32MHz for STM32L0
#define CPU_FREQUENCY_MHZ 	((CPU_FREQUENCY) / 1000000)
#define CPU_FREQUENCY_MHZ_1 	(1000000 / (CPU_FREQUENCY))
#define CPU_CLOCK_PERIOD 	(1.0 / (CPU_FREQUENCY))

extern uint32_t uwTick;
//extern HAL_TickFreqTypeDef uwTickFreq;
uint8_t systick_missing_ms;

// Hal function
void HAL_IncTick(void)
{
	// Increase milliseconds
	uwTick += uwTickFreq;
	//uwTick += uwTickFreq;
//
//	// Millis increased, so reset the missing_ms to 0
//	systick_missing_ms = 0;

	// Reset COUNTFLAG by reading SysTick->CTRL register
	// TODO: This line might get ignored if compiler optimizer is on
	SysTick->CTRL;
}

void at_hal_init ()
{
	// set load value for 1ms overflow
	SysTick->LOAD = CPU_FREQUENCY_MHZ * 1000 - 1; // (168000 - 1) = 0x2903F,
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
	SysTick->VAL = 0;
}

struct systickArgs
{
	uint32_t 	systickValue_now;
	uint32_t 	ms_now;
	uint8_t		ms_missing;
};

static struct systickArgs acquire_systick_value ()
{
	static struct systickArgs lSystickValues;

	// TODO: After this block interrupts are enabled regardless of interrupt's current state
	// Change this block with a critical_section

	// Disable interrupts to avoid a race condition with systick_handler
	__disable_irq();

	// Read current systick value and current milliseconds
	lSystickValues.systickValue_now = SysTick->VAL;
	lSystickValues.ms_now = uwTick;

	// Check if there is any new systick tick event occured
	if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
	{
		// New event occured but interrupts are disabled. So increase ms by one
		systick_missing_ms = 1;

		// New event occured, so systick value has changed drastically (from near 0 to systick reset value)
		// Re-read the value
		lSystickValues.systickValue_now = SysTick->VAL;
	}
	// Copy systick_missing_ms in a buffer, because after enabling the interrupts, systick_handler may set the systick_missing_ms to 0
	lSystickValues.ms_missing = systick_missing_ms;

	// Enable interrupts
	__enable_irq();

	return lSystickValues;
}

// return type uint64_t: 2^64 * 1ns = 213 503 d + 23 h + 34 min + 33.709552 s
// return type uint32_t: 2^32 * 1ns = 4.2949673 seconds!!
uint64_t at_hal_nanos(void)
{
	uint32_t 	systickValue_now;
	uint32_t 	ms_now;

	// Disable interrupts to avoid a race condition with systick_handler
	__disable_irq();

	// Read current systick value and current milliseconds
	systickValue_now = SysTick->VAL;
	ms_now = uwTick;

	// Check if there is any new systick tick event occured
	if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
	{
		// New event occured, so systick value has changed drastically (from near 0 to systick reset value)
		// Re-read the value
		systickValue_now = SysTick->VAL;

		// Enable interrupts
		__enable_irq();

		// New event occured but interrupts are disabled. So increase ms by one
		return 1000 * (1000 * (uint64_t)(ms_now + 1) +
				(1000 - systickValue_now * (1 / CPU_FREQUENCY_MHZ) ));
	}

	// Enable interrupts
	__enable_irq();

	// Calculate nanoseconds and return
	return 1000 * (1000 * (uint64_t)(ms_now) +
			(1000 - systickValue_now * (1 / CPU_FREQUENCY_MHZ) ));
//	lResult += (1024*1000 - systickValue_now * 1024 * (1 / (32)));
//	uint64_t lResult = (uint64_t)1024*1024 * (ms_now + systick_missing_ms);
//	lResult += (1024*1000 - systickValue_now * 1024 * (1 / (32)));
//	lResult += (1000000 - ((systickValue_now * 1000) >> 5));
//	return lResult;
}

void at_hal_nanos2(struct at_hal_time *time)
{
	uint32_t 	systickValue_now;

	// Disable interrupts to avoid a race condition with systick_handler
	__disable_irq();

	// Read current systick value and current milliseconds
	systickValue_now = SysTick->VAL;

	// Check if there is any new systick tick event occured
	if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
	{
		// New event occured, so systick value has changed drastically (from near 0 to systick reset value)
		// Re-read the value
		systickValue_now = SysTick->VAL;

		// New event occured but interrupts are disabled. So increase ms by one
		time->ms = uwTick + 1;
		time->ns = 1000000 - (systickValue_now + 1) * 1000 * (1 / CPU_FREQUENCY_MHZ);
	}
	else
	{
		time->ms = uwTick;
		time->ns = 1000000 - (systickValue_now + 1) * 1000 * (1 / CPU_FREQUENCY_MHZ);
	}

	// Enable interrupts
	__enable_irq();
}

float at_hal_nanosf(void)
{
	struct systickArgs lSystickValues = acquire_systick_value ();

	// Calculate nanoseconds and return
	float lResult;
	lResult = 1000000.0f * (lSystickValues.ms_now + lSystickValues.ms_missing);
	lResult += (1000000.0f - lSystickValues.systickValue_now * 1000.0f / CPU_FREQUENCY_MHZ);
	return lResult;
}

// return type 64: 2^64 * 1us = 213 503 982 d + 8 h + 1 min + 49.551616 s
// return type 32: 2^32 * 1us = 1 h + 11 min + 34.967296 s
uint64_t at_hal_micros(void)
{
	struct systickArgs lSystickValues = acquire_systick_value ();

	// Calculate microseconds and return
	uint64_t result;
	result = 1000 * (uint64_t)(lSystickValues.ms_now + lSystickValues.ms_missing);
	result += (1000 - lSystickValues.systickValue_now / CPU_FREQUENCY_MHZ);
	return result;
}

// TODO: Overflow will occur after 49days. Change to uint64?
// return type 32: 2^32 × 1ms = 49 d + 17 h + 2 min + 47.296 s
int32_t at_hal_millis(void)
{
    return uwTick;
}
