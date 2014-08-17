#include <xc.h>
#include <stdint.h>
#include "waveforms.h"
#include "effects.h"

#define _XTAL_FREQ 48000000

#define LED_GREEN(v) { TRISB7 = !(v); }
#define LED_YELLOW(v) { TRISB6 = !(v); }
#define LED_RED(v) { TRISC6 = !(v); }

#define TICKS_PER_REG_UPDATE 1600

uint16_t volatile bank0 ticks = TICKS_PER_REG_UPDATE;

// channel 1 registers
uint32_t bank0 rfreq1;
uint8_t  bank0 rwave1;
uint8_t  bank0 rvol1;

// channel 2 registers
uint16_t bank0 rfreq2;
uint8_t  bank0 rwave2;
uint8_t  bank0 rvol2;

// channel 2 registers
uint16_t bank0 rfreq3;
uint8_t  bank0 rwave3;
uint8_t  bank0 rvol3;

void main()
{    
    // Set up the LEDs
    LATB7 = 1;
    LATB6 = 1;
    LATC6 = 0;

    // enable DAC and DACOUT pin: DACEN = 1, DACOE = 1
    VREFCON2 = 15;
    VREFCON1 = 0b10100000;

    // set up Timer2: period = 125 cycles (12 MHz instruction clock / 125 = 96 kHz); TMR2ON = 1
    PR2 = 124;
    T2CON = 0b00000100;


    PIR1bits.TMR2IF = 0; // clear Timer2-to-PR2-match interrupt flag
    PIE1bits.TMR2IE = 1; // enable Timer2-to-PR2-match interrupt

    // enable interrupts
    RCONbits.IPEN = 1;    // Enable interrupt priority levels
    INTCONbits.GIE = 1;   // Enable high priority interrupts

    uint8_t const * e[3];
    e[0] = &insert_coin;
    e[1] = back_sounds[0];
    e[2] = other_sounds[0];

    uint8_t init[3], dir_reverse[3];
    uint8_t base_freq[3], freq[3], duration[3], repeat[3], vol[3];
    int8_t freq_inc[3];

    uint8_t wait[3] = { 0 }, sixtieths[3], index[3];

    while (1)
    {
        
        if (ticks >= TICKS_PER_REG_UPDATE)
        {
            ticks = 0;

            for (uint8_t c = 0; c < 3; c++)
            {
                sixtieths[c]++;

                if (wait[c])
                {
                    wait[c]--;
                    continue;
                }

                if (!init[c])
                {
                    init[c] = 1;
                    dir_reverse[c] = 0;
                    freq[c] = base_freq[c] = e[c][1];
                    freq_inc[c] = e[c][2];
                    duration[c] = e[c][3] & 0x7f;
                    repeat[c] = e[c][5];
                    vol[c] = e[c][6] & 0xf;

                    sixtieths[c] = 0;
                }

                if (--duration[c] == 0 ||  sixtieths[c] > 240)
                {
                    if (repeat[c]-- <= 1  ||  sixtieths[c] > 240)
                    {
                        // done with this effect
                        init[c] = 0;
                        vol[c] = 0;
                        index[c]++;

                        switch (c)
                        {
                            case 0:
                                wait[c] = 60;
                                break;

                            case 1:
                                if (index[1] >= 7)
                                    index[1] = 0;
                                e[1] = back_sounds[index[1]];
                                break;
                                
                            default: // 2
                                if (index[2] >= 6)
                                    index[2] = 0;
                                e[2] = other_sounds[index[2]];
                                break;
                        }
                    }
                    else
                    {
                        duration[c] = e[c][3] & 0x7f;

                        if (e[c][3] & 0x80)
                        {
                            // reverse
                            freq_inc[c] = -freq_inc[c];
                            dir_reverse[c] = ~dir_reverse[c];
                        }

                        if (!dir_reverse[c])
                        {
                            base_freq[c] += e[c][4];
                            freq[c] = base_freq[c];
                            vol[c] += e[c][7];
                        }
                    }
                }

                freq[c] += freq_inc[c];
            }
            rfreq1 = (uint32_t)freq[0] << ((e[0][0] & 0x70) >> 4);
            rwave1 = e[0][0] & 0x7;
            rvol1 = vol[0];
            rfreq2 = (uint16_t)freq[1] << ((e[1][0] & 0x70) >> 4);
            rwave2 = e[1][0] & 0x7;
            rvol2 = vol[1];
            rfreq3 = (uint16_t)freq[2] << ((e[2][0] & 0x70) >> 4);
            rwave3 = e[2][0] & 0x7;
            rvol3 = vol[2];
        }
    }
}

/*void interrupt highIsr()
{
    //static uint32_t acc1 = 0;
    static uint16_t acc2 = 0;
    static uint16_t acc3 = 0;
    static uint8_t channel = 1;
    
    LED_RED(0);

    PIR1bits.TMR2IF = 0;
    ticks++;

    //acc1 += rfreq1;
    acc2 += rfreq2;
    acc3 += rfreq3;

    if (++channel >= 3)
        channel = 0;

    switch (channel)
    {
        case 0:
            VREFCON2 = 15;//vol_products[waveforms[rwave1][(*((uint8_t *)&acc1 + 2) & 0xF) << 1 | (*((uint8_t *)&acc1 + 2) & 0x80 ? 1 : 0)]][rvol1];
            break;

        case 1:
            VREFCON2 = vol_modify[waveform[*((uint8_t *)&acc2 + 1) & 0xf8 | rwave2]][rvol2];
            break;

        default: // 2
            VREFCON2 = vol_modify[waveform[*((uint8_t *)&acc3 + 1) & 0xf8 | rwave3]][rvol3];
            break;
    }
        
    LED_RED(1);
}*/