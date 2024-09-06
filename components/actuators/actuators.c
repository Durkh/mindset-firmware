#include "actuators.h"

#include <esp_log.h>
#include "driver/gpio.h"

#include <string.h>

char const * const TAG = "actuator";

#define UNUSED_PIN      -1
#define LED_PINS        2
#define RELAY_PINS      18
#define BUZZER_PINS     19
#define IR_PINS         20
#define MOTOR_PINS      UNUSED_PIN, UNUSED_PIN, UNUSED_PIN, UNUSED_PIN
#define DISPLAY_PINS    UNUSED_PIN, UNUSED_PIN

typedef struct{
    int nPins;
    union {
        const gpio_num_t pin;
        const gpio_num_t *pins;
    };
}Actuactor_Pins_t;

//this array is indexed with the Actuator_t enum
const Actuactor_Pins_t pins[] = {
    {.nPins = UNUSED_PIN, .pin = UNUSED_PIN}, //None
    {.nPins = 1, .pin = LED_PINS}, //LED
    {.nPins = 1, .pin = RELAY_PINS}, //relay
    {.nPins = 1, .pin = BUZZER_PINS}, //buzzer
    {.nPins = 1, .pin = IR_PINS}, //IR
    //{.nPins = 4, .pins = {MOTOR_PINS}} //motor
    //{.nPins = 2, .pins = {DISPLAY_PINS}} //display (i2c) TODO
};

#define GPIO_CONFIG ((1ULL << pins[Actuator_LED].pin)   | \
                    (1ULL << pins[Actuator_relay].pin)  | \
                    (1ULL << pins[Actuator_buzzer].pin) | \
                    (1ULL << pins[Actuator_IR].pin)     \
)

esp_err_t InitActuatorPins(){

    gpio_config_t io_conf = {.mode = GPIO_MODE_OUTPUT, .pin_bit_mask = GPIO_CONFIG};
    return gpio_config(&io_conf);
}

#define TOGGLE_ACTUATOR(actuator, state) do {          \
        if ((state)){                             \
            gpio_set_level(pins[actuator].pin, 0);  \
        }else {                                 \
            gpio_set_level(pins[actuator].pin, 1);  \
        }                                       \
    } while(0)

void doAction(Actuators_t actuator, bool state){

    switch (actuator){
        case Actuator_None:
        break;
        case Actuator_LED:
            TOGGLE_ACTUATOR(Actuator_LED, state);
        break;
        case Actuator_relay:
            TOGGLE_ACTUATOR(Actuator_relay, state);
        break;
        case Actuator_buzzer:
        break;
        case Actuator_IR:
        break;
        case Actuator_motor:
        break;
        case Actuator_display:
        break;
        default:
            ESP_LOGE(TAG, "unknown actuator");
    }
}

#define CMP_ENDPOINT(len, name) (endpointLen == (len) && strncmp(endpoint, (name), (len)) == 0)

Actuators_t __attribute__((pure)) toActuator(char const * const topic, const size_t len){
    if (len <= 8) return Actuator_None;

    char const * const endpoint = topic + 8;
    const size_t endpointLen = len - 8;

    if (CMP_ENDPOINT(2, "IR")){
        return Actuator_IR;
    }else if (CMP_ENDPOINT(3, "LED")){
        return Actuator_LED;
    }else if (CMP_ENDPOINT(4, "motor")){
        return Actuator_motor;
    }else if (CMP_ENDPOINT(5, "relay")){
        return Actuator_relay;
    }else if (CMP_ENDPOINT(5, "buzzer")){
        return Actuator_buzzer;
    }else if (CMP_ENDPOINT(7, "display")){
        return Actuator_display;
    }

    return Actuator_None;
}