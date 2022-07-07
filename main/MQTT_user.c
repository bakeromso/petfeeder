/**
 *  Created by Bakeromso on 13/01/2020
 *
 */

#include "esp_log.h"

#include "MQTT_user.h"
#include "MQTT_task.h"
#include "mqtt_client.h"
#include "motor.h"
#include "message_buffer.h"
#include "scale.h"
#include "math.h"


#define TICK "/petfeeder/calibrate/tick"
#define RAW_REQ "/petfeeder/calibrate/raw/req"
#define RAW_RESP "/petfeeder/calibrate/raw/resp"



static const char *TAG = "MQTT_USER";
//static const int RAW_DECIMALS = 2 * sizeof(long); // how many decimal positions a long value can consist of
//// way to conservative, but alternative would be log10 calculation here




void mqtt_user_event_connected(esp_mqtt_client_handle_t client){
    esp_mqtt_client_subscribe(client, TICK, 1);
    esp_mqtt_client_subscribe(client, RAW_REQ, 2);
    ESP_LOGI(TAG, "sent subscribe successful");
}

void mqtt_user_event_disconnected(esp_mqtt_client_handle_t client){

}

void mqtt_user_event_subscribed(esp_mqtt_client_handle_t client){

}

void mqtt_user_event_published(esp_mqtt_client_handle_t client){

}

void mqtt_user_event_data(esp_mqtt_event_handle_t event, esp_mqtt_client_handle_t client){
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    if(strncmp(event->topic, TICK, strlen(TICK)) == 0){ // if the incoming event is of the topic TICK
        ESP_LOGI(TAG, "Turn the motor on for one tick");
        if(xSemaphoreGive(xSemaphoreTicks) != pdTRUE){
            // An error occurred, probably the queue was full
            ESP_LOGW(TAG, "xSemaphoreTicks could not be given, queue probably full");
        }
    }
    else if(strncmp(event->topic, RAW_REQ, strlen(RAW_REQ)) == 0) { // if the incoming event is of the topic RAW
        ESP_LOGI(TAG, "Request for raw data");
        // Received a request for the raw scale value, so unblock that task
        xTaskNotifyGive(xHandleScaleRaw);
    }
}

void mqtt_user_event_error(esp_mqtt_client_handle_t client){

}

void mqtt_user_event_before_connect(esp_mqtt_client_handle_t client){

}
void mqtt_user_event_any(esp_mqtt_client_handle_t client){

}

void mqtt_user_event_deleted(esp_mqtt_client_handle_t client){

}

void mqtt_publish_raw_scale(long raw) {
    // value can't be 0, so give it an offset and make it positively increasing
    raw = (long) (raw - (long) 200000);
    raw = raw * -1;
    ESP_LOGI(TAG, "Corrected raw value = %ld", raw);
    if (raw < 0) {
        ESP_LOGW(TAG, "mqtt_publish_raw_scale must not be fed a negative argument, argument= %ld", raw);
        int ret = mqtt_publish(RAW_RESP, "0", 0, 2, 0);
        if (ret == -1) { // -1 is returned when error, otherwise message ID is returned
            ESP_LOGE(TAG, "MQTT_publish returned %d", ret);
        }
    } else {
        // determine how many digits the input value consists of by computing the log10 value and rounding it up
        int nr_of_decimals = (int) ceil(log10((double) raw));
        char str_raw[nr_of_decimals + 1]; // make the array large enough and include space for null terminator
        snprintf(str_raw, nr_of_decimals + 1, "%ld", raw); // +1 for null terminator
        str_raw[nr_of_decimals] = 0; // force null termination
        int ret = mqtt_publish(RAW_RESP, str_raw, 0, 2, 0);
        if (ret == -1) {
            ESP_LOGE(TAG, "MQTT_publish returned %d", ret);
        }
    }
}

