/**
 * scale.c
 *
 *  Created on: Apr 13, 2020
 *      Author: Bakeromso
 *
 */


#include <limits.h>
#include "freertos/FreeRTOS.h"
#include "HX711.h"
#include "scale.h"
#include "esp_log.h"
#include "user_defines.h"
#include "MQTT_user.h"

static const char *TAG = "SCALE";

_Noreturn void vTaskScale(void *arg){
    ESP_LOGI(TAG, "Setting up scale");
    begin(GPIO_OUTPUT_IO_D6, GPIO_OUTPUT_IO_D7, 128);

    BaseType_t ret;
    for(;;){
        BaseType_t ret;
        xTaskNotifyStateClear(xHandleScaleRaw); // clear any pending notifications
        // wait for MQTT request for raw value
        ret = xTaskNotifyWait(0x00,
                              ULONG_MAX,
                              NULL,
                              portMAX_DELAY); // wait indefinitely
        if(ret == pdFALSE) { //timeout occurred, interrupt didn't happen
            //TODO improve handling when timeout happened
            ESP_LOGW(TAG, "Timeout occurred waiting for raw calibration scale request!");
        }
        else{
            ESP_LOGI(TAG, "Successfully requested raw value");
        }
        // Perform raw value measurement
        //power up (power cycle needed according to https://community.particle.io/t/strange-behavior-of-load-cell-hx711/49566/4
        power_up();
        long value = read_average(10);
        ESP_LOGI(TAG, "Value read: %ld", value);
        power_down();
        // send measurement over MQTT
        mqtt_publish_raw_scale(value);
    }
}

_Noreturn void vTaskScaleLive(void *arg){
    ESP_LOGI(TAG, "Setting up scale");
    begin(GPIO_OUTPUT_IO_D6, GPIO_OUTPUT_IO_D7, 128);

    BaseType_t ret;
    for(;;){
        vTaskDelayMs(5000);
        // Perform raw value measurement
        long value = get_reading();
        ESP_LOGI(TAG, "Value read: %ld, 0x%02x, 0x%02x, 0x%02x, 0x%02x", value, (uint8_t) value, \
        (uint8_t) (value >> 8), (uint8_t) (value >> 16), (uint8_t) (value >> 24));
        // send measurement over MQTT
        // TODO find out why this causes a crash when timeout is 2000
        mqtt_publish_raw_scale(value);
    }
}



_Noreturn void vTaskTestScale(void *arg){
    ESP_LOGI(TAG, "Setting up scale");
    begin(GPIO_OUTPUT_IO_D6, GPIO_OUTPUT_IO_D7, 128);
    ESP_LOGI(TAG, "Taring...");
    tare(100); // tare, has busy wait for scale to be ready.
    //TODO increase this value in release code
    ESP_LOGI(TAG, "Taring complete. Offset = %ld", get_offset());
    long value;
    long offset;
    double scale;
    float units;
    for(;;) {
        ESP_LOGI(TAG, "Measuring...");
//        value = get_value(10);
//        offset = get_offset();
//        scale = get_scale();
//        ESP_LOGI(TAG, "Value = %ld, offset = %ld, scale = %lf", value, offset, scale);
        ESP_LOGI(TAG, "Units = %f grams", get_units(10));
        vTaskDelayMs(1000);
//        calibrate();
    }
}

void calibrate(void){
    uint16_t depth = 200;
    ESP_LOGI(TAG, "Make sure there is no wait on the scale");
    vTaskDelayMs(5000);
    ESP_LOGI(TAG, "Start taring (longer than on startup)...");
    tare(depth);
    ESP_LOGI(TAG, "Taring complete. Offset = %ld", get_offset());
    ESP_LOGI(TAG, "Place wait on the scale");
    vTaskDelayMs(5000);
    ESP_LOGI(TAG, "Starting calibration...");
    ESP_LOGI(TAG, "Calibrate value = %f", get_value(depth));
}