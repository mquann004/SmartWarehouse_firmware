#ifndef IOT_NETWORK_H
#define IOT_NETWORK_H

#include <stdbool.h>

// Hàm khởi tạo Wi-Fi
void wifi_init_sta(const char* ssid, const char* pass);

// Hàm khởi tạo MQTT
void mqtt_app_start(const char* broker_uri);

// Kiểm tra trạng thái kết nối MQTT
bool is_mqtt_connected_status(void);

// Hàm gửi dữ liệu lên MQTT Broker
void mqtt_publish_data(const char* topic, const char* payload);

#endif // IOT_NETWORK_H