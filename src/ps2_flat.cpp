/*
 * ps2.cpp - an interface library for ps2 devices.  Common devices are
 * mice, keyboard, barcode scanners etc.  See the examples for mouse and
 * keyboard interfacing.
 * limitations:
 *      we do not handle parity errors.
 *      The timing constants are hard coded from the spec. Data rate is
 *         not impressive.
 *      probably lots of room for optimization.
 */

#include "ps2_flat.h"

/*
 * the clock and data pins can be wired directly to the clk and data pins
 * of the PS2 connector.  No external parts are needed.
 */
bool PS2Error;
int _ps2clk;
int _ps2data;

void PS2Begin(int clk, int data)
{
	_ps2clk = clk;
	_ps2data = data;
	gohi(_ps2clk);
	gohi(_ps2data);
}

/*
 * according to some code I saw, these functions will
 * correctly set the clock and data pins for
 * various conditions.  It's done this way so you don't need
 * pullup resistors.
 */
void gohi(int pin)
{
	pinMode(pin, INPUT);
	digitalWrite(pin, HIGH);
}

void golo(int pin)
{
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);

}

void waitpin(int pin, bool state, bool padded, unsigned int timeout) {
    unsigned long timeOutBy = micros() + timeout;  // timeout

    timeout = 1000;
    if (!PS2Error) {
        while (digitalRead(pin) == state) {
            if  (micros() > timeOutBy) {
                PS2Error = true;
                break;
            }
        }
        
        // Always set data on the beginning of clock (no padding)
        // Always read data part way through the clock cycle (padded) 
        if (padded) delayMicroseconds(5);
    }
}

/* write a byte to the PS2 device */
void PS2Write(unsigned char data)
{
    PS2Error = false;
	unsigned char i;
	unsigned char parity = 1;
	
    // This is the idle bus state
	gohi(_ps2data);
	gohi(_ps2clk);
	delayMicroseconds(100);
	
    // Inhibit communication
    golo(_ps2clk);  
	delayMicroseconds(100);

    // Request to send state
	golo(_ps2data); 
	gohi(_ps2clk);

	// wait for device to take control of clock
	waitpin(_ps2clk, HIGH, false);

	// clear to send data
	for (i=0; i < 8; i++) {
		if (data & 0x01)
		{
			gohi(_ps2data);
		} else {
			golo(_ps2data);
		}
		// wait for clock
        waitpin(_ps2clk, LOW, false);
        waitpin(_ps2clk, HIGH, false);

		parity = parity ^ (data & 0x01);
		data = data >> 1;
	}
	// parity bit
	if (parity)
	{
		gohi(_ps2data);
	} else {
		golo(_ps2data);
	}
    waitpin(_ps2clk, LOW, false);
    waitpin(_ps2clk, HIGH, false);

	// stop bit
	gohi(_ps2data);

     // Wait for mouse to pull data and clock line
    waitpin(_ps2data, LOW, false); 
    waitpin(_ps2clk, LOW, false); 

    // Wait for moue to release data and clock
    waitpin(_ps2data, HIGH, false); 
    waitpin(_ps2clk, HIGH, false); 

	// Let's be greedy and hold the clock to ourselves
	golo(_ps2clk);
    if (PS2Error) delayMicroseconds(500);
    if (!PS2Error) delayMicroseconds(50);
}


/*
 * read a byte of data from the ps2 device.  Ignores parity.
 */
unsigned char PS2Read(void)
{
	unsigned char data = 0x00;
	unsigned char i;
	unsigned char bit = 0x01;

    PS2Error = false;

	// high clock / data for 50 micros means ready
	gohi(_ps2clk);
	gohi(_ps2data);
	delayMicroseconds(50); 

  // Start bit == 0
  waitpin(_ps2clk, HIGH, true);
  waitpin(_ps2clk, LOW, true);

	for (i=0; i < 8; i++)
	{
		waitpin(_ps2clk, HIGH, true);
		if (digitalRead(_ps2data) == HIGH)
		{
			data = data | bit;
		}
        waitpin(_ps2clk, LOW, true);
			;
		bit = bit << 1;
	}

    waitpin(_ps2clk, HIGH, true);
    if (digitalRead(_ps2data) != has_even_parity(data)) PS2Error=true;
    waitpin(_ps2clk, LOW, true);

	// eat stop bit
    waitpin(_ps2clk, HIGH, true);
    waitpin(_ps2clk, LOW, true);


	golo(_ps2clk);	// hold incoming data
    if (PS2Error) delayMicroseconds(200);
    if (!PS2Error) delayMicroseconds(50);
	return data;
}

bool has_even_parity(unsigned char x) 
{
    unsigned char shift=1;
    while (shift < (sizeof(x)*8))
    {
            x ^= (x>>shift);
            shift<<=1;
    }
    return !(x & 0x1);
}
