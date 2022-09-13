#include <EEPROM.h>



void read_settings_from_eeprom()
{
  mdrive_dsc = EEPROM.read(1);
  mdrive_power = EEPROM.read(2);
  mdrive_edc = EEPROM.read(3);
  mdrive_svt = EEPROM.read(4);

  #if DEBUG_MODE
    sprintf(serial_debug_string, "Loaded MDrive settings from EEPROM: DSC 0x%X POWER 0x%X EDC 0x%X SVT 0x%X.\n", 
            mdrive_dsc, mdrive_power, mdrive_edc, mdrive_svt);
    Serial.print(serial_debug_string);
  #endif

  mdrive_message[1] = mdrive_dsc - 2;                      // Difference between iDrive settting and MDrive CAN message (off) is always 2. 0x1 unchanged, 0x5 off, 0x11 MDM, 0x9 On
  mdrive_message[2] = mdrive_power;                        // Copy POWER as is.
  mdrive_message[3] = mdrive_edc;                          // Copy EDC as is.
  if (mdrive_svt == 0xE9) {
      mdrive_message[4] = 0x41;                            // SVT normal, MDrive off.
  } else if (mdrive_svt == 0xF1) {
      mdrive_message[4] = 0x81;                            // SVT sport, MDrive off.
  } 
}


void update_mdrive_settings_in_eeprom()
{
  EEPROM.update(1, mdrive_dsc);                            // EEPROM lifetime approx. 100k writes. Always update, never write()!
  EEPROM.update(2, mdrive_power);
  EEPROM.update(3, mdrive_edc);
  EEPROM.update(4, mdrive_svt);
  #if DEBUG_MODE
    Serial.println(F("Saved MDrive settings to EEPROM."));
  #endif
}
