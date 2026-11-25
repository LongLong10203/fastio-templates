# cpp-dropin: drop-in fast `cin` / `cout`

A single block you paste at the top of a C++17 solution. Your code keeps using `cin >> x` and `cout << x`, unchanged, but input and output become **2–11× faster than `cin.tie(0)->sync_with_stdio(0)`**. Output is byte-identical to what `cout` would print.

## Usage

Copy everything from the top of [`fastio_template.cpp`](fastio_template.cpp) down to the `END FAST IO` line into your file. Your `#include`s and code can go before or after it.

```cpp
/* ...fast io template... */

#include <bits/stdc++.h>
using namespace std;

int main() {
    cin.tie(0)->sync_with_stdio(0);   // still compiles; it's a no-op now
    int n;
    cin >> n;
    vector<long long> a(n);
    for (auto& x : a) cin >> x;
    cout << fixed << setprecision(9) << accumulate(a.begin(), a.end(), 0.0) / n << '\n';
}
```

The two `#define`s at the end of the template map `cin` / `cout` (and `std::cin` / `std::cout`) to the fast versions. Delete them if you prefer writing `fastio::fin >> x` / `fastio::fout << x` explicitly.

## Benchmark

Median wall time, output written to a file, `g++ -std=gnu++17 -O2`, on a 2 vCPU cloud VM (Intel Xeon 2.1 GHz). The fastio column runs the exact same source code as the `cin` column.

| Test | `sync_with_stdio(0)` | `scanf`/`printf` | classic `fread` snippet | **fastio** | **speed-up** |
|---|---|---|---|---|---|
| Read 10⁷ ints | 566 ms | 680 ms | 188 ms | **76 ms** | **7.4×** |
| Read 5·10⁶ long longs | 435 ms | 520 ms | 166 ms | **71 ms** | **6.1×** |
| Read 5·10⁶ words | 216 ms | 317 ms | 173 ms | **94 ms** | **2.3×** |
| Read 5·10⁶ doubles | 990 ms | 791 ms | 588 ms | **94 ms** | **10.6×** |
| Write 10⁷ ints | 475 ms | 618 ms | 318 ms | **118 ms** | **4.0×** |
| Write 5·10⁶ doubles (`fixed`) | 1,673 ms | 1,428 ms | 1,385 ms | **147 ms** | **11.4×** |
| Write 5·10⁶ doubles (no `fixed`) | 1,551 ms | 1,359 ms | 1,497 ms | **187 ms** | **8.3×** |
| Read + write 10⁷ ints | 1,056 ms | 1,262 ms | 372 ms | **176 ms** | **6.0×** |

Full results for every method (`getchar`, `getchar_unlocked`, `mmap`, synced `cin`, the `endl` trap, mixed-length numbers): [`bench/results.md`](bench/results.md). Run-to-run noise on that VM is about ±10%.

## What it supports

- `cin >>` `int`, `long long`, `unsigned`, `__int128`, `char`, `bool`, `string`, `char[]`, `double`, `float`, `long double`, `ws`
- `while (cin >> x)`, `getline(cin, s)` (also strips a trailing `\r`), `cin.get()`, `cin.peek()`, `cin.ignore()`, `cin.eof()`
- `cout <<` all of the above plus `const char*`, `string`, `string_view`, `endl`, `flush`
- `fixed`, `scientific`, `setprecision`, `setw`, `setfill`, `hex`, `boolalpha`, … and any type with an `ostream& operator<<`

## Don't use it for

- **Interactive problems.** Input is read in 64 KB blocks, so it waits for data the judge never sends. Use plain `cin`/`cout` there.
- **Mixing with `printf` / `scanf` / `puts`.** They use separate buffers, so the output order breaks.
- **`endl` in a loop.** It flushes on every line and erases the speed-up. Use `'\n'`.

## Compatibility

Needs C++17. The test matrix in [`test/run_tests.sh`](test/run_tests.sh) passes on:

- GCC 9, 10, 11 and 13, and Clang 18, with the template pasted before `bits/stdc++.h` and pb_ds
- AddressSanitizer + UBSan and valgrind
- `-pedantic-errors -Werror` (g++ and clang++)
- the fallback paths for big-endian machines, compilers without `<charconv>` (like GCC 7/8), non-GCC/Clang/MSVC compilers, and x87 floating point (32-bit judges)

It's written to also build on GCC 7/8 and MSVC 2017+, but those weren't available for testing.

## How it works

- **Buffers:** 64 KB `fread` / `fwrite` buffers; no locale, no stdio sync.
- **Integers in:** read 8 digits at a time as one 64-bit word with multiply/shift steps. Numbers of 1–7 digits take no per-digit branch, and `long long` takes two 8-digit steps. The minus sign is handled without a branch.
- **Integers out:** 8 ASCII digits are built inside one 64-bit register and stored at once; a lookup table counts the digits.
- **Strings:** word ends are found 8 bytes at a time; `getline` uses `memchr`.
- **Doubles in:** decimals without an exponent are parsed directly and converted with one exactly-rounded division (Clinger's fast path). Everything else goes to `from_chars`.
- **Doubles out:** `fixed` and default (`%g`-style) output multiply by a power of 10, round, and print integers. A value too close to a rounding tie falls back to `to_chars`, so the output always matches `cout`.
- **Formatting fallback:** anything unusual (`setw`, `hex`, user types) goes through a real `ostringstream`.

## Testing and benchmarking yourself

```sh
sh test/run_tests.sh   # every compiler/configuration found on your machine
sh bench/run.sh        # needs ~400 MB free in /tmp; writes bench/results.md
```
