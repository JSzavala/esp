idf_component_register(SRCS "backup.cpp" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES protocol_examples_common mqtt esp_wifi esp_system
                             nvs_flash esp_event esp_netif freertos lwip log)
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)
