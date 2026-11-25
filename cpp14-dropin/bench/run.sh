#!/bin/sh
# Build everything, generate ~400 MB of input in /tmp/fio, run the benchmark, write results.md
set -e
cd "$(dirname "$0")"
HERE=$(pwd)
mkdir -p bin /tmp/fio
g++ -std=gnu++14 -O2 -o bin/fastio fastio.cpp
for m in cin_sync cin_nosync getchar getchar_unlocked fread_func scanf_printf mmap_raw gen; do
  g++ -std=gnu++17 -O2 -o bin/$m $m.cpp
done
(cd /tmp/fio && "$HERE/bin/gen")
python3 run_bench.py /tmp/fio
python3 make_report.py > /dev/null
echo "done -> results.md"
