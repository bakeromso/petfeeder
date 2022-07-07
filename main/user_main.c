/**
 * user_main.c
 *
 *  Created on: Apr 13, 2020
 *      Author: Bakeromso
 *
 */

#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <hw_timer.h>

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/message_buffer.h"

#include "esp_log.h"

#include "user_defines.h"
#include "MQTT_task.h"
#include "motor.h"
#include "HX711.h"
#include "scale.h"
#include "timer_user.h"
#include "app_user.h"


static const char *TAG = "MAIN";
//TaskHandle_t xHandleTestMotor;
//TaskHandle_t xHandleTestScale;

esp_err_t ret;


_Noreturn void app_main()
{
////// --------------- START GPIO PART ----------------------/////
    // configure gpio output
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //bit mask of the pins
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    //enable interrupt
    io_conf.intr_type = CLICKER_EDGE;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    printf("GPIO has been configured, hello world!\n");
    gpio_set_level(PIN_DEBUG_0, LOW);
    gpio_set_level(PIN_DEBUG_1, LOW);
    gpio_set_level(PIN_DEBUG_2, LOW);
    gpio_set_level(PIN_DEBUG_3, LOW);
    gpio_set_level(PIN_LED, HIGH);      // active low


////// --------------- END GPIO PART ----------------------/////

////// --------------- START ISR PART ----------------------/////
    // install generic ISR with gpio_install_isr_service
    esp_err_t ret = gpio_install_isr_service(0);
    if(ret != ESP_OK){
        //no way to handle this, just reboot
        ESP_LOGE(TAG, "gpio_install_isr_service did not return EPS_OK, return value %d", ret);
        esp_restart();
    }
    ESP_LOGI(TAG, "Successfully installed gpio ISR service");

    // create counting semaphore to hold requests of revolving the motor until the next click
    xSemaphoreTicks = xSemaphoreCreateCounting(10,0);
    if(xSemaphoreTicks == NULL){
        //no way to handle this, just reboot
        ESP_LOGE(TAG, "xSemaphoreTicks could not be created %d", ret);
        esp_restart();
    }
//    // create binary semaphore to hold requests of  the motor until the next click
//    xSemaphoreTicks = xSemaphoreCreateCounting(10,0);
//    if(xSemaphoreTicks == NULL){
//        //no way to handle this, just reboot
//        ESP_LOGE(TAG, "xSemaphoreTicks could not be created %d", ret);
//        esp_restart();
//    }

    // install callback for hardware timer
    ret = hw_timer_init(hw_timer_callback, NULL);
    if(ret != ESP_OK){
        //no way to handle this, just reboot
        ESP_LOGE(TAG, "hw_timer_init did not return EPS_OK, return value %d", ret);
        esp_restart();
    }


    ESP_LOGI(TAG, "Successfully initialized hardware timer callback");


////// --------------- END ISR PART ----------------------/////c

//    xTaskCreate(vTaskTestMotor, "vtask test motor", 2048, NULL, 10, &xHandleTestMotor);
//    xTaskCreate(vTaskTestScale, "vtask test scale", 2048, NULL, 10, &xHandleTestScale);
//    xTaskCreate(vTaskScale, "vtask scale raw", 2048, NULL, 10, &xHandleScaleRaw);
    xTaskCreate(vTaskScaleLive, "vtask scale raw", 2048, NULL, 10, &xHandleScaleRaw);
    xTaskCreate(vTaskMotorTick, "vtask motor tick", 2048, NULL, 10, &xHandleMotorTick);
    xTaskCreate(vTaskAppUser, "vtask for the user main user application", 2048, NULL, 10, &xHandleAppUser);


//// --------------- START MQTT PART ----------------------/////
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    nvs_flash_init();
    wifi_init();
    mqtt_task_start();

//// --------------- END MQTT PART ----------------------/////

    while (1) {
        vTaskDelayMs(3000);
    }

}


