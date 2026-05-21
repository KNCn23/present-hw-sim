# PRESENT-80 Hardware Simulator

A bit-level simulator for the [PRESENT-80](https://en.wikipedia.org/wiki/PRESENT) lightweight block cipher that accounts for gate count, clock cycles, and dynamic energy. Inspired by the original Bogdanov et al. 2007 paper that targeted resource-constrained hardware (RFID, IoT, defense embedded systems).

## What it does

- Implements PRESENT-80 in software for correctness
- Reruns the same algorithm against a synthetic **gate-level model** that counts:
  - Every AND / OR / XOR / NOT / MUX gate
  - Every D flip-flop in pipeline registers
  - Dynamic switching energy (pJ) per gate type
- Reports total **Gate Equivalents (GE)** — the standard hardware area metric
- Compares **round-iterated** (serial) vs **fully pipelined** architectures
- Estimates throughput at a target clock frequency

## Sample output

```
PRESENT-80 — bit-level hardware simulator
═════════════════════════════════════════

Key:         00000000000000000000
Plaintext:   0000000000000000
Ciphertext:  5579c1387b228445

[ Round-iterated implementation ]
Hardware statistics
───────────────────────────────────────
  AND gates       :       2015
  XOR gates       :       7100
  D Flip-Flops    :       1984
  ─────────────
  Total area      :    26430.5 GE
  Clock cycles    :         32
  Dynamic energy  :    3450.20 pJ

Estimated throughput @ 100 MHz:
  Round-iterated :   200.0 Mbps
  Pipelined      :  6400.0 Mbps
```

## Build & run

```bash
make run
```

## Architecture

```
├── include/
│   ├── present.h   # Cipher API
│   └── hw_sim.h    # HwStats struct + hardware-accounting wrapper
├── src/
│   ├── present.c   # S-box, P-layer, key schedule (software reference)
│   ├── hw_sim.c    # Per-operation gate accounting + GE conversion
│   └── main.c      # Demo: compares iterative vs pipelined
└── Makefile
```

## Why this matters

PRESENT was specifically designed as a *lightweight* cipher for hardware deployment — its 1570 GE area target was a major contribution. This simulator lets you explore the trade-off behind that number: how the S-box, P-layer, key schedule, and pipeline registers each contribute to area, latency, and energy. Useful for crypto courses, embedded security work, and hardware design exploration.

## References

- A. Bogdanov et al., *"PRESENT: An Ultra-Lightweight Block Cipher"*, CHES 2007.
- ISO/IEC 29192-2:2019 — Lightweight cryptography Part 2: Block ciphers.

## License

MIT
