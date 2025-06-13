#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
// #include <Fonts/FreeSans9pt7b.h>
#include <SPI.h>
#include <Wire.h>

#define PIN_ENCODER_CLK 17
#define PIN_ENCODER_DT 16
#define PIN_ENCODER_BT 4

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// #define SCREEN_ADDRESS 0x3C
#define SCREEN_RESET -1
#define PIN_SCREEN_SCL 22
#define PIN_SCREEN_SDA 21

typedef enum {
    NONE = 0,
    ROTARY_BUTTON = 1,
    ROTARY_LEFT,
    ROTARY_RIGHT,
    BUTTON_A,
    LONG_ROTARY_BUTTON,
    LONG_BUTTON_A,
} BUTTON;

typedef struct crimp_configuration_t {
    int cable_type;
    int copy_count;
    int cable_thickness;
    int cable_length;
    int front_end_crimp_length;
    int back_end_crimp_length;
} crimp_configuration_t;

extern Adafruit_SH1106 oled;

/* Button */
BUTTON read_button();

/* Display */
void splash_anim();
void startup_display();
/* Configuration */
int home(crimp_configuration_t *history, int history_length);
int cable_configuration(crimp_configuration_t *crimp_configuration);
int strip_cut_configuration(crimp_configuration_t *crimp_configuration);
/* Processing */
int processing_display(int state);
int test_motor_display(int motor_type, int steps);

/* Animation */
void gear(int x_pos, int y_pos, int width, int height, int time);
void logo(int x_pos, int y_pos, int width, int height, int time);
void process(int frame);
void pause_process();
void deleted_process();