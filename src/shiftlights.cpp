#include <Arduino.h>
#include <globals.h>

const uint32_t START_UPSHIFT_WARN_RPM = 3500*4;
const uint32_t MID_UPSHIFT_WARN_RPM = 4000*4;
const uint32_t MAX_UPSHIFT_WARN_RPM = 4500*4;
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
uint32_t var_redline_position;
uint8_t last_var_rpm_can = 0;
char serial_debug_string[128];

void evaluate_shiftlight_display(const CAN_message_t &KCAN_Received_Msg)
{
  if (!engine_running && (RPM > 2000)) {                                                                                            // Show off shift light segments during engine startup (>500rpm).
    engine_running = true;
    activate_shiftlight_segments(shiftlights_startup_buildup);
    #if DEBUG_MODE
      Serial.println(F("Showing shift light on engine startup."));
    #endif
    ignore_shiftlights_off_counter = 10;                                                                                            // Skip a few off cycles to allow segments to light up.

    #if EXHAUST_FLAP_WITH_M_BUTTON
      exhaust_flap_action_timer = millis();                                                                                         // Start tracking the exhaust flap.
    #endif
  }

  if (RPM >= START_UPSHIFT_WARN_RPM_ && RPM <= MID_UPSHIFT_WARN_RPM_) {                                                             // First yellow segment.                                                              
    activate_shiftlight_segments(shiftlights_start);
    #if DEBUG_MODE
      sprintf(serial_debug_string, "Displaying first warning at RPM: %ld\n", RPM / 4);
      Serial.print(serial_debug_string);
    #endif                     
  } else if (RPM >= MID_UPSHIFT_WARN_RPM_ && RPM <= MAX_UPSHIFT_WARN_RPM_) {                                                        // Buildup from second yellow segment to reds.
    activate_shiftlight_segments(shiftlights_mid_buildup);
    #if DEBUG_MODE
      sprintf(serial_debug_string, "Displaying increasing warning at RPM: %ld\n", RPM / 4);
      Serial.print(serial_debug_string);
    #endif
  } else if (RPM >= MAX_UPSHIFT_WARN_RPM_) {                                                                                        // Flash all segments.
    activate_shiftlight_segments(shiftlights_max_flash);
    #if DEBUG_MODE
      sprintf(serial_debug_string, "Flash max warning at RPM: %ld\n", RPM / 4);
      Serial.print(serial_debug_string);
    #endif
  } else {                                                                                                                          // RPM dropped. Disable lights.
    if (shiftlights_segments_active) {
      if (ignore_shiftlights_off_counter == 0) {
        
        sendPTCanMessage(0x206, shiftlights_off);                                                                               
        shiftlights_segments_active = false;
        #if DEBUG_MODE
          Serial.println(F("Deactivated shiftlights segments"));
        #endif 
      } else {
        ignore_shiftlights_off_counter--;
      }
    }
  }
}


void activate_shiftlight_segments(uint8_t* data)
{
    sendPTCanMessage(0x206, data);                                                                    
    shiftlights_segments_active = true;
}


void evaluate_shiftlight_sync(const CAN_message_t &PTCAN_Received_Msg)
{
  if (!engine_warmed_up) {
    if (PTCAN_Received_Msg.buf[0] != last_var_rpm_can) {
      var_redline_position = ((PTCAN_Received_Msg.buf[0] * 0x32) + VAR_REDLINE_OFFSET_RPM) * 4;                                                    // This is where the variable redline actually starts on the KOMBI (x4).
      START_UPSHIFT_WARN_RPM_ = var_redline_position;                                                                              
      MID_UPSHIFT_WARN_RPM_ = var_redline_position + 2000;                                                                          // +500 RPM
      MAX_UPSHIFT_WARN_RPM_ = var_redline_position + 4000;                                                                          // +1000 RPM
      if (PTCAN_Received_Msg.buf[0] == 0x88) {                                                                                                     // DME is sending 6800 RPM.
        engine_warmed_up = true;
      }
      #if DEBUG_MODE
        sprintf(serial_debug_string, "Set shiftlight RPMs to %lu %lu %lu. Variable redline is at %lu \n", 
                (START_UPSHIFT_WARN_RPM_ / 4), (MID_UPSHIFT_WARN_RPM_ / 4), 
                (MAX_UPSHIFT_WARN_RPM_ / 4), (var_redline_position / 4));
        Serial.print(serial_debug_string);
      #endif
      last_var_rpm_can = PTCAN_Received_Msg.buf[0];
    }
  } else {
    if (START_UPSHIFT_WARN_RPM_ != START_UPSHIFT_WARN_RPM) {
      START_UPSHIFT_WARN_RPM_ = START_UPSHIFT_WARN_RPM;                                                                             // Return shiftlight RPMs to default setpoints.
      MID_UPSHIFT_WARN_RPM_ = MID_UPSHIFT_WARN_RPM;
      MAX_UPSHIFT_WARN_RPM_ = MAX_UPSHIFT_WARN_RPM;
      #if DEBUG_MODE
        Serial.println(F("Engine warmed up. Shiftlight setpoints reset to default."));
      #endif
    }
  }
}

void deactivate_shiftlight_segments()
{
  if (shiftlights_segments_active) {
    if (ignore_shiftlights_off_counter == 0) {
      
      CAN_message_t msgPTcan_out;
      msgPTcan_out.id = 0x206;
      for ( uint8_t i = 0; i < sizeof(shiftlights_off); i++ ) {
        msgPTcan_out.buf[i] = shiftlights_off[i];
      }
      sendPTCanMessage(msgPTcan_out.id, msgPTcan_out.buf);                                 
                                                                                    
      shiftlights_segments_active = false;
      #if DEBUG_MODE
        Serial.println("Deactivated shiftlights segments");
      #endif 
    } else {
      ignore_shiftlights_off_counter--;
    }
  }
}

void reset_shiftlights_runtime_vars()
{
  shiftlights_segments_active = engine_warmed_up = false;
  ignore_shiftlights_off_counter = 0;
  last_var_rpm_can = 0;
  START_UPSHIFT_WARN_RPM_ = START_UPSHIFT_WARN_RPM;
  MID_UPSHIFT_WARN_RPM_ = MID_UPSHIFT_WARN_RPM;
  MAX_UPSHIFT_WARN_RPM_ = MAX_UPSHIFT_WARN_RPM;
}
