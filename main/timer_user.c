/**
 *  Created by Bakeromso on 13/01/2020
 *
 *  This file contains the callback for the timer.
 *  Initially this will be the callback handling the debouncing of the buttons. Possibly more to come later.
 */

#include <esp_system.h>
#include "FreeRTOS.h"

#include "timer_user.h"
#include "hw_timer.h"
#include "stdbool.h"
#include "task.h"
#include "motor.h"
#include "user_defines.h"
#include "driver/gpio.h"


static const char *TAG = "TIMER";
//const int CLICKER_COUNT_MAX = 40; // only need to debounce for 40ms
const int CLICKER_INTEGRATION_MAX = 10;
const int CLICKER_INTEGRATION_DEFAULT = 5;
static const uint32_t debounceMicros = 1000; // 1 ms integration periods
static bool lastEdge;

volatile bool integratorClickerIsEnabled = false;
volatile int integratorClicker = 0;


// als die  naar het ANDERE level gaat, dan roep je debouncetimercallback(level)

void hw_timer_callback(void *arg)
{
    gpio_set_level(PIN_DEBUG_1, HIGH);
    if(integratorClickerIsEnabled){
        // poll the clicker level
        int level = gpio_get_level(PIN_CLICKER);
        // level is high and integrator is not maximum value yet
        if ((level == HIGH) && (integratorClicker < CLICKER_INTEGRATION_MAX)){
            integratorClicker++;
            // integrator reached maximum and the previous edge was a falling edge
            if((integratorClicker == CLICKER_INTEGRATION_MAX) && (lastEdge == false)){
                lastEdge = true; // update the lastEdge value since we just detected rising edge
                debounceTimerCallback(true);
            }
        }
        else if((level == LOW) && (integratorClicker > 0)){ // level == LOW
            gpio_set_level(PIN_DEBUG_0, HIGH);
            integratorClicker--;
            if((integratorClicker == 0) && lastEdge == true) {
                lastEdge = false;
                debounceTimerCallback(false);
            }
            gpio_set_level(PIN_DEBUG_0, LOW);
        }
    }
    else{ // in the case that no integrators were enabled, disable the hardware timer
        esp_err_t ret = hw_timer_disarm();
        if(ret != ESP_OK){
            // don't know how else to handle this error from here
            esp_restart();
        }
    }
    gpio_set_level(PIN_DEBUG_1, LOW);
}

/**
 * This function is called from the clicker interrupt ISR
 * Maybe should be renamed to something like set or reset counter
 */
void integrator_clicker_enable(){
    taskENTER_CRITICAL();
    // Set the timer. If timer was already set, it will override
    esp_err_t ret = hw_timer_alarm_us(debounceMicros,TIMER_RELOAD);
    if(ret != ESP_OK){
        // don't know how else to handle this error from here
        esp_restart();
    }
    integratorClickerIsEnabled = true;
    // set initial value for the integrator, so that we can focus on an edge from thereon
    if(gpio_get_level(PIN_CLICKER)){
        integratorClicker = CLICKER_INTEGRATION_MAX;
        lastEdge = true;
    }
    else{
        integratorClicker = 0;
        lastEdge = false;
    }
    taskEXIT_CRITICAL();
}

/**
 * This function is called from the clicker interrupt ISR
 * Maybe should be renamed to something like set or reset counter
 */
void integrator_clicker_disable(){
    taskENTER_CRITICAL();
    integratorClickerIsEnabled = false;
    taskEXIT_CRITICAL();
}
