#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stddef.h>
#include <stdbool.h>
#include <esp_err.h>

typedef enum {
    Actuator_None = 0,
    Actuator_LED,
    Actuator_relay,
    Actuator_buzzer,
    Actuator_IR,
    Actuator_display,
    Actuator_motor
}Actuators_t;

typedef struct {
    const size_t len;
    union {
        const bool state;
        char const * message;
    };
}Actuator_payload_t;

esp_err_t InitActuatorPins();
Actuators_t __attribute__((pure)) toActuator(char const * const topic, const size_t len);
void doAction(Actuators_t actuator, Actuator_payload_t state);

#ifdef __cplusplus
}
#endif