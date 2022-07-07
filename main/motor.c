/**
 * motor.c
 *
 *  Created on: Apr 13, 2020
 *      Author: Bakeromso
 *
 */

#include <limits.h>

/**
 *  Created by Bakeromso on 13/01/2020
 *  This file contains the code to interract with the motor
 *
 *  Note: ideally xTaskNotifyWaitIndexed would have been used to communicate between MQTT handler
 *  and the motor task. In this case, the notifications could also be used by the
 *  handler instead of the semaphore (since notifications are more efficient than
 *  semaphores). Since the call is made most frequently from interrupt to debouncer, the
 *  single indexed notification mechanism xTaskNotifyWait is used here, and a semaphore
 *  to communicate between MQTT handler and the motor task
 */

#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_system.h"

#include "motor.h"
#include "user_defines.h"
#include "timer_user.h"

esp_timer_handle_t timerHandle;
volatile TickType_t lastRisingEdge;
volatile TickType_t lastFallingEdge;
volatile int clickerLevel;
static const char *TAG = "MOTOR";


/**
 *  Interrupt that is called by the system whenever there is an [type of] edge from the clicker button
 *  After this interrupt has been triggered, it still needs to be debounced. This
 *  is done by setting a notification to the debouncer from this handler,
 *  and then leaving this handler.
 *
 *  https://www.switchdoc.com/2018/04/esp32-tutorial-debouncing-a-button-press-using-interrupts/
 *
 */
//void IRAM_ATTR handleClickerInterrupt(void *arg){
//    UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
//    gpio_set_level(PIN_DEBUG_0, HIGH);
//    // TODO handle if resetting the timer happens too often, so debouncing takes too long
//    counter_clicker_enable();
//    gpio_set_level(PIN_DEBUG_0, LOW);
//    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
//}

/**
 * Debouncer, blocks until a debounced edge is detected
 */
void blockDebouncedEdge(){
    BaseType_t ret = xTaskNotifyStateClear(xHandleMotorTick); // clear any pending notifications from the interrupt
    ESP_LOGI(TAG,"Notify state clear result was %d", ret);
    // enable the clicker counter
    integrator_clicker_enable();
    // blocking wait for signal from interrupt
    ret = xTaskNotifyWait(0x00,
                                     ULONG_MAX,
                                     NULL,
                                     10000 / portTICK_RATE_MS); // generate timeout if waiting for interrupt took longer than 10 sec
    if(ret == pdFALSE) { //timeout occurred, interrupt didn't happen
        //TODO improve handling when timeout happened
        ESP_LOGW(TAG, "Interrupt timeout occurred!");
    }
    else{
        ESP_LOGI(TAG, "Successfully debounced");
    }
    integrator_clicker_disable();
}

/**
 * Turn the motor for one tick
 * More specifically, turn until a tick is detected
 */
void motorTick(){
    gpio_set_level(PIN_MOTOR, HIGH);
    ESP_LOGI(TAG, "Start blocking for debounced edge");
    blockDebouncedEdge();
    ESP_LOGI(TAG, "Finished blocking for debounced edge");
    gpio_set_level(PIN_MOTOR, LOW);
}

/**
 * This function is called when the debounce timer has expired.
 * Only from the clicker ISR is the timer (re)set. So when this callback
 * is executed, there has not been an interrupt edge for a set time.
 * In other words, the debounce time has passed and the button is debounced.
 *
 * Note: this is called from ISR, so can't have any print/ESP_LOG
 */
void debounceTimerCallback(bool edge){
    // Handle the fact that the clicker is debounced.

    // check if we should even handle this bounce (clicker configured to any edge, or to low and the
    // level currently is low, or to high and the level currently is high)
    if((CLICKER_EDGE == GPIO_INTR_ANYEDGE) ||
        ((edge == LOW) && (CLICKER_EDGE == GPIO_INTR_NEGEDGE)) ||
        ((edge == HIGH) && (CLICKER_EDGE == GPIO_INTR_POSEDGE))){
        // send out notification so that blocking on debounce can stop
        gpio_set_level(PIN_DEBUG_3, HIGH);
        xTaskNotifyGive(xHandleMotorTick); // notify the debouncer
        gpio_set_level(PIN_DEBUG_3, LOW);
    }
}

/**
 * This function receives requests to tick from other tasks in it's queue
 * For testing purposes, the queue is left out and the tick just reoccurs
 * after a delay
 *
 * @param arg
 */
_Noreturn void vTaskMotorTick(void *arg){
    UBaseType_t count;
    for(;;){
        count = uxSemaphoreGetCount(xSemaphoreTicks);
        ESP_LOGI(TAG, "Count before blocking %d", count);
        xSemaphoreTake(xSemaphoreTicks,portMAX_DELAY); // block indefinitely for command to tick
        motorTick();
        // no xSemaphoreRelease or something?
    }
}

_Noreturn void vTaskTestMotor(void *arg){
    for(;;){
        ESP_LOGI(TAG, "Turning on the motor");
        gpio_set_level(PIN_MOTOR, HIGH);
        gpio_set_level(PIN_LED, LOW); // Active LOW
        vTaskDelayMs(10000);
        ESP_LOGI(TAG, "Turning off the motor");
        gpio_set_level(PIN_MOTOR, LOW);
        gpio_set_level(PIN_LED, HIGH);
        vTaskDelayMs(3000);
    }
}