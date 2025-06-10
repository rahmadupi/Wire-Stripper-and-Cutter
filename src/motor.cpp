#include "motor.h"

AccelStepper motor_cutter(AccelStepper::DRIVER, PIN_MOTOR_CUTTER_STEP, PIN_MOTOR_CUTTER_DIR_STEP);
AccelStepper motor_selector(AccelStepper::DRIVER, PIN_MOTOR_SELECTOR, PIN_MOTOR_SELECTOR_DIR);
AccelStepper motor_feeder(AccelStepper::DRIVER, PIN_MOTOR_FEEDER_STEP, PIN_MOTOR_FEEDER_DIR);

int current_cable_type = 0;
int current_cable_thickness = 0;

int motor_cutter_steps = 0;
int motor_selector_steps = 0;

int current_selector_position = 0;

int limiter(limiter_t type) {
    if (type == CUTTER_LIMITER) {
        return digitalRead(PIN_CUTTER_LIMITER);
    } else if (type == SELECTOR_LIMITER) {
        return digitalRead(PIN_SELECTOR_LIMITER);
    }
    return -1;
}

int mm_to_steps(int mm) {
    // return mm * 10;
}

bool move_cutter(int steps) {
    // - up, + down
    return true;
}
bool move_selector(int steps) {
    // - back, + forward
    return true;
}

bool move_feeder(int steps) {
    // - pull, + push
    return true;
}
int calculate_feeder_steps(int length) {}

bool cut_wire() {}

bool strip_wire(int thickness) {}

void motor_startup(void *pvParameters) {
    //Calibrate
    Serial.println("Motor startup...");
    int length = 1000;
    while (length) {
        vTaskDelay(1 / portTICK_PERIOD_MS);  // Simulate motor startup delay
        --length;
    }
    Serial.println("Selesai");
    vTaskDelete(NULL);
}

void execute_command(void *pvParameters) {
    crimp_configuration_t *config = (crimp_configuration_t *)pvParameters;
    Serial.println("[+] Executing command with configuration:");
    Serial.print("Cable Type: ");
    Serial.println(config->cable_type);
    Serial.print("Cable Thickness: ");
    Serial.println(config->cable_thickness);
    Serial.print("Cable Length: ");
    Serial.println(config->cable_length);
    Serial.print("Front End Crimp Length: ");
    Serial.println(config->front_end_crimp_length);
    Serial.print("Back End Crimp Length: ");
    Serial.println(config->back_end_crimp_length);
    Serial.print("Copy Count: ");
    Serial.println(config->copy_count);
    // delay(20000);
    // vTaskDelay(20000 / portTICK_PERIOD_MS);  // Simulate processing time
    int test = 20000;
    while (test) {
        test--;
        Serial.println(test);
    }
    vTaskDelete(NULL);
}

bool reset_cable_position() {
    // Check sensor induksi ne
}
bool reset_cutter_position() {}
bool reset_feeder_position() {
    // Check sensor induksi ne
}
bool reset_selector() {}