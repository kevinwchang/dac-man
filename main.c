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

uint16_t volatile r_freq[3];
uint8_t volatile r_wave[3], r_vol[3];

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
    e[1] = back_sounds[0];
    e[2] = other_sounds[0];

    uint8_t init[3], dir_reverse[3];
    uint8_t base_freq[3], freq[3], duration[3], repeat[3], vol[3];
    int8_t freq_inc[3];

    uint8_t wait[3], sixtieths[3], index[3];

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
                        r_vol[c] = 0;
                        index[c]++;

                        switch (c)
                        {
                            case 0:
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
                        continue;
                    }

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

                freq[c] += freq_inc[c];
                r_freq[c] = (uint16_t)freq[c] << ((e[c][0] & 0x70) >> 4);
                r_wave[c] = e[c][0] & 0x7;
                r_vol[c] = vol[c];
            }
        }
    }
}

void interrupt highIsr()
{
    static uint16_t acc2 = 0;
    static uint16_t acc3 = 0;
    static uint8_t channel = 1;
    
    LED_RED(0);

    PIR1bits.TMR2IF = 0;
    ticks++;

    acc2 += r_freq[1];
    acc3 += r_freq[2];

    if (++channel >= 3)
        channel = 0;

    switch (channel)
    {
        case 0:
            VREFCON2 = 15;
            break;

        case 1:
            VREFCON2 =  vol_product[waveforms[r_wave[1]][*((uint8_t *)&acc2 + 1) >> 3]][r_vol[1]];
            break;

        default: // 2
            VREFCON2 = vol_product[waveforms[r_wave[2]][*((uint8_t *)&acc3 + 1) >> 3]][r_vol[2]];
            break;
    }
        
    LED_RED(1);
}