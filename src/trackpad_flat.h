
#ifndef Trackpad_h
#define Trackpad_h

#if ARDUINO >= 100
  #include <Arduino.h> // Arduino 1.0
#else
  #include <WProgram.h> // Arduino 0022
	#include "cppfix.h"
#endif
#include "ps2_flat.h"

#define PS2_RESET 0xFF
#define PS2_RESEND 0xFE
#define PS2_SETDEFAULTS 0xF6
#define PS2_DISABLE 0xF5
#define PS2_ENABLE 0xF4
#define PS2_SETRATE 0xF3
#define PS2_GETDEVICEID 0xF2
#define PS2_SETPOLL 0xF0
#define PS2_SETWRAPMODE 0xEE
#define PS2_RESETWRAPMODE 0xEC
#define PS2_READDATA 0xEB
#define PS2_SETSTREAMMODE 0xEA
#define PS2_STATUSREQUEST 0xE9
#define PS2_ERROR 0xFC
#define PS2_ACK 0xFA
#define PS2_CAPABILITIES 0x02
#define PS2_MODES 0x01
#define PS2_IDENTIFY 0x00
#define PS2_READRESOLUTIONS 0x08
#define PS2_EXTENDEDMODELID 0x09
#define PS2_EXTENDEDCONTINUED 0x0C
#define PS2_MAXIMUMCOORDS 0x0D
#define PS2_MINIMUMCOORDS 0x0F

#define PS2_SETRES 0xE8   // sends one argument
#define PS2_SETSCALE11 0xE6 // no arguments
#define PS2_SETSCALE21 0xE7 // no arguments
#define PS2_GETINFO 0xE9  // receives 3 arg. bytes

typedef struct _status_t {
	unsigned int x;
	unsigned int y;
	unsigned int z;
	unsigned int w; // 0 == Two fingers
					// 1 == Three or more fingers
					// 2 == Extended W mode
					// 3 == Pass-Through
					// 4 == One finger (if palm detect off)
					// 4 to 15 == Palm detect 
	bool mouse1 = false;
	bool mouse2 = false;
	bool mouse3 = false;
	bool mouse4 = false;
	byte p1;
	byte p2;
	byte p3;
	byte p4;
	byte p5;
	byte p6;

	byte d1;
	byte d2;
	byte d3;

	byte synapticMinor;
	byte synapticMajor;
	byte synapticModel;

	unsigned int numExtended;
	unsigned int numModelSub;
	bool capPassThrough;
	bool capMultiFingerReport;
	bool capMultiFinger;
	bool capPalmDetect;

	unsigned int numButtons;
	unsigned int dpmmX; // X and Y resolution (units per mm)
	unsigned int dpmmY;	
	unsigned int extendedButtons;
	unsigned int clickPad;
	bool capMax;
	bool capMin;
	bool capV;

	unsigned int xMax;
	unsigned int yMax;
	unsigned int xMin;
	unsigned int yMin;
} status_t;

extern void trackpadBegin (int clk, int data);
extern void trackpadSendComSeq (byte arg, boolean setMode);
extern status_t * trackpadGetNewStatus();
extern status_t status;

#endif /* trackpad_h */