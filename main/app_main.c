#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "mqtt_manager.h"

static const char *TAG = "MQTT_SAMPLE";
void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Example WiFi connection.
    ESP_ERROR_CHECK(example_connect());

    QueueHandle_t mqttPublishQueue = mqtt_manager_init();

    if (mqttPublishQueue == NULL) {
        ESP_LOGE(TAG, "MQTT init error.");
    }

    MqttPublishMessage_t publish_msg = {
            .topic = "test_topic2/qos2",
            .message = "data",
            .qos = 2
    };

    // Subscribe for topic /test/topic
    QueueHandle_t test_topic_queue = mqtt_manager_subscribe("test/topic", 0);

    while (1) {
        ESP_LOGI(TAG, "Publishing message.");
        if (xQueueSend(mqttPublishQueue, &publish_msg, portMAX_DELAY) != pdPASS) {
            ESP_LOGE(TAG, "Error sending to the queue");
        }

        ESP_LOGI(TAG, "Receiving message.");
        MqttSubscribeMessage_t rcvd_msg;
        // Receive message from /test/topic
        if (xQueueReceive(test_topic_queue, &rcvd_msg, (100 / portTICK_PERIOD_MS)) == pdPASS) {
            ESP_LOGI(TAG, "Topic: %s, message: %.*s", rcvd_msg.topic, rcvd_msg.message_len, rcvd_msg.message);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }



}