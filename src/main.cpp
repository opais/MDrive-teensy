#include <globals.h>
#include <avr/power.h>



void setup() 
{
  #if DEBUG_MODE == 1
    Serial.begin(115200);
    while(!Serial);                                                                                                                 // 32U4, wait until virtual port initialized.
  #endif

  // Initialise screen
  initialise_screen();

  // Init pins for the central console button cluster
  initialise_buttons_pins();

  // Initialise can controllers for KCAN and PTCAN
  initialise_can_controllers();
  
  // Read M-Drive settings from eeprom + other settings
  read_settings_from_eeprom();
  

  
}

void loop()
{
  non_blocking_dsc_off();
  non_blocking_second_dtc_press();
  non_blocking_mdm_to_off();

  if(ignition) {
    evaluate_dsc_button();
    evaluate_power_button();
  }
  // Handle any CAN events
  KCAN.events();
  PTCAN.events();



  // Handle mdrive messages when car is awake
  mdrive_awake_listener();

  // Handle the power/dsc button presses
  //center_console();
}



// /********************************
//         General functions
// ********************************/
void evaluate_ignition_status_cas(const CAN_message_t &KCAN_Received_Msg) {
  if (KCAN_Received_Msg.id == 0x130) {
    if (KCAN_Received_Msg.buf[0] == 0x40) {
      if(ignition == true && ign_acc == true){
        ignition = false;
        ign_acc = false;

        reset_runtime_variables();
        #if DEBUG_MODE
          Serial.println(F("Ignition & ACC OFF. Reset values."));
        #endif
      }
    } else if (KCAN_Received_Msg.buf[0] == 0x41) {
      if(ign_acc == false){
        ign_acc = true;

        #if DEBUG_MODE
          Serial.println(F("IGN ACC ON."));
        #endif
      }
    } else if (KCAN_Received_Msg.buf[0] == 0x45 || KCAN_Received_Msg.buf[0] == 0x55) {
      if(ignition == false) {
        ignition = true;
        #if DEBUG_MODE
          Serial.println(F("Ignition ON."));
        #endif
      }
    }
    
  }
}

void reset_runtime_variables()                                                                                                      // Ignition off. Set variables to original state and commit MDrive settings.
{
  dsc_program_last_status_can = 0xEA;
  dsc_program_status = 0;
  if (mdrive_status) {
    toggle_mdrive_message_active();
  }
  engine_running = false;
  RPM = 0;
  ignore_m_press = false;
  mdrive_power_active = console_power_mode = restore_console_power_mode = false;
  sending_dsc_off = send_second_dtc_press = send_dsc_off_from_mdm = false;
  sending_dsc_off_counter = 0;
  
  // #if EXHAUST_FLAP_CONTROL
  //   exhaust_flap_sport = false;
  //   digitalWrite(EXHAUST_FLAP_SOLENOID_PIN, LOW);
  //   exhaust_flap_open = true;
  // #endif

  // #if LAUNCH_CONTROL_INDICATOR
  //   lc_cc_active = clutch_pressed = vehicle_moving = false;
  // #endif

  #if CONTROL_SHIFTLIGHTS
    reset_shiftlights_runtime_vars();
  #endif
  //digitalWrite(BTNS_POWER_LED_PIN, LOW);

  if (mdrive_settings_change) {
    update_mdrive_settings_in_eeprom();
    mdrive_settings_change = false;
  }
}

// void initialise_timers()
// {
//   #if F_ZBE_WAKE
//     power_button_debounce_timer = dsc_off_button_debounce_timer = mdrive_message_timer 
//     = vehicle_awake_timer = zbe_wakeup_timer = millis();
//   #else
//     power_button_debounce_timer = dsc_off_button_debounce_timer = mdrive_message_timer 
//     = vehicle_awake_timer = millis();
//   #endif
// }

// // TODO: adapt to teensy platform
// void toggle_ptcan_sleep()
// {
//   if (!vehicle_awake) {
//     //PTCAN.setMode(MCP_SLEEP);
//     #if DEBUG_MODE
//       Serial.println(F("Deactivated PT-CAN MCP2515."));
//     #endif
//   } else {
//     //PTCAN.setMode(MCP_NORMAL);
//     #if DEBUG_MODE
//       Serial.println(F("Re-activated PT-CAN MCP2515."));
//     #endif
//   }
// }
