#include "wifi.h"
#include "mqtt_conn.h"
#include "actuators.h"

#include "nvs_flash.h"

#include <esp_log.h>
#include <esp_err.h>

#include "Arduino.h"
#include "driver/gpio.h"

extern "C" [[noreturn]] void app_main()
{
    initArduino();
    Serial.begin(115200);
    while(!Serial);


    ESP_ERROR_CHECK(InitActuatorPins());

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    ESP_LOGI("main", "Initializing Wifi");
    wifi_init_sta();

    mqtt_app_start();

    gpio_set_level(GPIO_NUM_17, 0);

    while(true){
        vTaskDelay(portMAX_DELAY);
        /*
        vTaskDelay(pdMS_TO_TICKS(2000));
        printf("toggle\r\n");
        gpio_set_level(static_cast<gpio_num_t>(17), 1);
        vTaskDelay(pdMS_TO_TICKS(2000));
        printf("toggle\r\n");
        gpio_set_level(static_cast<gpio_num_t>(17), 0);

        vTaskDelay(pdMS_TO_TICKS(2000));
        printf("toggle\r\n");
        gpio_set_level(static_cast<gpio_num_t>(17), 1);
        */
    }
}
