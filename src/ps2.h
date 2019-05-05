/*
 * ps2.h - a library to interface with ps2 devices. See comments in
 * ps2.cpp.
 * Written by Chris J. Kiick, January 2008.
 * Release into public domain.
 */

#ifndef ps2_h
#define ps2_h

#if ARDUINO >= 100
  #include <Arduino.h> // Arduino 1.0
#else
  #include <WProgram.h> // Arduino 0022
#endif

extern void PS2Begin (int clk, int data);
extern void PS2Write(unsigned char data);
extern unsigned char PS2Read(void);
extern bool PS2Error;

bool has_even_parity(unsigned char x);
void waitpin(int pin, bool state, bool padded, unsigned int timeout = 1000);
void gohi(int pin);
void golo(int pin);

#endif /* ps2_h */

