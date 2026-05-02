#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"

// --- THƯ VIỆN ĐÃ CHIA ---
#include "iot_network.h" // Thư viện Wi-Fi & MQTT
#include "dht11.h"
#include "rc522.h"
#include "mq2_sensor.h"  // <-- THÊM: Thư viện MQ2 (thay cho IR)

// --- ĐỊNH NGHĨA CHÂN & THÔNG SỐ ---
#define DHT11_GPIO      GPIO_NUM_4
// GPIO 34 tương ứng với ADC1 Channel 6
#define MQ2_ADC_CHANNEL ADC_CHANNEL_6 

// --- CẤU HÌNH WIFI & MQTT ---
#define ESP_WIFI_SSID      "Your_SSID"
#define ESP_WIFI_PASS      "Your_PASSWORD"
#define MQTT_BROKER_URI    "mqtt://broker.hivemq.com"
#define MQTT_PUBLISH_TOPIC "esp32/sensors/data"

static const char *TAG = "MAIN_APP";

// --- BIẾN TOÀN CỤC BẢO VỆ DHT11 ---
static SemaphoreHandle_t dht_mutex = NULL;
static dht11_data_t g_sensor_data = {0};
static bool g_dht_ok = false;

// --- TASK ĐỌC DHT11 (Chạy Core 1) ---
static void dht11_task(void *pvParameters)
{
    dht11_data_t local_data;
    while (1) {
        esp_err_t err = dht11_read(DHT11_GPIO, &local_data);
        if (xSemaphoreTake(dht_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (err == ESP_OK) {
                g_sensor_data = local_data;
                g_dht_ok = true;
            } else {
                g_dht_ok = false;
            }
            xSemaphoreGive(dht_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// --- CHƯƠNG TRÌNH CHÍNH ---
void app_main(void)
{
    ESP_LOGI(TAG, "--- KHOI TAO HE THONG ---");

    // 1. Khởi tạo NVS (Bắt buộc cho Wi-Fi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Khởi tạo Wi-Fi và MQTT qua thư viện iot_network
    wifi_init_sta(ESP_WIFI_SSID, ESP_WIFI_PASS);
    vTaskDelay(pdMS_TO_TICKS(3000));
    mqtt_app_start(MQTT_BROKER_URI);

    // 3. Khởi tạo Mutex & Task DHT11
    dht_mutex = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(dht11_task, "dht11_task", 2048, NULL, 5, NULL, 1);

    // 4. Khởi tạo RFID RC522
    rc522_config_t config = {
        .miso_io = 19, .mosi_io = 23, .sck_io = 18,
        .sda_io = 5, .rst_io = 22, .spi_host = VSPI_HOST
    };
    rc522_init(&config);

    // 5. Khởi tạo cảm biến khí gas MQ2 (Analog)
    mq2_adc_config_t mq2_sensor_obj;
    if (mq2_init_analog(&mq2_sensor_obj, MQ2_ADC_CHANNEL) == ESP_OK) {
        ESP_LOGI(TAG, "Khoi tao cam bien MQ2 thanh cong.");
    } else {
        ESP_LOGE(TAG, "Loi khoi tao cam bien MQ2!");
    }

    uint8_t uid[5];
    uint8_t uid_len;
    char mqtt_payload[250]; // Tăng kích thước chuỗi một chút cho thoải mái
    char rfid_str[20];

    ESP_LOGI(TAG, "Bat dau doc du lieu cac cam bien...");

    while (1) {
        printf("\n-----------------------------------\n");
        strcpy(rfid_str, "none");
        esp_task_wdt_reset();

        // --- ĐỌC CẢM BIẾN MQ2 ---
        int mq2_raw = mq2_read_raw(&mq2_sensor_obj);
        float mq2_voltage = (float)mq2_raw * 3.1 / 4095.0;
        int gas_alert = 0; // Mặc định là 0 (An toàn)
        
        printf("[MQ2]  : Raw: %d | Dien ap: %.2f V\n", mq2_raw, mq2_voltage);
        if (mq2_raw < 700) {
            ESP_LOGW(TAG, "[MQ2]  : NGUY HIEM - Nong do khi gas cao!");
            gas_alert = 1; // Bật cờ cảnh báo lên 1
        }

        // --- ĐỌC DHT11 (Từ biến toàn cục) ---
        dht11_data_t snapshot;
        bool dht_ok_snapshot;
        if (xSemaphoreTake(dht_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            snapshot = g_sensor_data;
            dht_ok_snapshot = g_dht_ok;
            xSemaphoreGive(dht_mutex);
        } else {
            dht_ok_snapshot = false;
        }

        if (dht_ok_snapshot) {
            printf("[DHT11]: Temp: %d°C | Hum: %d%%\n", snapshot.temperature, snapshot.humidity);
        } else {
            printf("[DHT11]: Loi doc cam bien!\n");
        }

        // --- ĐỌC RFID RC522 ---
        if (rc522_check_card()) {
            if (rc522_get_uid(uid, &uid_len)) {
                ESP_LOGI(TAG, "Card detected! UID: %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3]);
                snprintf(rfid_str, sizeof(rfid_str), "%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
            }
        }

        // --- GỬI MQTT ---
        if (is_mqtt_connected_status() && dht_ok_snapshot) {
            // Thay đổi cục JSON: Xóa ir_obstacle, thêm gas_raw và gas_alert
            snprintf(mqtt_payload, sizeof(mqtt_payload),
                "{\"temperature\": %d, \"humidity\": %d, \"gas_raw\": %d, \"gas_alert\": %d, \"rfid_uid\": \"%s\"}",
                snapshot.temperature, snapshot.humidity, mq2_raw, gas_alert, rfid_str);
            
            mqtt_publish_data(MQTT_PUBLISH_TOPIC, mqtt_payload);
        }

        printf("-----------------------------------\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}