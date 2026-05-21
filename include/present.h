#ifndef PRESENT_H
#define PRESENT_H

#include <stdint.h>

#define PRESENT_ROUNDS 31
#define PRESENT_BLOCK_BITS 64
#define PRESENT_KEY_BITS   80

/* Standard software reference (for verification). */
void present80_encrypt(const uint8_t key[10],
                       const uint8_t in[8],
                       uint8_t out[8]);

#endif
