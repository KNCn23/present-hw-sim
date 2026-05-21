#ifndef HW_SIM_H
#define HW_SIM_H

#include <stdint.h>

/* Counters that accumulate as the bit-level circuit operates. */
typedef struct {
    uint64_t and_gates;     /* 2-input AND  */
    uint64_t or_gates;      /* 2-input OR   */
    uint64_t xor_gates;     /* 2-input XOR  */
    uint64_t not_gates;     /* inverter     */
    uint64_t mux_gates;     /* 2:1 MUX      */
    uint64_t flip_flops;    /* D-FF used    */
    uint64_t clock_cycles;  /* total cycles */
    double   energy_pj;     /* estimated pJ */
} HwStats;

void hw_stats_reset(HwStats *s);
void hw_stats_print(const HwStats *s);

/* Bit-level encryption with full hardware accounting. */
void present80_encrypt_hw(const uint8_t key[10],
                          const uint8_t in[8],
                          uint8_t out[8],
                          HwStats *stats,
                          int pipelined);

#endif
