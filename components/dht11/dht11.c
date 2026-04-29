#include "dht11.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

esp_err_t dht11_read(gpio_num_t gpio_num, dht11_data_t *data) {
    uint8_t bits[5] = {0};
    int timeout = 0;


    gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_num, 0);
    vTaskDelay(pdMS_TO_TICKS(20)); 
    gpio_set_level(gpio_num, 1);
    esp_rom_delay_us(30);

    
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT);

    
    timeout = 0;
    while (gpio_get_level(gpio_num) == 1) {
        if (timeout++ > 100) return ESP_ERR_TIMEOUT; 
        esp_rom_delay_us(1);
    }

    timeout = 0;
    while (gpio_get_level(gpio_num) == 0) {
        if (timeout++ > 100) return ESP_ERR_TIMEOUT; 
        esp_rom_delay_us(1);
    }

    while (gpio_get_level(gpio_num) == 1); 
    for (int i = 0; i < 40; i++) {
        while (gpio_get_level(gpio_num) == 0);   
        esp_rom_delay_us(30); 
        if (gpio_get_level(gpio_num) == 1) {
            bits[i / 8] |= (1 << (7 - (i % 8)));
            while (gpio_get_level(gpio_num) == 1);
        }
    }

    if (bits[4] == ((bits[0] + bits[1] + bits[2] + bits[3]) & 0xFF)) {
        data->humidity = bits[0];
        data->temperature = bits[2];
        return ESP_OK;
    }
    
    return ESP_ERR_INVALID_CRC;
}