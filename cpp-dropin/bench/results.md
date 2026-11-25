# C++ I/O benchmark results

Median wall time of 5 runs (3 for runs over 2 s), in milliseconds. Input comes from a file in the page cache.
Machine: g++ 13.3 (Ubuntu 24.04), Intel Xeon @ 2.10GHz, 2 vCPU cloud VM, Linux 6.18. Compiled with `g++ -std=gnu++17 -O2`.

Every method produced byte-identical output for every test (checked by MD5).
Run-to-run noise on this shared VM is roughly ±10%, so treat differences under ~15% as ties.

## Reading: wall time (ms)

| Method | read 10^7 ints (10 digits) | read 10^7 ints (1-10 digits) | read 5·10^6 long longs | read 5·10^6 words | read 5·10^6 doubles |
|---|---|---|---|---|---|
| `cin`/`cout` (default, synced) | 2,949 | 2,101 | 2,394 | 943 | 2,765 |
| `cin.tie(0)->sync_with_stdio(0)` | 566 | 595 | 435 | 216 | 990 |
| `scanf` / `printf` | 680 | 670 | 520 | 317 | 791 |
| `getchar` / `putchar` readInt() | 448 | 361 | 304 | 247 | 679 |
| `getchar_unlocked` readInt() | 211 | 189 | 136 | 149 | 536 |
| `fread` buffer + readInt() (classic snippet) | 188 | 176 | 166 | 173 | 588 |
| **fastio (this template)** | 76 | 143 | 71 | 94 | 94 |
| `mmap` + raw pointer (Linux only, reference) | 109 | 163 | 95 | 126 | 131 |

## Writing: wall time (ms), output written to a file

| Method | write 10^7 ints | write 5·10^6 long longs | write 5·10^6 doubles (fixed, 6) | write 5·10^6 doubles (setprecision(10), no fixed) | read + write 10^7 ints | read + write 10^7 mixed ints |
|---|---|---|---|---|---|---|
| `cin`/`cout` (default, synced) | 538 | 485 | 1,785 | 1,612 | 6,884 | 6,264 |
| `cin.tie(0)->sync_with_stdio(0)` | 475 | 325 | 1,673 | 1,551 | 1,056 | 896 |
| `scanf` / `printf` | 618 | 359 | 1,428 | 1,359 | 1,262 | 1,193 |
| `getchar` / `putchar` readInt() | 521 | 476 | 1,601 | 1,518 | 910 | 613 |
| `getchar_unlocked` readInt() | 369 | 303 | 1,450 | 1,441 | 442 | 347 |
| `fread` buffer + readInt() (classic snippet) | 318 | 270 | 1,385 | 1,497 | 372 | 314 |
| **fastio (this template)** | 118 | 108 | 147 | 187 | 176 | 212 |
| `mmap` + raw pointer (Linux only, reference) | 291 | 236 | 342 | 440 | 441 | 323 |

## Speed-up relative to `cin.tie(0)->sync_with_stdio(0)` (higher is better)

| Method | read 10^7 ints (10 digits) | read 10^7 ints (1-10 digits) | read 5·10^6 long longs | read 5·10^6 words | read 5·10^6 doubles | write 10^7 ints | write 5·10^6 long longs | write 5·10^6 doubles (fixed, 6) | write 5·10^6 doubles (setprecision(10), no fixed) | read + write 10^7 ints | read + write 10^7 mixed ints |
|---|---|---|---|---|---|---|---|---|---|---|---|
| `cin`/`cout` (default, synced) | 0.2× | 0.3× | 0.2× | 0.2× | 0.4× | 0.9× | 0.7× | 0.9× | 1.0× | 0.2× | 0.1× |
| `cin.tie(0)->sync_with_stdio(0)` | 1.0× | 1.0× | 1.0× | 1.0× | 1.0× | 1.0× | 1.0× | 1.0× | 1.0× | 1.0× | 1.0× |
| `scanf` / `printf` | 0.8× | 0.9× | 0.8× | 0.7× | 1.3× | 0.8× | 0.9× | 1.2× | 1.1× | 0.8× | 0.8× |
| `getchar` / `putchar` readInt() | 1.3× | 1.7× | 1.4× | 0.9× | 1.5× | 0.9× | 0.7× | 1.0× | 1.0× | 1.2× | 1.5× |
| `getchar_unlocked` readInt() | 2.7× | 3.1× | 3.2× | 1.5× | 1.8× | 1.3× | 1.1× | 1.2× | 1.1× | 2.4× | 2.6× |
| `fread` buffer + readInt() (classic snippet) | 3.0× | 3.4× | 2.6× | 1.2× | 1.7× | 1.5× | 1.2× | 1.2× | 1.0× | 2.8× | 2.9× |
| **fastio (this template)** | 7.4× | 4.2× | 6.1× | 2.3× | 10.6× | 4.0× | 3.0× | 11.4× | 8.3× | 6.0× | 4.2× |
| `mmap` + raw pointer (Linux only, reference) | 5.2× | 3.6× | 4.6× | 1.7× | 7.6× | 1.6× | 1.4× | 4.9× | 3.5× | 2.4× | 2.8× |

