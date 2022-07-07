/*
 * user_defines.h
 *
 *  Created on: Mar 18, 2020
 *      Author: Bakeromso
 */

#ifndef MAIN_USER_DEFINES_H_
#define MAIN_USER_DEFINES_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

/**
 * Brief:
 *
 *
 * GPIO status:
 * D3 / GPIO0: 	output
 */

#define GPIO_OUTPUT_IO_D4   2   // Wemos pin D4 mapped to ESP8266 GPIO2, used for the builtin LED
#define GPIO_OUTPUT_IO_D1   5   // Motor output
#define GPIO_OUTPUT_IO_D7   13  // Blue cable, HX711 SCK
#define GPIO_OUTPUT_IO_D6   12  // Yellow cable, HX711 DAT
#define GPIO_OUTPUT_IO_D0   16  // debug pin connected to logic analyzer, to debug ISR
#define GPIO_OUTPUT_IO_D3   0   // debug
#define GPIO_OUTPUT_IO_D5   14  // debug
#define GPIO_OUTPUT_IO_D8   15  // debug

#define GPIO_INPUT_IO_D2   4   // clicker input pin (jitter on falling edge for 1.6ms to 3 ms )

#define PIN_CLICKER         GPIO_INPUT_IO_D2

#define PIN_LED             GPIO_OUTPUT_IO_D4
#define PIN_MOTOR           GPIO_OUTPUT_IO_D1
#define PIN_DEBUG_0         GPIO_OUTPUT_IO_D8
#define PIN_DEBUG_1         GPIO_OUTPUT_IO_D0
#define PIN_DEBUG_2         GPIO_OUTPUT_IO_D3
#define PIN_DEBUG_3         GPIO_OUTPUT_IO_D5

/*
 * Active high, peak normally takes about 500ms
 * trigger on either edge, see what stops the rotator on the right spot
 * delay of 5ms should be enough as minimum,
 */
#define GPIO_INPUT_PIN_SEL ((1ULL<<GPIO_INPUT_IO_D2))

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<PIN_LED) | (1ULL<<PIN_MOTOR) | \
                                (1ULL<<PIN_DEBUG_0) | (1ULL<<PIN_DEBUG_1) | \
                                (1ULL<<PIN_DEBUG_2) | (1ULL<<PIN_DEBUG_3))


#define HIGH                1
#define LOW                 0
#define vTaskDelayMs(milliseconds)     vTaskDelay(milliseconds / portTICK_RATE_MS)
#define CLICKER_EDGE        GPIO_INTR_NEGEDGE


#endif /* MAIN_USER_DEFINES_H_ */
