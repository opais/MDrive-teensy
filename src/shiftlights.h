#include <Arduino.h>

extern void evaluate_shiftlight_display(const CAN_message_t &KCAN_Received_Msg);
extern void activate_shiftlight_segments(uint8_t* data);
extern void evaluate_shiftlight_sync();
extern void deactivate_shiftlight_segments();
