// Interactive judge used by run_tests.sh. Talks to the solution through stdin/stdout.
//   guess : 1000 games of "guess the number in [1, 1e9]" (answers < > =), at most 31 queries each
//   split : sends numbers in two pieces with a pause in between; expects each number doubled back
//   mixed : sends "<double> <word>" lines; expects "<double*2 with 6 decimals> <word reversed>"
// Exit code 0 = accepted.
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <thread>

int main(int argc, char** argv) {
    if (argc < 2) return 9;
    std::string mode = argv[1];
    std::mt19937_64 rng(42);
    if (mode == "guess") {
        int T = 1000;
        printf("%d\n", T);
        fflush(stdout);
        for (int g = 0; g < T; ++g) {
            long long secret = 1 + (long long)(rng() % 1000000000ull);
            for (int q = 0;; ++q) {
                char t[4];
                long long x;
                if (scanf("%3s %lld", t, &x) != 2) return 2;
                if (t[0] == '!') { if (x != secret) return 3; break; }
                if (q >= 31) return 4;
                puts(x < secret ? "<" : x > secret ? ">" : "=");
                fflush(stdout);
            }
        }
        return 0;
    }
    if (mode == "split") {
        int T = 300;
        printf("%d\n", T);
        fflush(stdout);
        for (int i = 0; i < T; ++i) {
            long long v = (long long)(rng() % 2000000000001ull) - 1000000000000ll;
            char s[32];
            int n = snprintf(s, sizeof s, "%lld", v), cut = 1 + int(rng() % (n - 1 > 0 ? n - 1 : 1));
            fwrite(s, 1, cut, stdout);
            fflush(stdout);
            std::this_thread::sleep_for(std::chrono::microseconds(300));
            printf("%s\n", s + cut);
            fflush(stdout);
            long long got;
            if (scanf("%lld", &got) != 1) return 2;
            if (got != 2 * v) return 3;
        }
        return 0;
    }
    if (mode == "mixed") {
        int T = 300;
        printf("%d\n", T);
        fflush(stdout);
        for (int i = 0; i < T; ++i) {
            double d = double(int(rng() % 2000001) - 1000000) / 1000;
            std::string w;
            for (int k = 0, L = 1 + int(rng() % 10); k < L; ++k) w += char('a' + rng() % 26);
            printf("%.3f %s\n", d, w.c_str());
            fflush(stdout);
            char want[64], got[64], word[64];
            snprintf(want, sizeof want, "%.6f", 2 * d);
            if (scanf("%63s %63s", got, word) != 2) return 2;
            std::reverse(w.begin(), w.end());
            if (strcmp(got, want) || w != word) return 3;
        }
        return 0;
    }
    return 9;
}
