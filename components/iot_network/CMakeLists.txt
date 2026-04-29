idf_component_register(
    SRCS "iot_network.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi esp_event mqtt
)