#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
#include <SoftwareSerial.h>
#endif

#define FACTORYRESET_ENABLE         0

#include <Wire.h>
#include "./src/trackpad.h"
#include "./src/ps2.h"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

bool lastPress = false;

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// Mouse Positions
int mouseLastAbsX = 0;
int mouseLastAbsY = 0;
int mouseLastX = 0;
int mouseLastY = 0;

void setup()
{
  Serial.begin(115200);

  while (!Serial);
  delay(500);

  Serial.println ("Start");

  startAdv();

  // Turn off that damn red light
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  // CLK, DATA
  pinMode(12, INPUT);
  pinMode(11, INPUT);

  //clk, data
  trackpadBegin (11, 12);
}


void startAdv(void) {

  ble.begin(VERBOSE_MODE);

  if ( FACTORYRESET_ENABLE )
  {
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }
  ble.echo(false);

  ble.println(F( "AT+BleHIDEn=On"));
  ble.println(F( "AT+BLEPOWERLEVEL=4"));
  ble.println(F( "AT+GAPDEVNAME=KeyboardTest"));
  //ble.println(F( "AT+GAPINTERVALS=32,100,244,30"));
  ble.println(F( "AT+GAPINTERVALS=128,100,976,30"));

  ble.reset();
}

unsigned long wideMillis = 0; 

void checkMouse () {
  // Is device enabled?


  // Get Moues Status
  status_t * status;
  status = trackpadGetNewStatus();


  // Calculate Mouse Pos
  int mouseAbsX = status->x;
  int mouseAbsY = status->y;
  int X = mouseLastAbsX - mouseAbsX;
  int Y = -(mouseLastAbsY - mouseAbsY);


  // If mouse is touched and last location isn't zero, move it
  if (
    (true || mouseAbsX != 0 && mouseAbsY != 0 ) &&
    (true || mouseLastAbsX != 0 && mouseLastAbsY != 0)
  )
  {

    if ((mouseAbsX == mouseAbsY == 0)) {
      if ((mouseLastAbsX == mouseLastAbsY == 0)) {
        int smoothX = constrain((X + mouseLastX) / 2, -127, 128);
        int smoothY = constrain((Y + mouseLastY) / 2, -127, 128);
        if (status->w == 0) {
          wideMillis = millis();
        }
        
  
        Serial.println();
        Serial.println(smoothX);
        Serial.println(smoothY);

        // 250ms buffer for scrolling
        if (millis() < wideMillis + 250) { 
          String command = "AT+BleHidMouseMove=,," + String(constrain(smoothY/10, -2, 2));
          ble.println(command);
        } else {
          String command = "AT+BleHidMouseMove=" + String(smoothX) + "," + String(smoothY);
          ble.println(command);
        }
      }
    }
  }

  mouseLastAbsX = mouseAbsX;
  mouseLastAbsY = mouseAbsY;

  mouseLastX = X;
  mouseLastY = Y;
}

void loop() {
  //   Read mouse on click
  checkMouse();

  //  delay (1);
}
