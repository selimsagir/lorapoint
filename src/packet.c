/*
 * packet.c
 *
 *  Created on: Nov 22, 2019
 *      Author: asus
 */
#include <stdlib.h>
#include <strings.h>


		struct packet_s {
			uint8_t Subsecond;
			uint8_t Second;
			uint8_t Minute;
			uint8_t Hour;
			uint32_t uwTickl;
			uint16_t Sensor;
		};

