#include <interface.h>

// Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SH1106 oled(-1);

bool rotary_clk_state = LOW;
BUTTON read_button() {
    int temp_rotary_clk_state = digitalRead(PIN_ENCODER_CLK);
    if (rotary_clk_state != temp_rotary_clk_state) {
        rotary_clk_state = temp_rotary_clk_state;
        int dt_value = digitalRead(PIN_ENCODER_DT);
        if (temp_rotary_clk_state == LOW && dt_value == HIGH) {
            // Serial.println("[INPUT] Putar Kiri");
            return ROTARY_LEFT;
        }
        if (temp_rotary_clk_state == LOW && dt_value == LOW) {
            // Serial.println("[INPUT] Putar Kanan");
            return ROTARY_RIGHT;
        }
    }
    if (digitalRead(PIN_ENCODER_BT) == LOW) {
        int low_millis = millis();
        while (digitalRead(PIN_ENCODER_BT) == LOW) {
            if (millis() - low_millis > 1000) {
                // Serial.println("[INPUT] Tombol Putar Panjang");
                return LONG_ROTARY_BUTTON;
            }
        }
        // Serial.println("[INPUT] Tombol Putar");
        return ROTARY_BUTTON;
    }
    // if (digitalRead(PIN_BUTTON_A) == LOW) {
    //     int low_millis = millis();
    //     while (digitalRead(PIN_BUTTON_A) == LOW) {
    //         if (millis() - low_millis > 1000) {
    //             Serial.println("[INPUT] Tombol A Panjang");
    //             return LONG_BUTTON_A;
    //         }
    //     }
    //     Serial.println("[INPUT] Tombol A");
    //     return BUTTON_A;
    // }
    return NONE;
}

void startup_display() {
    oled.clearDisplay();
    oled.display();

    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0, 0);
    oled.println("StripCutter 1.0");
    oled.display();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    delay(500);
    logo(17, 21, 94, 23, 50);
    do {
        oled.clearDisplay();
        oled.setCursor(0, 19);
        oled.println("Tekan Tombol Apa Saja");
        oled.setCursor(0, 29);
        oled.println("untuk mengkonfigurasi");
        oled.setCursor(62, 45);
        oled.println("v");
        oled.display();

        oled.clearDisplay();
        oled.setCursor(0, 19);
        oled.println("Tekan Tombol Apa Saja");
        oled.setCursor(0, 29);
        oled.println("untuk mengkonfigurasi");
        oled.setCursor(62, 48);
        oled.println("v");
        oled.display();
    } while (!read_button());
    // gear(48, 15, 32, 32, 25);
}

int home(crimp_configuration_t *history, int history_length) {
    // for (int i = 0; i < history_length; ++i) {
    //     Serial.print("History ");
    //     Serial.print(i);
    //     Serial.print(": ");
    //     Serial.print("Cable Type: ");
    //     Serial.println(history[i].cable_type);
    //     Serial.print("Cable Length: ");
    //     Serial.println(history[i].cable_length);
    //     Serial.print("Cable Thickness: ");
    //     Serial.println(history[i].cable_thickness);
    //     Serial.print("Front End Crimp Length: ");
    //     Serial.println(history[i].front_end_crimp_length);
    //     Serial.print("Back End Crimp Length: ");
    //     Serial.println(history[i].back_end_crimp_length);
    // }
    int position[4][2] = {
        {7, 14},  // Position for cable type
        {7, 27},  // Position for cable thickness
        {7, 40},  // Position for copy count
    };

    int selection = 0;
    BUTTON button;
    do {
        oled.clearDisplay();
        oled.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
        oled.setCursor(7, 3);
        oled.setTextSize(1);
        oled.setTextColor(WHITE);
        oled.println("--- Konfigurasi ---");

        for (int i = 0; i < history_length + 1; ++i) {
            oled.setCursor(4, 14 + i * 10);
            oled.setTextColor(BLACK);
            if (i == selection) {
                oled.setTextColor(BLACK);
                oled.fillRect(position[i][0], position[i][1], 128 - 14, 14, WHITE);
            } else {
                oled.setTextColor(WHITE);
                oled.drawRect(position[i][0], position[i][1], 128 - 14, 14, WHITE);
            }
            if (i == 0) {
                oled.setCursor(position[i][0] + 4, position[i][1] + 3);
                oled.println("Baru");
            } else {
                String text = "C" + String(history[i - 1].cable_type) +
                              ",CN" + String(history[i - 1].copy_count) +
                              ",T" + String(history[i - 1].cable_thickness) +
                              ",F" + String(history[i - 1].front_end_crimp_length) +
                              ",B" + String(history[i - 1].back_end_crimp_length) +
                              ",L" + String(history[i - 1].cable_length);
                oled.setCursor(position[i][0] + 4, position[i][1] + 3);
                oled.println(text);
            }
        }
        oled.display();

        do {
            button = read_button();
            if (button) break;
        } while (true);
        if (button == ROTARY_LEFT) {
            selection++;
            if (selection > history_length) {
                selection = 0;
            }
        } else if (button == ROTARY_RIGHT) {
            selection--;
            if (selection < 0) {
                selection = history_length;
            }
        } else if (button == ROTARY_BUTTON) {
            // Serial.println("[+] Memilih konfigurasi kabel");
            // Serial.print("Cable Type: ");
            // Serial.println(selected_config.cable_type);
            // Serial.print("Cable Thickness: ");
            // Serial.println(selected_config.cable_thickness);
            // Serial.print("Cable Length: ");
            // Serial.println(selected_config.cable_length);
            // Serial.print("Front End Crimp Length: ");
            // Serial.println(selected_config.front_end_crimp_length);
            // Serial.print("Back End Crimp Length: ");
            // Serial.println(selected_config.back_end_crimp_length);
            return selection;  // Return success
        } else if (button == LONG_ROTARY_BUTTON) {
            return 2;
        }
    } while (true);
}

