# MQTT Manager
## Overview
The MQTT Manager is an ESP-IDF component designed to simplify handling MQTT communication for your ESP32 or ESP-IDF
compatible microcontroller. It uses FreeRTOS tasks and queues to provide a simple way to publish and subscribe to
MQTT topics. The manager supports publishing messages and subscribing to topics, along with basic error handling.

No dynamic allocation - the size of queues and other stuff can be configured using `idf.py menuconfig`.

## Features
- Publish to MQTT topics with varying QoS. 
- Subscribe to MQTT topics with configurable QoS.
- Unsubscribe from MQTT topics.
- Handle MQTT messages and topics in a simplified way.
- Manage MQTT topics subscriptions.

## API
### MQTT Manager Initialization
```c
QueueHandle_t mqtt_manager_init();
```

This function initializes the MQTT Manager. It will return a queue handle for the MQTT publish queue.
To send data, push a `MqttPublishMessage_t` to the queue. Note that ESP-IDF MQTT looks for null termination,
so if you plan to send binary data - every 0 will break the message. In that situation consider BASE64 instead
of pure binary data.

### MQTT Manager Subscription
```c
QueueHandle_t mqtt_manager_subscribe(const char* topic, int qos);
```

This function will allow you to subscribe to an MQTT topic with a specified QoS. It returns a queue handle for the
MQTT subscribe queue. If the subscription fails, it will return NULL.

### MQTT Manager Unsubscription
```c
void mqtt_manager_unsubscribe(const char* topic);
```

This function will allow you to unsubscribe from an MQTT topic.

## How to use

1. Include the MQTT Manager header file in your code: `#include "mqtt_manager.h"`
2. Call `mqtt_manager_init()` function to initialize the MQTT Manager.
3. Call `mqtt_manager_subscribe()` function to subscribe to a topic and receive messages.
4. Call `mqtt_manager_unsubscribe()` to unsubscribe from a topic.

Please refer to the example code provided for detailed usage.

## Prerequisites
Please use `idf.py menuconfig` to set the required parameters.

### Config the MQTT Manager parameters

The parameters can be found in `MQTT Manager Configuration`.

- *MAX_TOPIC_LENGTH*
- *MAX_MESSAGE_LENGTH*
- *MAX_SUBSCRIBED_TOPICS* - be careful here, this is quite memory consuming .
- *BROKER_URL*
- *BROKER_USER_NAME*
- *BROKER_PASSWORD*
- *MQTT_PUBLISHING_QUEUE_SIZE*

### Setup WiFi connection

The configuration of WiFi connection can be found in: `Example Connection Configuration` menu.

## Contributing
Feel free to contribute to this project by submitting issues, or making pull requests. All contributions are welcome.

## License
This project is licensed under the MIT License.

