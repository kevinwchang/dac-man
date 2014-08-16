#include <xc.h>
#include <stdint.h>
#include "waveforms.h"
#include "effects.h"

#define _XTAL_FREQ 48000000

#define LED_GREEN(v) { TRISB7 = !(v); }
#define LED_YELLOW(v) { TRISB6 = !(v); }
#define LED_RED(v) { TRISC6 = !(v); }

#define TICKS_PER_REG_UPDATE 1600

uint16_t volatile ticks = 0;

uint16_t volatile freq2 = 0;
uint8_t volatile wave2 = 0, vol2 = 0;

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

    uint8_t const * e = all_sounds[0];

    static bit init = 0, dir_reverse = 0;
    uint8_t base_freq, freq, duration, repeat, vol;
    int8_t freq_inc;

    uint8_t wait = 0, sixtieths = 0, index = 0;

    while (1)
    {
        if (ticks >= TICKS_PER_REG_UPDATE)
        {
            ticks = 0;
            sixtieths++;

            if (wait)
            {
                wait--;
                continue;
            }

            if (!init)
            {
                init = 1;
                dir_reverse = 0;
                freq = base_freq = e[1];
                freq_inc = e[2];
                duration = e[3] & 0x7f;
                repeat = e[5];
                vol = e[6] & 0xf;

                sixtieths = 0;
            }

            if (--duration == 0 ||  sixtieths > 240)
            {
                if (repeat-- <= 1  ||  sixtieths > 240)
                {
                    // done with this effect
                    init = 0;
                    vol2 = 0;
                    index++;
                    if (index >= 13)
                        index = 0;
                    e = all_sounds[index];
                    continue;
                }

                duration = e[3] & 0x7f;

                if (e[3] & 0x80)
                {
                    // reverse
                    freq_inc = -freq_inc;
                    dir_reverse = ~dir_reverse;
                }

                if (!dir_reverse)
                {
                    base_freq += e[4];
                    freq = base_freq;
                    vol -= e[7];
                }
            }

            freq += freq_inc;
            freq2 = (uint16_t)freq << ((e[0] & 0x70) >> 4);
            wave2 = e[0] & 0x7;
            vol2 = vol;
        }
    }
}
 uint16_t volatile acc2 = 0;
void interrupt highIsr()
{
    //LED_RED(1);

    PIR1bits.TMR2IF = 0;
    ticks++;

    acc2 += freq2;
    VREFCON2 = vol2 ? waveforms[wave2][*((uint8_t *)&acc2 + 1) >> 3] : 7;
    //VREFCON2 = ((uint16_t)waveforms[wave2][*((uint8_t *)&acc2 + 1) >> 3] * vol2 * 31 + 112) / 225;
    
    //LED_RED(0);
}