int cable_configuration(crimp_configuration_t *crimp_configuration) {
    Serial.println("[+] Screen 0 Konfigurasi Kabel (Jenis, Kuantitas , Ketebalan)");

    int i;  // Iterator
    int position[4][2] = {
        {4, 14},   // Position for cable type
        {66, 14},  // Position for cable thickness
        {4, 39},   // Position for copy count
        {66, 39}   // Position next button
    };
    int box_size[2] = {58, 20};  // Width and height of the boxes
    // outline selection -2 x -2 y + 4 width + 4 height

    int data[3] = {1, 1, 0};  // 0: cable type, 1: copy count, 2: cable thickness
    int selection = 0;
    bool in_selected = false;

    BUTTON button;

    do {
        oled.clearDisplay();
        oled.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);

        oled.setTextSize(1);
        oled.setTextColor(WHITE);
        oled.setCursor(15, 3);
        oled.println("Konfigurasi Kabel");
        String text_1 = "Type :" + String(data[0]);
        String text_2 = "Count:" + String(data[1]);
        String text_3 = "Thick:" + String(data[2]);
        String text_4 = "Next";
        int text_x_offset[4] = {6, 6, 6, 17};
        int text_y_offset = 6;

        for (i = 0; i < 4; ++i) {
            if (i == selection) {
                if (in_selected || i == 3) {
                    oled.setTextColor(BLACK);  // Invert color for selection
                    oled.fillRect(position[i][0], position[i][1], box_size[0], box_size[1], WHITE);

                } else {
                    oled.drawRect(position[i][0], position[i][1], box_size[0], box_size[1], WHITE);
                    oled.drawRect(position[i][0] - 2, position[i][1] - 2, box_size[0] + 4, box_size[1] + 4, WHITE);
                }
            } else {
                oled.drawRect(position[i][0], position[i][1], box_size[0], box_size[1], WHITE);
            }

            if (in_selected) {
                oled.setTextSize(1);
                if (i == selection) {
                    oled.setTextColor(BLACK);
                    if (i == 0) {
                        oled.setCursor(position[i][0] + text_x_offset[0], position[i][1] + text_y_offset);
                        oled.println(text_1);
                    } else if (i == 1) {
                        oled.setCursor(position[i][0] + text_x_offset[1], position[i][1] + text_y_offset);
                        oled.println(text_2);
                    } else if (i == 2) {
                        oled.setCursor(position[i][0] + text_x_offset[2], position[i][1] + text_y_offset);
                        oled.println(text_3);
                    }
                } else {
                    oled.setTextColor(WHITE);
                    if (i == 0) {
                        oled.setCursor(position[i][0] + text_x_offset[0], position[i][1] + text_y_offset);
                        oled.println(text_1);
                    } else if (i == 1) {
                        oled.setCursor(position[i][0] + text_x_offset[1], position[i][1] + text_y_offset);
                        oled.println(text_2);
                    } else if (i == 2) {
                        oled.setCursor(position[i][0] + text_x_offset[2], position[i][1] + text_y_offset);
                        oled.println(text_3);
                    } else if (i == 3) {
                        if (i == selection) {
                            oled.setTextColor(BLACK);
                        } else {
                            oled.setTextColor(WHITE);
                        }
                        oled.setCursor(position[i][0] + text_x_offset[3], position[i][1] + text_y_offset);
                        oled.println(text_4);
                    }
                }
            } else {
                oled.setTextColor(WHITE);
                oled.setTextSize(1);
                if (i == 0) {
                    oled.setCursor(position[i][0] + text_x_offset[0], position[i][1] + text_y_offset);
                    oled.println(text_1);
                } else if (i == 1) {
                    oled.setCursor(position[i][0] + text_x_offset[1], position[i][1] + text_y_offset);
                    oled.println(text_2);
                } else if (i == 2) {
                    oled.setCursor(position[i][0] + text_x_offset[2], position[i][1] + text_y_offset);
                    oled.println(text_3);
                } else if (i == 3) {
                    if (i == selection) {
                        oled.setTextColor(BLACK);
                    } else {
                        oled.setTextColor(WHITE);
                    }
                    oled.setCursor(position[i][0] + text_x_offset[3], position[i][1] + text_y_offset);
                    oled.println(text_4);
                }
            }
        }
        oled.display();

        do {
            button = read_button();
            if (button) break;
        } while (true);

        if (button == ROTARY_LEFT) {
            if (in_selected) {
                if ((data[selection] > 0)) {
                    data[selection]--;
                    if (data[0] < 1) {
                        data[0] = 1;  // Reset cable type if it goes below 1
                    } else if (data[1] < 1) {
                        data[1] = 1;  // Reset copy count if it goes below 1
                    }
                }
            } else {
                selection--;
                if (selection < 0) {
                    selection = 3;
                }
            }
        } else if (button == ROTARY_RIGHT) {
            if (in_selected) {
                if (data[selection] >= 0) {
                    data[selection]++;
                    if (data[0] > 5) {
                        data[0] = 5;  // Reset cable type if it exceeds the limit
                    }
                }
            } else {
                selection++;
                if (selection > 3) {
                    selection = 0;
                }
            }
        } else if (button == ROTARY_BUTTON) {
            if (selection != 3)
                in_selected = !in_selected;
            else {
                // Save configuration
                crimp_configuration->cable_type = data[0];
                crimp_configuration->copy_count = data[1];
                crimp_configuration->cable_thickness = data[2];
                crimp_configuration->front_end_crimp_length = 0;  // Default value
                crimp_configuration->back_end_crimp_length = 0;   // Default value
                crimp_configuration->cable_length = 0;            // Default value
                Serial.println("[+] Konfigurasi Kabel Disimpan");
                Serial.print("Jenis Kabel: ");
                Serial.println(crimp_configuration->cable_type);
                Serial.print("Kuantitas: ");
                Serial.println(crimp_configuration->copy_count);
                Serial.print("Ketebalan Kabel: ");
                Serial.println(crimp_configuration->cable_thickness);
                return 0;  // Exit configuration
            }
        } else if (button == LONG_ROTARY_BUTTON) {
            return -1;
        }
        oled.clearDisplay();
        // oled.display();
    } while (true);
}

