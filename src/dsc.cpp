#include <dsc.h>

void evaluate_dsc_ign_status(const CAN_message_t &KCAN_Received_Msg)
{
  if (dsc_program_last_status_can != KCAN_Received_Msg.buf[1]) {
    if (KCAN_Received_Msg.buf[1] == 0xEA) {
      ignition = false;
      reset_runtime_variables();
      #if DEBUG_MODE
        Serial.println(F("Ignition OFF. Reset values."));
      #endif
    } else if (KCAN_Received_Msg.buf[1] == 0xEC) {
      ignition = true;
      #if DEBUG_MODE
        Serial.println(F("Ignition ON."));
      #endif
    } else if (KCAN_Received_Msg.buf[1] == 0xE0) {
      ignition = true;                                                                                                              // Just in case 0xEC was missed.
      dsc_program_status = 0;
      #if DEBUG_MODE
          Serial.println(F("Stability control fully activated."));
      #endif
    } else if (KCAN_Received_Msg.buf[1] == 0xF0) {
      dsc_program_status = 1;
      #if DEBUG_MODE
          Serial.println(F("Stability control in DTC mode."));
      #endif
    } else if (KCAN_Received_Msg.buf[1] == 0xE4) {
      dsc_program_status = 2;
      #if DEBUG_MODE
          Serial.println(F("Stability control fully OFF."));
      #endif
    }
    dsc_program_last_status_can = KCAN_Received_Msg.buf[1];
  }

  if (KCAN_Received_Msg.buf[1] == 0xEA) {
    if (!vehicle_awake) {
      vehicle_awake = true;    
      //toggle_ptcan_sleep();                                                                                                         // Re-activate the controller.                                                                                         
      #if DEBUG_MODE
        Serial.println(F("Vehicle Awake."));
      #endif
    }
    vehicle_awake_timer = millis();                                                                                                 // Keep track of this message.
  }
  
  #if F_ZBE_WAKE
    send_zbe_wakeup();
  #endif
}


void non_blocking_dsc_off()
{
  if (sending_dsc_off) {
    if (sending_dsc_off_counter == 1) {
     sendKCanMessage(0x5A9, mdm_fake_cc_status);                                                                              // Start flashing MDM/DTC symbol to indicate transition.
    }

    if ((millis() - sending_dsc_off_timer) >= 100) {                                                                                // Hopefully, none of the code blocks for 100ms+.
      if (sending_dsc_off_counter < 25) {
        sendKCanMessage(0x316, dtc_button_pressed);
        
        sending_dsc_off_timer = millis();
        sending_dsc_off_counter++;
      } else {
        sendKCanMessage(0x5A9, mdm_fake_cc_status_off);
        sendPTCanMessage(0x316, dtc_button_released);
        
        #if DEBUG_MODE
          Serial.println(F("Sent DSC OFF sequence."));
        #endif
        
        sending_dsc_off = false;
        sending_dsc_off_counter = 0;
      }
    }
  }
}


void non_blocking_second_dtc_press()
{
  if (send_second_dtc_press) {
    if ((millis() - send_second_dtc_press_timer) >= 300) {
      send_second_dtc_press = false;
      send_dtc_button_press();
    } 
  }
}


void non_blocking_mdm_to_off()
{
  if (send_dsc_off_from_mdm) {
    if ((millis() - send_dsc_off_from_mdm_timer) >= 300) {
      send_dsc_off_from_mdm = false;
      send_dsc_off_sequence();
    }
  }       
}


void send_dtc_button_press() 
// Correct timing sequence as per trace is: 
// button press -> delay(100) -> button press -> delay(50) -> button release -> delay(160) -> button release -> delay(160)
// However, that interferes with program timing. A small delay will still be accepted.
{
  if (!sending_dsc_off) {                                                                                                         // Ignore while DSC OFF seq is still being transmitted.
    sendPTCanMessage(0x316, dtc_button_pressed);                                                                                // Two messages are sent during a quick press of the button (DTC mode).
    delay(5);
    sendPTCanMessage(0x316, dtc_button_pressed);
    delay(5);
    sendPTCanMessage(0x316, dtc_button_released);                                                                                // Send one DTC released to indicate end of DTC button press.
    #if DEBUG_MODE                        
      Serial.println(F("Sent single DTC button press."));
    #endif
  }
} 


void send_dsc_off_sequence() 
{
  if (!sending_dsc_off) {
    sending_dsc_off = true;   
    sendPTCanMessage(0x316, dtc_button_pressed); // Send the first press.
    sendKCanMessage(0x5A9, dsc_off_fake_cc_status);      // Trigger DSC OFF CC in Kombi, iDrive as soon as sequence starts.                                                                                                 // Begin the non-blocking sequence.
  }
}
