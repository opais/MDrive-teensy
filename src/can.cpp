#include <Arduino.h>
#include <FlexCAN_T4.h>
#include <globals.h>

// CAN_message_t PTCAN_Received_Msg;
// CAN_message_t KCAN_Received_Msg;

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> PTCAN;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> KCAN;


void initialise_can_controllers()
{
  /*
   *  PT-CAN init
   */
  PTCAN.begin();
  PTCAN.setBaudRate(500000);
  PTCAN.setMaxMB(16);
  PTCAN.enableFIFO();
  PTCAN.enableFIFOInterrupt();

  // MailBox for sending CAN messages
  PTCAN.setMBFilter(ACCEPT_ALL);
  PTCAN.setMB(MB8, TX);
  //PTCAN.distribute();

  // Filters for receiving CAN messsages via FIFO
  PTCAN.setFIFOFilter(ACCEPT_ALL);
  PTCAN.setFIFOFilter(0, 0x01D60000, STD); // MFL button status.                                         Cycle time 1s, 100ms (pressed)
  PTCAN.setFIFOFilter(1, 0x021A0000, STD); // Light status
  PTCAN.setFIFOFilter(2, 0x031D0000, STD); // MDrive status.                                             Cycle time 10s (idle), 160ms (change)
  PTCAN.setFIFOFilter(3, 0x03320000, STD); // FTM status broadcast by DSC                                  Cycle time 5s (idle)
  PTCAN.setFIFOFilter(4, 0x03CA0000, STD); // CIC MDrive settings
  PTCAN.setFIFOFilter(5, 0x58E00000, STD); // SVT70 Servotronic - Forward SVT CC to KCAN for KOMBI to display                  Cycle time 10

  // Print status to console
  PTCAN.onReceive(ptcanHandle);
  PTCAN.mailboxStatus();

  /*
   *  K-CAN init
   */
  KCAN.begin();
  KCAN.setBaudRate(100000);
  
  KCAN.setMaxMB(16);
  KCAN.enableFIFO();
  KCAN.enableFIFOInterrupt();

  // MailBox for sending CAN messages
  KCAN.setMBFilter(ACCEPT_ALL);
  KCAN.setMB(MB9, TX);
  //KCAN.distribute();

  // Filters for receiving CAN messsages via FIFO
  KCAN.setFIFOFilter(ACCEPT_ALL);
  KCAN.setFIFOFilter(0, 0x019E0000, STD); // DSC status and ignition                                      Cycle time 200ms (KCAN)
  KCAN.setFIFOFilter(1, 0x00AA0000, STD); // RPM, throttle pos.                                           Cycle time 100ms (KCAN)
  KCAN.setFIFOFilter(2, 0x00A80000, STD); // Clutch status                                                Cycle time 100ms (KCAN)
  KCAN.setFIFOFilter(3, 0x01B40000, STD); // Kombi status (speed, handbrake)                              Cycle time 100ms (terminal R on)
  KCAN.setFIFOFilter(4, 0x02320000, STD); // Driver's seat heating status                                 Cycle time 10s (idle), 150ms (change)
  KCAN.setFIFOFilter(5, 0x02CA0000, STD); // Ambient temperature                                          Cycle time 1s
  KCAN.setFIFOFilter(6, 0x01300000, STD); // Ignition status
  
  KCAN.distribute();
  KCAN.onReceive(kcanHandle);
  KCAN.mailboxStatus();

  #if DEBUG_MODE
    Serial.println("CAN-BUSes initialized successfully.");
  #endif
}


void debug_can_message(uint16_t canid, uint8_t len, uint8_t* message)
{   
  sprintf(serial_debug_string, "0x%.2X: ", canid);
  Serial.print(serial_debug_string);
  for (uint8_t i = 0; i<len; i++) {
    sprintf(serial_debug_string, " 0x%.2X", message[i]);
    Serial.print(serial_debug_string);
  }
  Serial.println();
}

void sendPTCanMessage(uint16_t canid, uint8_t* data) {
  CAN_message_t message;
  message.id = canid;
  for ( uint8_t i = 0; i < sizeof(data); i++ ) {
    message.buf[i] = data[i];
  }
  PTCAN.write(message);
}

void sendKCanMessage(uint16_t canid, uint8_t* data) {
  CAN_message_t message;
  message.id = canid;
  for ( uint8_t i = 0; i < sizeof(data); i++ ) {
    message.buf[i] = data[i];
  }
  PTCAN.write(message);
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
          evaluate_shiftlight_sync(PTCAN_Received_Msg);
        }
      #endif

      else if (PTCAN_Received_Msg.id == 0x3CA) {   
        Serial.println("idrive received");                                                                                                // Receive settings from iDrive.      
        if (PTCAN_Received_Msg.buf[4] == 0xEC || PTCAN_Received_Msg.buf[4] == 0xF4 || PTCAN_Received_Msg.buf[4] == 0xE4) {                                                       // Reset requested.
          Serial.println("idrive reset");
          update_mdrive_message_settings(true, PTCAN_Received_Msg);
          send_mdrive_message();
        //} else if ((PTCAN_Received_Msg.buf[4] == 0xE0 || PTCAN_Received_Msg.buf[4] == 0xE1)) {                                                                    // Ignore E0/E1 (Invalid). 
          // Do nothing.
        //  Serial.println("idrive invalid");
        } else {
          Serial.println("idrive update");
          update_mdrive_message_settings(false, PTCAN_Received_Msg);
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
  if (KCAN_Received_Msg.id == 0x130) {
    evaluate_ignition_status_cas(KCAN_Received_Msg);
  }

  if (KCAN_Received_Msg.id == 0x19E) {                                                                                                           // Monitor DSC K-CAN status.
    //evaluate_dsc_ign_status(KCAN_Received_Msg);
    if (ignition) {
      // send_power_mode();                                                                                                            // state_spt request from DME.   
      
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
