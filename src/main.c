#include <stdio.h>
#include <string.h>
#include "present.h"
#include "hw_sim.h"

static void hex_print(const char *label, const uint8_t *b, int n) {
    printf("%-12s ", label);
    for (int i = 0; i < n; i++) printf("%02x", b[i]);
    printf("\n");
}

int main(void) {
    /* Standard PRESENT-80 test vector */
    const uint8_t key[10]  = {0,0,0,0,0,0,0,0,0,0};
    const uint8_t plain[8] = {0,0,0,0,0,0,0,0};
    /* Expected ciphertext: 0x5579C1387B228445 (per the PRESENT paper) */

    uint8_t out[8];
    HwStats stats;
    hw_stats_reset(&stats);

    printf("PRESENT-80 — bit-level hardware simulator\n");
    printf("═════════════════════════════════════════\n\n");
    hex_print("Key:",        key,   10);
    hex_print("Plaintext:",  plain,  8);

    present80_encrypt_hw(key, plain, out, &stats, /*pipelined=*/0);
    hex_print("Ciphertext:", out,    8);
    printf("\n[ Round-iterated implementation ]\n");
    hw_stats_print(&stats);

    hw_stats_reset(&stats);
    present80_encrypt_hw(key, plain, out, &stats, /*pipelined=*/1);
    printf("[ Fully pipelined implementation ]\n");
    hw_stats_print(&stats);

    /* Throughput estimate at 100 MHz */
    double freq_mhz = 100.0;
    double tput_iter = (freq_mhz * 1e6 * 64) / (32 * 1e6);   /* Mbps */
    double tput_pipe = (freq_mhz * 1e6 * 64) / 1e6;          /* Mbps */
    printf("Estimated throughput @ %.0f MHz:\n", freq_mhz);
    printf("  Round-iterated : %7.1f Mbps\n", tput_iter);
    printf("  Pipelined      : %7.1f Mbps\n", tput_pipe);
    return 0;
}
