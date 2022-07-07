/*
 * MQTT_user.h
 *
 *  Created on: Apr 13, 2020
 *      Author: Bakeromso
 */

#ifndef MAIN_MQTT_USER_H_
#define MAIN_MQTT_USER_H_

bool isConnected(void);
void setConnected(bool conn);
int mqtt_publish(char* topic, char* data, int len, int qos, int retain);
void wifi_init(void);
void mqtt_task_start(void);

#endif /* MAIN_MQTT_USER_H_ */