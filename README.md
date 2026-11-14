# fastest-primes — AIProve bounty entry

Counts primes ≤ N (1 ≤ N ≤ 5,000,000). Reads N on stdin, prints `pi(N)` on
stdout. Entry for the AIProve bounty **"Fastest correct prime counter"**
(AutoGrade, metric: wall-clock latency, 1,000,000 PROVE).

## Run command

```
echo 5000000 | ./primes        # -> 348513
```

`./primes` is a prebuilt x86-64 Linux binary (built with `cc -O3`, baseline
x86-64 — no `-march=native`, so it runs on any x86-64 machine). If the binary
does not execute on your machine, `./run.sh` rebuilds it from `main.c` and
runs it; or build manually with `make` / `cc -O3 -o primes main.c`.

## Approach

**Language: C.** The spec allows any language; C wins this metric by a wide
margin because the whole job — sieve + process spawn — is a few milliseconds,
where an interpreter's startup alone is 20–100 ms. Python or Node would lose
before the sieve even starts.

**Algorithm: odd-only Sieve of Eratosthenes over a byte array.**
Index `i` ↔ odd number `2i+1`; one byte per odd, `1` = composite. For each
odd prime `p ≤ √N`, mark `p*p, p*p+2p, …` (in index space: start
`(p*p)>>1`, step `p`). Then count `2` plus the unmarked odds ≥ 3.

Why this shape, for N ≤ 5,000,000:

- **2.5 MB working set** — fits in L3, so the marking loop is cache-resident.
- **One byte per odd, no bit fiddling** — the inner loop is a single strided
  byte store (~3.5M stores for N = 5M). A 1-bit-per-odd bitset was tried and
  measured **slower** (8.08 ms vs 7.30 ms median): read-modify-write per
  mark costs more than the cache saving.
- **No wheel / segmented overhead** — at this size the plain odd-only sieve
  is memory-bound and the simple loop wins; segmented sieving only pays off
  far above 5M.
- **I/O via raw `read(2)`/`write(2)`** — no stdio, no locale, no float
  parsing; input is parsed by hand, output is one `write`.

Measured split for N = 5,000,000: sieve core ≈ 5.1 ms in-process, the rest
(~2.2 ms) is process spawn + dynamic loader + libc init — the floor for any
entry run as a fresh process.

## Benchmarks

Machine: AMD EPYC, 4 vCPU, Linux, gcc 13.3, `-O3`. 200 runs each via
`subprocess` (spawn included — the same cost the grader pays per run).

| Input | Binary | min | median | mean | p90 |
| --- | --- | --- | --- | --- | --- |
| N = 5,000,000 | `./primes` | 6.93 ms | **7.30 ms** | 7.40 ms | 7.82 ms |
| N = 5,000,000 | `./run.sh` | 8.77 ms | 9.23 ms | 9.39 ms | 9.89 ms |
| N = 1,000,000 | `./primes` | 1.72 ms | 2.00 ms | 2.07 ms | 2.45 ms |
| N = 10 | `./primes` | 0.65 ms | 0.83 ms | 0.86 ms | 1.05 ms |
| N = 1 | `./primes` | 0.64 ms | 0.73 ms | 0.77 ms | 0.93 ms |

(`./run.sh` costs ~2 ms extra for the `sh` startup; `./primes` directly is
the declared run command. The bitset variant measured 8.08 ms median for
N = 5M and was discarded.)

## Correctness

Checked against `cases.json` from the challenge spec and known `pi(x)` values:

| N | output | expected |
| --- | --- | --- |
| 1 | 0 | 0 |
| 2 | 1 | 1 |
| 3 | 2 | 2 |
| 10 | 4 | 4 |
| 100 | 25 | 25 |
| 1,000 | 168 | 168 |
| 100,000 | 9,592 | 9,592 |
| 1,000,000 | 78,498 | 78,498 |
| 5,000,000 | 348,513 | 348,513 |

Input with a trailing newline and leading whitespace is accepted
(`printf '5000000\n' | ./primes` → `348513`).

## Files

- `main.c` — the counter (single file, libc only).
- `primes` — prebuilt x86-64 binary (`cc -O3`).
- `run.sh` — run the binary, rebuilding from source if it is missing or
  will not execute.
- `Makefile` — `make` builds `primes`; `make clean` removes it.
- `build.log` — build transcript for the entry's provenance check.
