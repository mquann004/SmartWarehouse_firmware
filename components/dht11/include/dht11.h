#ifndef DHT11_H
#define DHT11_H

#include "esp_err.h"
#include "driver/gpio.h"


typedef struct {
    int temperature;
    int humidity;
} dht11_data_t;

/**
 * @brief Đọc dữ liệu từ DHT11
 * @param gpio_num Chân GPIO kết nối với Data của DHT11
 * @param data Con trỏ lưu kết quả trả về
 * @return ESP_OK nếu thành công, lỗi nếu thất bại
 */
esp_err_t dht11_read(gpio_num_t gpio_num, dht11_data_t *data);

#endif