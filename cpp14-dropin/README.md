# cpp14-dropin: drop-in fast `cin` / `cout` for C++14

The C++14 version of [`cpp-dropin`](../cpp-dropin/), for judges whose C++ compiler only goes up to C++14 (SPOJ, older judges). Same features and same speed: your code keeps using `cin >> x` and `cout << x`, unchanged, and output is byte-identical to `cout`.

If your judge has C++17 or newer, either version works; they behave the same.

## Usage

Paste the whole of [`fastio.hpp`](fastio.hpp) at the top of your solution. When testing locally you can write `#include "fastio.hpp"` instead. Your own `#include`s and code can go before or after it.

```cpp
/* ...fast io template... */

#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    cin >> n;
    long long sum = 0;
    for (int i = 0; i < n; ++i) { long long x; cin >> x; sum += x; }
    cout << sum << '\n';
}
```

## How it differs from the C++17 version

Only in how the code is written, not in what it does:

- Overloads and `std::enable_if` replace `if constexpr`.
- `std::is_same<...>::value` replaces `std::is_same_v<...>`.
- `fin` / `fout` are `static` globals instead of `inline` variables. The template is meant for single-file solutions, so this makes no difference.
- No `std::string_view` support (it doesn't exist in C++14).
- No `<charconv>`. The template's own fast paths handle typical doubles. For rare cases (exponents, more than 15–16 significant digits) it falls back to `strtod` / `snprintf` instead of `from_chars` / `to_chars`, with the same exact output, just slower for those values.

Speed, CPU time compared with the C++17 version on the benchmark in [`bench/`](bench/): ints, long longs, words, doubles and read+write are all within the ±2–4% noise of the test machine.

## Interactive problems

Supported. Every read that has to wait for the judge flushes `cout` first, so you don't need `endl` or `flush` after a query.

## Don't use it for

- **Mixing with `printf` / `scanf` / `puts`.** They use separate buffers, so the output order breaks.
- **`endl` in a loop.** It flushes on every line and erases the speed-up. Use `'\n'`.

## Compatibility

Strict standard C++14, with no compiler extensions. [`test/run_tests.sh`](test/run_tests.sh) passes on:

- GCC 9, 10, 11 and 13, and Clang 18, all with `-std=c++14`, with the template pasted before `bits/stdc++.h` and pb_ds
- `-std=c++14 -pedantic-errors -Werror` (g++ and clang++)
- `-std=c++17` and `-std=gnu++20`
- AddressSanitizer + UBSan and valgrind
- three interactive judges over named pipes, with no flush calls in the solution
- the fallback paths for big-endian machines, non-GCC/Clang/MSVC compilers, and x87 floating point (32-bit judges)

It's written to also build on older C++14 compilers and MSVC, but those weren't available for testing.

## Testing and benchmarking yourself

```sh
sh test/run_tests.sh   # every compiler/configuration found on your machine
sh bench/run.sh        # needs ~400 MB free in /tmp; writes bench/results.md
```
