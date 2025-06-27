#pragma once

#include <AccelStepper.h>
#include <Arduino.h>

#include "interface.h"

#define PIN_MOTOR_CUTTER_STEP 13
#define PIN_MOTOR_CUTTER_DIR 14
#define PIN_MOTOR_SELECTOR_DIR 27
#define PIN_MOTOR_SELECTOR_STEP 26
#define PIN_MOTOR_FEEDER_STEP 18
#define PIN_MOTOR_FEEDER_DIR 19

#define PIN_PROXIMITY_SENSOR 35
// #define PIN_CUTTER_LIMITER 32
// #define PIN_SELECTOR_LIMITER 12
#define PIN_JANCOK 5
#define PIN_DANCOK 33

#define MOTOR_STEPS 200

typedef enum {
    CUTTER_LIMITER = 0,
    SELECTOR_LIMITER,
} limiter_t;

typedef enum {
    MOTOR_CUTTER = 0,
    MOTOR_SELECTOR,
    MOTOR_FEEDER,
} motor_t;

typedef enum {
    CLEAR = 0,
    LIMIT,
} motor_state_t;

/* Variable */
extern int current_selector_position;

extern int motor_cutter_steps;
extern int motor_selector_steps;

extern AccelStepper motor_cutter;
extern AccelStepper motor_selector;
extern AccelStepper motor_feeder;

/* Fungsi */
int limiter(limiter_t type, int current_step = 200, int target_step = 0);
void motor_startup(void *pvParameters);
void execute_command(void *pvParameters);

int move_motor(motor_t motor, int steps);
int mm_to_steps(motor_t motor, double mm);

void reset_selector();
void reset_cutter();
void reset_feeder();

void move_cutter(double mm);
void move_feeder(double mm);
void change_selector(int position);
void strip_wire(double mm);
void cut_wire();

// Proximity sensor
int check_proximity_sensor();

// interrupt
extern volatile bool cutter_limit_triggered;
extern volatile bool selector_limit_triggered;
void IRAM_ATTR cutter_limit();
void IRAM_ATTR selector_limit();