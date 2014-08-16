#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 48000000

#define LED_GREEN(v) { TRISB7 = !(v); }
#define LED_YELLOW(v) { TRISB6 = !(v); }
#define LED_RED(v) { TRISC6 = !(v); }

uint8_t const sin[218] = {16,16,16,17,17,18,18,19,
19,19,20,20,21,21,22,22,
22,23,23,24,24,24,25,25,
25,26,26,26,27,27,27,28,
28,28,28,29,29,29,29,29,
30,30,30,30,30,30,31,31,
31,31,31,31,31,31,31,31,
31,31,31,31,31,31,31,31,
30,30,30,30,30,30,29,29,
29,29,29,28,28,28,28,27,
27,27,26,26,26,25,25,25,
24,24,24,23,23,22,22,22,
21,21,20,20,19,19,19,18,
18,17,17,16,16,16,15,15,
14,14,13,13,12,12,12,11,
11,10,10,9,9,9,8,8,
7,7,7,6,6,6,5,5,
5,4,4,4,3,3,3,3,
2,2,2,2,2,1,1,1,
1,1,1,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,1,1,1,
1,1,1,2,2,2,2,2,
3,3,3,3,4,4,4,5,
5,5,6,6,6,7,7,7,
8,8,9,9,9,10,10,11,
11,12,12,12,13,13,14,14,
15,15};

uint8_t count = 0;

void main()
{
    // Set up the LEDs
    LATB7 = 1;
    LATB6 = 1;
    LATC6 = 0;

    // enable DAC and DACOUT pin: DACEN = 1, DACOE = 1
    VREFCON1 = 0b10100000;

    // set up Timer2: period = 125 cycles (12 MHz instruction clock / 125 = 96 kHz); TMR2ON = 1
    PR2 = 124;
    T2CON = 0b00000100;


    PIR1bits.TMR2IF = 0; // clear Timer2-to-PR2-match interrupt flag
    PIE1bits.TMR2IE = 1; // enable Timer2-to-PR2-match interrupt

    // enable interrupts
    RCONbits.IPEN = 1;    // Enable interrupt priority levels
    INTCONbits.GIE = 1;   // Enable high priority interrupts

    while(1) {}
}

void interrupt highIsr()
{
    PIR1bits.TMR2IF = 0;
    LED_RED(1);

    count++;
    if (count > 217)
        count = 0;

    VREFCON2 = sin[count];

    LED_RED(0);
}