int strip_cut_configuration(crimp_configuration_t *crimp_configuration) {
    Serial.println("[+] Screen 0 Konfigurasi Strip dan Potong (Potongan awal, Potongan belakang, Panjang Kabel)");

    int i;  // Iterator
    int position[4][2] = {
        {4, 14},   // Position for cable type
        {66, 14},  // Position for cable thickness
        {4, 39},   // Position for copy count
        {66, 39}   // Position next button
    };
    int box_size[2] = {58, 20};  // Width and height of the boxes
    // outline selection -2 x -2 y + 4 width + 4 height

    int data[3] = {0, 0, 0};  // 0: cable type, 1: copy count, 2: cable thickness
    int selection = 0;
    bool in_selected = false;

    BUTTON button;

    do {
        oled.clearDisplay();
        oled.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);

        oled.setTextSize(1);
        oled.setTextColor(WHITE);
        oled.setCursor(15, 3);
        oled.println("Konfigurasi Strip");

        String text_1 = "Front:" + String(data[0]);
        String text_2 = "Back :" + String(data[1]);
        String text_3 = "Length:" + String(data[2]);
        String text_4 = "Start";

        int text_x_offset[4] = {6, 6, 6, 15};
        int text_y_offset = 6;

        for (i = 0; i < 4; ++i) {
            if (i == selection) {
                if (in_selected || i == 3) {
                    oled.setTextColor(BLACK);  // Invert color for selection
                    oled.fillRect(position[i][0], position[i][1], box_size[0], box_size[1], WHITE);

                } else {
                    oled.drawRect(position[i][0], position[i][1], box_size[0], box_size[1], WHITE);
                    oled.drawRect(position[i][0] - 2, position[i][1] - 2, box_size[0] + 4, box_size[1] + 4, WHITE);
                }
            } else {
                oled.drawRect(position[i][0], position[i][1], box_size[0], box_size[1], WHITE);
            }

            if (in_selected) {
                oled.setTextSize(1);
                if (i == selection) {
                    oled.setTextColor(BLACK);
                    if (i == 0) {
                        oled.setCursor(position[i][0] + text_x_offset[0], position[i][1] + text_y_offset);
                        oled.println(text_1);
                    } else if (i == 1) {
                        oled.setCursor(position[i][0] + text_x_offset[1], position[i][1] + text_y_offset);
                        oled.println(text_2);
                    } else if (i == 2) {
                        oled.setCursor(position[i][0] + text_x_offset[2], position[i][1] + text_y_offset);
                        oled.println(text_3);
                    }
                } else {
                    oled.setTextColor(WHITE);
                    if (i == 0) {
                        oled.setCursor(position[i][0] + text_x_offset[0], position[i][1] + text_y_offset);
                        oled.println(text_1);
                    } else if (i == 1) {
                        oled.setCursor(position[i][0] + text_x_offset[1], position[i][1] + text_y_offset);
                        oled.println(text_2);
                    } else if (i == 2) {
                        oled.setCursor(position[i][0] + text_x_offset[2], position[i][1] + text_y_offset);
                        oled.println(text_3);
                    } else if (i == 3) {
                        if (i == selection) {
                            oled.setTextColor(BLACK);
                        } else {
                            oled.setTextColor(WHITE);
                        }
                        oled.setCursor(position[i][0] + text_x_offset[3], position[i][1] + text_y_offset);
                        oled.println(text_4);
                    }
                }
            } else {
                oled.setTextColor(WHITE);
                oled.setTextSize(1);
                if (i == 0) {
                    oled.setCursor(position[i][0] + text_x_offset[0], position[i][1] + text_y_offset);
                    oled.println(text_1);
                } else if (i == 1) {
                    oled.setCursor(position[i][0] + text_x_offset[1], position[i][1] + text_y_offset);
                    oled.println(text_2);
                } else if (i == 2) {
                    oled.setCursor(position[i][0] + text_x_offset[2], position[i][1] + text_y_offset);
                    oled.println(text_3);
                } else if (i == 3) {
                    if (i == selection) {
                        oled.setTextColor(BLACK);
                    } else {
                        oled.setTextColor(WHITE);
                    }
                    oled.setCursor(position[i][0] + text_x_offset[3], position[i][1] + text_y_offset);
                    oled.println(text_4);
                }
            }
        }
        oled.display();

        do {
            button = read_button();
            if (button) break;
        } while (true);

        if (button == ROTARY_LEFT) {
            if (in_selected) {
                if (!(data[selection] <= 0)) {
                    data[selection]--;
                }
            } else {
                selection--;
                if (selection < 0) {
                    selection = 3;
                }
            }
        } else if (button == ROTARY_RIGHT) {
            if (in_selected) {
                if (data[selection] >= 0) {
                    data[selection]++;
                }
            } else {
                selection++;
                if (selection > 3) {
                    selection = 0;
                }
            }
        } else if (button == ROTARY_BUTTON) {
            if (selection != 3)
                in_selected = !in_selected;
            else {
                // Save configuration
                crimp_configuration->front_end_crimp_length = data[0];
                crimp_configuration->back_end_crimp_length = data[1];
                crimp_configuration->cable_length = data[2];
                Serial.println("[+] Konfigurasi Strip Disimpan");
                Serial.print("Strip front(mm): ");
                Serial.println(crimp_configuration->front_end_crimp_length);
                Serial.print("Strip back(mm): ");
                Serial.println(crimp_configuration->back_end_crimp_length);
                Serial.print("Panjang kabel: ");
                Serial.println(crimp_configuration->cable_length);
                return 0;  // Exit configuration
            }
        } else if (button == LONG_ROTARY_BUTTON) {
            return -1;
        }
        oled.clearDisplay();
        // oled.display();
    } while (true);
}

