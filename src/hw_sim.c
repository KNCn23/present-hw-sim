#include "hw_sim.h"
#include <stdio.h>
#include <string.h>

/* Approximate energy per gate switch (pJ) — based on 65nm CMOS estimates */
#define E_AND  0.30
#define E_OR   0.30
#define E_XOR  0.55
#define E_NOT  0.15
#define E_MUX  0.50
#define E_FF   1.20

/* Gate count of one PRESENT S-box implemented in optimized AOI logic.
 * From Bogdanov et al. "PRESENT: An Ultra-Lightweight Block Cipher" (2007):
 *   ≈ 32 NAND-equivalent GE. We model it as 14 XOR + 4 AND + 4 OR + 4 NOT.
 */
static void account_sbox(HwStats *s) {
    s->xor_gates += 14;
    s->and_gates += 4;
    s->or_gates  += 4;
    s->not_gates += 4;
    s->energy_pj += 14 * E_XOR + 4 * E_AND + 4 * E_OR + 4 * E_NOT;
}

/* The P-layer is wire-only — no gate cost, only a 1-cycle register delay. */
static void account_player(HwStats *s) {
    s->flip_flops += 64;
    s->energy_pj  += 64 * E_FF;
}

/* AddRoundKey: 64 XORs per round. */
static void account_addkey(HwStats *s) {
    s->xor_gates += 64;
    s->energy_pj += 64 * E_XOR;
}

void hw_stats_reset(HwStats *s) { memset(s, 0, sizeof(*s)); }

void hw_stats_print(const HwStats *s) {
    /* GE (Gate Equivalent) — industry convention: 1 GE = 1 NAND2 area. */
    /* Rough conversions: AND=1.25, OR=1.25, XOR=2.5, NOT=0.75, MUX=2.25, FF=4.5 */
    double ge =  1.25 * s->and_gates
              + 1.25 * s->or_gates
              + 2.50 * s->xor_gates
              + 0.75 * s->not_gates
              + 2.25 * s->mux_gates
              + 4.50 * s->flip_flops;

    printf("Hardware statistics\n");
    printf("───────────────────────────────────────\n");
    printf("  AND gates       : %10lu\n", s->and_gates);
    printf("  OR  gates       : %10lu\n", s->or_gates);
    printf("  XOR gates       : %10lu\n", s->xor_gates);
    printf("  NOT gates       : %10lu\n", s->not_gates);
    printf("  MUX gates       : %10lu\n", s->mux_gates);
    printf("  D Flip-Flops    : %10lu\n", s->flip_flops);
    printf("  ─────────────\n");
    printf("  Total area      : %10.1f GE\n", ge);
    printf("  Clock cycles    : %10lu\n", s->clock_cycles);
    printf("  Dynamic energy  : %10.2f pJ\n", s->energy_pj);
    if (s->clock_cycles)
        printf("  Energy / block  : %10.2f pJ\n", s->energy_pj);
    printf("\n");
}

/* Forward decl for reference cipher */
void present80_encrypt(const uint8_t key[10], const uint8_t in[8],
                       uint8_t out[8]);

void present80_encrypt_hw(const uint8_t key[10],
                          const uint8_t in[8],
                          uint8_t out[8],
                          HwStats *stats,
                          int pipelined) {
    /* For correctness, defer to the software reference for the output. */
    present80_encrypt(key, in, out);

    /* Per-round costs */
    for (int r = 0; r < 31; r++) {
        account_addkey(stats);
        for (int i = 0; i < 16; i++) account_sbox(stats);
        account_player(stats);
    }
    account_addkey(stats);  /* final whitening */

    /* Key schedule: 1 S-box + 5 XORs per round */
    for (int r = 0; r < 31; r++) {
        account_sbox(stats);
        stats->xor_gates += 5;
        stats->energy_pj += 5 * E_XOR;
    }

    if (pipelined) {
        /* Fully unrolled, deep-pipelined: 1 block / cycle after warm-up. */
        stats->clock_cycles = 32;   /* 32-stage pipeline depth */
        stats->flip_flops  += 32 * 64;  /* pipeline registers */
        stats->energy_pj   += 32 * 64 * E_FF;
    } else {
        /* Round-iterated (serial): 1 round / cycle, 32 cycles total. */
        stats->clock_cycles = 32;
    }
}
