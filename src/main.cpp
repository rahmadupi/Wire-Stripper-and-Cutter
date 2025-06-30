#include "main.h"

TaskHandle_t motor_handle = NULL;

int screen_state = 0;

int history_length = 0;
crimp_configuration_t configuration_history[3];

crimp_configuration_t crimp_configuration = {
    .cable_type = 0,
    .copy_count = 1,
    .cable_thickness = 0,
    .cable_length = 0,
    .front_end_crimp_length = 0,
    .back_end_crimp_length = 0};

void additional_loop(void *pvParameters) {
    while (true) {
        // if (cutter_limit_triggered) {
        //     Serial.println("[+] Cutter Limit Triggered");
        //     digitalWrite(LED_BUILTIN, LOW);
        //     cutter_limit_triggered = false;
        //     motor_cutter.stop();
        //     motor_cutter.setCurrentPosition(0);
        // }
        // if (selector_limit_triggered) {
        //     Serial.println("[+] Selector Limit Triggered");
        //     digitalWrite(LED_BUILTIN, LOW);
        //     selector_limit_triggered = false;
        //     motor_selector.stop();
        //     motor_selector.setCurrentPosition(0);
        // }
        // delay(10);
        motor_cutter.run();
        motor_selector.run();
        motor_feeder.run();
        delay(1);
    }
    vTaskDelete(NULL);
}

void serial_reader() {
    while (Serial.available()) {
        // char c = Serial.read();
        // if (c == 'a') {
        //     Serial.println("Received 'a'");
        // } else if (c == 'b') {
        //     Serial.println("Received 'b'");
        // } else {
        //     Serial.print("Received: ");
        //     Serial.println(c);
        // }
    }
}

void cycle_config_history(crimp_configuration_t *configuration_history,
                          crimp_configuration_t current_config,
                          int *history_length) {
    Serial.println("[+] Cycle configuration history");
    memcpy(configuration_history + 1, configuration_history, sizeof(crimp_configuration_t));
    mempcpy(configuration_history, &current_config, sizeof(crimp_configuration_t));

    for (int i = 0; i < 5; i++) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
}

