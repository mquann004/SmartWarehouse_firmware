
#include "mq2_sensor.h"

esp_err_t mq2_init_analog(mq2_adc_config_t *config, adc_channel_t channel) {
    config->channel = channel;

    // 1. Khởi tạo Unit ADC1
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &config->unit_handle));

    // 2. Cấu hình Channel (Độ phân giải 12-bit, Độ suy giảm 11dB để đọc tới ~3.1V)
    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12, 
    };
    return adc_oneshot_config_channel(config->unit_handle, config->channel, &chan_config);
}

int mq2_read_raw(mq2_adc_config_t *config) {
    int raw_val = 0;
    adc_oneshot_read(config->unit_handle, config->channel, &raw_val);
    return raw_val;
}