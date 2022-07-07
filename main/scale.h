/**
 * scale.h
 *
 *  Created on: Apr 13, 2020
 *      Author: Bakeromso
 *
 */

#include <limits.h>
#include <task.h>

#ifndef PETFEEDER_SCALE_H
#define PETFEEDER_SCALE_H

TaskHandle_t xHandleScaleRaw;

_Noreturn void vTaskTestScale(void *arg);
_Noreturn void vTaskScale(void *arg);
_Noreturn void vTaskScaleLive(void *arg);
void calibrate(void);



#endif //PETFEEDER_SCALE_H
