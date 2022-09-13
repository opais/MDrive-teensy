#include <Arduino.h>

#if CONTROL_SHIFTLIGHTS
  extern uint8_t mdrive_message[] = {0, 0, 0, 0, 0, 0x97};                                                                                   // byte 5: shiftlights always on
#else
  extern uint8_t mdrive_message[] = {0, 0, 0, 0, 0, 0x87};                                                                                   // byte 5: shiftlights unchanged
#endif

extern uint8_t mdrive_dsc = 0x03, mdrive_power = 0, mdrive_edc = 0x20, mdrive_svt = 0xE9;
extern bool mdrive_status = false; 
extern bool mdrive_power_active = false;
extern unsigned long mdrive_message_timer;

extern void mdrive_awake_listener();
extern void send_power_mode();
extern void can_checksum_update(uint16_t canid, uint8_t len,  uint8_t *message);
extern void update_mdrive_message_settings(bool reset);
extern void send_mdrive_message();
extern void toggle_mdrive_dsc();
extern void toggle_mdrive_message_active();
