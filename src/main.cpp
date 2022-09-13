#include <Arduino.h>
#include <main.h>

#include <board.cpp>
#include <buttons.cpp>
#include <mdrive.cpp>
#include <dsc.cpp>
#include <shiftlights.cpp>
#include <launch-control.cpp>
#include <screen.cpp>
#include <zbe.cpp>

#include <avr/power.h>



void setup() 
{
  #if DEBUG_MODE == TRUE
    Serial.begin(115200);
    while(!Serial);                                                                                                                 // 32U4, wait until virtual port initialized.
  #endif

  // Init pins for the central console button cluster
  initialise_buttons_pins();

  // Initialise can controllers for KCAN and PTCAN
  initialise_can_controllers();
  
  // Read M-Drive settings from eeprom + other settings
  read_settings_from_eeprom();
  
  // Init timers
  initialise_timers();
  
  // Initialise screen
  initialise_screen();
  
  
}

void loop()
{
  // Handle any CAN events
  KCAN.events();
  PTCAN.events();

  non_blocking_dsc_off();
  non_blocking_second_dtc_press();
  non_blocking_mdm_to_off();

  // Handle mdrive messages when car is awake
  mdrive_awake_listener();

  // Handle the power/dsc button presses
  center_console();
}

/************************************************************************************************
        CAN handler functions
    These functions are called when a new message is received on that bus
    They are here for visibility
************************************************************************************************/
void ptcanHandle(const CAN_message_t &PTCAN_Received_Msg) {
  if (ignition) {
      if (PTCAN_Received_Msg.id == 0x1D6) {
        if (PTCAN_Received_Msg.buf[1] == 0x4C && !ignore_m_press) {                                                                                // M button is pressed.
          ignore_m_press = true;                                                                                                    // Ignore further pressed messages until the button is released.
          toggle_mdrive_message_active();
          send_mdrive_message();
          toggle_mdrive_dsc();                                                                                                      // Run DSC changing code after MDrive is turned on to hide how long DSC-OFF takes.
        } 
        if (PTCAN_Received_Msg.buf[0] == 0xC0 && PTCAN_Received_Msg.buf[1] == 0x0C && ignore_m_press) {                                                           // Button is released.
          ignore_m_press = false;
        }
      }
     
      #if CONTROL_SHIFTLIGHTS
        else if (PTCAN_Received_Msg.id == 0x332) {                                                                                                  // Monitor variable redline broadcast from DME.
          evaluate_shiftlight_sync();
        }
      #endif

      else if (PTCAN_Received_Msg.id == 0x3CA) {                                                                                                   // Receive settings from iDrive.      
        if (PTCAN_Received_Msg.buf[4] == 0xEC || PTCAN_Received_Msg.buf[4] == 0xF4 || PTCAN_Received_Msg.buf[4] == 0xE4) {                                                       // Reset requested.
          update_mdrive_message_settings(true);
          send_mdrive_message();
        } else if ((PTCAN_Received_Msg.buf[4] == 0xE0 || PTCAN_Received_Msg.buf[4] == 0xE1)) {                                                                    // Ignore E0/E1 (Invalid). 
          // Do nothing.
        } else {
          update_mdrive_message_settings(false);
          send_mdrive_message();                                                                                                    // Respond to iDrive.
        }
      }

      #if SERVOTRONIC_SVT70
      else if (PTCAN_Received_Msg.id == 0x58E) {
        if (PTCAN_Received_Msg.buf[1] == 0x49 && PTCAN_Received_Msg.buf[2] == 0) {                                                                                // Change from CC-ID 73 (EPS Inoperative) to CC-ID 70 (Servotronic).
          PTCAN_Received_Msg.buf[1] = 0x46;
        }
        //KCAN.sendMsgBuf(ptrxId, ptlen, ptrxBuf);                                                                                    // Forward the SVT status to KCAN.
      }
      #endif
    }
}

