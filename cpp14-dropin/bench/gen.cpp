// Generates the benchmark inputs.
#include <cstdio>
#include <random>
int main() {
    std::mt19937_64 rng(2026);
    const int NI = 10000000, NS = 5000000, ND = 5000000;

    FILE* f = fopen("in_int.txt", "w");  // 10^7 ints in [-1e9, 1e9]
    fprintf(f, "%d\n", NI);
    std::uniform_int_distribution<int> di(-1000000000, 1000000000);
    for (int i = 0; i < NI; ++i) fprintf(f, "%d%c", di(rng), i % 10 == 9 ? '\n' : ' ');
    fclose(f);

    f = fopen("in_str.txt", "w");  // 5*10^6 lowercase words, length 1..20
    fprintf(f, "%d\n", NS);
    std::uniform_int_distribution<int> dl(1, 20), dc('a', 'z');
    for (int i = 0; i < NS; ++i) {
        int L = dl(rng);
        for (int j = 0; j < L; ++j) fputc(dc(rng), f);
        fputc(i % 10 == 9 ? '\n' : ' ', f);
    }
    fclose(f);

    f = fopen("in_dbl.txt", "w");  // 5*10^6 doubles in [-1e6, 1e6] with 6 decimals
    fprintf(f, "%d\n", ND);
    std::uniform_real_distribution<double> dd(-1e6, 1e6);
    for (int i = 0; i < ND; ++i) fprintf(f, "%.6f%c", dd(rng), i % 10 == 9 ? '\n' : ' ');
    fclose(f);

    f = fopen("in_mix.txt", "w");  // 10^7 ints with 1..10 digits (uniform length), 20% negative
    std::mt19937_64 rm(11);
    fprintf(f, "%d\n", NI);
    for (int i = 0; i < NI; ++i) {
        int L = 1 + rm() % 10;
        long long lo = 1, hi;
        for (int k = 1; k < L; ++k) lo *= 10;
        hi = lo * 10 - 1;
        if (L == 1) lo = 0;
        if (hi > 2147483647) hi = 2147483647;
        long long v = lo + (long long)(rm() % (unsigned long long)(hi - lo + 1));
        if (rm() % 5 == 0) v = -v;
        fprintf(f, "%lld%c", v, i % 10 == 9 ? '\n' : ' ');
    }
    fclose(f);

    f = fopen("in_ll.txt", "w");  // 5*10^6 long longs in [-1e18, 1e18]
    std::mt19937_64 rl(7);
    std::uniform_int_distribution<long long> dll(-1000000000000000000ll, 1000000000000000000ll);
    fprintf(f, "%d\n", ND);
    for (int i = 0; i < ND; ++i) fprintf(f, "%lld%c", dll(rl), i % 10 == 9 ? '\n' : ' ');
    fclose(f);

    f = fopen("in_n_int.txt", "w"); fprintf(f, "%d\n", NI); fclose(f);  // write tests only read n
    f = fopen("in_n_ll.txt", "w"); fprintf(f, "%d\n", ND); fclose(f);
    f = fopen("in_n_dbl.txt", "w"); fprintf(f, "%d\n", ND); fclose(f);
}
