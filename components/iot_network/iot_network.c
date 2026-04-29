#include "iot_network.h"
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "IOT_NETWORK";


static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool is_mqtt_connected = false;

// --- XỬ LÝ SỰ KIỆN WIFI ---
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Mat ket noi Wi-Fi, dang thu lai...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Da ket noi Wi-Fi thanh cong!");
    }
}

void wifi_init_sta(const char* ssid, const char* pass) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
    
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t)); // Xóa sạch dữ liệu rác trong struct
    
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, pass);
    
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

// --- XỬ LÝ SỰ KIỆN MQTT ---
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    if (event_id == MQTT_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "Da ket noi MQTT Broker!");
        is_mqtt_connected = true;
    } else if (event_id == MQTT_EVENT_DISCONNECTED) {
        ESP_LOGI(TAG, "Mat ket noi MQTT Broker!");
        is_mqtt_connected = false;
    }
}

void mqtt_app_start(const char* broker_uri) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = broker_uri
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

bool is_mqtt_connected_status(void) {
    return is_mqtt_connected;
}

void mqtt_publish_data(const char* topic, const char* payload) {
    if (is_mqtt_connected && mqtt_client != NULL) {
        esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);
        ESP_LOGI(TAG, "Da gui MQTT: %s", payload);
    }
}