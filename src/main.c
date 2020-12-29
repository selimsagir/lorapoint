#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
#include <string.h>
#include <strings.h>
#include "stm32l0xx.h"

#include "hw_gpio.h"
#include "hw_msp.h"
#include "hw_rtc.h"
#include "radio.h"
#include "vcom.h"
#include "timeServer.h"


#include "AT_hal.h"
#include "fonts.h"
#include "ssd1306.h"

/* 				 Yapilan Degisiklikler
 *   1. ADC sampling time deðiþtim böylelikle döngüyü hýzlandýracaðýmý düþünüyorum, pek sonuç vermedi gibi!
 *
 *
 */

static TimerEvent_t ledTimer;
extern UART_HandleTypeDef UartHandle;

static uint16_t databuffer;
uint16_t rec_buffer;
// deneme is this changing???
int garbageTxDataCount = 0 ;
int garbageRxDataCount = 0 ;

#define LORA_FREQUENCY 868000000
#define LORA_TX_POWER 14
#define LORA_BANDWIDTH 2//0  [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_DATARATE 7//10
#define LORA_CODERATE 1  // LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LEN 8
/**
 * For Receive "RX" , For Transmit "TX"
 */
#define RX

#define BUFFER_SIZE   64 // Define the payload size here

uint8_t Buffer[BUFFER_SIZE];

I2C_HandleTypeDef hi2c1;
extern uint32_t uwTick;

union tempframe{
	 struct  Mpacket {
		uint32_t subsec;
		uint8_t sec;
		uint8_t min;
		uint8_t hour;
		uint16_t sensor;
	}__attribute__((packed)) frame ;
	unsigned char data[9];
}dat;

struct ms_packet{
		uint32_t ms_s;
		uint16_t sensor_s;
		uint16_t packetCount;
}__attribute__((packed)) data_s;


static void gpioCallback()
{
//	//BSP_LED_On(LED3);
//	uint32_t airtime = 0 ;
//	UNUSED(airtime);
//    airtime = Radio.TimeOnAir(1, sizeof(data_s));

	uint64_t times[8];

	times[0] = at_hal_micros();
    databuffer = HW_AdcReadChannel(ADC_CHANNEL_0);

	times[1] = at_hal_micros();

	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	HW_RTC_GetCalendarValue( &date , &time );
	times[2] = at_hal_micros();

	dat.frame.subsec = (int)(time.SubSeconds * 1000 / (time.SecondFraction + 1));
	dat.frame.sec = time.Seconds;
	dat.frame.min = time.Minutes;
	dat.frame.hour = time.Hours;
	dat.frame.sensor = databuffer;
	times[3] = at_hal_micros();

	/*
	 *  1.  Send Milisecond data for plot ms-data figure
	 *  2.  Calculate ms, subtract from 1000 because of subsec count reverse
	 *  	Main purpose should be reduce to execute time of this loop I can't get enough resolution for milisecond reading
	 *  	Decrease to resolution at least 1 ms for precise calculation of time difference between sensors
	 */

	data_s.packetCount++;
	dat.frame.subsec = 1000 - dat.frame.subsec;
	data_s.ms_s= dat.frame.hour * 3600000 + dat.frame.min * 60000 + dat.frame.sec * 1000 + dat.frame.subsec;
	data_s.sensor_s = databuffer;
	times[4] = at_hal_micros();

    if(databuffer < 4095)
		{
			Radio.Send((uint8_t*)&data_s, (sizeof(data_s)));
			times[5] = at_hal_micros();

//			PRINTF("%d; %d; %d; %d;%d \n",dat.frame.hour,  dat.frame.min , dat.frame.sec, dat.frame.subsec, dat.frame.sensor);
//			PRINTF("[%d]%d\n",data_s.packetCount, dat.frame.sensor);
//			PRINTF("%d:%d\n",data_s.ms_s  , dat.frame.sensor);//,dat.frame.subsec, dat.frame.sec);
			PRINTF("%d:%d\n", data_s.ms_s, dat.frame.sensor,data_s.packetCount);
			times[6] = at_hal_micros();


		}
    else
    	{
    		garbageTxDataCount++;

    	}
    times[7] = at_hal_micros();


}

static void ledTimerCallback()
{
	//BSP_LED_Off(LED2);
}

