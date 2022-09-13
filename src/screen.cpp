#include <screen.h>
#include <SPI.h>
#include <ST7735_t3.h>
#include <ST7789_t3.h>
#include <ST7735_t3_font_Arial.h>
//#include <ST7735_t3_font_ArialBold.h>



ST7789_t3 tft = ST7789_t3(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void initialise_screen() {
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);

    tft.init(240, 240);
    tft.setRotation(0);
    tft.fillScreen(ST77XX_WHITE);

    
    tft.setTextColor(ST77XX_BLACK);
    
    tft.setFont(Arial_28);
    tft.setCursor(65,10);
    tft.println("M330ii");
    
    tft.setFont(Arial_20);
    tft.setCursor(55, 80);
    tft.println("CAN-BUS");
    tft.setCursor(30, 120);
    tft.println("interfacing tool");

    tft.setFont(Arial_16);
    tft.setCursor(25, 212);
    tft.println("www.bosolanu.com");
}
