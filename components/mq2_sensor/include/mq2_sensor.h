#ifndef MQ2_SENSOR_H
#define MQ2_SENSOR_H

#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

// Cấu trúc để quản lý ADC
typedef struct {
    adc_oneshot_unit_handle_t unit_handle;
    adc_channel_t channel;
} mq2_adc_config_t;

/**
 * @brief Khởi tạo ADC cho MQ2
 */
esp_err_t mq2_init_analog(mq2_adc_config_t *config, adc_channel_t channel);

/**
 * @brief Đọc giá trị raw (0 - 4095)
 */
int mq2_read_raw(mq2_adc_config_t *config);

#endif