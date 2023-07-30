#ifndef MQTT_MANAGER_H_
#define MQTT_MANAGER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef struct {
    char topic[CONFIG_MAX_TOPIC_LENGTH];
    char message[CONFIG_MAX_MESSAGE_LENGTH];
    int qos;
} MqttPublishMessage_t;

typedef struct {
    char topic[CONFIG_MAX_TOPIC_LENGTH];
    int topic_len;
    char message[CONFIG_MAX_MESSAGE_LENGTH];
    int message_len;
} MqttSubscribeMessage_t;

/**
 * @brief Initialization of the MQTT manager.
 *
 * To send data, just push a MqttPublishMessage_t to the queue.
 * Note that ESP-IDF MQTT look for null termination, so if you
 * plan to send binary data - the every 0 will brake the message.
 * In that situation consider BASE64 instead of pure binary data.
 *
 * @return QueueHandle_t The queue handle for the MQTT publish queue.
 */
QueueHandle_t mqtt_manager_init();

/**
 * @brief Subscribe to a topic.
 *
 * @param topic The topic to subscribe to.
 * @arg qos The quality of service for the subscription.
 * @return QueueHandle_t The queue handle for the MQTT subscribe queue. NULL if failed.
 */
QueueHandle_t mqtt_manager_subscribe(const char* topic, int qos);

/**
 * @brief Unsubscribe from a topic.
 *
 * @param topic The topic to unsubscribe from.
 */
void mqtt_manager_unsubscribe(const char* topic);

#endif // MQTT_MANAGER_H_