// 8) reference "upper bound": mmap the whole input, parse with a raw pointer,
//    build the whole output in one buffer and write() it once.  Linux only, stdin must be a file.
#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static unsigned long long lcg_state = 88172645463325252ull;
inline unsigned long long nxt() { return lcg_state = lcg_state * 6364136223846793005ull + 1442695040888963407ull; }
inline int gen_int() { return int((nxt() >> 33) % 2000000001ull) - 1000000000; }
inline long long gen_ll() { return (long long)(nxt() % 2000000000000000001ull) - 1000000000000000000ll; }
inline double gen_dbl() { return (double(nxt() >> 11) * (1.0 / 9007199254740992.0) - 0.5) * 2e6; }

static const char *ip, *iend;
static char *ob, *op;

inline void skip() { while (ip < iend && (unsigned char)*ip <= ' ') ++ip; }
inline int readInt() {
    skip();
    bool neg = *ip == '-'; ip += neg;
    unsigned x = 0;
    while (ip < iend && (unsigned)(*ip - '0') < 10u) x = x * 10 + unsigned(*ip++ - '0');
    return neg ? -int(x) : int(x);
}
inline long long readLL() {
    skip();
    bool neg = *ip == '-'; ip += neg;
    unsigned long long x = 0;
    while (ip < iend && (unsigned)(*ip - '0') < 10u) x = x * 10 + unsigned(*ip++ - '0');
    return neg ? -(long long)x : (long long)x;
}
static char D2[200];
inline void writeLL(long long v) {
    unsigned long long x = v;
    if (v < 0) *op++ = '-', x = 0 - x;
    int n = 1;
    for (unsigned long long y = x; y >= 10; y /= 10) ++n;
    char* p = op + n; op = p;
    while (x >= 100) { unsigned r = unsigned(x % 100); x /= 100; p -= 2; memcpy(p, D2 + 2 * r, 2); }
    if (x >= 10) p -= 2, memcpy(p, D2 + 2 * x, 2); else *--p = char('0' + x);
}
inline double readDbl() {
    skip(); double x; auto r = std::from_chars(ip, iend, x); ip = r.ptr; return x;
}
inline void writeDbl6(double x) { op = std::to_chars(op, op + 64, x, std::chars_format::fixed, 6).ptr; }

int main(int, char** argv) {
    for (int i = 0; i < 100; ++i) D2[2 * i] = char('0' + i / 10), D2[2 * i + 1] = char('0' + i % 10);
    struct stat st; fstat(0, &st);
    size_t sz = st.st_size;
    ip = (const char*)mmap(nullptr, sz, PROT_READ, MAP_PRIVATE | MAP_POPULATE, 0, 0);
    iend = ip + sz;
    std::string test = argv[1];
    int n = readInt();
    ob = op = (char*)malloc(size_t(n) * 24 + 64);
    if (test == "read_int") {
        long long s = 0; for (int i = 0; i < n; ++i) s += readInt();
        writeLL(s), *op++ = '\n';
    } else if (test == "read_ll") {
        unsigned long long s = 0; for (int i = 0; i < n; ++i) s += readLL();
        op += sprintf(op, "%llu\n", s);
    } else if (test == "write_ll") {
        for (int i = 0; i < n; ++i) writeLL(gen_ll()), *op++ = '\n';
    } else if (test == "write_int") {
        for (int i = 0; i < n; ++i) writeLL(gen_int()), *op++ = '\n';
    } else if (test == "echo_int") {
        for (int i = 0; i < n; ++i) writeLL(readInt()), *op++ = '\n';
    } else if (test == "read_str") {
        std::string s; unsigned long long tot = 0, h = 0;
        for (int i = 0; i < n; ++i) {
            skip(); const char* b = ip; while (ip < iend && (unsigned char)*ip > ' ') ++ip;
            s.assign(b, ip - b); tot += s.size(); h = h * 131 + s[0] * 7 + s.back();
        }
        op += sprintf(op, "%llu %llu\n", tot, h);
    } else if (test == "read_dbl") {
        double s = 0; for (int i = 0; i < n; ++i) s += readDbl();
        writeDbl6(s), *op++ = '\n';
    } else if (test == "write_dbl") {
        for (int i = 0; i < n; ++i) writeDbl6(gen_dbl()), *op++ = '\n';
    } else if (test == "write_dblg") {
        for (int i = 0; i < n; ++i) op = std::to_chars(op, op + 64, gen_dbl(), std::chars_format::general, 10).ptr, *op++ = '\n';
    } else return 2;
    for (char* p = ob; p < op;) { ssize_t w = write(1, p, op - p); if (w <= 0) return 1; p += w; }
}
