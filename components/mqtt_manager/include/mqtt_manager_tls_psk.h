//
// Created by mw on 29.08.23.
//

#ifndef ESP_IDF_MQTT_MQTT_MANAGER_TLS_PSK_H
#define ESP_IDF_MQTT_MQTT_MANAGER_TLS_PSK_H

#if CONFIG_TLS_PSK
const uint8_t MQTT_TLS_PSK_KEY[] = {0x70, 0xa3, 0xc4, 0x27, 0x1c, 0xea, 0xcd, 0x2d, 0x6d, 0xc1, 0xb0, 0x41, 0x8b, 0x7e, 0x85, 0xfc};
const char MQTT_TLS_PSK_IDENTITY[] = "psk_identity";
#endif

#endif //ESP_IDF_MQTT_MQTT_MANAGER_TLS_PSK_H
