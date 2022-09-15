#include <Arduino.h>
#include <globals.h>

uint8_t dsc_program_status = 0;                                                    // 0 = on, 1 = DTC, 2 = DSC OFF

#if CONTROL_SHIFTLIGHTS
  uint8_t mdrive_message[] = {0, 0, 0, 0, 0, 0x97};                // byte 5: shiftlights always on
#else
  uint8_t mdrive_message[] = {0, 0, 0, 0, 0, 0x87};                // byte 5: shiftlights unchanged
#endif

// Vehicle status variables
uint32_t RPM = 0;
bool ignition = false;
bool ign_acc = false;
bool engine_running = false;
bool vehicle_awake = true;
bool vehicle_moving = true;

bool engine_warmed_up = false;
bool clutch_pressed = false;

bool mdrive_status = false;
bool mdrive_settings_change = false;

bool console_power_mode = false;
bool restore_console_power_mode = false;
bool mdrive_power_active = false;


// DSC
bool sending_dsc_off = false;
uint8_t sending_dsc_off_counter = 0;
bool send_second_dtc_press = false;
bool send_dsc_off_from_mdm = false;

bool ignore_m_press = false;
