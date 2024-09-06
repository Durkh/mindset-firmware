idf_component_register(SRCS "mqtt_conn.c"
                        INCLUDE_DIRS "include"
                        REQUIRES mqtt esp_event esp_netif nvs_flash actuators
                        )