void txDoneEventCallback()
{
//	BSP_LED_Off(LED3);
//    Radio.Rx(0);
	__NOP();
}


void rxDoneEventCallback(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	  //BSP_LED_On(LED2);
	  TimerStart(&ledTimer);
	  RTC_TimeTypeDef time ;
	  RTC_DateTypeDef date ;
	  HW_RTC_GetCalendarValue( &date , &time );
	  memcpy(Buffer,payload, (BUFFER_SIZE));
	  databuffer = HW_AdcReadChannel(ADC_CHANNEL_0);
//		if( (payload[7] + (payload[1]<<8)) > 4095 )  // Only 1 and 1 true so do nothing
//			{
//				garbageRxDataCount++ ;			// TODO: create a buffer and insert garbage datas into it
//			}
//		else
//			{
//			vcom_Send("Second: %d Minute: %d Data: %d"  ,tStruct->Seconds, tStruct->Minutes, payload[0] +  (payload[1]<<8));
//			struct Mpacket packetR;
//			dat.frame.subsec = Buffer[0] + ( Buffer[1] << 8 );
//			dat.frame.sec = Buffer[4];
//			dat.frame.min = Buffer[5];
//			dat.frame.hour = Buffer[6];
//			dat.frame.sensor = Buffer[7] + ( Buffer[8] << 8 );
//
//			vcom_Send("%d;%d;%d;%d;%d:\n", dat.frame.subsec, dat.frame.sec, dat.frame.min,
//															 dat.frame.hour, dat.frame.sensor);
	  	  	/**
	  	  	 * Compare receiver and transmiter time,adc values
	  	  	 */

	  	  	uint32_t subs_r = 1000 - (int)(time.SubSeconds * 1000 / (time.SecondFraction + 1));
	  	  	uint32_t ms_s_r = time.Hours * 3600000 + time.Minutes * 60000 + time.Seconds * 1000 + subs_r;

			data_s.ms_s = Buffer[0] | (Buffer[1] << 8) | (Buffer[2] << 16) | (Buffer[3] << 24);
			data_s.sensor_s = Buffer[4] + (Buffer[5] << 8);
	  	  	uint16_t packetCounter = Buffer[6] + (Buffer[7] << 8);

		//	vcom_Send("[%d]Transmitter : %d,%d Receiver : %d,%d\n",packetCounter, data_s.ms_s, data_s.sensor_s , ms_s_r,databuffer);
	  	  	/**
	  	  	 *  Print ADC data only therefore I should print only data_s.sensor_s
	  	  	 */
	  	    vcom_Send("%d:%d\n",data_s.ms_s, data_s.sensor_s);
}



int main(void)
{
	HAL_Init();
	SystemClock_Config();
	HW_Init();
	at_hal_init();

    /* GPIO init for PA11*/
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef  GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00300F38;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	  {
	    while(1);
	  };


	ssd1306_Init();
	HAL_Delay(10);
	ssd1306_Fill(White);
	HAL_Delay(10);
	ssd1306_UpdateScreen();
	HAL_Delay(10);

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
    data_s.packetCount = 0;

//
//	while(1){
//		RTC_TimeTypeDef time ;
//		RTC_DateTypeDef date ;
//
//		HW_RTC_GetCalendarValue( &date , &time );
//		vcom_Send("%d;%d;%d;%d;%d "  ,time.Hours, time.Minutes, time.Seconds, (int)(time.SubSeconds * 1000 / (time.SecondFraction + 1)), uwTick  );
//		PRINTF("\n");
//
//
//		uint64_t timeinmicro;
//		timeinmicro = at_hal_micros();
//		UNUSED(timeinmicro);
//		char buf[128];
//		snprintf (buf, sizeof (buf), " %lu ", uwTick);
//		ssd1306_SetCursor(1,23);
//		ssd1306_WriteString(buf,Font_7x10,White);
//		ssd1306_UpdateScreen();
//		//vcom_Send("Value is: %" PRIu64 "\n", timeinmicro);//"Value is: " PRIu64 "\n",timeinmicro);
//		//	HAL_UART_Transmit(&UartHandle,(uint8_t *)&timeinmicro, sizeof(timeinmicro), 300);
//	}

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
