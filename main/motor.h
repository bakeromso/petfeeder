/**
 * motor.h
 *
 *  Created on: Apr 13, 2020
 *      Author: Bakeromso
 *
 */
#ifndef PETFEEDER_MOTOR_H
#define PETFEEDER_MOTOR_H
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include <limits.h>
#include <semphr.h>

TaskHandle_t xHandleMotorTick;
SemaphoreHandle_t xSemaphoreTicks;
void IRAM_ATTR handleClickerInterrupt(void *arg);
_Noreturn void vTaskTestMotor(void *arg);
_Noreturn void vTaskMotorTick(void *arg);
void motorTick();

/**
 *
 * @param edge true when the edge is rising, false for falling edge
 */
void debounceTimerCallback(bool edge);

#endif //PETFEEDER_MOTOR_H