void load_config(int *history_length, crimp_configuration_t *configuration_history) {
    Serial.println("[+] Load konfigurasi dari SPIFFS");
    if (!(SPIFFS.exists("/config.json"))) {
        File file = SPIFFS.open("/config.json", "w");
        file.close();
    } else {
        File file = SPIFFS.open("/config.json", "r");
        if (!file) {
            Serial.println("Failed to open config file for reading");
            return;
        }
        Serial.println("Config file found, loading...");

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error) {
            Serial.println("Failed to parse config file");
            return;
        }
        *history_length = doc["config_length"];
        JsonArray config = doc["config"];
        int i = 0;
        for (JsonObject obj : config) {
            configuration_history[i].cable_type = obj["type"];
            configuration_history[i].copy_count = obj["quantity"];
            configuration_history[i].cable_thickness = obj["thickness"];
            configuration_history[i].cable_length = obj["length"];
            configuration_history[i].front_end_crimp_length = obj["front"];
            configuration_history[i].back_end_crimp_length = obj["back"];
            i++;
            if (i > *history_length) break;
        }
        Serial.println("Config loaded from SPIFFS.");
        if (*history_length > 2) {
            *history_length = 2;
        }
        Serial.print("History Length: ");
        Serial.println(*history_length);
    }
}
void save_config(int history_length) {
    if (history_length > 2) {
        history_length = 2;
    }
    StaticJsonDocument<1024> doc;
    doc["config_length"] = history_length;
    JsonArray config = doc.createNestedArray("config");

    for (int i = 0; i < history_length; i++) {
        JsonObject obj = config.createNestedObject();
        obj["type"] = configuration_history[i].cable_type;
        obj["quantity"] = configuration_history[i].copy_count;
        obj["thickness"] = configuration_history[i].cable_thickness;
        obj["length"] = configuration_history[i].cable_length;
        obj["front"] = configuration_history[i].front_end_crimp_length;
        obj["back"] = configuration_history[i].back_end_crimp_length;
    }

    File file = SPIFFS.open("/config.json", "w");
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return;
    }
    serializeJsonPretty(doc, file);
    file.close();
    Serial.println("Config saved to SPIFFS.");

    for (int i = 0; i < 5; i++) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(PIN_SCREEN_SDA, PIN_SCREEN_SCL);
    delay(500);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_ENCODER_CLK, INPUT);
    pinMode(PIN_ENCODER_DT, INPUT);
    pinMode(PIN_ENCODER_BT, INPUT_PULLUP);
    pinMode(PIN_PROXIMITY_SENSOR, INPUT);
    pinMode(PIN_JANCOK, INPUT_PULLDOWN);
    pinMode(PIN_DANCOK, INPUT_PULLDOWN);
    pinMode(PIN_MOTOR_CUTTER_STEP, OUTPUT);
    pinMode(PIN_MOTOR_CUTTER_DIR, OUTPUT);
    pinMode(PIN_MOTOR_SELECTOR_DIR, OUTPUT);
    pinMode(PIN_MOTOR_SELECTOR_STEP, OUTPUT);
    pinMode(PIN_MOTOR_FEEDER_STEP, OUTPUT);
    pinMode(PIN_MOTOR_FEEDER_DIR, OUTPUT);
    Serial.println("[+] Starting Serial...");

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    motor_cutter.setMaxSpeed(1000);
    motor_cutter.setAcceleration(500);
    motor_selector.setMaxSpeed(500);
    motor_selector.setAcceleration(250);
    motor_feeder.setMaxSpeed(1000);
    motor_feeder.setAcceleration(500);

    // if (!oled.begin(SH1106_SWITCHCAPVCC, 0x3C)) {
    //     Serial.println(F("failed to start OLED"));
    // }
    oled.begin(SH1106_SWITCHCAPVCC, 0x3C);
    oled.setRotation(2);
    oled.clearDisplay();
    Serial.println("[+] StripCutter 1.0");
    Serial.println("[+] Starting up...");
    Serial.println("[+] Melakukan Konfigurasi alat...");
    startup_display();

    xTaskCreate(motor_startup, "Motor Startup", 5000, NULL, 2, &motor_handle);
    do {
        gear(48, 15, 32, 32, 5);
    } while (eTaskGetState(motor_handle) != eDeleted);
    Serial.println("[+] Motor Startup Selesai");
    oled.clearDisplay();
    oled.display();
    load_config(&history_length, configuration_history);
    motor_handle = NULL;
}

