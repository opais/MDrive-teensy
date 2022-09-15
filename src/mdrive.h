#include <Arduino.h>
struct mdrive_settings
{
  uint8_t dsc = 0x03;
  uint8_t power = 0;
  uint8_t edc = 0x20;
  uint8_t svt = 0xE9;
};

struct mdrive_settings mdrive_settings;


