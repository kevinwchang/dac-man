#include <xc.h>

#include "waveforms.h"
#include "songs.h"

// channel registers
uint32_t bank0 rfreq[3]; // channel 1: 20 bits; channels 2 & 3: 16 bits
uint8_t  bank0 rwave[3];
uint8_t  bank0 rvol[3];

uint32_t active_freq[3];
uint8_t active_wave[3];
uint8_t active_vol[3];

static uint8_t const * effect[3];
static uint8_t effect_trigger[3];
static uint8_t effect_active[3];

static uint8_t const * song[2];
static uint8_t song_active[2];

static uint8_t song_wave[2];
static uint8_t song_freq_shift[2];
static uint8_t song_vol[2];
static uint8_t song_vol_adj[2];
static uint8_t song_note_freq_dbl[2];
static uint8_t song_note_dur[2];
static uint8_t song_note_vol[2];
static uint8_t song_note_freq[2];

static uint8_t vol_dec_toggle = 0; // used for decrementing volume every other reg update cycle


// channel is 1-3
void play_effect(uint8_t channel, uint8_t const * effect_to_play)
{
    effect[channel - 1] = effect_to_play;
    effect_active[channel - 1] = 1;
    effect_trigger[channel - 1] = 1;
}

void stop_effect(uint8_t channel)
{
    effect_active[channel - 1] = 0;
}

// channel is 1-3
bit effect_is_playing(uint8_t channel)
{
    return effect_active[channel - 1];
}

// channel is 1-2
void play_song(uint8_t channel, uint8_t const * song_to_play)
{
    song[channel - 1] = song_to_play;
    song_active[channel - 1] = 1;
}

void stop_song(uint8_t channel)
{
    song_active[channel - 1] = 0;
}

// channel is 1-2
uint8_t song_is_playing(uint8_t channel)
{
    return song_active[channel - 1];
}

// assumptions:
//  c is 0-2
//  effect[c] points to a valid effect
static void update_effect(uint8_t c)
{
    static uint8_t dir_reverse[3];
    static uint8_t base_freq[3], freq[3], dur[3], repeat[3], vol[3];
    static int8_t freq_inc[3];

    if (!effect_active[c])
        return;
    
    if (effect_trigger[c])
    {
        effect_trigger[c] = 0;
        dir_reverse[c] = 0;
        freq[c] = base_freq[c] = effect[c][1];
        freq_inc[c] = effect[c][2];
        dur[c] = effect[c][3] & 0x7f;
        repeat[c] = effect[c][5];
        vol[c] = effect[c][6] & 0xf;
    } 

    if (!--dur[c])
    {
        // done with duration counter; check repeat counter
        if (repeat[c] && !--repeat[c])
        {
            // done with this effect
            effect_active[c] = 0;
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

    active_freq[c] = (uint32_t)freq[c] << shift;
    active_wave[c] = effect[c][0] & 0x7;
    active_vol[c] = vol[c];
}

// returns 1 if song ended
static uint8_t parse_song_byte(uint8_t c)
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
//  song[c] points to a valid song
static void update_song(uint8_t c)
{
    if (c > 1 || !song_active[c])
        return;

    if (!song_note_dur[c] || !--song_note_dur[c])
    {
        if (parse_song_byte(c))
        {
            // done with this song
            song_active[c] = 0;
            return;
        }
    }

    active_freq[c] = (uint32_t)song_note_freq[c] << (song_note_freq_dbl[c] + song_freq_shift[c]);
    active_wave[c] = song_wave[c];

    if (song_note_vol[c] && (song_vol_adj[c] == 1 || (song_vol_adj[c] == 2 && vol_dec_toggle)))
        song_note_vol[c]--;

    active_vol[c] = song_note_vol[c];
}

// should be called at 60 Hz
void update_sounds()
{
    vol_dec_toggle = !vol_dec_toggle;

    for (uint8_t c = 0; c < 3; c++)
    {
        active_vol[c] = 0;

        update_effect(c);
        update_song(c);

        // set registers
        rfreq[c] = active_freq[c];
        rwave[c] = active_wave[c];
        rvol[c] = active_vol[c];
    }
}