## Output tests with output sent to /dev/null (pure formatting cost, no disk)

| Method | write 10^7 ints | write 5·10^6 long longs | write 5·10^6 doubles (fixed, 6) | write 5·10^6 doubles (setprecision(10), no fixed) | read + write 10^7 ints | read + write 10^7 mixed ints |
|---|---|---|---|---|---|---|
| `cin`/`cout` (default, synced) | 453 | 258 | 1,884 | 1,536 | 4,932 | 4,041 |
| `cin.tie(0)->sync_with_stdio(0)` | 400 | 235 | 1,608 | 1,556 | 940 | 872 |
| `scanf` / `printf` | 492 | 336 | 1,336 | 1,253 | 1,309 | 1,115 |
| `getchar` / `putchar` readInt() | 497 | 437 | 1,503 | 1,416 | 819 | 563 |
| `getchar_unlocked` readInt() | 253 | 227 | 1,352 | 1,408 | 362 | 293 |
| `fread` buffer + readInt() (classic snippet) | 234 | 273 | 1,406 | 1,403 | 342 | 294 |
| **fastio (this template)** | 91 | 75 | 121 | 158 | 154 | 211 |
| `mmap` + raw pointer (Linux only, reference) | 246 | 211 | 327 | 392 | 341 | 280 |

## The `endl` trap: write 10^7 ints with `endl` instead of `'\n'`

| Method | with `'\n'` | with `endl` |
|---|---|---|
| `cin`/`cout` (default, synced) | 538 | 3,457 |
| `cin.tie(0)->sync_with_stdio(0)` | 475 | 3,393 |
| **fastio (this template)** | 118 | 3,055 |

`endl` flushes, i.e. makes one `write()` system call per line, so it erases the gain of any fast I/O.

## Input sizes

- ints: 10^7 values uniform in [-10^9, 10^9] — 104 MB
- mixed ints: 10^7 values whose digit count is uniform in 1..10, 20% negative — 67 MB
- long longs: 5·10^6 values uniform in [-10^18, 10^18] — 97 MB
- words: 5·10^6 lowercase words of length 1..20 — 57 MB
- doubles: 5·10^6 values in [-10^6, 10^6] with 6 decimals — 72 MB
- write tests print the same pseudo-random sequence (LCG) in every method

## Notes

- The fastio row runs the *same source code* as the two `cin`/`cout` rows (`body_iostream.inc`); only the template is pasted on top.
- What makes it fast: 64 KB `fread`/`fwrite` buffers; numbers parsed 8 digits at a time inside a 64-bit register (no per-digit branch for 1-7 digit numbers, 16 digits in two steps for long long); ints printed by building 8 ASCII digits in one register; no branch on the minus sign in either direction; word ends found 8 bytes at a time; `getline` via `memchr`; doubles without an exponent parsed directly and converted with one exactly-rounded division; `fixed` and default-format double output done by exact scale-and-round, falling back to the library only near a rounding tie. All output is byte-identical to iostream.
- Tried and rejected (measured slower or no better): a fully branch-free 16-digit int parser, a branch-free int printer, an SSE2 reader that finds all token starts in 64-byte blocks, mmap input (~7% less CPU on reads but Linux-only and file-only), a 256 KB buffer, `-O3`, BMI2 and `-march=native`.
- The `mmap` row skips the kernel-to-buffer copy but parses one digit at a time and pays page faults on its single huge output buffer.
- `getchar_unlocked` is POSIX-only; it does not exist on Windows judges such as Codeforces.
- The classic `fread` snippet reads ints well but has no fast path for doubles (it falls back to strtod/snprintf).
