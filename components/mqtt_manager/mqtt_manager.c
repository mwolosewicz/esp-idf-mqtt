#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "mqtt_client.h"

#include "mqtt_manager.h"

#define min(a,b) ((a) < (b) ? (a) : (b))

static const char *TAG = "MQTT_MANAGER";

static QueueHandle_t s_mqttPublishQueue = NULL;
static esp_mqtt_client_handle_t s_client = NULL;
static char s_subscribed_topics[CONFIG_MAX_SUBSCRIBED_TOPICS][CONFIG_MAX_TOPIC_LENGTH];
static QueueHandle_t s_subscription_queues[CONFIG_MAX_SUBSCRIBED_TOPICS];


static void mqtt_publisher_task(void *pvParameters) {
    MqttPublishMessage_t msg;

    while (1) {
        if (xQueueReceive(s_mqttPublishQueue, &msg, portMAX_DELAY) == pdPASS) {
            esp_mqtt_client_handle_t s_client = (esp_mqtt_client_handle_t)pvParameters;
            int msg_id = esp_mqtt_client_publish(s_client, msg.topic, msg.message, strlen(msg.message), msg.qos, 0);
            ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
        }
    }
}

static bool check_if_there_is_space_for_new_topic(void) {
    for (int i = 0; i < CONFIG_MAX_SUBSCRIBED_TOPICS; i++) {
        if (strlen(s_subscribed_topics[i]) == 0) {
            return true;
        }
    }
    return false;
}

static bool check_topic_len(const char* topic) {
    if (strlen(topic) > CONFIG_MAX_TOPIC_LENGTH) {
        return false;
    }
    return true;
}

static void unregister_topic(const char* topic) {
    for (int i = 0; i < CONFIG_MAX_SUBSCRIBED_TOPICS; i++) {
        if (strcmp(s_subscribed_topics[i], topic) == 0) {
            memset(s_subscribed_topics[i], 0, CONFIG_MAX_TOPIC_LENGTH);
            return;
        }
    }
}

static bool check_if_already_subscribed(const char* topic) {
    for (int i = 0; i < CONFIG_MAX_SUBSCRIBED_TOPICS; i++) {
        if (strcmp(s_subscribed_topics[i], topic) == 0) {
            return true;
        }
    }
    return false;
}

static void handle_message(const char* topic, int topic_len, const char* message, int message_len) {
    for (int i = 0; i < CONFIG_MAX_SUBSCRIBED_TOPICS; i++) {
        if (strncmp(s_subscribed_topics[i], topic, min(topic_len, CONFIG_MAX_TOPIC_LENGTH)) == 0) {
            MqttSubscribeMessage_t msg;

            memset(&msg, 0, sizeof(msg)); // Clear the struct to ensure clean data.

            int length = min(topic_len, CONFIG_MAX_TOPIC_LENGTH - 1);
            strncpy(msg.topic, topic, length);
            msg.topic[length] = '\0';
            msg.topic_len = length;

            length = min(message_len, CONFIG_MAX_MESSAGE_LENGTH - 1);
            strncpy(msg.message, message, length);
            msg.message_len = length;

            if(xQueueSendToBack(s_subscription_queues[i], &msg, (10 / portTICK_PERIOD_MS)) != pdPASS) {
                ESP_LOGW(TAG, "Queue is full. Could not send message to topic: %s", topic);
            }
            return;
        }
    }
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGD(TAG, "MQTT_EVENT_CONNECTED");
            // TODO: Implement if needed.
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "MQTT_EVENT_DISCONNECTED");
            // TODO: Implement if needed.
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGD(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGD(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            // TODO: Implement if needed.
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            // TODO: Implement if needed.
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGD(TAG, "MQTT_EVENT_DATA");
            handle_message(event->topic, event->topic_len, event->data, event->data_len);
            break;
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGD(TAG, "MQTT_EVENT_BEFORE_CONNECT");
            // TODO: Implement if needed.
            break;
        default:
            ESP_LOGD(TAG, "Other event id:%d", event->event_id);
            // TODO: Implement if needed.
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
            .broker = {
                    .address = {
                            .uri = CONFIG_BROKER_URL
                    }
            },
            .credentials = {
                    .username = CONFIG_BROKER_USER_NAME,
                    .authentication = {
                            .password = CONFIG_BROKER_PASSWORD,
                    }
            }
    };

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    if (s_client != NULL) {
        esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, s_client);
        esp_mqtt_client_start(s_client);
    } else {
        ESP_LOGE(TAG, "MQTT client initialization failed");
    }
}

QueueHandle_t mqtt_manager_init(void) {
    mqtt_app_start();
    // Create publisher topic:
    s_mqttPublishQueue = xQueueCreate(CONFIG_MQTT_PUBLISHING_QUEUE_SIZE, sizeof(MqttPublishMessage_t));

    // Create subscriber queues
    for (int i = 0; i < CONFIG_MAX_SUBSCRIBED_TOPICS; i++) {
        s_subscription_queues[i] = xQueueCreate(CONFIG_MAX_SUBSCRIBED_TOPICS, sizeof(MqttSubscribeMessage_t));
    }

    xTaskCreate(mqtt_publisher_task, "mqtt_publisher_task", 4096, s_client, 1, NULL);
    return s_mqttPublishQueue;
}

QueueHandle_t mqtt_manager_subscribe(const char* topic, int qos) {
    if (!check_topic_len(topic)) {
        ESP_LOGE(TAG, "Topic: %s length is too long", topic);
        return NULL;
    }

    if (!check_if_there_is_space_for_new_topic()) {
        ESP_LOGW(TAG, "There is no space for new topic");
        return NULL;
    }

    if (check_if_already_subscribed(topic)) {
        ESP_LOGW(TAG, "Already subscribed to topic: %s", topic);
        return NULL;
    }

    for (int i = 0; i < CONFIG_MAX_SUBSCRIBED_TOPICS; i++) {
        if (strlen(s_subscribed_topics[i]) == 0) {
            strcpy(s_subscribed_topics[i], topic);
            // Actually subscribe to the MQTT topic.
            esp_mqtt_client_subscribe(s_client, topic, qos);
            return s_subscription_queues[i];
        }
    }
    return NULL;
}

void mqtt_manager_unsubscribe(const char* topic) {
    if (check_if_already_subscribed(topic)) {
        unregister_topic(topic);
        // Actually unsubscribe from the MQTT topic.
        esp_mqtt_client_unsubscribe(s_client, topic);
    } else {
        ESP_LOGW(TAG, "Not subscribed to topic: %s", topic);
    }
}