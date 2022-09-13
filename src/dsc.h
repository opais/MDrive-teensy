#include <Arduino.h>

extern void evaluate_dsc_ign_status(const CAN_message_t &KCAN_Received_Msg);
extern void non_blocking_dsc_off();
extern void non_blocking_second_dtc_press();
extern void non_blocking_mdm_to_off();
extern void send_dtc_button_press();
extern void send_dsc_off_sequence();
