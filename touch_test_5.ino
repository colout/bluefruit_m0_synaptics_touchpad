#include <bluefruit.h>
#include <Wire.h>
#include "./ps2.h"
#include "./Trackpad.h"

// Bluetooth
BLEDis bledis;
BLEHidAdafruit blehid;

//clk, data
Trackpad t(MOSI,MISO);

// Mouse Positions
int mouseLastAbsX = 0;
int mouseLastAbsY = 0;
int mouseLastX = 0;
int mouseLastY = 0;

void checkMouse () {
  status_t * status;
  status = t.getNewStatus();

  int mouseAbsX = status->x;
  int mouseAbsY = status->y;
  int X = mouseLastAbsX - mouseAbsX;
  int Y = -(mouseLastAbsY - mouseAbsY);

  // If mouse is touched and last location isn't zero
  if (
    (true || mouseAbsX != 0 && mouseAbsY !=0 ) &&
    (true || mouseLastAbsX != 0 && mouseLastAbsY != 0)
  )
  {
    Serial.println(digitalRead(A3));
    Serial.println(digitalRead(A4));
    Serial.println(digitalRead(A5));
    Serial.println();
    Serial.println(X);
    Serial.println(Y);

    if ((X != 0) && (Y != 0)) blehid.mouseMove ((X + mouseLastX) / 2, (Y + mouseLastY) / 2);
    /*
    Serial.println(String(status->p1, BIN));
    Serial.println(String(status->p2, BIN));
    Serial.println(String(status->p3, BIN));
    Serial.println(String(status->p4, BIN));
    Serial.println(String(status->p5, BIN));
    Serial.println(String(status->p6, BIN));
    */ 
    Serial.println();
    Serial.println();
  }

  mouseLastAbsX = mouseAbsX;
  mouseLastAbsY = mouseAbsY;

  mouseLastX = X;
  mouseLastY = Y;
}


void setup()
{
  Serial.begin(115200);
  
  Serial.println ("Start");



  // Begin Bluetooth
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
  
  Bluefruit.begin();
  
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("Bluefruit52");
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather 52");
  bledis.begin();
  blehid.begin();
  
  startAdv();
  //delay (1000);
  pinMode(MOSI, INPUT);
  pinMode(MISO, INPUT);
  pinMode(15, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP); 
}


void startAdv(void) {
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);

  // Include BLE HID service
  Bluefruit.Advertising.addService(blehid);

  // There is enough room for the dev name in the advertising packet
  Bluefruit.Advertising.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32 * 1, 244 * 1);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
} 

bool lastPress = false;

void loop() {
   //   Read mouse on click
  checkMouse();

  delay (1);
}