int processing_display(int state) {
    static int frame = 0;
    if (state == 0) {
        process(frame);
    } else if (state == 1) {
        pause_process();
    } else if (state == 2) {
        deleted_process();
    }
    return read_button();

    frame++;
    if (frame > 4) {
        frame = 0;
    }
    return frame;
}

int test_motor_display(int motor_type, int steps) {
    BUTTON button;
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    oled.setTextColor(WHITE);
    oled.setCursor(11, 3);
    oled.println("--- Test Motor ---");
    oled.drawRect(0, 12, SCREEN_WIDTH, 2, WHITE);
    oled.setCursor(4, 18);
    if (motor_type == 0) {
        oled.println("Motor Type: Cutter");
    } else if (motor_type == 1) {
        oled.println("Motor Type: Selector");
    } else if (motor_type == 2) {
        oled.println("Motor Type: Feeder");
    }
    oled.setCursor(4, 32);
    String text;
    if (motor_type == 1) {
        text = "Selector Position: " + String(steps);
    } else if (motor_type == 0) {
        text = "Cutter Steps: " + String(steps);
    } else if (motor_type == 2) {
        text = "None";
    } else
        text = "Steps: " + String(steps);
    oled.println(text);
    oled.display();
    do {
        button = read_button();
        if (button) return button;

    } while (true);
}