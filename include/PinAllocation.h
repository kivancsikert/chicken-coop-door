#pragma once

#ifdef ESP32

#include "pins_arduino_ttgo_call.h"

#define MOTOR_PIN1 GPIO_NUM_23
#define MOTOR_PIN2 GPIO_NUM_19
#define MOTOR_PIN3 GPIO_NUM_18
#define MOTOR_PIN4 GPIO_NUM_5

#define LIGHT_SDA GPIO_NUM_15
#define LIGHT_SCL GPIO_NUM_13

#define OPEN_PIN GPIO_NUM_14
#define CLOSED_PIN GPIO_NUM_12

#elif defined(ESP8266)

#define OPEN_PIN D5
#define CLOSED_PIN D6

#define MOTOR_PIN1 D0
#define MOTOR_PIN2 D1
#define MOTOR_PIN3 D2
#define MOTOR_PIN4 D3

#define LIGHT_SDA D7
#define LIGHT_SDC D4

#endif
