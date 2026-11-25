#!/usr/bin/env python3
"""Runs every I/O method on every test, checks outputs match, records median wall time."""
import hashlib, json, os, statistics, subprocess, sys, time

BIN = os.path.join(os.path.dirname(os.path.abspath(__file__)), "bin")
DATA = sys.argv[1] if len(sys.argv) > 1 else "/tmp/fio"
OUT = os.path.join(DATA, "out.txt")

METHODS = ["cin_sync", "cin_nosync", "scanf_printf", "getchar", "getchar_unlocked",
           "fread_func", "fastio", "mmap_raw"]
TESTS = [  # (test name, input file, methods that run it)
    ("read_int",       "in_int.txt",   METHODS),
    ("read_mix",       "in_mix.txt",   METHODS),
    ("read_ll",        "in_ll.txt",    METHODS),
    ("write_int",      "in_n_int.txt", METHODS),
    ("write_ll",       "in_n_ll.txt",  METHODS),
    ("echo_int",       "in_int.txt",   METHODS),
    ("echo_mix",       "in_mix.txt",   METHODS),
    ("read_str",       "in_str.txt",   METHODS),
    ("read_dbl",       "in_dbl.txt",   METHODS),
    ("write_dbl",      "in_n_dbl.txt", METHODS),
    ("write_dblg",     "in_n_dbl.txt", METHODS),
    ("write_int_endl", "in_n_int.txt", ["cin_sync", "cin_nosync", "fastio"]),
]
ALIAS = {"read_mix": "read_int", "echo_mix": "echo_int"}  # same program code, different input file

def run_once(method, test, inp, sink=None):
    with open(os.path.join(DATA, inp), "rb") as fi, open(sink or OUT, "wb") as fo:
        t = time.perf_counter()
        r = subprocess.run([os.path.join(BIN, method), ALIAS.get(test, test)], stdin=fi, stdout=fo)
        dt = time.perf_counter() - t
    if r.returncode != 0:
        raise RuntimeError(f"{method} {test} exit {r.returncode}")
    return dt

def digest():
    h = hashlib.md5()
    with open(OUT, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest(), os.path.getsize(OUT)

results = {}
for test, inp, methods in TESTS:
    ref = None
    for m in methods:
        first = run_once(m, test, inp)              # warm-up run (also fills page cache)
        dg = digest()
        if ref is None: ref = dg
        same = dg == ref
        runs = 5 if first < 2 else 3
        times = [run_once(m, test, inp) for _ in range(runs)]
        med = statistics.median(times)
        rec = {"median": med, "min": min(times), "runs": times,
               "output_md5": dg[0], "output_bytes": dg[1], "matches": same}
        if dg[1] > 1000:  # big output: also time with output to /dev/null (pure CPU cost)
            nt = [run_once(m, test, inp, "/dev/null") for _ in range(runs)]
            rec["devnull_median"] = statistics.median(nt)
        results.setdefault(test, {})[m] = rec
        extra = f"  /dev/null {rec['devnull_median']*1000:8.1f} ms" if "devnull_median" in rec else ""
        print(f"{test:15s} {m:17s} median {med*1000:9.1f} ms  min {min(times)*1000:9.1f} ms{extra}  "
              f"out {dg[1]:>11,d} B  {'OK' if same else 'OUTPUT DIFFERS'}", flush=True)

with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "results.json"), "w") as f:
    json.dump(results, f, indent=1)
