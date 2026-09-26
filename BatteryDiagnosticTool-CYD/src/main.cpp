#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include "INA226.h"
#include "BluetoothSerial.h"


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft); 
INA226 ina(0x40); 
BluetoothSerial SerialBT;


#define CYBER_CYAN  0x03EF
#define NEON_GREEN  0x07E0
#define ALERT_RED   0xF800
#define HUD_GRAY    0x3186
#define GRID_COLOR  0x0821 


int currentMode = 0; 
unsigned long lastDisplayUpdate = 0;

void drawGrid() {

  for (int i = 0; i < 320; i += 20) tft.drawFastVLine(i, 0, 240, GRID_COLOR);
  for (int i = 0; i < 240; i += 20) tft.drawFastHLine(0, i, 320, GRID_COLOR);
}

void drawSciFiHUD(String modeName) {
  tft.fillScreen(TFT_BLACK);
  drawGrid();
  
  
  tft.drawRect(5, 5, 310, 230, CYBER_CYAN);
  tft.drawRect(8, 8, 304, 224, HUD_GRAY);
  
  
  tft.fillRect(5, 5, 15, 15, CYBER_CYAN);
  tft.fillRect(300, 5, 15, 15, CYBER_CYAN);
  tft.fillRect(5, 220, 15, 15, CYBER_CYAN);
  tft.fillRect(300, 220, 15, 15, CYBER_CYAN);

 
  tft.setTextColor(TFT_BLACK, CYBER_CYAN);
  tft.fillRect(60, 15, 200, 25, CYBER_CYAN);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(">" + modeName + "_", 160, 27, 2);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("battery tool");

  tft.init();
  tft.invertDisplay(true);
  tft.setRotation(1); 
  

  spr.createSprite(280, 120);
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(CYBER_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM); 
  
  tft.drawString("SYS.BOOT_SEQ_INIT...", 160, 100, 2);
  tft.drawString("AWAITING BLUETOOTH", 160, 140, 2);
  delay(1500);

  Wire.begin(27, 22);

  if (!ina.begin()) {
    tft.fillScreen(ALERT_RED);
    tft.setTextColor(TFT_WHITE, ALERT_RED);
    tft.drawString("ERR_FATAL: INA226_NOT_FOUND", 160, 120, 2);
    while (1); 
  }

  drawSciFiHUD("SYS.CAR_12V");
}

void loop() {
 
  if (SerialBT.available()) {
    char incomingChar = SerialBT.read();
    bool modeChanged = false;

    if (incomingChar == '0') { currentMode = 0; modeChanged = true; }
    else if (incomingChar == '1') { currentMode = 1; modeChanged = true; }
    else if (incomingChar == '2') { currentMode = 2; modeChanged = true; }
    else if (incomingChar == '3') { currentMode = 3; modeChanged = true; }
    else if (incomingChar == '4') { currentMode = 4; modeChanged = true; } 
    else if (incomingChar == 'n' || incomingChar == 'N') {
      currentMode++;
      if (currentMode > 4) currentMode = 0; 
      modeChanged = true;
    }

    if (modeChanged) {
      String newTitle = "";
      if (currentMode == 0) newTitle = "SYS.CAR_12V";
      if (currentMode == 1) newTitle = "SYS.LIPO_2S";
      if (currentMode == 2) newTitle = "SYS.PWR_9V";
      if (currentMode == 3) newTitle = "SYS.CELL_AA";
      if (currentMode == 4) newTitle = "SYS.CELL_AAA"; 
      
      drawSciFiHUD(newTitle);
      lastDisplayUpdate = 0; 
    }
  }

  
  if (millis() - lastDisplayUpdate > 150) {
    lastDisplayUpdate = millis();

    float carVoltage = ina.getBusVoltage();
    int batteryPercentage = 0;
    float minV = 0, maxV = 0;

    switch (currentMode) {
      case 0: minV = 11.9; maxV = 12.6; break;
      case 1: minV = 6.4; maxV = 8.4; break;
      case 2: minV = 6.0; maxV = 9.5; break;
      case 3: minV = 1.0; maxV = 1.6; break; 
      case 4: minV = 1.0; maxV = 1.6; break; 
    }

    if (carVoltage >= maxV) {
      batteryPercentage = 100;
    } else if (carVoltage <= minV) {
      batteryPercentage = 0;   
    } else {
      batteryPercentage = ((carVoltage - minV) / (maxV - minV)) * 100;
    }

    uint16_t activeColor = CYBER_CYAN;
    if (batteryPercentage <= 20) activeColor = ALERT_RED;
    else if (batteryPercentage >= 80) activeColor = NEON_GREEN;

    char voltStr[10];
    dtostrf(carVoltage, 5, 2, voltStr);

    
    spr.fillSprite(TFT_BLACK); 

   
    spr.setTextColor(activeColor, TFT_BLACK); 
    spr.setTextDatum(MC_DATUM);
    spr.drawString(String(voltStr) + " V", 140, 30, 7); 
    
  
    spr.setTextColor(HUD_GRAY, TFT_BLACK);
    spr.setTextDatum(ML_DATUM);
    spr.drawString("var.cap == " + String(batteryPercentage) + "%", 20, 80, 2); 

    
    int segments = 20;
    int segWidth = 10;
    int segSpacing = 2;
    int startX = 20;
    int activeSegments = (batteryPercentage * segments) / 100;

    for (int i = 0; i < segments; i++) {
      if (i < activeSegments) {
        spr.fillRect(startX + (i * (segWidth + segSpacing)), 100, segWidth, 15, activeColor);
      } else {
        spr.drawRect(startX + (i * (segWidth + segSpacing)), 100, segWidth, 15, HUD_GRAY);
      }
    }

    
    spr.pushSprite(20, 70); 
  }
}