#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <esp_err.h>

typedef enum {
    Actuator_None = 0,
    Actuator_LED,
    Actuator_relay,
    Actuator_buzzer,
    Actuator_IR,
    Actuator_motor,
    Actuator_display
}Actuators_t;


esp_err_t InitActuatorPins();
Actuators_t __attribute__((pure)) toActuator(char const * const topic, const size_t len);
void doAction(Actuators_t actuator, bool state);