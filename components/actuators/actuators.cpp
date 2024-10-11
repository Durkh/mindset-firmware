#include "actuators.h"

#include <esp_log.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <LiquidCrystal.h>

#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRutils.h>

#include <cstdio>
#include <array>
#include <cstring>
#include <cassert>

char const * const TAG = "actuator";

struct Actuactor_Pins_t{
    int nPins {};
    union {
        const gpio_num_t pin;
        //TODO change array size
        const std::array<gpio_num_t, 6> pins;
    };
};

class Actuators {
    public:
        enum display_pins_t {RS = 0, EN,  D4, D5, D6, D7, _pinsLen};

    private:
        constexpr static gpio_num_t LED_PINS    =  GPIO_NUM_2 ;
        constexpr static gpio_num_t RELAY_PINS  =  GPIO_NUM_17;
        constexpr static gpio_num_t IR_PINS     =  GPIO_NUM_16;
        constexpr static gpio_num_t BUZZER_PINS =  GPIO_NUM_18;
        constexpr static std::array<gpio_num_t, _pinsLen> display_pins{GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_22, GPIO_NUM_23, GPIO_NUM_32, GPIO_NUM_33};
        constexpr static std::array<gpio_num_t, 4> MOTOR_PINS  =  {GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC, GPIO_NUM_NC};

        //this array is indexed with the Actuator_t enum
        constexpr static const Actuactor_Pins_t pins[6] = {
            {.nPins{0}, .pin{GPIO_NUM_NC}}, //None
            {.nPins{1}, .pin{LED_PINS}}, //LED
            {.nPins{1}, .pin{RELAY_PINS}}, //relay
            {.nPins{1}, .pin{BUZZER_PINS}}, //buzzer
            {.nPins{1}, .pin{IR_PINS}}, //IR
            {.nPins{_pinsLen}, .pins{display_pins}} //display
            //{.nPins = 4, .pins = {MOTOR_PINS}} //motor
        };

    public:
        Actuators();

        static Actuactor_Pins_t getActuatorPins(const Actuators_t actuator){ return pins[actuator];}
        static gpio_num_t getPinOf(const Actuators_t actuator) {
            const auto pin = pins[actuator].pin;
            assert(pin != GPIO_NUM_NC);

            return pin;
        }
        static gpio_num_t getPinOf(const Actuators_t actuator, const display_pins_t pinName) {
            const auto pin = pins[actuator].pins[pinName];
            assert(pin != GPIO_NUM_NC);

            return pin;
        }
        static std::array<gpio_num_t, 6> getPinsOf(const Actuators_t actuator) {return pins[actuator].pins;}

        static void test(){
            for (auto i = 0; i < 6; ++i){
                printf("pp %d\r\n", pins[5].pins[i]);
            }
        }

        virtual ~Actuators() = 0;
};

static TaskHandle_t buzzerTask = nullptr;
static IRac ac(Actuators::getPinOf(Actuator_IR));
static LiquidCrystal lcd(
        Actuators::getPinOf(Actuator_display, Actuators::RS),
        Actuators::getPinOf(Actuator_display, Actuators::EN),
        Actuators::getPinOf(Actuator_display, Actuators::D4),
        Actuators::getPinOf(Actuator_display, Actuators::D5),
        Actuators::getPinOf(Actuator_display, Actuators::D6),
        Actuators::getPinOf(Actuator_display, Actuators::D7)
);

#define GPIO_CONFIG ((1ULL << Actuators::getPinOf(Actuator_LED))   | \
                    (1ULL << Actuators::getPinOf(Actuator_relay))  | \
                    (1ULL << Actuators::getPinOf(Actuator_buzzer)) | \
                    (1ULL << Actuators::getPinOf(Actuator_IR))     \
)

