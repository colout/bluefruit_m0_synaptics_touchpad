/*
  ArduixPL - xPL library for Arduino(tm)
  Copyright (c) 2012/2013 Mathieu GRENET.  All right reserved.

  This file is part of ArduixPL.

    ArduixPL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ArduixPL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ArduixPL.  If not, see <http://www.gnu.org/licenses/>.

	  Modified 2013-2-4 by Mathieu GRENET 
	  mailto:mathieu@mgth.fr
	  http://www.mgth.fr
*/
/*
  trackpad.cpp - Library for interfacing with synaptic trackpads.
  Created by Joe Rickerby, July 2008.
  Released into the public domain.
*/
#include "Trackpad.h"

#define BITPOS(n) (1<<(n))
#define DUMP(d) Serial.print(#d ": ");\
Serial.print(d,HEX);Serial.print(" ");\
Serial.print(d,BIN);Serial.println()

Trackpad::Trackpad(int clk, int data)
{
	//I've created my own new operator, see the cppfix.cpp file
	mouse = new PS2(clk,data);
	
	mouse->write(0xff);	// reset
	mouse->read();	 // ack byte
	mouse->read();	 // blank 
	mouse->read();	 // blank 
	mouse->write(0xf0);	// remote mode
	mouse->read();	 // ack
	
	//set mode byte- absolute mode, wmode
	/*
	b7				b6		b5					b4					b3			b2					b1			b0
	|Absolute mMde	|Rate	|Transparent Mode	|Guest ACPI Mode	|Sleep		|DisGest / EWmode	|PackSize	|Wmode
	*/
	sendComSeq(B10000001, true);
	sendComSeq(PS2_IDENTIFY, false);
	sendComSeq(PS2_MODES, false);
	sendComSeq(PS2_CAPABILITIES, false);
	sendComSeq(PS2_READRESOLUTIONS, false);
	sendComSeq(PS2_EXTENDEDMODELID, false);
	sendComSeq(PS2_EXTENDEDCONTINUED, false);
	sendComSeq(PS2_MAXIMUMCOORDS, false);
	sendComSeq(PS2_MINIMUMCOORDS, false);
	
	delayMicroseconds(100);
}

Trackpad::~Trackpad()
{
	delete mouse;
}

//Details are available in the synaptic interfacing guide- this is a bit of a nasty one.
//Synaptic special commands are hidden in Set Resolution commands, so that the data will make
//it through the BIOS on a PC unscathed.
void Trackpad::sendComSeq(byte arg, boolean setMode)
{
	
	mouse->write(PS2_SETRES);
	mouse->write((arg & 0xc0)>>6);
	mouse->read();	 // ack byte
	mouse->write(PS2_SETRES);
	mouse->write((arg & 0x30)>>4);
	mouse->read();	 // ack byte
	mouse->write(PS2_SETRES);
	mouse->write((arg & 0x0c)>>2);
	mouse->read();	 // ack byte
	mouse->write(PS2_SETRES);
	mouse->write(arg & 0x03);
	mouse->read();	 // ack byte

	if(setMode == false) {
		mouse->write(PS2_GETINFO);
		mouse->read();	 // ack byte
		
		status.d1 = mouse->read();
		status.d2 = mouse->read();
		status.d3 = mouse->read();

		if (arg == PS2_IDENTIFY) {
			status.synapticMinor=status.d1;
			// d2 is ignored
			status.synapticModel = (status.d3 & 0xf0);
			status.synapticMajor = (status.d3 & 0x0f);
		} else if (arg == PS2_CAPABILITIES) {
			status.numExtended =  (status.d1 & B01110000)>>4;
			status.numModelSub =  status.d2;
			status.capPassThrough = (status.d3 & B10000000)>>7;
			status.capMultiFingerReport = (status.d3 & B00100000)>>5;
			status.capMultiFinger = (status.d3 & B00000010)>>1;
			status.capPalmDetect = (status.d3 & B00100001);
		} else if (arg == PS2_READRESOLUTIONS) {
			status.dpmmX = status.d1;
			status.dpmmY = status.d3;
		} else if (arg == PS2_EXTENDEDMODELID) {
			status.extendedButtons = (status.d2 & B11110000) >> 4;
		} else if (arg == PS2_EXTENDEDCONTINUED) {
			status.clickPad = (status.d1 & B00010000) >> 4;
			status.clickPad |= (status.d2 & B00000001) >> 1;

			status.capMax |= (status.d1 & B00000010) >> 1;
			status.capMin |= (status.d2 & B00100000) >> 5;
			status.capV |= (status.d2 & B00001000) >> 3;

		} else if (arg == PS2_MAXIMUMCOORDS) {
			status.xMax = (status.d2 & B00001111);
			status.xMax |= (status.d1) << 4;

			status.yMax = (status.d2 & B11110000) >> 4;
			status.yMax |= (status.d3) << 4;
		} else if (arg == PS2_MINIMUMCOORDS) {
			status.xMin = (status.d2 & B00001111);
			status.xMin |= (status.d1) << 4;

			status.yMin = (status.d2 & B11110000) >> 4;
			status.yMin |= (status.d3) << 4;
		}
	} else {
		mouse->write(PS2_SETRATE);
		mouse->read();	 // ack byte
		mouse->write(0x14);
		mouse->read();	 // ack byte
	}
}

