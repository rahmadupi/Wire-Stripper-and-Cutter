#include "motor.h"

AccelStepper motor_cutter(AccelStepper::DRIVER, PIN_MOTOR_CUTTER_STEP, PIN_MOTOR_CUTTER_DIR);
AccelStepper motor_selector(AccelStepper::DRIVER, PIN_MOTOR_SELECTOR_STEP, PIN_MOTOR_SELECTOR_DIR);
AccelStepper motor_feeder(AccelStepper::DRIVER, PIN_MOTOR_FEEDER_STEP, PIN_MOTOR_FEEDER_DIR);

volatile bool cutter_limit_triggered = false;
volatile bool selector_limit_triggered = false;
void IRAM_ATTR cutter_limit() {
    cutter_limit_triggered = true;
}
void IRAM_ATTR selector_limit() {
    selector_limit_triggered = true;
}

const double cutter_steps_mm_per_step = 0.04;
const double feeder_mm_per_step = 0.1123;
const double selector_steps_per_position = 40 / 1.8;
const int max_selector_position = 5;
const int idle_cutter_pos = 10;

int motor_cutter_steps = 0;
int motor_cutter_pos_mm = 0;
int motor_selector_steps = 0;

int current_selector_position = 1;

int mm_to_steps(motor_t motor, int mm) {
    switch (motor) {
        case MOTOR_CUTTER:
            return (int)(mm * cutter_steps_mm_per_step);
            break;
        case MOTOR_FEEDER:
            return (int)(mm / feeder_mm_per_step);
            break;

        default:
            break;
    }
    return 0;
}
int check_proximity_sensor() {
    // Check if the proximity sensor is triggered
    // If triggered, move the feeder motor to pull the wire
    if (!digitalRead(PIN_PROXIMITY_SENSOR)) {
        Serial.println("[+] Proximity sensor triggered");
        return 1;
    }
    return 0;
}
int limiter(limiter_t type, int current_step, int target_step) {
    // if (current_step - target_step < 1) return 1;
    if (type == CUTTER_LIMITER) {
        return !(digitalRead(PIN_JANCOK));
    } else if (type == SELECTOR_LIMITER) {
        return !(digitalRead(PIN_DANCOK));
    }
    return 0;
}

int move_motor(motor_t motor, int steps) {
    digitalWrite(LED_BUILTIN, LOW);
    if (motor == MOTOR_CUTTER) {
        // motor_cutter.move(steps);
        motor_cutter.move(steps);
        while (motor_cutter.distanceToGo() != 0) {
            if (limiter(CUTTER_LIMITER, motor_cutter_steps, steps)) {
                motor_cutter.stop();
                Serial.println("[!] Cutter limiter triggered");
                for (int i = 0; i < 10; i++) {
                    digitalWrite(LED_BUILTIN, HIGH);
                    delay(10);
                    digitalWrite(LED_BUILTIN, LOW);
                    delay(10);
                }

                motor_cutter.setCurrentPosition(0);
                motor_cutter_steps = 0;
                motor_cutter_pos_mm = 0;
                return LIMIT;
            }
            motor_cutter.run();
        }

        // int increment = steps < 0 ? -1 : 1;
        // do {
        //     if ((limiter(CUTTER_LIMITER, motor_cutter_steps, steps))) {
        //         Serial.println("[!] Cutter limiter triggered");
        //         for (int i = 0; i < 10; i++) {
        //             digitalWrite(LED_BUILTIN, HIGH);
        //             delay(10);
        //             digitalWrite(LED_BUILTIN, LOW);
        //             delay(10);
        //         }
        //         return LIMIT;
        //     }
        //     motor_cutter.move(increment);
        //     motor_cutter_steps += increment;
        //     if (steps < 0) {
        //         steps++;
        //     } else {
        //         steps--;
        //     }
        //     delay(1);
        // } while (steps);
        Serial.println("[+] Motor Cutter Steps: " + String(motor_cutter_steps));
        return CLEAR;
    } else if (motor == MOTOR_SELECTOR) {
        // int steps = mm_to_steps(motor, distance);
        // int increment = steps < 0 ? -1 : 1;
        // do {
        //     if ((limiter(SELECTOR_LIMITER, motor_selector_steps, steps))) {
        //         Serial.println("[!] Selector limiter triggered");
        //         for (int i = 0; i < 10; i++) {
        //             digitalWrite(LED_BUILTIN, HIGH);
        //             delay(10);
        //             digitalWrite(LED_BUILTIN, LOW);
        //             delay(10);
        //         }
        //         return LIMIT;
        //     }
        //     motor_selector.move(increment);
        //     motor_selector.run();
        //     motor_selector_steps += increment;
        //     if (steps < 0) {
        //         steps++;
        //     } else {
        //         steps--;
        //     }
        // } while (steps);
        motor_selector.move(steps);
        while (motor_selector.distanceToGo() != 0) {
            if (limiter(SELECTOR_LIMITER, motor_selector_steps, steps)) {
                motor_selector.stop();
                Serial.println("[!] Selector limiter triggered");
                for (int i = 0; i < 10; i++) {
                    digitalWrite(LED_BUILTIN, HIGH);
                    delay(10);
                    digitalWrite(LED_BUILTIN, LOW);
                    delay(10);
                }
                motor_selector.setCurrentPosition(0);
                motor_selector_steps = 0;
                current_selector_position = 1;
                return LIMIT;
            }
            motor_selector.run();
        }
        Serial.println("[+] Motor Selector Steps: " + String(motor_selector_steps));
        return CLEAR;
    } else if (motor == MOTOR_FEEDER) {
        // int steps = mm_to_steps(motor, distance);
        motor_feeder.move(steps);
        while (motor_feeder.distanceToGo() != 0) {
            motor_feeder.run();
        }
        Serial.println("[+] Motor Feeder Move: " + String(steps) + " mm");
        return CLEAR;
    }
    // - up, + down
    // return true;
    // - back, + forward
    // return true;
    // - pull, + push
    // return true;
}

