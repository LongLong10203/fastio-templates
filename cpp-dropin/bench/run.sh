#!/bin/sh
# Build everything, generate ~400 MB of input in /tmp/fio, run the benchmark, write results.md
set -e
cd "$(dirname "$0")"
HERE=$(pwd)
mkdir -p bin /tmp/fio
sed -n '1,/END FAST IO/p' ../fastio_template.cpp > fastio_macro.hpp
for m in cin_sync cin_nosync fastio getchar getchar_unlocked fread_func scanf_printf mmap_raw gen; do
  g++ -std=gnu++17 -O2 -o bin/$m $m.cpp
done
(cd /tmp/fio && "$HERE/bin/gen")
python3 run_bench.py /tmp/fio
python3 make_report.py > /dev/null
echo "done -> results.md"