void loop() {
    digitalWrite(LED_BUILTIN, LOW);
    // Home
    if (screen_state == 0) {
        int ret = home(configuration_history, history_length);
        if (ret == 0) {
            screen_state = 1;
        } else if (ret == 2) {
            Serial.println("[+] Masuk mode test motor");
            screen_state = 4;
        } else {
            memcpy(&crimp_configuration, &configuration_history[ret - 1], sizeof(crimp_configuration_t));
            screen_state = 3;
        }
    }
    // Konfigurasi Kabel
    if (screen_state == 1) {
        int ret = cable_configuration(&crimp_configuration);
        if (ret == -1)
            screen_state = 0;
        else
            screen_state = 2;
        Serial.println("[+] Konfigurasi Kabel Selesai");
    }
    // Konfigurasi Strip dan Potong
    else if (screen_state == 2) {
        int ret = strip_cut_configuration(&crimp_configuration);
        if (ret == -1)
            screen_state = 1;
        else
            screen_state = 3;
        Serial.println("[+] Konfigurasi Strip dan Potong Selesai");

    }
    // Proses
    else if (screen_state == 3) {
        Serial.println("[+] Proses dimulai");
        processing_display(0);
        execute_command_seq(&crimp_configuration);
        // xTaskCreate(execute_command, "Execute Command", 25000, &crimp_configuration, 3, &motor_handle);
        // int process_state = 0;
        // do {
        //     int ret_state = processing_display(process_state);
        //     if (ret_state == LONG_ROTARY_BUTTON) {
        //         Serial.println("[+] Proses Dihentikan (Long Rotary Button)");
        //         deleted_process();
        //         if (eTaskGetState(motor_handle) != eDeleted && eTaskGetState(motor_handle) != eBlocked)
        //             vTaskDelete(motor_handle);
        //         process_state = 2;
        //         screen_state = 0;
        //         break;
        //     } else if (ret_state == ROTARY_BUTTON) {
        //         if (eTaskGetState(motor_handle) == eSuspended) {
        //             Serial.println("[+] Proses Dilanjutkan (ROTARY_BUTTON)");
        //             process_state = 0;
        //             vTaskResume(motor_handle);
        //             // } else if (eTaskGetState(motor_handle) == eRunning) {
        //         } else {
        //             Serial.println("[+] Proses Dijeda (ROTARY_BUTTON)");
        //             process_state = 1;
        //             vTaskSuspend(motor_handle);
        //         }
        //     }

        // } while (eTaskGetState(motor_handle) != eDeleted);
        screen_state = 0;
        Serial.println("[+] Proses Selesai");
        cycle_config_history(configuration_history, crimp_configuration, &history_length);
        save_config(history_length);
        for (int i = 0; i < 5; i++) {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        }
    } else if (screen_state == 4) {
        int motor_type = 0;
        int selector_position = current_selector_position;
        int steps = motor_cutter_steps;
        int ret;

        do {
            ret = test_motor_display(motor_type, steps);
            if (ret == ROTARY_LEFT) {
                if (motor_type == 0) {
                    move_motor(MOTOR_CUTTER, -5);
                } else if (motor_type == 1) {
                    // move_motor(MOTOR_SELECTOR, -1);
                    change_selector(current_selector_position - 1);
                } else if (motor_type == 2) {
                    move_motor(MOTOR_FEEDER, -10);
                }
                // Serial.println("[+] Motor Type: " + String(motor_type));
                // Serial.println("[+] Steps: --" + String(steps));
            } else if (ret == ROTARY_RIGHT) {
                if (motor_type == 0) {
                    move_motor(MOTOR_CUTTER, 5);
                } else if (motor_type == 1) {
                    change_selector(current_selector_position + 1);
                } else if (motor_type == 2) {
                    move_motor(MOTOR_FEEDER, 10);
                }
                // Serial.println("[+] Motor Type: " + String(motor_type));
                // Serial.println("[+] Steps: ++" + String(steps));
            } else if (ret == ROTARY_BUTTON) {
                motor_type++;
                if (motor_type > 2) {
                    motor_type = 0;
                }
            } else if (ret == LONG_ROTARY_BUTTON) {
                screen_state = 0;
                break;
            }

            if (motor_type == 0) {
                steps = motor_cutter_steps;
            } else if (motor_type == 1) {
                steps = current_selector_position;
            } else if (motor_type == 2) {
                steps = 0;
            }

        } while (true);
        /* Reset Motor Position */
        xTaskCreate([](void *pvParameters) {
            // kalibrasi cutter sek

            // Save Posisi Selector
            // Check ae nek step selector e berubah iku di set flag
            // nek onok kabel seng metu tarik disek
            // balik nak posisi selector awal ta iso langsung reset ae
            // iku ae nek setiap ganti nak kontrol selector, feeder e narik kabel iso disurung sek sampek nemu proximity ne
            reset_selector();
            reset_cutter();
            reset_feeder();
            vTaskDelete(NULL);
        },
                    "Reset Position", 1024, NULL, 1, &motor_handle);
        do {
            gear(48, 15, 32, 32, 5);
        } while (eTaskGetState(motor_handle) != eDeleted);

        screen_state = 0;
    }
    motor_handle = NULL;
}