void change_selector(int position) {
    if (position < 1 || position >= max_selector_position) {
        Serial.println("[!] Invalid selector position");
        return;
    }
    int steps = (current_selector_position - position) * selector_steps_per_position;
    move_motor(MOTOR_SELECTOR, steps);
    current_selector_position = position;
}

void move_cutter(int mm) {
    // Move cutter motor to the specified position in mm
    int steps = mm_to_steps(MOTOR_CUTTER, mm);
    move_motor(MOTOR_CUTTER, steps);
    motor_cutter_pos_mm += mm;
    motor_cutter_steps += steps;
}

void move_feeder(int mm) {
    // Move feeder motor to the specified position in mm
    int steps = mm_to_steps(MOTOR_FEEDER, mm);
    move_motor(MOTOR_FEEDER, steps);
}

void motor_startup(void *pvParameters) {
    // Calibrate
    // nek onok kabel(proximity e kedeteksi) feeder e langsung tarik sik kabel e
    reset_feeder();  // Reset feeder position

    reset_selector();  // Reset selector position
    reset_cutter();    // Reset cutter position

    // move_motor(MOTOR_CUTTER, 10);  // Munggah cutter 10mm
    move_cutter(10);  // Munggah cutter 10mm

    // kalibrasi cutter drop terus munggah 10 cm 10mm 10/0.04
    Serial.println("Selesai");

    vTaskDelete(NULL);
}

void strip_wire(int thickness) {
    // Implement wire stripping logic based on thickness
    // This is a placeholder function
    Serial.print("[+] Stripping wire with thickness: ");
    Serial.println(thickness);
    move_cutter(idle_cutter_pos - thickness);
    move_cutter((idle_cutter_pos - thickness) * -1);
}

void cut_wire() {
    // Implement wire cutting logic
    // This is a placeholder function
    move_cutter(idle_cutter_pos);
    move_cutter(-idle_cutter_pos);
}

void execute_command(void *pvParameters) {
    crimp_configuration_t *config = (crimp_configuration_t *)pvParameters;

    if (check_proximity_sensor()) {
        do {
            move_motor(MOTOR_FEEDER, -1);
            delay(1);
        } while (check_proximity_sensor());
        move_motor(MOTOR_FEEDER, -30);  // Tarik mundur feeder motor 30mm
        Serial.println("[+] Proximity sensor triggered, moving feeder motor back");
    }
    if (current_selector_position != config->cable_type) {
        change_selector(config->cable_type);
    }
    do {
        move_motor(MOTOR_FEEDER, 1);
    } while (!check_proximity_sensor());
    move_feeder(30);
    // pilih kabel
    // Serial.println("[+] Executing command with configuration:");
    // Serial.print("Cable Type: ");
    // Serial.println(config->cable_type);
    // Serial.print("Cable Thickness: ");
    // Serial.println(config->cable_thickness);
    // Serial.print("Cable Length: ");
    // Serial.println(config->cable_length);
    // Serial.print("Front End Crimp Length: ");
    // Serial.println(config->front_end_crimp_length);
    // Serial.print("Back End Crimp Length: ");
    // Serial.println(config->back_end_crimp_length);
    // Serial.print("Copy Count: ");
    // Serial.println(config->copy_count);
    // // delay(20000);
    // // vTaskDelay(20000 / portTICK_PERIOD_MS);  // Simulate processing time
    // int test = 20000;
    // while (test) {
    //     test--;
    //     Serial.println(test);
    // }
    for (int i = 0; i < config->copy_count; i++) {
        // Pull cable
        move_feeder(config->front_end_crimp_length);
        strip_wire(config->cable_thickness);
        move_feeder(config->cable_length);
        strip_wire(config->cable_thickness);
        move_cutter(config->back_end_crimp_length);
        cut_wire();
    }

    reset_feeder();

    // Reset Cutter nek perlu nak posisi 1cm teko nisor
    // Reset Feeder ambek selector e

    vTaskDelete(NULL);
}

void reset_selector() {
    do {
        move_motor(MOTOR_SELECTOR, -1);
        delay(1);
    } while (!limiter(SELECTOR_LIMITER));
    motor_selector.setCurrentPosition(0);
    current_selector_position = 0;
    //
    move_motor(MOTOR_SELECTOR, (int)5 / 1.8);  // Munggah selector 10mm
    motor_selector_steps = 0;
    motor_selector.setCurrentPosition(0);
    // balik ke posisi 0;
    // move_motor(MOTOR_SELECTOR, 10);
}
void reset_cutter() {
    do {
        move_motor(MOTOR_CUTTER, -1);
        delay(1);
    } while (!limiter(CUTTER_LIMITER));
    motor_cutter.setCurrentPosition(0);
    motor_cutter_steps = 0;
    motor_cutter_pos_mm = 0;
}
void reset_feeder() {
    // Cek onok kabel seng metu tagak Check sensor induksi ne
    // Nek onok kabel seng metu, tarik mundur pirang pirang senti
    if (check_proximity_sensor()) {
        // Tarik mundur feeder motor
        do {
            move_motor(MOTOR_FEEDER, -1);
            delay(1);
        } while (check_proximity_sensor());
        move_motor(MOTOR_FEEDER, -30);  // Tarik mundur feeder motor 30mm
    }
}