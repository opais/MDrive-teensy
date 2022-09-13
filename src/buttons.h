#include <Arduino.h>

#define BTNS_BACKLIGHT 2
#define BTNS_POWER_LED_PIN 3
#define BTNS_POWER_BUTTON_PIN 4
#define BTNS_DSC_BUTTON_PIN 5
#define BTNS_EDC_BUTTON_PIN 6
#define BTNS_EDC_LED_1_PIN 7
#define BTNS_EDC_LED_2_PIN 8

extern void initialise_buttons_pins();
extern void center_console();
