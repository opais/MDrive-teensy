#include <FlexCAN_T4.h>

#define CONTROL_SHIFTLIGHTS 1
#define DEBUG_MODE 1
#if DEBUG_MODE
    extern char serial_debug_string[128];
#endif


// screen.cpp
extern void initialise_screen();

// teensy-eeprom.cpp
extern void read_settings_from_eeprom();
extern void update_mdrive_settings_in_eeprom();

// buttons.cpp
extern void initialise_buttons_pins();
extern void evaluate_dsc_button();
extern void evaluate_power_button();

extern bool ignition;


// Mdrive - eeprom
extern uint8_t dsc_program_status;
extern uint8_t mdrive_message[];

// Vehicle status variables
extern uint32_t RPM;
extern bool ignition;
extern bool ign_acc;
extern bool engine_running;
extern bool vehicle_awake;
extern bool vehicle_moving;

extern bool engine_warmed_up;
extern bool clutch_pressed;
extern unsigned long vehicle_awake_timer;

extern bool mdrive_status;
extern bool mdrive_settings_change;

extern bool console_power_mode;
extern bool restore_console_power_mode;
extern bool mdrive_power_active;

// DSC.cpp
extern bool sending_dsc_off;
extern uint8_t sending_dsc_off_counter;
extern unsigned long sending_dsc_off_timer;
extern bool send_second_dtc_press;
extern bool send_dsc_off_from_mdm;
extern unsigned long send_second_dtc_press_timer;
extern unsigned long send_dsc_off_from_mdm_timer;
extern uint8_t dsc_program_last_status_can;

extern CAN_message_t PTCAN_Received_Msg;
extern CAN_message_t KCAN_Received_Msg;
extern FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> PTCAN;
extern FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> KCAN;

extern void evaluate_dsc_ign_status(const CAN_message_t &KCAN_Received_Msg);
extern void send_dtc_button_press();
extern void non_blocking_dsc_off();
extern void non_blocking_second_dtc_press();
extern void non_blocking_mdm_to_off();
extern void send_dtc_button_press();
extern void send_dsc_off_sequence();

// mdrive
extern unsigned long mdrive_message_timer;
extern void send_mdrive_message();
extern void update_mdrive_message_settings(bool reset, const CAN_message_t &PTCAN_Received_Msg);
extern void toggle_mdrive_message_active();
extern void toggle_mdrive_dsc();
extern void send_power_mode();
extern void can_checksum_update(uint16_t canid, uint8_t len,  uint8_t *message);
extern void mdrive_awake_listener();

// can.cpp
extern void sendPTCanMessage(uint16_t canid, uint8_t* data);
extern void sendKCanMessage(uint16_t canid, uint8_t* data);
extern void debug_can_message(uint16_t canid, uint8_t len, uint8_t* message);
extern void initialise_can_controllers();
extern void ptcanHandle(const CAN_message_t &PTCAN_Received_Msg);
extern void kcanHandle(const CAN_message_t &PTCAN_Received_Msg);


extern bool ignore_m_press;

// main.cpp
extern void evaluate_ignition_status_cas(const CAN_message_t &KCAN_Received_Msg);
extern void toggle_ptcan_sleep();
extern void reset_runtime_variables();


// shiftlights.cpp
extern void evaluate_shiftlight_display(const CAN_message_t &KCAN_Received_Msg);
extern void activate_shiftlight_segments(uint8_t* data);
extern void evaluate_shiftlight_sync(const CAN_message_t &PTCAN_Received_Msg);
extern void deactivate_shiftlight_segments();
extern void reset_shiftlights_runtime_vars();
