#ifndef EFFECTS_H
#define	EFFECTS_H

uint8_t const eat_dot1[8] = { 0x42, 0x18, 0xfd, 0x06, 0x00, 0x01, 0x0c, 0x00 };
uint8_t const eat_dot2[8] = { 0x42, 0x04, 0x03, 0x06, 0x00, 0x01, 0x0c, 0x00 };
uint8_t const eat_fruit[8] = { 0x56, 0x0c, 0xff, 0x8c, 0x00, 0x02, 0x0f, 0x00 };
uint8_t const eat_ghost[8] = { 0x05, 0x00, 0x02, 0x20, 0x00, 0x01, 0x0c, 0x00 };
uint8_t const death1[8]     = { 0x41, 0x20, 0xff, 0x86, 0xfe, 0x1c, 0x0f, 0xff };
uint8_t const death2[8] = {0x70, 0x00, 0x01, 0x0c, 0x00, 0x01, 0x08, 0x00};
uint8_t const siren1[8] = {0x36, 0x20, 0x04, 0x8C, 0x00, 0x00, 0x06, 0x00};
uint8_t const siren2[8] = { 0x36, 0x28, 0x05, 0x8B, 0x00, 0x00, 0x06, 0x00 };
uint8_t const siren3[8] = { 0x36, 0x30, 0x06, 0x8A, 0x00, 0x00, 0x06, 0x00 };
uint8_t const siren4[8] = { 0x36, 0x3C, 0x07, 0x89, 0x00, 0x00, 0x06, 0x00 };
uint8_t const siren5[8] = { 0x36, 0x48, 0x08, 0x88, 0x00, 0x00, 0x06, 0x00 };
uint8_t const blue_ghosts[8] = { 0x24, 0x00, 0x06, 0x08, 0x00, 0x00, 0x0A, 0x00 };
uint8_t const ghost_eyes[8] = { 0x40, 0x70, 0xFA, 0x10, 0x00, 0x00, 0x0A, 0x00 };

uint8_t const * const all_sounds[13] = { &eat_dot1, &eat_dot2, &eat_fruit, &eat_ghost, &death1, &death2, &siren1, &siren2, &siren3, &siren4, &siren5, &blue_ghosts, &ghost_eyes};


#endif	/* EFFECTS_H */

