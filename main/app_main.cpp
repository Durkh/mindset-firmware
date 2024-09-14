#include "wifi.h"
#include "mqtt_conn.h"
#include "actuators.h"

#include "nvs_flash.h"

#include <esp_log.h>
#include <esp_err.h>

#include "Arduino.h"

extern "C" void app_main()
{
    initArduino();

    ESP_ERROR_CHECK(InitActuatorPins());

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }

    ESP_LOGI("main", "Initializing Wifi");
    wifi_init_sta();

    mqtt_app_start();

    while(1);
}
