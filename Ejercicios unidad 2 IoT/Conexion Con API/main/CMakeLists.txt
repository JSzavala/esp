idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES protocol_examples_common esp_wifi nvs_flash esp_http_client esp_netif esp_event)