void kcanHandle(const CAN_message_t &KCAN_Received_Msg) {
  if (KCAN_Received_Msg.id == 0x26E) {
    evaluate_ignition_status_cas(KCAN_Received_Msg);
  }
  //Serial.println(KCAN_Received_Msg.id);

  if (KCAN_Received_Msg.id == 0x19E) {                                                                                                           // Monitor DSC K-CAN status.
    evaluate_dsc_ign_status(KCAN_Received_Msg);
    if (ignition) {
      send_power_mode();                                                                                                            // state_spt request from DME.   
      
      #if SERVOTRONIC_SVT70
        send_servotronic_message();
      #endif
    }
  }

  if (ignition) {
    if (KCAN_Received_Msg.id == 0xAA) {                                                                                                          // Monitor 0xAA (rpm/throttle status).
      RPM = ((uint32_t)KCAN_Received_Msg.buf[5] << 8) | (uint32_t)KCAN_Received_Msg.buf[4];
      #if CONTROL_SHIFTLIGHTS
        evaluate_shiftlight_display(KCAN_Received_Msg);
      #endif
      #if EXHAUST_FLAP_CONTROL
        if (engine_running) {
          evaluate_exhaust_flap_position();
        }
      #endif
      #if LAUNCH_CONTROL_INDICATOR
        evaluate_lc_display();
      #endif
    }

    #if LAUNCH_CONTROL_INDICATOR
    else if (KCAN_Received_Msg.id == 0xA8) {
      evaluate_clutch_status(KCAN_Received_Msg);
    }

    else if (KCAN_Received_Msg.id == 0x1B4) {
      evaluate_vehicle_moving(KCAN_Received_Msg);
    }
    #endif
  }
}

/********************************
        General functions
********************************/
void evaluate_ignition_status_cas(const CAN_message_t &KCAN_Received_Msg) {
  if (KCAN_Received_Msg.id == 0x26E) {
    #if DEBUG_MODE
      Serial.println("Caught CAS message - Evaluating ignition status.");
    #endif

    if (KCAN_Received_Msg.buf[1] == 0x00) {
      ignition = false;
      ign_acc = false;

      reset_runtime_variables();
      #if DEBUG_MODE
        Serial.println(F("Ignition & ACC OFF. Reset values."));
      #endif
    } else if (KCAN_Received_Msg.buf[1] == 0x40) {
      ign_acc = true;

      #if DEBUG_MODE
        Serial.println(F("IGN ACC ON."));
      #endif
      
      if (KCAN_Received_Msg.buf[0] == 0x40) {
        ignition = true;
        #if DEBUG_MODE
          Serial.println(F("Ignition ON."));
        #endif
      }
    }
  }

}

void initialise_timers()
{
  #if F_ZBE_WAKE
    power_button_debounce_timer = dsc_off_button_debounce_timer = mdrive_message_timer 
    = vehicle_awake_timer = zbe_wakeup_timer = millis();
  #else
    power_button_debounce_timer = dsc_off_button_debounce_timer = mdrive_message_timer 
    = vehicle_awake_timer = millis();
  #endif
}

// TODO: adapt to teensy platform
void toggle_ptcan_sleep()
{
  if (!vehicle_awake) {
    //PTCAN.setMode(MCP_SLEEP);
    #if DEBUG_MODE
      Serial.println(F("Deactivated PT-CAN MCP2515."));
    #endif
  } else {
    //PTCAN.setMode(MCP_NORMAL);
    #if DEBUG_MODE
      Serial.println(F("Re-activated PT-CAN MCP2515."));
    #endif
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

  #if LAUNCH_CONTROL_INDICATOR
    lc_cc_active = clutch_pressed = vehicle_moving = false;
  #endif

  #if CONTROL_SHIFTLIGHTS
    shiftlights_segments_active = engine_warmed_up = false;
    ignore_shiftlights_off_counter = 0;
    last_var_rpm_can = 0;
    START_UPSHIFT_WARN_RPM_ = START_UPSHIFT_WARN_RPM;
    MID_UPSHIFT_WARN_RPM_ = MID_UPSHIFT_WARN_RPM;
    MAX_UPSHIFT_WARN_RPM_ = MAX_UPSHIFT_WARN_RPM;
  #endif
  digitalWrite(BTNS_POWER_LED_PIN, LOW);

  if (mdrive_settings_change) {
    update_mdrive_settings_in_eeprom();
    mdrive_settings_change = false;
  }
}
