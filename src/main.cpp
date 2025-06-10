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
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_ENCODER_CLK, INPUT);
    pinMode(PIN_ENCODER_DT, INPUT);
    pinMode(PIN_ENCODER_BT, INPUT_PULLUP);
    pinMode(PIN_BUTTON_A, INPUT_PULLUP);

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    // if (!oled.begin(SH1106_SWITCHCAPVCC, 0x3C)) {
    //     Serial.println(F("failed to start OLED"));
    // }
    oled.begin(SH1106_SWITCHCAPVCC, 0x3C);
    oled.clearDisplay();
    Serial.println("[+] StripCutter 1.0");
    Serial.println("[+] Starting up...");
    startup_display();
    Serial.println("[+] Melakukan Konfigurasi alat...");
    // membaca serial untuk test perintah
    // xTaskCreate(serial_reader, "Serial Reader", 2048, NULL, 1, NULL);
    xTaskCreate(motor_startup, "Motor Startup", 1024, NULL, 1, &motor_handle);
    do {
        gear(48, 15, 32, 32, 100);
    } while (eTaskGetState(motor_handle) != eDeleted);
    Serial.println("[+] Motor Startup Selesai");
    oled.clearDisplay();
    oled.display();
    load_config(&history_length, configuration_history);
    motor_handle = NULL;
}

void loop() {
    // Home
    if (screen_state == 0) {
        int ret = home(configuration_history, history_length);
        if (ret == 0) {
            screen_state = 1;
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
        xTaskCreate(execute_command, "Execute Command", 2048, &crimp_configuration, 1, &motor_handle);
        int process_state = 0;
        do {
            int ret_state = processing_display(process_state);
            if (ret_state == LONG_ROTARY_BUTTON) {
                Serial.println("[+] Proses Dihentikan (Long Rotary Button)");
                deleted_process();
                if (eTaskGetState(motor_handle) != eDeleted && eTaskGetState(motor_handle) != eBlocked)
                    vTaskDelete(motor_handle);
                process_state = 2;
                screen_state = 0;
            } else if (ret_state == ROTARY_BUTTON) {
                if (eTaskGetState(motor_handle) == eSuspended) {
                    Serial.println("[+] Proses Dilanjutkan (ROTARY_BUTTON)");
                    process_state = 0;
                    vTaskResume(motor_handle);
                    // } else if (eTaskGetState(motor_handle) == eRunning) {
                } else {
                    Serial.println("[+] Proses Dijeda (ROTARY_BUTTON)");
                    process_state = 1;
                    vTaskSuspend(motor_handle);
                }
            }

        } while (eTaskGetState(motor_handle) != eDeleted);
        screen_state = 0;
        Serial.println("[+] Proses Selesai");
        cycle_config_history(configuration_history, crimp_configuration, &history_length);
        save_config(history_length);
    } else if (screen_state == 3) {
    }
    motor_handle = NULL;
}
