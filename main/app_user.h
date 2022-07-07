//
// Created by user on 3/15/21.
//

#ifndef PETFEEDER_APP_USER_H
#define PETFEEDER_APP_USER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t xHandleAppUser;
_Noreturn void vTaskAppUser(void *arg);

#endif //PETFEEDER_APP_USER_H
