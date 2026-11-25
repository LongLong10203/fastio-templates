// Shared body for the "classic" function-style readers: getchar, getchar_unlocked, fread.
// The including file defines GC() (get char) and PC(c) (put char) and FLUSH().
static unsigned long long lcg_state = 88172645463325252ull;
inline unsigned long long nxt() { return lcg_state = lcg_state * 6364136223846793005ull + 1442695040888963407ull; }
inline int gen_int() { return int((nxt() >> 33) % 2000000001ull) - 1000000000; }
inline long long gen_ll() { return (long long)(nxt() % 2000000000000000001ull) - 1000000000000000000ll; }
inline double gen_dbl() { return (double(nxt() >> 11) * (1.0 / 9007199254740992.0) - 0.5) * 2e6; }

inline int readInt() {
    int c = GC();
    while (c != '-' && (c < '0' || c > '9')) c = GC();
    bool neg = false;
    if (c == '-') neg = true, c = GC();
    int x = 0;
    while (c >= '0' && c <= '9') x = x * 10 + (c - '0'), c = GC();
    return neg ? -x : x;
}
inline long long readLL() {
    int c = GC();
    while (c != '-' && (c < '0' || c > '9')) c = GC();
    bool neg = false;
    if (c == '-') neg = true, c = GC();
    long long x = 0;
    while (c >= '0' && c <= '9') x = x * 10 + (c - '0'), c = GC();
    return neg ? -x : x;
}
inline void writeLL(long long x) {
    if (x < 0) PC('-'), x = -x;
    char s[24]; int n = 0;
    do s[n++] = char('0' + x % 10); while (x /= 10);
    while (n) PC(s[--n]);
}
inline void readStr(std::string& s) {
    s.clear();
    int c = GC();
    while (c <= ' ') c = GC();
    while (c > ' ') s += char(c), c = GC();
}
inline double readDbl() {  // token + strtod (exact)
    char t[64]; int n = 0, c = GC();
    while (c <= ' ') c = GC();
    while (c > ' ' && n < 63) t[n++] = char(c), c = GC();
    t[n] = 0;
    return strtod(t, nullptr);
}
inline void writeDbl6(double x) {
    char t[64]; int n = snprintf(t, sizeof t, "%.6f", x);
    for (int i = 0; i < n; ++i) PC(t[i]);
}

int run(const std::string& test) {
    if (test == "read_int") {
        int n = readInt(); long long s = 0;
        for (int i = 0; i < n; ++i) s += readInt();
        writeLL(s), PC('\n');
    } else if (test == "read_ll") {
        int n = readInt(); unsigned long long s = 0;
        for (int i = 0; i < n; ++i) s += readLL();
        char t[32]; int k = snprintf(t, sizeof t, "%llu\n", s); for (int i = 0; i < k; ++i) PC(t[i]);
    } else if (test == "write_ll") {
        int n = readInt();
        for (int i = 0; i < n; ++i) writeLL(gen_ll()), PC('\n');
    } else if (test == "write_int") {
        int n = readInt();
        for (int i = 0; i < n; ++i) writeLL(gen_int()), PC('\n');
    } else if (test == "echo_int") {
        int n = readInt();
        for (int i = 0; i < n; ++i) writeLL(readInt()), PC('\n');
    } else if (test == "read_str") {
        int n = readInt(); std::string s;
        unsigned long long tot = 0, h = 0;
        for (int i = 0; i < n; ++i) { readStr(s); tot += s.size(); h = h * 131 + s[0] * 7 + s.back(); }
        writeLL((long long)tot), PC(' ');
        char t[32]; int k = snprintf(t, sizeof t, "%llu\n", h); for (int i = 0; i < k; ++i) PC(t[i]);
    } else if (test == "read_dbl") {
        int n = readInt(); double s = 0;
        for (int i = 0; i < n; ++i) s += readDbl();
        writeDbl6(s), PC('\n');
    } else if (test == "write_dbl") {
        int n = readInt();
        for (int i = 0; i < n; ++i) writeDbl6(gen_dbl()), PC('\n');
    } else if (test == "write_dblg") {
        int n = readInt();
        for (int i = 0; i < n; ++i) {
            char t[64]; int k = snprintf(t, sizeof t, "%.10g\n", gen_dbl());
            for (int j = 0; j < k; ++j) PC(t[j]);
        }
    } else return 2;
    FLUSH();
    return 0;
}
