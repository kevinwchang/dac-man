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

// channel registers
uint32_t bank0 rfreq[3]; // channel 1: 20 bits; channels 2 & 3: 16 bits
uint8_t  bank0 rwave[3];
uint8_t  bank0 rvol[3];

static uint8_t const * effect[3];
static uint8_t effect_trigger[3];
static uint8_t effect_is_playing[3];

// assumptions:
//  c is 0-2
//  effect[c] points to a valid effect
void update_effect(uint8_t c)
{
    static uint8_t dir_reverse[3];
    static uint8_t base_freq[3], freq[3], duration[3], repeat[3], vol[3];
    static int8_t freq_inc[3];

    if (effect_trigger[c])
    {
        effect_trigger[c] = 0;
        effect_is_playing[c] = 1;
        dir_reverse[c] = 0;
        freq[c] = base_freq[c] = effect[c][1];
        freq_inc[c] = effect[c][2];
        duration[c] = effect[c][3] & 0x7f;
        repeat[c] = effect[c][5];
        vol[c] = effect[c][6] & 0xf;
    }
    else if (!effect_is_playing[c])
        return;

    if (--duration[c] == 0)
    {
        // done with duration counter; check repeat counter
        if (repeat[c]-- <= 1)
        {
            // done with this effect
            effect_is_playing[c] = 0;
            rvol[c] = 0;
            return;
        }
        else
        {
            // reset duration counter
            duration[c] = effect[c][3] & 0x7f;

            if (effect[c][3] & 0x80)
            {
                // reverse
                freq_inc[c] = -freq_inc[c];
                dir_reverse[c] = ~dir_reverse[c];
            }

            if (!dir_reverse[c])
            {
                // not reversing, so apply increments
                base_freq[c] += effect[c][4];
                freq[c] = base_freq[c];
                vol[c] += effect[c][7];
            }
        }
    }

    freq[c] += freq_inc[c];
    uint8_t shift = (effect[c][0] & 0x70) >> 4;

    // set registers
    rfreq[c] = (uint32_t)freq[c] << shift;
    rwave[c] = effect[c][0] & 0x7;
    rvol[c] = vol[c];
}

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

    effect[0] = &insert_coin;
    effect[1] = back_sounds[0];
    effect[2] = other_sounds[0];

    uint8_t wait_after[3], timeout[3], index[3];

    while (1)
    {  
        if (ticks >= TICKS_PER_REG_UPDATE)
        {
            ticks = 0;

            for (uint8_t c = 0; c < 3; c++)
            {
                if (effect_is_playing[c] && timeout[c])
                    timeout[c]--;
                else
                {
                    if (wait_after[c])
                    {
                        wait_after[c]--;
                        continue;
                    }
                    else
                    {
                        index[c]++;

                        switch (c)
                        {
                            case 0:
                                effect[c] = &insert_coin;
                                wait_after[c] = 60;
                                break;

                            case 1:
                                if (index[c] >= 7)
                                    index[c] = 0;
                                effect[c] = back_sounds[index[c]];
                                break;

                            case 2: // 2
                                if (index[c] >= 6)
                                    index[c] = 0;
                                effect[c] = other_sounds[index[c]];
                                break;
                        }
                        timeout[c] = 240;
                        effect_trigger[c] = 1;
                    }
                }
                update_effect(c);
            }
        }
    }
}