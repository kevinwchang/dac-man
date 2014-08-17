#include <xc.h>
#include <stdint.h>
#include "pacsounds.h"
#include "effects.h"
#include "songs.h"

#define _XTAL_FREQ 48000000

#define LED_GREEN(v)  ( TRISB7 = !(v) )
#define LED_YELLOW(v) ( TRISB6 = !(v) )
#define LED_RED(v)    ( TRISC6 = !(v) )

#define TICKS_PER_SOUND_UPDATE 1600

uint16_t volatile bank0 ticks = TICKS_PER_SOUND_UPDATE;

uint8_t const * const back_effects[7] = { &siren1, &siren2, &siren3, &siren4, &siren5, &blue_ghosts, &ghost_eyes };
uint8_t const * const event_effects[6] = { &eat_dot1, &eat_dot2, &eat_fruit, &eat_ghost, &death1, &death2 };


void select_sounds_loop()
{
    static uint8_t state, started, e2, e3, timeout;

    switch (state)
    {
        case 0:
            if (!started)
            {
                started = 1;
                play_song(1, &introc1);
                play_song(2, &introc2);
            }
            else if (!song_is_playing(1))
            {
                state++;
                started = 0;
            }
            break;

        case 2:
            if (!started)
            {
                started = 1;
                play_song(1, &intermissionc1);
                play_song(2, &intermissionc2);
            }
            else if (!song_is_playing(1))
            {
                state++;
                started = 0;
            }
            break;

        case 4:
            if (!started)
            {
                started = 1;
                e2 = e3 = 0;
                timeout = 180;
                play_effect(2, back_effects[e2++]);
                play_effect(3, event_effects[e3++]);
            }
            else
            {
                if (!effect_is_playing(3))
                {
                    if (e3 == 6)
                        e3 = 0;
                    play_effect(3, event_effects[e3++]);
                }
                if (!--timeout)
                {
                    if (e2 == 7)
                    {
                        stop_effect(2);
                        stop_effect(3);
                        state++;
                        started = 0;
                    }
                    else
                    {
                        timeout = 180;
                        play_effect(2, back_effects[e2++]);
                    }
                }
            }
            break;

        default:
            if (!started)
            {
                started = 1;
                timeout = 60;
            }
            else if (!--timeout)
            {
                state++;
                if (state == 6)
                    state = 0;
                started = 0;
            }
    }
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

    while (1)
    {  
        if (ticks >= TICKS_PER_SOUND_UPDATE)
        {
            ticks = 0;
            
            select_sounds_loop();
            update_sounds();
        }
    }
}