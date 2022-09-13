#include <board.h>

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

  KCAN.distribute();
  KCAN.onReceive(kcanHandle);
  KCAN.mailboxStatus();

  #if DEBUG_MODE
    Serial.println("CAN-BUSes initialized successfully.");
  #endif
}


void debug_can_message(uint16_t canid, uint8_t len, uint8_t* message)
{   
  char serial_debug_string[128];
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


