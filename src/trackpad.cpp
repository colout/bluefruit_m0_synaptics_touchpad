#include "trackpad.h"

status_t status;

void trackpadBegin (int clk, int data)
{
	PS2Begin(clk,data);
	
	PS2Write(0xff);	// reset
	PS2Read();	 // ack byte
	PS2Read();	 // blank 
	PS2Read();	 // blank 
	PS2Write(0xf0);	// remote mode
	PS2Read();	 // ack
	
	//set mode byte- absolute mode, wmode
	/*
	b7				b6		b5					b4					b3			b2					b1			b0
	|Absolute mMde	|Rate	|Transparent Mode	|Guest ACPI Mode	|Sleep		|DisGest / EWmode	|PackSize	|Wmode
	*/
	trackpadSendComSeq(B11000001, true);
	trackpadSendComSeq(PS2_IDENTIFY, false);
	trackpadSendComSeq(PS2_MODES, false);
	trackpadSendComSeq(PS2_CAPABILITIES, false);
	trackpadSendComSeq(PS2_READRESOLUTIONS, false);
	trackpadSendComSeq(PS2_EXTENDEDMODELID, false);
	trackpadSendComSeq(PS2_EXTENDEDCONTINUED, false);
	trackpadSendComSeq(PS2_MAXIMUMCOORDS, false);
	trackpadSendComSeq(PS2_MINIMUMCOORDS, false);
	
	delayMicroseconds(100);
}

void trackpadSendComSeq (byte arg, boolean setMode) {
	PS2Write(PS2_SETRES);
	PS2Write((arg & 0xc0)>>6);
	PS2Read();	 // ack byte
	PS2Write(PS2_SETRES);
	PS2Write((arg & 0x30)>>4);
	PS2Read();	 // ack byte
	PS2Write(PS2_SETRES);
	PS2Write((arg & 0x0c)>>2);
	PS2Read();	 // ack byte
	PS2Write(PS2_SETRES);
	PS2Write(arg & 0x03);
	PS2Read();	 // ack byte

	if(setMode == false) {
		PS2Write(PS2_GETINFO);
		PS2Read();	 // ack byte
		
		status.d1 = PS2Read();
		status.d2 = PS2Read();
		status.d3 = PS2Read();

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
		PS2Write(PS2_SETRATE);
		PS2Read();	 // ack byte
		PS2Write(0x14);
		PS2Read();	 // ack byte
	}
}



status_t * trackpadGetNewStatus()
{
	bool error = false;

    byte p1;
	byte p2;
	byte p3;
	byte p4;
	byte p5;
	byte p6;

	PS2Write(0xeb);	 // give me data!
	byte ack = PS2Read();			// ignore ack
    if (PS2Error) error = true;

	if (ack == 0xfa) {
		p1 = PS2Read();
        if (PS2Error) error = true;

		p2 = PS2Read();
        if (PS2Error) error = true;

		p3 = PS2Read();
        if (PS2Error) error = true;
        
		p4 = PS2Read();
        if (PS2Error) error = true;

		p5 = PS2Read();
        if (PS2Error) error = true;

		p6 = PS2Read();
        if (PS2Error) error = true;

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
        if (Serial) {if (error) { Serial.println('*************SERIAL ERROR');}}
	} else
	{
		//delay(2);
	}
	

	return &status;
}
