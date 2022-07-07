/**
 *  Created by Bakeromso on 13/01/2020
 */

#include "esp_log.h"


#ifndef PETFEEDER_TIMER_USER_H
#define PETFEEDER_TIMER_USER_H

#define TIMER_ONE_SHOT    false
#define TIMER_RELOAD      true

/**
 * Callback to organize all the different things to do when the timer expires
 * When any of the buttons are to be debounce, the timer is enabled and set to reload every 5ms
 * Each button to be debounced, should have its debounce time enabled (factor of 5ms)
 * In the button interrupt (with highest priority, no further interrupts) the timer is enabled if it was disabled. Also
 * from that button interrupt, the debouncing of the related buttons is started. This is done by setting the
 * button counter to the debounce time, see example below. When the debounce time expires, the related debounce
 * callback function is called. If the same button interrupt is triggered again, the counter is reset. This is done
 * by restoring the counter value to its initial value.
 *
 * Example in pseudocode:
 *
 * <button_1 interrupt is triggered>
 * [in button_1 interrupt]
 * counter_button_1_enable
 *
 * [in counter_button_1_enable]
 * disable all interrupts
 * if not hw_timer enabled
 *  enable hw_timer
 * counter_button_1_enable
 * counter_button_1_reset
 * enable interrupts again
 *
 * [in counter_button_1_enable]
 * set some flag to keep track of which buttons are to be debounced
 *
 * [in counter_button_1_reset]
 * no matter the current value of counter_button_1 counter, set it to its debounce time again. So if the button
 * should debounce for 35 ms, its counter is set to 7 (since the timer is called every 5ms).
 *
 * [in hw_timer_callback]
 * decrement the counters of the buttons that are to be debounced (which timer is enables)
 * if an enabled counter reaches 0, perform the counter callback
 *
 * [in counter_1_callback]
 * notify the blocked task
 *

 *
 * @param arg
 */
void hw_timer_callback(void *arg);
void integrator_clicker_enable();
void integrator_clicker_disable();


#endif //PETFEEDER_TIMER_USER_H
