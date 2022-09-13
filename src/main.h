#include <Arduino.h>


/********************************
        Main board
********************************/
#define DEBUG_MODE TRUE

                                                                           
/********************************
        Features to enable
********************************/
#define FTM_INDICATOR 0
#define FRONT_FOG_INDICATOR 0
#define SERVOTRONIC_SVT70 0
#define F_ZBE_WAKE 0
#define DTC_WITH_M_BUTTON 0
#define EDC_WITH_M_BUTTON 0
#define LAUNCH_CONTROL_INDICATOR 1
#define CONTROL_SHIFTLIGHTS 1
#define EXHAUST_FLAP_CONTROL 0

#if DEBUG_MODE
  char serial_debug_string[128];
#endif


/********************************
             Misc
********************************/
uint16_t DME_FAKE_VEH_MODE_CANID = 0x7F1;  

/********************************
        Runtime variables
********************************/

// Vehicle status variables
uint32_t RPM = 0;
bool ignition = false;
bool ign_acc = false;
bool engine_running = false;
bool vehicle_awake = true;
bool vehicle_moving = true;

bool engine_warmed_up = false;
bool clutch_pressed = false;
unsigned long vehicle_awake_timer;

bool mdrive_status = false;
bool mdrive_settings_change = false;

bool console_power_mode = false;
bool restore_console_power_mode = false;
bool mdrive_power_active = false;

uint32_t var_redline_position;
uint8_t last_var_rpm_can = 0;

bool ignore_m_press = false;

// Current actions status vars
bool holding_dsc_off_console = false;
bool sending_dsc_off = false;

// DTC button messages
uint8_t dtc_button_pressed[] = {0xFD, 0xFF};
uint8_t dtc_button_released[] = {0xFC, 0xFF};
uint8_t dsc_off_fake_cc_status[] = {0x40, 0x24, 0, 0x1D, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t mdm_fake_cc_status[] = {0x40, 0xB8, 0, 0x45, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t mdm_fake_cc_status_off[] = {0x40, 0xB8, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Debounce, central console vars
unsigned long mdrive_message_timer;
unsigned long power_button_debounce_timer;
unsigned long dsc_off_button_debounce_timer;
unsigned long dsc_off_button_hold_timer;

const uint16_t power_debounce_time_ms = 300;
const uint16_t dsc_debounce_time_ms = 200;
const uint16_t dsc_hold_time_ms = 400;

// DSC, DTC button
uint8_t dsc_program_status = 0;                                                                  // 0 = on, 1 = DTC, 2 = DSC OFF
uint8_t dsc_program_last_status_can = 0xEA;
uint8_t power_mode_only_dme_veh_mode[] = {0xE8, 0xF1};                                           // E8 is the last checksum. Start will be from 0A.

uint8_t sending_dsc_off_counter = 0;
unsigned long sending_dsc_off_timer;
bool send_second_dtc_press = false;
bool send_dsc_off_from_mdm = false;
unsigned long send_second_dtc_press_timer;
unsigned long send_dsc_off_from_mdm_timer;

// EDC
uint8_t edc_status = 1;                                                                          // 1 = comfort, 2 = sport, 0xA = msport
uint8_t edc_last_status_can = 0xF1;

// Launch control
bool lc_cc_active = false;
const uint32_t LC_RPM = 2500*4;
const uint32_t LC_RPM_MIN = LC_RPM - 250;
const uint32_t LC_RPM_MAX = LC_RPM + 250;

uint8_t lc_cc_on[] = {0x40, 0xBE, 0x01, 0x39, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t lc_cc_off[] = {0x40, 0xBE, 0x01, 0x30, 0xFF, 0xFF, 0xFF, 0xFF};

// Shift Lights
const uint32_t START_UPSHIFT_WARN_RPM = 5500*4;
const uint32_t MID_UPSHIFT_WARN_RPM = 6000*4;
const uint32_t MAX_UPSHIFT_WARN_RPM = 6500*4;
const uint32_t VAR_REDLINE_OFFSET_RPM = -300;

uint32_t START_UPSHIFT_WARN_RPM_ = START_UPSHIFT_WARN_RPM;
uint32_t MID_UPSHIFT_WARN_RPM_ = MID_UPSHIFT_WARN_RPM;
uint32_t MAX_UPSHIFT_WARN_RPM_ = MAX_UPSHIFT_WARN_RPM;

bool shiftlights_segments_active = false;
byte shiftlights_start[] = {0x86, 0x3E};
byte shiftlights_mid_buildup[] = {0xF6, 0};
byte shiftlights_startup_buildup[] = {0x56, 0};
byte shiftlights_max_flash[] = {0x0A, 0};
byte shiftlights_off[] = {0x05, 0};

uint8_t ignore_shiftlights_off_counter = 0;


// F-Series ZBE wakeup
byte f_wakeup[] = {0, 0, 0, 0, 0x57, 0x2F, 0, 0x60};
byte zbe_response[] = {0xE1, 0x9D, 0, 0xFF};
long zbe_wakeup_last_sent;


extern void ptcanHandle(const CAN_message_t &PTCAN_Received_Msg);
extern void kcanHandle(const CAN_message_t &KCAN_Received_Msg);
extern void evaluate_ignition_status_cas(const CAN_message_t &KCAN_Received_Msg);
extern void initialise_timers();
extern void toggle_ptcan_sleep();
extern void reset_runtime_variables();
