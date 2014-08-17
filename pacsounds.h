#ifndef PACSOUNDS_H
#define	PACSOUNDS_H

void play_effect(uint8_t channel, uint8_t const * effect_to_play);
void stop_effect(uint8_t channel);
uint8_t effect_is_playing(uint8_t channel);

void play_song(uint8_t channel, uint8_t const * song_to_play);
void stop_song(uint8_t channel);
uint8_t song_is_playing(uint8_t channel);

void update_sounds();

#endif	/* PACSOUNDS_H */

