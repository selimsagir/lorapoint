///*
// * calc_func_time.c
// *
// *  Created on: Nov 20, 2019
// *      Author: SelimS
// */
//
//#include "stm32l0xx_hal.h"
//#include "AT_hal.h"
//
///**
// *  Calculate and see time of code in term of bulk for processor
// *
// */
//	struct at_hal_time timenow;
//	at_hal_nanos2(&timepre);
//	uint32_t mspre;
//
//void calc1(void){
//	while (1)
//	{
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 0);
//		//		uint64_t msnow = at_hal_nanos();
//		at_hal_nanos2(&timenow);
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 1);
//		if ((timenow.ms * 1000000 + timenow.ns) < (timepre.ms * 1000000 + timepre.ns) )
//		{
//			break;
//		}
//		timepre = timenow;
//	}
//}
//
//void calc2(void) {
//	while (1)
//	{
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 0);
//		//		uint64_t msnow = at_hal_nanos();
//		uint64_t msnow = at_hal_nanos();
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 1);
//		if (msnow < mspre)
//		{
//			break;
//		}
//		mspre = msnow;
//	}
//
//}
