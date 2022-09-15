// //#include <FlexCAN_T4.h>
// #include <Arduino.h>
// /********************************
//         Main board
// ********************************/

                                                                           
// /********************************
//         Features to enable
// ********************************/
// #define FTM_INDICATOR 0
// #define FRONT_FOG_INDICATOR 0
// #define SERVOTRONIC_SVT70 0
// #define F_ZBE_WAKE 0
// #define DTC_WITH_M_BUTTON 0
// #define EDC_WITH_M_BUTTON 0
// #define LAUNCH_CONTROL_INDICATOR 1
// #define CONTROL_SHIFTLIGHTS 1
// #define EXHAUST_FLAP_CONTROL 0



// uint16_t DME_FAKE_VEH_MODE_CANID = 0x7F1;  
// uint8_t power_mode_only_dme_veh_mode[] = {0xE8, 0xF1};                                           // E8 is the last checksum. Start will be from 0A.



// // EDC
// uint8_t edc_status = 1;                                                                          // 1 = comfort, 2 = sport, 0xA = msport
// uint8_t edc_last_status_can = 0xF1;

// // Launch control
// bool lc_cc_active = false;
// const uint32_t LC_RPM = 2500*4;
// const uint32_t LC_RPM_MIN = LC_RPM - 250;
// const uint32_t LC_RPM_MAX = LC_RPM + 250;

// uint8_t lc_cc_on[] = {0x40, 0xBE, 0x01, 0x39, 0xFF, 0xFF, 0xFF, 0xFF};
// uint8_t lc_cc_off[] = {0x40, 0xBE, 0x01, 0x30, 0xFF, 0xFF, 0xFF, 0xFF};


// // F-Series ZBE wakeup
// byte f_wakeup[] = {0, 0, 0, 0, 0x57, 0x2F, 0, 0x60};
// byte zbe_response[] = {0xE1, 0x9D, 0, 0xFF};
// long zbe_wakeup_last_sent;


