# Algorithms 2 — Assignment I (C++)

This repository contains a reproducible C++ implementation for all three assignment parts.

## Implemented algorithms

Part 1A exact matching:
- Brute-force
- Sunday
- KMP
- FSM (finite-state machine matcher)
- Rabin-Karp (64-bit rolling hash)
- Gusfield Z

Part 1B special race comparisons:
- Binary Sunday vs Gusfield Z
- KMP vs Rabin-Karp
- Rabin-Karp vs Sunday

Part 2 wildcard matching (`?`, `*`, escaping with `\`):
- Boolean wildcard matcher usable as Brute-force/Sunday extension backend.

Part 3 “Jewish-style carp” notes:
- 2D rolling hash can be done with power-of-two bitmask arithmetic (`&`) to avoid modulo prime.
- Keep total runtime linear in number of pixels by scanning once over rows/columns with rolling updates.

## Build & run

```bash
g++ -std=c++17 -O2 main.cpp -o assignment
./assignment
```

Outputs:
- `benchmark.csv`: runtime results by algorithm, text length, and pattern size (small/large).
- `wacky_races.txt`: empirical timing ratios for Part 1B.

## Reproducibility

- Random text generation uses fixed seed `42`.
- Timing uses best-of-N measurements (`time_ms`) to reduce noise.
- Text lengths are fixed: 10k, 20k, 50k, 100k, 200k.
