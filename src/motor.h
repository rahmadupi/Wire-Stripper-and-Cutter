#pragma once

#include <AccelStepper.h>
#include <Arduino.h>

#include "interface.h"

#define PIN_MOTOR_CUTTER_STEP 12
#define PIN_MOTOR_CUTTER_DIR_STEP 9
#define PIN_MOTOR_SELECTOR_DIR 11
#define PIN_MOTOR_SELECTOR 13
#define PIN_MOTOR_FEEDER_STEP 14
#define PIN_MOTOR_FEEDER_DIR 8

#define PIN_PROXIMITY_SENSOR 15
#define PIN_CUTTER_LIMITER 18
#define PIN_SELECTOR_LIMITER 19

#define MOTOR_STEPS 500

typedef enum {
    CUTTER_LIMITER = 0,
    SELECTOR_LIMITER,
} limiter_t;

/* Variable */
extern int current_cable_type;
extern int current_cable_thickness;
extern int current_cable_length;

extern int current_selector_position;

extern AccelStepper motor_cutter;
extern AccelStepper motor_selector;
extern AccelStepper motor_feeder;

/* Fungsi */
void motor_startup(void *pvParameters);
void execute_command(void *pvParameters);