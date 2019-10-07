#include <stdbool.h>
#include "stm32l0xx.h"

#include "hw_gpio.h"
#include "hw_msp.h"
#include "hw_rtc.h"
#include "radio.h"
#include "vcom.h"
#include "timeServer.h"


/*
int16_t GetTemp( void )
{
	const uint16_t V25 =1750;
	const uint16_t Avg_Slope = 5;
	int16_t TemperatureC = 0 ;
	int16_t tempdata  = 0 ;
	tempdata = HW_AdcReadChannel( ADC_CHANNEL_TEMPSENSOR );
	TemperatureC = (uint16_t)((V25-tempdata)/Avg_Slope+25);
	return tempdata;
	}
*/

static TimerEvent_t ledTimer;

static uint16_t databuffer;
uint16_t rec_buffer;

int garbageTxDataCount = 0 ;
int garbageRxDataCount = 0 ;

#define LORA_FREQUENCY 868000000
#define LORA_TX_POWER 14
#define LORA_BANDWIDTH 2//0
#define LORA_DATARATE 7//10
#define LORA_CODERATE 1
#define LORA_PREAMBLE_LEN 8
/*
extern  TimerTime_t HW_RTC_GetCalendarValue( RTC_DateTypeDef* RTC_DateStruct, RTC_TimeTypeDef* RTC_TimeStruct );
void HW_RTC_TestLoop( void )
{
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;
	uint32_t next;
	uint32_t prev = HW_RTC_GetCalendarValue(&RTC_DateStruct, &RTC_TimeStruct );
	uint32_t diff;
	while(1)
	{
		next = HW_RTC_GetCalendarValue(&RTC_DateStruct, &RTC_TimeStruct );
		if( next < prev )
		{
			diff = next - prev;
			while(1)
			{
				PRINTF("-------------------------------\r\n");
				PRINTF("Difference unsigned: %u\r\n", diff);
				PRINTF("Difference signed: %d\r\n", (int32_t)diff);
				HAL_Delay(1000);
			}
		}
		prev = next;
	}
}
*/

/**
 * For Receive "RX"
 * For Transmit "TX"
 */
#define RX

static void gpioCallback()
{
	//BSP_LED_On(LED3);

	uint32_t airtime = 0 ;
    databuffer = HW_AdcReadChannel(ADC_CHANNEL_0);
    airtime = Radio.TimeOnAir(1, sizeof(databuffer));

    if(databuffer < 4095)
		{
			Radio.Send((uint8_t*)&databuffer, (sizeof(databuffer)));
			PRINTF("%d \n",databuffer);
		}
    else
    	{
    		garbageTxDataCount++;

    	}

}

static void ledTimerCallback()
{
	//BSP_LED_Off(LED2);
}

void txDoneEventCallback()
{
//	BSP_LED_Off(LED3);
    Radio.Rx(0);
}

void rxDoneEventCallback(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	//BSP_LED_On(LED2);

	TimerStart(&ledTimer);
	RTC_TimeTypeDef *tStruct = {0};
	tStruct = HW_RTC_GetTimerValue();
	/**
	 *
	 * TO-DO hw_rtc.c for HW_RTC_GetTimerValue how return(a struct) if you found a way for this, it would
	 *  be possible pass RTC_TimeStruct to main function but pay attention that it is a local function and stuck will be zero after pass to other function
	 *
	 *
	 */

		if( (payload[0] + (payload[1]<<8)) > 4095 )  // Only 1 and 1 true so do nothing
			{

			garbageRxDataCount++ ;			// TO-DO create a buffer and insert garbage datas into it
			}
		else
			{
				vcom_Send("Second: %d Minute: %d Data: %d"  ,tStruct->Seconds, tStruct->Minutes, payload[0] +  (payload[1]<<8));

				PRINTF("\n");
			}
}

int main(void)
{
	HAL_Init();
	SystemClock_Config();
	HW_Init();

	RadioEvents_t radioEvents;
    radioEvents.TxDone = txDoneEventCallback;
    radioEvents.RxDone = rxDoneEventCallback;
    Radio.Init(&radioEvents);

    Radio.SetChannel(LORA_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, LORA_TX_POWER, 0, LORA_BANDWIDTH, LORA_DATARATE, LORA_CODERATE, LORA_PREAMBLE_LEN,
                      false, true, false, 0, 0, 3000000);  // timeout

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_DATARATE, LORA_CODERATE, 0, LORA_PREAMBLE_LEN,
                      1000, false, 0, true, false, 0, false, true);

    Radio.Rx(0);

    TimerInit(&ledTimer, ledTimerCallback);
    TimerSetValue(&ledTimer, 500);




#ifdef  TX

	   while(1){

		   gpioCallback();  // for transmitter
	   	   }
#endif

#ifdef RX
		for(;;); // for receiver
#endif
    /*
    HW_GPIO_SetIrq(GPIOB, GPIO_PIN_2, 0, gpioCallback);
    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
    for(;;);
    */


}
