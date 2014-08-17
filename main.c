#include <xc.h>
#include <stdint.h>
#include "waveforms.h"
#include "effects.h"
#include "songs.h"

#define _XTAL_FREQ 48000000

#define LED_GREEN(v)  ( TRISB7 = !(v) )
#define LED_YELLOW(v) ( TRISB6 = !(v) )
#define LED_RED(v)    ( TRISC6 = !(v) )

#define TICKS_PER_REG_UPDATE 1600

uint16_t volatile bank0 ticks = TICKS_PER_REG_UPDATE;

// channel registers
uint32_t bank0 rfreq[3]; // channel 1: 20 bits; channels 2 & 3: 16 bits
uint8_t  bank0 rwave[3];
uint8_t  bank0 rvol[3];

static uint8_t const * effect[3];
static uint8_t effect_trigger[3];
static uint8_t effect_is_playing[3];

static uint8_t const * song[2];
static uint8_t song_trigger[2];
static uint8_t song_is_playing[2];

static uint8_t song_wave[2];
static uint8_t song_freq_shift[2];
static uint8_t song_vol[2];
static uint8_t song_vol_adj[2];
static uint8_t song_note_freq_dbl[2];
static uint8_t song_note_dur[2];
static uint8_t song_note_vol[2];
static uint8_t song_note_freq[2];

static uint8_t vol_dec_toggle = 0; // used for decrementing volume every other reg update cycle

// assumptions:
//  c is 0-2
//  effect[c] points to a valid effect
void update_effect(uint8_t c)
{
    static uint8_t dir_reverse[3];
    static uint8_t base_freq[3], freq[3], dur[3], repeat[3], vol[3];
    static int8_t freq_inc[3];

    if (effect_trigger[c])
    {
        effect_trigger[c] = 0;
        effect_is_playing[c] = 1;
        dir_reverse[c] = 0;
        freq[c] = base_freq[c] = effect[c][1];
        freq_inc[c] = effect[c][2];
        dur[c] = effect[c][3] & 0x7f;
        repeat[c] = effect[c][5];
        vol[c] = effect[c][6] & 0xf;
    }
    else if (!effect_is_playing[c])
        return;

    if (--dur[c] == 0)
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
            dur[c] = effect[c][3] & 0x7f;

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

// returns 1 if song ended
uint8_t parse_song_byte(uint8_t c)
{
    while (1) // keep parsing bytes until an actual note comes up
    {
        switch(song[c][0])
        {
            case 0xf1:
                // waveform select
                song[c]++;
                song_wave[c] = song[c][0];
                break;

            case 0xf2:
                // frequency increment
                song[c]++;
                song_freq_shift[c] = song[c][0];
                break;

            case 0xf3:
                // volume
                song[c]++;
                song_vol[c] = song[c][0];
                break;

            case 0xf4:
                // volume adjust type
                song[c]++;
                song_vol_adj[c] = song[c][0];
                break;

            case 0xf0: // jump - not implemented
            case 0xff:
                // end of song
                return 1;

            default:
                // note
                song_note_freq_dbl[c] = (song[c][0] & 0x10) ? 1 : 0;
                song_note_vol[c] = song_vol[c];
                song_note_dur[c] = 1 << (song[c][0] >> 5);
                song_note_freq[c] = song_freq_table[song[c][0] & 0xf];
                song[c]++;
                return 0;
        }
        song[c]++;
    }
    return 0; // should never happen - squelch compiler warning
}

// assumptions:
//  c is 0-1
//  song[c] points to a valid song
//  update_effect was called before this function and already set or zeroed rvol[c]
void update_song(uint8_t c)
{
    if (song_trigger[c])
    {
        song_trigger[c] = 0;
        song_is_playing[c] = 1;
        song_note_dur[c] = 0;
    }
    else if (!song_is_playing[c])
        return;

    if (song_note_dur[c] <= 1)
    {
        if (parse_song_byte(c))
        {
            // done with this song
            song_is_playing[c] = 0;
            if (!effect_is_playing[c])
                rvol[c] = 0;
            return;
        }
    }
    else
        song_note_dur[c]--;

    rfreq[c] = (uint32_t)song_note_freq[c] << (song_note_freq_dbl[c] + song_freq_shift[c]);
    rwave[c] = song_wave[c];
    
    if (song_note_vol[c] && (song_vol_adj[c] == 1 || (song_vol_adj[c] == 2 && vol_dec_toggle)))
        song_note_vol[c]--;

    rvol[c] = song_note_vol[c];
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

    song[0] = &startc1;// intermissionc1;
    song[1] = &startc2;//intermissionc2;
    song_trigger[0] = 1;
    song_trigger[1] = 1;

    uint8_t wait_after[3], timeout[3], index[3];

    while (1)
    {  
        if (ticks >= TICKS_PER_REG_UPDATE)
        {
            ticks = 0;
            vol_dec_toggle = !vol_dec_toggle;

            for (uint8_t c = 0; c < 3; c++)
            {
                if (effect_is_playing[c] && timeout[c])
                    timeout[c]--;
                else if (!song_is_playing[0])
                {
                    if (wait_after[c])
                    {
                        wait_after[c]--;
                        continue;
                    }
                    else
                    {
                        switch (c)
                        {
                            /*case 0:
                                effect[c] = &insert_coin;
                                wait_after[c] = 60;
                                break;*/

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
                        index[c]++;

                        timeout[c] = 240;
                        effect_trigger[c] = 1;
                    }
                }

                update_effect(c);

                if (c < 2)
                    update_song(c);
            }
        }
    }
}