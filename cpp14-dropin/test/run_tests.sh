#!/bin/sh
# Runs the fastio tests under every compiler and configuration found on this machine.
# All test variants are regenerated from ../fastio.hpp on every run.
# Usage (from this folder):  sh run_tests.sh
cd "$(dirname "$0")" || exit 1
HERE=$(pwd)
TPL="$HERE/../fastio.hpp"
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT

sed -n '1,/END FAST IO/p' "$TPL" | grep -v -e '^#define cin ' -e '^#define cout ' > "$T/fastio.hpp"
sed -n '1,/END FAST IO/p' "$TPL" > "$T/fastio_macro.hpp"
sed -e 's/^#if defined(__GNUC__) || defined(__clang__)$/#if 0/' -e 's/^#elif defined(_MSC_VER).*$/#elif 0/' \
    "$T/fastio.hpp" > "$T/fastio_generic.hpp"
sed 's#^\#include ".*fastio.hpp"#\#include "fastio.hpp"#' test_fastio.cpp > "$T/test_fastio.cpp"
sed 's#^\#include ".*fastio.hpp"#\#include "fastio_generic.hpp"#' test_fastio.cpp > "$T/test_generic.cpp"

cat > "$T/macro_body.cpp" <<'EOF'
#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
using namespace std;
using namespace __gnu_pbds;
template <class T> using oset = tree<T, null_type, less<T>, rb_tree_tag, tree_order_statistics_node_update>;
int main() {
    cin.tie(0)->sync_with_stdio(0);
    int t; cin >> t; cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string line; getline(cin, line);
    cout << "[" << line << "]\n";
    oset<long long> s;
    while (t--) { long long a, b; cin >> a >> b; s.insert(a + b); cout << a + b << " \n"[t == 0]; }
    string w; char c; double d; cin >> w >> c >> d;
    cout << w << ' ' << c << ' ' << fixed << setprecision(4) << d << ' ' << s.order_of_key(0) << endl;
    long long y; std::cin >> y; std::cout << y * 2 << std::endl;
}
EOF
cat "$T/fastio_macro.hpp" "$T/macro_body.cpp" > "$T/macro_test.cpp"
printf '3\nhello world  \n1 2\n3 4\n-5 -6\nabc x 2.718281\n21' > "$T/macro_in.txt"
printf '[hello world  ]\n3 7 -11\nabc x 2.7183 1\n42\n' > "$T/macro_want.txt"
cat "$T/fastio_macro.hpp" interactive_solution.cpp > "$T/inter_sol.cpp"
cp interactive_judge.cpp "$T/"

cd "$T" || exit 1
pass=0
fail=0
ok() { echo "PASS  $1"; pass=$((pass + 1)); }
bad() { echo "FAIL  $1"; fail=$((fail + 1)); }
run() {  # run <name> <compile command...>: compile into a.out, run it, expect exit 0
    name=$1; shift
    rm -f a.out
    if ! "$@" -o a.out > log 2>&1; then bad "$name (compile error)"; grep -m3 -E "error" log; return; fi
    if ./a.out > out 2>&1; then ok "$name"; else bad "$name"; tail -n 5 out; fi
}
macro() {  # macro <name> <compiler...>: template pasted before bits/stdc++.h + pb_ds, std::cin/std::cout
    name=$1; shift
    rm -f a.out
    if ! "$@" -o a.out macro_test.cpp > log 2>&1; then bad "$name (compile error)"; grep -m3 -E "error" log; return; fi
    if ./a.out < macro_in.txt 2>/dev/null | cmp -s - macro_want.txt; then ok "$name"; else bad "$name"; fi
}

for cxx in g++ g++-9 g++-10 g++-11 g++-12 g++-13 g++-14 clang++ clang++-17 clang++-18 clang++-19; do
    command -v "$cxx" > /dev/null 2>&1 || continue
    run "$cxx -std=c++14" "$cxx" -std=c++14 -O2 test_fastio.cpp
    macro "$cxx template-first macro mode" "$cxx" -std=gnu++14 -O2
done
run "g++ -std=c++17 (newer standards work too)" g++ -std=c++17 -O2 test_fastio.cpp
run "g++ -std=gnu++20" g++ -std=gnu++20 -O2 test_fastio.cpp
run "g++ AddressSanitizer + UBSan" g++ -std=c++14 -O1 -fsanitize=address,undefined test_fastio.cpp
run "no 8-byte tricks (FASTIO_SWAR=0, big-endian path)" g++ -std=c++14 -O2 -DFASTIO_SWAR=0 test_fastio.cpp
run "generic bit scan (non-GCC/Clang/MSVC path)" g++ -std=c++14 -O2 test_generic.cpp
case $(uname -m) in x86_64 | i?86)
    run "x87 doubles (FLT_EVAL_METHOD=2, like 32-bit judges)" g++ -std=c++14 -O2 -mfpmath=387 test_fastio.cpp ;;
esac
cat > pedantic_main.cpp <<'EOF'
#include <bits/stdc++.h>
using namespace std;
int main() {
    int n = 0;
    cin >> n;
    long long s = 0;
    for (int i = 0; i < n; ++i) { long long x; cin >> x; s += x; }
    cout << s << '\n';
}
EOF
cat fastio_macro.hpp pedantic_main.cpp > pedantic.cpp
for cxx in g++ clang++; do
    command -v "$cxx" > /dev/null 2>&1 || continue
    rm -f a.out
    if "$cxx" -std=c++14 -O2 -Wall -Wextra -pedantic-errors -Werror -o a.out pedantic.cpp > log 2>&1 \
        && [ "$(printf '3\n1 2 3' | ./a.out)" = 6 ]; then ok "$cxx -std=c++14 -pedantic-errors -Werror on the template"
    else bad "$cxx -std=c++14 -pedantic-errors -Werror on the template"; grep -m3 -E "error" log; fi
done
if command -v mkfifo > /dev/null 2>&1 && command -v timeout > /dev/null 2>&1; then
    if g++ -std=c++14 -O2 -o judge interactive_judge.cpp > log 2>&1 && g++ -std=c++14 -O2 -o inter_sol inter_sol.cpp >> log 2>&1; then
        for k in guess split mixed; do
            rm -f q a
            mkfifo q a
            timeout 60 ./judge $k > a < q &
            jp=$!
            timeout 60 ./inter_sol $k < a > q
            wait $jp
            jr=$?
            if [ "$jr" -eq 0 ]; then ok "interactive $k, no explicit flush"; else bad "interactive $k (judge exit $jr)"; fi
        done
    else bad "interactive tests (compile error)"; grep -m3 error log; fi
fi
if command -v valgrind > /dev/null 2>&1; then
    rm -f a.out
    if g++ -std=c++14 -O1 -g -o a.out test_fastio.cpp > log 2>&1 && valgrind -q --error-exitcode=9 ./a.out > out 2>&1; then ok "valgrind"
    else bad "valgrind"; tail -n 5 out; fi
fi

echo "---- $pass passed, $fail failed"
[ "$fail" -eq 0 ]
