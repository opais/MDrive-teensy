#include <zbe.h>

void send_zbe_wakeup()
{
  CAN_message_t msgKcan_out;
  msgKcan_out.id = 0x560;
  for ( uint8_t i = 0; i < sizeof(f_wakeup); i++ ) {
    msgKcan_out.buf[i] = f_wakeup[i];
  }
  KCAN.write(msgKcan_out);

  zbe_wakeup_last_sent = millis();
  #if DEBUG_MODE
    Serial.println("Sent F-ZBE wake-up message");
  #endif
}
