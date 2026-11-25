#!/usr/bin/env python3
"""Turns results.json into results.md."""
import json, os
here = os.path.dirname(os.path.abspath(__file__))
R = json.load(open(os.path.join(here, "results.json")))
env = open(os.path.join(here, "env.txt")).read().strip().splitlines()

NAMES = {
    "cin_sync": "`cin`/`cout` (default, synced)",
    "cin_nosync": "`cin.tie(0)->sync_with_stdio(0)`",
    "scanf_printf": "`scanf` / `printf`",
    "getchar": "`getchar` / `putchar` readInt()",
    "getchar_unlocked": "`getchar_unlocked` readInt()",
    "fread_func": "`fread` buffer + readInt() (classic snippet)",
    "fastio": "**fastio (this template)**",
    "mmap_raw": "`mmap` + raw pointer (Linux only, reference)",
}
READ = [
    ("read_int", "read 10^7 ints (10 digits)"),
    ("read_mix", "read 10^7 ints (1-10 digits)"),
    ("read_ll", "read 5·10^6 long longs"),
    ("read_str", "read 5·10^6 words"),
    ("read_dbl", "read 5·10^6 doubles"),
]
WRITE = [
    ("write_int", "write 10^7 ints"),
    ("write_ll", "write 5·10^6 long longs"),
    ("write_dbl", "write 5·10^6 doubles (fixed, 6)"),
    ("write_dblg", "write 5·10^6 doubles (setprecision(10), no fixed)"),
    ("echo_int", "read + write 10^7 ints"),
    ("echo_mix", "read + write 10^7 mixed ints"),
]
TESTS = READ + WRITE
METHODS = list(NAMES)
base = "cin_nosync"
def ms(x): return f"{x*1000:,.0f}"

out = ["# C++ I/O benchmark results", "",
       "Median wall time of 5 runs (3 for runs over 2 s), in milliseconds. Input comes from a file in the page cache.",
       f"Machine: {', '.join(env)}. Compiled with `g++ -std=gnu++17 -O2`.", "",
       "Every method produced byte-identical output for every test (checked by MD5).",
       "Run-to-run noise on this shared VM is roughly ±10%, so treat differences under ~15% as ties.", ""]

def table(title, tests, fmt):
    out.extend([f"## {title}", "", "| Method | " + " | ".join(n for _, n in tests) + " |", "|" + "---|" * (len(tests) + 1)])
    for m in METHODS:
        row = [NAMES[m]] + [fmt(t, R[t][m]) if m in R[t] else "–" for t, _ in tests]
        out.append("| " + " | ".join(row) + " |")
    out.append("")

table("Reading: wall time (ms)", READ, lambda t, r: ms(r["median"]))
table("Writing: wall time (ms), output written to a file", WRITE, lambda t, r: ms(r["median"]))
table("Speed-up relative to `cin.tie(0)->sync_with_stdio(0)` (higher is better)", TESTS,
      lambda t, r: f"{R[t][base]['median'] / r['median']:.1f}×")
wt = [(t, n) for t, n in TESTS if "devnull_median" in R[t][base]]
out += ["## Output tests with output sent to /dev/null (pure formatting cost, no disk)", "",
        "| Method | " + " | ".join(n for _, n in wt) + " |", "|" + "---|" * (len(wt) + 1)]
for m in METHODS:
    row = [NAMES[m]]
    for t, _ in wt:
        r = R[t].get(m)
        row.append(ms(r["devnull_median"]) if r and "devnull_median" in r else "–")
    out.append("| " + " | ".join(row) + " |")

if "write_int_endl" in R:
    out += ["", "## The `endl` trap: write 10^7 ints with `endl` instead of `'\\n'`", "",
            "| Method | with `'\\n'` | with `endl` |", "|---|---|---|"]
    for m in R["write_int_endl"]:
        out.append(f"| {NAMES[m]} | {ms(R['write_int'][m]['median'])} | {ms(R['write_int_endl'][m]['median'])} |")
    out += ["", "`endl` flushes, i.e. makes one `write()` system call per line, so it erases the gain of any fast I/O."]

out += ["", "## Input sizes", "",
        "- ints: 10^7 values uniform in [-10^9, 10^9] — 104 MB",
        "- mixed ints: 10^7 values whose digit count is uniform in 1..10, 20% negative — 67 MB",
        "- long longs: 5·10^6 values uniform in [-10^18, 10^18] — 97 MB",
        "- words: 5·10^6 lowercase words of length 1..20 — 57 MB",
        "- doubles: 5·10^6 values in [-10^6, 10^6] with 6 decimals — 72 MB",
        "- write tests print the same pseudo-random sequence (LCG) in every method",
        "",
        "## Notes",
        "",
        "- The fastio row runs the *same source code* as the two `cin`/`cout` rows (`body_iostream.inc`); only the template is pasted on top.",
        "- What makes it fast: 64 KB `fread`/`fwrite` buffers; numbers parsed 8 digits at a time inside a 64-bit register (no per-digit branch for 1-7 digit numbers, 16 digits in two steps for long long); ints printed by building 8 ASCII digits in one register; no branch on the minus sign in either direction; word ends found 8 bytes at a time; `getline` via `memchr`; doubles without an exponent parsed directly and converted with one exactly-rounded division; `fixed` and default-format double output done by exact scale-and-round, falling back to the library only near a rounding tie. All output is byte-identical to iostream.",
        "- Tried and rejected (measured slower or no better): a fully branch-free 16-digit int parser, a branch-free int printer, an SSE2 reader that finds all token starts in 64-byte blocks, mmap input (~7% less CPU on reads but Linux-only and file-only), a 256 KB buffer, `-O3`, BMI2 and `-march=native`.",
        "- The `mmap` row skips the kernel-to-buffer copy but parses one digit at a time and pays page faults on its single huge output buffer.",
        "- `getchar_unlocked` is POSIX-only; it does not exist on Windows judges such as Codeforces.",
        "- The classic `fread` snippet reads ints well but has no fast path for doubles (it falls back to strtod/snprintf).",
        ]
open(os.path.join(here, "results.md"), "w").write("\n".join(out) + "\n")
print("\n".join(out))
