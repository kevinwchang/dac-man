#ifndef SONGS_H
#define	SONGS_H

uint8_t const song_freq_table[16] = { 0x00, 0x57, 0x5c, 0x61, 0x67, 0x6d, 0x74, 0x7b, 0x82, 0x8a, 0x92, 0x9a, 0xa3, 0xad, 0xb8, 0xc3 };

uint8_t const introc1[] = {
    0xf1, 0x02, 0xf2, 0x03, 0xf3, 0x0f, 0xf4, 0x01,
    0x82, 0x70, 0x69, 0x82, 0x70, 0x69, 0x83, 0x70,
    0x6a, 0x83, 0x70, 0x6a, 0x82, 0x70, 0x69, 0x82,
    0x70, 0x69, 0x89, 0x8b, 0x8d, 0x8e, 0xff
};

uint8_t const introc2[] = {
    0xf1, 0x00, 0xf2, 0x02, 0xf3, 0x0f, 0xf4, 0x00,
    0x42, 0x50, 0x4e, 0x50, 0x49, 0x50, 0x46, 0x50,
    0x4e, 0x49, 0x70, 0x66, 0x70, 0x43, 0x50, 0x4f,
    0x50, 0x4a, 0x50, 0x47, 0x50, 0x4f, 0x4a, 0x70,
    0x67, 0x70, 0x42, 0x50, 0x4e, 0x50, 0x49, 0x50,
    0x46, 0x50, 0x4e, 0x49, 0x70, 0x66, 0x70, 0x45,
    0x46, 0x47, 0x50, 0x47, 0x48, 0x49, 0x50, 0x49,
    0x4a, 0x4b, 0x50, 0x6e, 0xff
};

uint8_t const intermissionc1[] = {
    0xf1, 0x02, 0xf2, 0x03, 0xf3, 0x0f, 0xf4, 0x01,
    0x67, 0x50, 0x30, 0x47, 0x30, 0x67, 0x50, 0x30,
    0x47, 0x30, 0x67, 0x50, 0x30, 0x47, 0x30, 0x4b,
    0x10, 0x4c, 0x10, 0x4d, 0x10, 0x4e, 0x10, 0x67,
    0x50, 0x30, 0x47, 0x30, 0x67, 0x50, 0x30, 0x47,
    0x30, 0x67, 0x50, 0x30, 0x47, 0x30, 0x4b, 0x10,
    0x4c, 0x10, 0x4d, 0x10, 0x4e, 0x10, 0x67, 0x50,
    0x30, 0x47, 0x30, 0x67, 0x50, 0x30, 0x47, 0x30,
    0x67, 0x50, 0x30, 0x47, 0x30, 0x4b, 0x10, 0x4c,
    0x10, 0x4d, 0x10, 0x4e, 0x10, 0x77, 0x20, 0x4e,
    0x10, 0x4d, 0x10, 0x4c, 0x10, 0x4a, 0x10, 0x47,
    0x10, 0x46, 0x10, 0x65, 0x30, 0x66, 0x30, 0x67,
    0x40, 0x70, 0xff
};

uint8_t const intermissionc2[] = {
    0xf1, 0x01, 0xf2, 0x01, 0xf3, 0x0f, 0xf4, 0x00,
    0x26, 0x67, 0x26, 0x67, 0x26, 0x67, 0x23, 0x44,
    0x42, 0x47, 0x30, 0x67, 0x2a, 0x8b, 0x70, 0x26,
    0x67, 0x26, 0x67, 0x26, 0x67, 0x23, 0x44, 0x42,
    0x47, 0x30, 0x67, 0x23, 0x84, 0x70, 0x26, 0x67,
    0x26, 0x67, 0x26, 0x67, 0x23, 0x44, 0x42, 0x47,
    0x30, 0x67, 0x29, 0x6a, 0x2b, 0x6c, 0x30, 0x2c,
    0x6d, 0x40, 0x2b, 0x6c, 0x29, 0x6a, 0x67, 0x20,
    0x29, 0x6a, 0x40, 0x26, 0x87, 0x70, 0xff
};

#endif	/* SONGS_H */

