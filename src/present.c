#include "present.h"
#include <string.h>

/* 4-bit S-box from the PRESENT specification */
static const uint8_t SBOX[16] = {
    0xC, 0x5, 0x6, 0xB, 0x9, 0x0, 0xA, 0xD,
    0x3, 0xE, 0xF, 0x8, 0x4, 0x7, 0x1, 0x2
};

/* Bit-level permutation: bit i goes to position P[i] */
static const uint8_t P[64] = {
     0,16,32,48, 1,17,33,49, 2,18,34,50, 3,19,35,51,
     4,20,36,52, 5,21,37,53, 6,22,38,54, 7,23,39,55,
     8,24,40,56, 9,25,41,57,10,26,42,58,11,27,43,59,
    12,28,44,60,13,29,45,61,14,30,46,62,15,31,47,63
};

static uint64_t bytes_to_u64(const uint8_t *b) {
    uint64_t r = 0;
    for (int i = 0; i < 8; i++) r = (r << 8) | b[i];
    return r;
}
static void u64_to_bytes(uint64_t v, uint8_t *b) {
    for (int i = 7; i >= 0; i--) { b[i] = v & 0xFF; v >>= 8; }
}

static uint64_t sbox_layer(uint64_t s) {
    uint64_t out = 0;
    for (int i = 0; i < 16; i++) {
        uint8_t nib = (s >> (4 * i)) & 0xF;
        out |= ((uint64_t)SBOX[nib]) << (4 * i);
    }
    return out;
}

static uint64_t pbox_layer(uint64_t s) {
    uint64_t out = 0;
    for (int i = 0; i < 64; i++)
        if (s & ((uint64_t)1 << i))
            out |= ((uint64_t)1 << P[i]);
    return out;
}

/* Key schedule: 80-bit register, output 64-bit round key per round. */
static void key_schedule(const uint8_t key[10], uint64_t round_keys[32]) {
    uint64_t hi = 0;  /* top 16 bits in low word */
    uint64_t lo = 0;  /* bottom 64 bits */
    /* key bytes: K[0..9], with K[0] being the MSB */
    for (int i = 0; i < 8; i++) lo = (lo << 8) | key[i + 2];
    hi = ((uint64_t)key[0] << 8) | key[1];

    for (int r = 0; r < 32; r++) {
        round_keys[r] = ((hi & 0xFFFF) << 48) | (lo >> 16);
        if (r == 31) break;

        /* Rotate (hi:lo, 80-bit) left by 61 == right by 19 */
        uint64_t new_lo = (lo >> 19) | (hi << 45);
        uint64_t new_hi = ((lo & 0x7FFFF) << (16 - 3)) | (hi >> 19);
        /* Simpler: implement as bit-level shift on the concatenated 80-bit */
        /* For correctness, redo the rotation by extracting bits cleanly. */
        uint8_t reg[80];
        for (int i = 0; i < 64; i++) reg[i] = (lo >> i) & 1;
        for (int i = 0; i < 16; i++) reg[64 + i] = (hi >> i) & 1;
        uint8_t newreg[80];
        for (int i = 0; i < 80; i++) newreg[i] = reg[(i + 19) % 80];
        /* Now apply S-box to top 4 bits (bits 76..79) */
        uint8_t top = 0;
        for (int i = 0; i < 4; i++) top |= (newreg[76 + i] << i);
        top = SBOX[top];
        for (int i = 0; i < 4; i++) newreg[76 + i] = (top >> i) & 1;
        /* XOR round counter (r+1) into bits 15..19 */
        uint8_t rc = (uint8_t)(r + 1);
        for (int i = 0; i < 5; i++)
            newreg[15 + i] ^= (rc >> i) & 1;
        /* Rebuild lo/hi */
        lo = 0; hi = 0;
        for (int i = 0; i < 64; i++) lo |= ((uint64_t)newreg[i]) << i;
        for (int i = 0; i < 16; i++) hi |= ((uint64_t)newreg[64 + i]) << i;

        (void)new_lo; (void)new_hi;  /* unused alternate path */
    }
}

void present80_encrypt(const uint8_t key[10],
                       const uint8_t in[8],
                       uint8_t out[8]) {
    uint64_t round_keys[32];
    key_schedule(key, round_keys);

    uint64_t state = bytes_to_u64(in);
    for (int r = 0; r < 31; r++) {
        state ^= round_keys[r];
        state  = sbox_layer(state);
        state  = pbox_layer(state);
    }
    state ^= round_keys[31];

    u64_to_bytes(state, out);
}
