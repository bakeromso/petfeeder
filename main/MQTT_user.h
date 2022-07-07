/**
 *  Created by Bakeromso on 13/01/2020
 *
 *  MQTT user contains all the project specific MQTT files
 *
 */

#ifndef PETFEEDER_MQTT_USER_H
#define PETFEEDER_MQTT_USER_H

#include "mqtt_client.h"


void mqtt_user_event_connected(esp_mqtt_client_handle_t client);
void mqtt_user_event_disconnected(esp_mqtt_client_handle_t client);
void mqtt_user_event_subscribed(esp_mqtt_client_handle_t client);
void mqtt_user_event_published(esp_mqtt_client_handle_t client);
void mqtt_user_event_data(esp_mqtt_event_handle_t event, esp_mqtt_client_handle_t client);
void mqtt_user_event_error(esp_mqtt_client_handle_t client);
void mqtt_user_event_before_connect(esp_mqtt_client_handle_t client);
void mqtt_user_event_any(esp_mqtt_client_handle_t client);
void mqtt_user_event_deleted(esp_mqtt_client_handle_t client);

void mqtt_publish_raw_scale(long raw);

#endif //PETFEEDER_MQTT_USER_H
