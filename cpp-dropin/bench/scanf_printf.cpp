// 7) scanf / printf
#include <bits/stdc++.h>
static unsigned long long lcg_state = 88172645463325252ull;
inline unsigned long long nxt() { return lcg_state = lcg_state * 6364136223846793005ull + 1442695040888963407ull; }
inline int gen_int() { return int((nxt() >> 33) % 2000000001ull) - 1000000000; }
inline long long gen_ll() { return (long long)(nxt() % 2000000000000000001ull) - 1000000000000000000ll; }
inline double gen_dbl() { return (double(nxt() >> 11) * (1.0 / 9007199254740992.0) - 0.5) * 2e6; }
int main(int, char** argv) {
    std::string test = argv[1];
    int n;
    if (scanf("%d", &n) != 1) return 1;
    if (test == "read_int") {
        long long s = 0; for (int i = 0; i < n; ++i) { int x; if (scanf("%d", &x) != 1) return 1; s += x; }
        printf("%lld\n", s);
    } else if (test == "read_ll") {
        unsigned long long s = 0; for (int i = 0; i < n; ++i) { long long x; if (scanf("%lld", &x) != 1) return 1; s += x; }
        printf("%llu\n", s);
    } else if (test == "write_ll") {
        for (int i = 0; i < n; ++i) printf("%lld\n", gen_ll());
    } else if (test == "write_int") {
        for (int i = 0; i < n; ++i) printf("%d\n", gen_int());
    } else if (test == "echo_int") {
        for (int i = 0; i < n; ++i) { int x; if (scanf("%d", &x) != 1) return 1; printf("%d\n", x); }
    } else if (test == "read_str") {
        static char buf[1 << 10]; unsigned long long tot = 0, h = 0;
        for (int i = 0; i < n; ++i) { if (scanf("%1000s", buf) != 1) return 1; size_t L = strlen(buf); tot += L; h = h * 131 + buf[0] * 7 + buf[L - 1]; }
        printf("%llu %llu\n", tot, h);
    } else if (test == "read_dbl") {
        double s = 0; for (int i = 0; i < n; ++i) { double x; if (scanf("%lf", &x) != 1) return 1; s += x; }
        printf("%.6f\n", s);
    } else if (test == "write_dbl") {
        for (int i = 0; i < n; ++i) printf("%.6f\n", gen_dbl());
    } else if (test == "write_dblg") {
        for (int i = 0; i < n; ++i) printf("%.10g\n", gen_dbl());
    } else return 2;
}
