#include <FlexCAN_T4.h>
#include <Arduino.h>

extern CAN_message_t PTCAN_Received_Msg;
extern CAN_message_t KCAN_Received_Msg;

extern void initialise_can_controllers();
extern void debug_can_message(uint16_t canid, uint8_t len, uint8_t* message);
extern void sendPTCanMessage(uint16_t canid, uint8_t* data);
extern void sendKCanMessage(uint16_t canid, uint8_t* data);
