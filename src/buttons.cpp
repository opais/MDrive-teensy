#include <Arduino.h>
#include <globals.h>

#define BTNS_BACKLIGHT 2
#define BTNS_POWER_LED_PIN 3
#define BTNS_POWER_BUTTON_PIN 4
#define BTNS_DSC_BUTTON_PIN 5
#define BTNS_EDC_BUTTON_PIN 6
#define BTNS_EDC_LED_1_PIN 7
#define BTNS_EDC_LED_2_PIN 8

const uint16_t power_debounce_time_ms = 300;
const uint16_t dsc_debounce_time_ms = 200;
const uint16_t dsc_hold_time_ms = 400;

uint16_t power_button_debounce_timer;

uint16_t dsc_off_button_debounce_timer;
uint16_t dsc_off_button_hold_timer;



bool holding_dsc_off_console = false;

void initialise_buttons_pins() {
    // LEDs
    pinMode(BTNS_BACKLIGHT, OUTPUT);
    pinMode(BTNS_POWER_LED_PIN, OUTPUT);
    pinMode(BTNS_EDC_LED_1_PIN, OUTPUT);
    pinMode(BTNS_EDC_LED_2_PIN, OUTPUT);

    // Buttons
    pinMode(BTNS_POWER_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BTNS_DSC_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BTNS_EDC_BUTTON_PIN, INPUT_PULLUP);

    #if DEBUG_MODE
        Serial.println("CC button cluster pins initialized successfully.");
    #endif

}

void evaluate_power_button() {
  // Handle POWER button
  if (!digitalRead(BTNS_POWER_BUTTON_PIN)) {
    if ((millis() - power_button_debounce_timer) >= power_debounce_time_ms) {                                                     // POWER console button should only change throttle mapping.
      power_button_debounce_timer = millis();
      
      if (!console_power_mode) {
        if (!mdrive_power_active) {
          console_power_mode = true;
          #if DEBUG_MODE
            Serial.println(F("Console: POWER mode ON."));
          #endif 
        } else {
          mdrive_power_active = false;                                                                                            // If POWER button was pressed while MDrive POWER is active, disable POWER.
          #if DEBUG_MODE
            Serial.println(F("Deactivated MDrive POWER with console button press."));
          #endif
        }
      } else {
        #if DEBUG_MODE
          Serial.println(F("Console: POWER mode OFF."));
        #endif
        console_power_mode = false;
        if (mdrive_power_active) {
          mdrive_power_active = false;                                                                                            // If POWER button was pressed while MDrive POWER is active, disable POWER.
          #if DEBUG_MODE
            Serial.println(F("Deactivated MDrive POWER with console button press."));
          #endif
        }
      }
    }
  } 
}
  
void evaluate_dsc_button() {
  // Handle DSC button
  if (!digitalRead(BTNS_DSC_BUTTON_PIN)) {
    if (dsc_program_status == 0) {
      if (!holding_dsc_off_console) {
        holding_dsc_off_console = true;
        dsc_off_button_hold_timer = millis();
      } else {
        if ((millis() - dsc_off_button_hold_timer) >= dsc_hold_time_ms) {                                                         // DSC OFF sequence should only be sent after user holds button for a configured time
          #if DEBUG_MODE
            if (!sending_dsc_off) {
              Serial.println(F("Console: DSC OFF button held. Sending DSC OFF."));
            }
          #endif
          send_dsc_off_sequence();
          dsc_off_button_debounce_timer = millis();
        }
      }      
    } else {
      if ((millis() - dsc_off_button_debounce_timer) >= dsc_debounce_time_ms) {                                                   // A quick tap re-enables everything
        #if DEBUG_MODE
          if (!sending_dsc_off) {
            Serial.println(F("Console: DSC button tapped. Re-enabling DSC normal program."));
          }
        #endif
        dsc_off_button_debounce_timer = millis();
        send_dtc_button_press();
      }
    }
  } else {
    holding_dsc_off_console = false;
  }
}
