#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

#include "interface.h"
#include "motor.h"

#define PIN_ENCODER_CLK 17
#define PIN_ENCODER_DT 16
#define PIN_ENCODER_BT 4
#define PIN_BUTTON_A 10

extern crimp_configuration_t crimp_configuration;
