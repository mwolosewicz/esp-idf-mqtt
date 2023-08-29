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
- Connect to MQTT broker using TLS-PSK.

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
### Choose your architecture
Please choose your architecture, for ESP32-S3 use `idf.py set-target esp32s3`

### Config the MQTT Manager parameters
Please use `idf.py menuconfig` to set the required parameters.

The parameters can be found in `MQTT Manager Configuration`.

- *MAX_TOPIC_LENGTH*
- *MAX_MESSAGE_LENGTH*
- *MAX_SUBSCRIBED_TOPICS* - be careful here, this is quite memory consuming.
- *BROKER_URL*
- *BROKER_PORT*
- *BROKER_USER_NAME*
- *BROKER_PASSWORD*
- *MQTT_PUBLISHING_QUEUE_SIZE*
- *TLS_PSK*

#### **NOTE** about TLS_PSK:

- If the TLS_PSK is set, then the KEY and HINT should be set in mqtt_manager_tls_psk.h file.
- The protocol in menuconfig for TLS-PSK connection should be set to `mqtts://` instead of `mqtt://`, so use address
`mqtts://test.mosquitto.org` instead of `mqtt://test.mosquitto.org`
- Usually the port number for secure connection is 8883.
- The ESP-TLS should be enabled in menuconfig to use TLS-PSK.

#### Configure of Mosquitto broker for TLS-PSK connection
Below you can find the example configuration of mosquitto broker for TLS-PSK connection.
```
$cat conf.d/psk.conf
per_listener_settings true

# Listener for Internet connections on port 8883 with PSK
listener 8883
psk_hint hint
psk_file /etc/mosquitto/psk.txt
allow_anonymous true

# Listener for local network connections on port 1883 without PSK
listener 1883
allow_anonymous true
```

And the psk.txt file:
```
$cat psk.txt 
psk_identity:70a3c4271ceacd2d6dc1b0418b7e85fc
```

The `psk_identity` is a kind of user name, and the `psk_identity` is a key used to get into the broker. The file can
contain more than one user name and key, which can be used to easier manage the access to the broker. For example
you can use a MAC address or unique ID instead of `psk_identity` to identify the device.

For generation of the key you can use the following command: `openssl rand -hex 16` which will generate a 128-bit key.
Which should be enough for most of the cases.

For more advanced security you should use the full TLS implementation. The TLS-PSK is fast and easy to setup, but it
is not the most secure way to go.

There is nothing against using the TLS-PSK and the user name and password at the same time to level up security a
little bit.

Here you can read about [configuration of Mosquitto broker](https://www.howtoforge.com/how-to-install-mosquitto-mqtt-message-broker-on-debian-11/)
in more details.

### Setup WiFi connection
Please use `idf.py menuconfig` to set the required parameters.

The configuration of WiFi connection can be found in: `Example Connection Configuration` menu.

## Contributing
Feel free to contribute to this project by submitting issues, or making pull requests. All contributions are welcome.

## License
This project is licensed under the MIT License.

