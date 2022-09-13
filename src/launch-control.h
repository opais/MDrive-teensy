#include <Arduino.h>

extern void evaluate_lc_display();
extern void deactivate_lc_display();
extern void evaluate_vehicle_moving(const CAN_message_t &KCAN_Received_Msg);
extern void evaluate_clutch_status(const CAN_message_t &KCAN_Received_Msg);