/*
	See the Synaptic interfacing guide for the derivation of all that bitwise nonsense
	down there.
*/
status_t * Trackpad::getNewStatus()
{
	bool error = false;

    byte p1;
	byte p2;
	byte p3;
	byte p4;
	byte p5;
	byte p6;

	mouse->write(0xeb);	 // give me data!
	byte ack = mouse->read();			// ignore ack
    if (mouse->error) error = true;

	if (ack == 0xfa) {
		p1 = mouse->read();
        if (mouse->error) error = true;

		p2 = mouse->read();
        if (mouse->error) error = true;

		p3 = mouse->read();
        if (mouse->error) error = true;
        
		p4 = mouse->read();
        if (mouse->error) error = true;

		p5 = mouse->read();
        if (mouse->error) error = true;

		p6 = mouse->read();
        if (mouse->error) error = true;

		/*
		Packets when WMode == 0
			b7		b6		b5		b4		b3		b2		b1		b0
		p1	|1		|0		|Fngr	|Rsrvd	|0		|Gestr	|Right	|Left
		p2	|Y pos 11..8					|X Position 11..8
		p3	|Z Pressure 7..0
		p4	|1		|1		|Y 12 	|X 12	| 0		|Gestr	|Right	|Left
		p5	| X position 7..0
		p6	| Y position 7..0

		Packets when WMode == 1
			b7		b6		b5		b4		b3		b2		b1		b0
		p1	|1		|0		|W val 3..2		|0		|W val 1|Right	|Left
		p2	|Y pos 11..8					|X Position 11..8
		p3	|Z Pressure 7..0
		p4	|1		|1		|Y 12 	|X 12	| 0		|W val 0|R/D	|L/U
		p5	| X position 7..0
		p6	| Y position 7..0

		When multi-button, we round robbin shift X / Y (byte 5/6)
		With 3 buttons (our case):
		p5	| X position 7..2								| b3	|b1
		p6	| Y position 7..1										|b2
		*/
        if (!error) {
            status.x = (p5 & B11111100);
            status.x |= (p2 & 0x0f) << 8;
            status.x |= (p4 & 0x10) << 8;
            status.x = status.x >> 2;
            
            status.y = (p6 & B11111100);
            status.y |= ((p2 & 0xf0) << 4);
            status.y |= ((p4 & 0x20) << 7);
            status.y = status.y >> 2;

            status.z = p3;
            
            status.w = 0;
            status.w |= (p4 & 0x04) >> 2;
            status.w |= (p1 & 0x04) >> 1;
            status.w |= (p1 & 0x30) >> 2;
            
            status.mouse1 = (p1 & 0x01);
            status.mouse2 = (p1 & 0x02) >> 1;
            status.mouse3 = (p4 & 0x01);
            status.mouse4 = (p4 & 0x02) >> 1;
            
            status.p1 = p1;
            status.p2 = p2;
            status.p3 = p3;
            status.p4 = p4;
            status.p5 = p5;
            status.p6 = p6;
        }
        if (Serial) {Serial.print("Error:"); Serial.println(error);}
	} else
	{
		//delay(2);
	}
	

	return &status;
}