static void Buzzer_task [[noreturn]] (void *pvParameters){
    (void)pvParameters;
    
    uint32_t temp{0}, code{0};

    while (true) {
        code = xTaskNotifyWait(pdFALSE, UINT32_MAX, &temp ,pdMS_TO_TICKS(1000)) == pdPASS? temp : code;

        switch (code){
            case 0:
                gpio_set_level(Actuators::getPinOf(Actuator_buzzer), 0);
            break;
            case 1:
                gpio_set_level(Actuators::getPinOf(Actuator_buzzer), 1);
                vTaskDelay(pdMS_TO_TICKS(1000));
                gpio_set_level(Actuators::getPinOf(Actuator_buzzer), 0);
                vTaskDelay(pdMS_TO_TICKS(500));
            break;
            default:
            break;
        }
    }
}    

esp_err_t InitActuatorPins(){

    esp_err_t err;

    lcd.begin(16, 2);
    lcd.print("Initializing");

    gpio_config_t io_conf{};
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_CONFIG;

    err = gpio_config(&io_conf);

    REG_WRITE(GPIO_OUT_W1TC_REG, GPIO_CONFIG);

    if (err) return err;

    xTaskCreate(Buzzer_task, "buzzer_task", 2048, NULL, 10, &buzzerTask);

    ac.next = (stdAc::state_t){
        .protocol = decode_type_t::DAIKIN,
        .mode = stdAc::opmode_t::kCool,
        .fanspeed = stdAc::fanspeed_t::kMedium,
        .filter = true,
        .beep = true,
    };

    return ESP_OK;
}

#define TOGGLE_ACTUATOR(actuator, state) do {          \
        if ((state)){                             \
            gpio_set_level(Actuators::getPinOf(actuator), 1);  \
        }else {                                 \
            gpio_set_level(Actuators::getPinOf(actuator), 0);  \
        }                                       \
    } while(0)

#define LCD_PRINT(msg) do { \
                            lcd.clear(); \
                            lcd.print(payload.message); \
                        }while(0)

void doAction(Actuators_t actuator, const Actuator_payload_t payload){

    printf("actuator %d,\tpayload len %d\r\n", actuator, payload.len);

    switch (actuator){
        case Actuator_None:
        break;
        case Actuator_LED:
            assert(payload.len == 1);
            TOGGLE_ACTUATOR(Actuator_LED, payload.state);
            //LCD_PRINT("TOGGLING RELAY");
        break;
        case Actuator_relay:
            assert(payload.len == 1);
            TOGGLE_ACTUATOR(Actuator_relay, payload.state);
            //LCD_PRINT("TOGGLING RELAY");
        break;
        case Actuator_buzzer:
            assert(payload.len == 1);
            TOGGLE_ACTUATOR(Actuator_buzzer, payload.state);
            //xTaskNotify(buzzerTask, (uint32_t)payload.state, eSetValueWithOverwrite);
            //LCD_PRINT("SOUNDING BUZZER");
        break;
        case Actuator_IR:
            break; // TOFIX
            // sending ON | OFF to supported AC
            ac.next.power = payload.state;
            for (int i = 1; i < kLastDecodeType; i++) {
                decode_type_t protocol = (decode_type_t)i;
                
                if (ac.isProtocolSupported(protocol)) {
                    ac.next.protocol = protocol;
                    ac.sendAc();
                }
                LCD_PRINT("SENDING IR COMMAND");
            }
        break;
        case Actuator_display:
            if (payload.len != 1){
                LCD_PRINT(payload.message);
            }
        break;
        case Actuator_motor:
        break;
        default:
            ESP_LOGE(TAG, "unknown actuator");
    }
}

#define CMP_ENDPOINT(len, name) (endpointLen == (len) && strncmp(endpoint, (name), (len)) == 0)

Actuators_t toActuator(char const * const topic, const size_t len){
    if (len <= 8) return Actuator_None;

    char const * const endpoint = topic + 8;
    const size_t endpointLen = len - 8;

    if (CMP_ENDPOINT(2, "IR")){
        return Actuator_IR;
    }else if (CMP_ENDPOINT(3, "led")){
        return Actuator_LED;
    }else if (CMP_ENDPOINT(4, "motor")){
        return Actuator_motor;
    }else if (CMP_ENDPOINT(5, "relay")){
        return Actuator_relay;
    }else if (CMP_ENDPOINT(6, "buzzer")){
        return Actuator_buzzer;
    }else if (CMP_ENDPOINT(7, "display")){
        return Actuator_display;
    }

    return Actuator_None;
}