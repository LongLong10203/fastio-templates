/* ================================ FAST IO ================================
 * Drop-in replacement for cin / cout that keeps the >> and << syntax.
 * 2-10x faster than  cin.tie(0)->sync_with_stdio(0)  on big inputs.
 *
 *  - Paste this whole file at the top of your solution (judges need a single
 *    file), or  #include "fastio.hpp"  when testing locally. Your #includes and
 *    code can go before or after it.
 *  - The two #defines at the end make `cin` / `cout` (and `std::cin` /
 *    `std::cout`) mean the fast versions, so the rest of your code stays the same.
 *    Delete them if you prefer writing  fastio::fin >> x  /  fastio::fout << x.
 *  - C++14 version (for judges such as SPOJ). Strict standard C++14, no
 *    compiler extensions. Tested on GCC 9-13 and Clang 18; written to also
 *    build on older C++14 compilers and MSVC (not tested there).
 *
 * Supported:
 *   cin  >> int / long long / unsigned / __int128 / char / bool / string / char[]
 *        >> double / float / long double  >> ws ;  while (cin >> x) ;  getline(cin, s)
 *        cin.get()  cin.peek()  cin.ignore()  cin.eof()
 *   cout << all of the above, const char*, string, endl, flush,
 *        fixed / scientific / setprecision / setw / setfill / hex / boolalpha ...
 *        and any type that has an  ostream& operator<<(ostream&, const T&).
 *
 * Interactive problems work too: like std::cin, reading flushes cout first,
 * so you don't even need endl / flush after a query.
 *
 * Do NOT use for:
 *   - mixing with printf / puts / scanf (separate buffers, output order breaks).
 *   - `endl` in a loop: it flushes every line and erases the speed-up; use '\n'.
 * ========================================================================= */
#ifndef FASTIO_CPP14_DROPIN_HPP
#define FASTIO_CPP14_DROPIN_HPP
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#ifdef _MSC_VER
#include <intrin.h>
#endif
#if defined(_WIN32)
#include <io.h>
#define FASTIO_RAW_READ 1
#elif defined(__unix__) || defined(__APPLE__)
#include <cerrno>
#include <unistd.h>
#define FASTIO_RAW_READ 1
#else
#define FASTIO_RAW_READ 0
#endif

#if defined(__GNUC__) || defined(__clang__)
#define FASTIO_COLD __attribute__((noinline, cold))
#elif defined(_MSC_VER)
#define FASTIO_COLD __declspec(noinline)
#else
#define FASTIO_COLD
#endif
#ifndef FASTIO_SWAR
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define FASTIO_SWAR 0
#else
#define FASTIO_SWAR 1
#endif
#endif

namespace fastio {

template <class T> struct is_int : std::is_integral<T> {};
template <class T> struct to_uns : std::make_unsigned<T> {};
#ifdef __SIZEOF_INT128__
__extension__ typedef __int128 i128_t;
__extension__ typedef unsigned __int128 u128_t;
template <> struct is_int<i128_t> : std::true_type {};
template <> struct is_int<u128_t> : std::true_type {};
template <> struct to_uns<i128_t> { using type = u128_t; };
template <> struct to_uns<u128_t> { using type = u128_t; };
#endif
template <class T> struct is_chr : std::integral_constant<bool, std::is_same<T, char>::value ||
    std::is_same<T, signed char>::value || std::is_same<T, unsigned char>::value> {};
template <class T> struct is_num : std::integral_constant<bool,
    is_int<T>::value && !is_chr<T>::value && !std::is_same<T, bool>::value> {};
template <class T> struct is_text : std::integral_constant<bool,
    std::is_convertible<const T&, const char*>::value || std::is_same<T, std::string>::value> {};
template <class T> struct is_other : std::integral_constant<bool, !is_num<T>::value && !is_chr<T>::value &&
    !std::is_same<T, bool>::value && !std::is_floating_point<T>::value && !is_text<T>::value> {};

struct Digits2 {
    char d[200];
    constexpr Digits2() : d() {
        for (int i = 0; i < 100; ++i) d[2 * i] = char('0' + i / 10), d[2 * i + 1] = char('0' + i % 10);
    }
};
constexpr Digits2 D2{};

struct DigitCount {
    unsigned long long t[32];
    constexpr DigitCount() : t() {
        for (int i = 0; i < 32; ++i) {
            unsigned long long d = 1, p = 10;
            while (p <= (1ull << i)) p *= 10, ++d;
            t[i] = (d << 32) + (p < (1ull << 32) ? (1ull << 32) - p : 0);
        }
    }
};
constexpr DigitCount DC{};
constexpr unsigned long long P10[20] = {1ull, 10ull, 100ull, 1000ull, 10000ull, 100000ull, 1000000ull,
    10000000ull, 100000000ull, 1000000000ull, 10000000000ull, 100000000000ull, 1000000000000ull,
    10000000000000ull, 100000000000000ull, 1000000000000000ull, 10000000000000000ull,
    100000000000000000ull, 1000000000000000000ull, 10000000000000000000ull};
constexpr double P10D[23] = {1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11,
    1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};

inline int ctz64(unsigned long long m) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(m);
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
    unsigned long i;
    _BitScanForward64(&i, m);
    return int(i);
#else
    int n = 0;
    while (!(m & 1)) m >>= 1, ++n;
    return n;
#endif
}
inline int ilog2(unsigned x) {
#if defined(__GNUC__) || defined(__clang__)
    return 31 - __builtin_clz(x);
#elif defined(_MSC_VER)
    unsigned long i;
    _BitScanReverse(&i, x);
    return int(i);
#else
    int n = 0;
    while (x >>= 1) ++n;
    return n;
#endif
}
inline int ndigits(unsigned x) { return int((x + DC.t[ilog2(x | 1)]) >> 32); }
inline unsigned long long nondigits(unsigned long long c) {
    return ((c + 0x7676767676767676ull) | c) & 0x8080808080808080ull;
}
inline unsigned long long swar8(unsigned long long c) {
    c = (c * 10 + (c >> 8)) & 0x00FF00FF00FF00FFull;
    c = (c * 100 + (c >> 16)) & 0x0000FFFF0000FFFFull;
    return (c * 10000 + (c >> 32)) & 0xFFFFFFFFull;
}

static void flush_tied();

class Reader {
    static constexpr int SZ = 1 << 16;
    static constexpr char SENT = 'X';
    char buf[SZ + 16];
    char *cur = buf, *end = buf;
    bool eof_ = false, good_ = true;
    FILE* in;

    long raw_read(char* p, size_t n) {
#if FASTIO_RAW_READ && defined(_WIN32)
        int fd = _fileno(in);
        if (fd >= 0) return _read(fd, p, unsigned(n));
#elif FASTIO_RAW_READ
        int fd = fileno(in);
        if (fd >= 0) {
            long r;
            do r = long(::read(fd, p, n));
            while (r < 0 && errno == EINTR);
            return r;
        }
#endif
        return long(std::fread(p, 1, n, in));
    }
    FASTIO_COLD void refill() {
        size_t rem = size_t(end - cur);
        if (rem) std::memmove(buf, cur, rem);
        cur = buf, end = buf + rem;
        if (!eof_ && rem < size_t(SZ)) {
            flush_tied();
            long got = raw_read(end, SZ - rem);
            if (got <= 0) eof_ = true;
            else end += got;
        }
        *end = SENT;
    }
    bool terminated() const {
        for (const char* p = cur; p < end; ++p)
            if ((unsigned char)*p <= ' ') return true;
        return false;
    }
    FASTIO_COLD bool prepare() {
        for (;;) {
            while (cur < end && (unsigned char)*cur <= ' ') ++cur;
            if (end - cur >= 64 || eof_ || (cur < end && terminated())) return cur < end;
            refill();
        }
    }
    FASTIO_COLD void complete_token() {
        while (!eof_ && end - cur < SZ && !terminated()) refill();
    }
    bool skip_ws() {
        char* p = cur;
        while ((unsigned char)*p <= ' ') ++p;
        cur = p;
        return end - p >= 64 || prepare();
    }
    template <class T> void read_int(T& x) {
        if (!skip_ws()) { good_ = false, x = 0; return; }
        const char* p = cur;
        bool neg = *p == '-';
        p += neg | (*p == '+');
        using U = typename to_uns<T>::type;
        U v = 0;
        const char* st = p;
#if FASTIO_SWAR
        if (sizeof(U) >= 4 && sizeof(U) <= 8) {
            unsigned long long c;
            std::memcpy(&c, p, 8);
            c ^= 0x3030303030303030ull;
            if (unsigned long long m = nondigits(c)) {
                int len = ctz64(m) >> 3;
                if (len) v = U(swar8(c << (64 - 8 * len))), p += len;
            } else {
                v = U(swar8(c)), p += 8;
                if (sizeof(U) == 8) {
                    std::memcpy(&c, p, 8);
                    c ^= 0x3030303030303030ull;
                    if (!nondigits(c)) v = U(v * 100000000u + swar8(c)), p += 8;
                }
            }
        }
#endif
        while ((unsigned)(*p - '0') < 10u) v = U(v * 10 + U(*p++ - '0'));
        if (p == st) { good_ = false, x = 0; return; }
        cur = const_cast<char*>(p);
        x = T(neg ? U(U(0) - v) : v);
    }
    static int eat_digits(const char*& r, unsigned long long& w) {
        const char* s0 = r;
#if FASTIO_SWAR
        for (;;) {
            unsigned long long c;
            std::memcpy(&c, r, 8);
            c ^= 0x3030303030303030ull;
            unsigned long long m = nondigits(c);
            int len = m ? ctz64(m) >> 3 : 8;
            if (!len) break;
            w = w * P10[len] + swar8(c << ((64 - 8 * len) & 63));
            r += len;
            if (len < 8) break;
        }
#else
        while ((unsigned)(*r - '0') < 10u) w = w * 10 + unsigned(*r++ - '0');
#endif
        return int(r - s0);
    }
    static float strto(const char* p, char** q, float) { return std::strtof(p, q); }
    static double strto(const char* p, char** q, double) { return std::strtod(p, q); }
    static long double strto(const char* p, char** q, long double) { return std::strtold(p, q); }
    template <class T> bool fast_float(char*, T&) { return false; }
    bool fast_float(char* p, double& x) {
#if FLT_EVAL_METHOD == 0
        const char* q = p;
        bool neg = *q == '-';
        q += neg;
        unsigned long long w = 0;
        int nd = eat_digits(q, w), e = 0;
        if (*q == '.') ++q, e = -eat_digits(q, w), nd -= e;
        if (nd && nd <= 19 && w <= (1ull << 53) && e >= -22 && (*q | 32) != 'e') {
            double d = e ? double(w) / P10D[-e] : double(w);
            x = neg ? -d : d;
            cur = const_cast<char*>(q);
            return true;
        }
#else
        (void)p, (void)x;
#endif
        return false;
    }
    template <class T> void read_float(T& x) {
        if (!skip_ws()) { good_ = false, x = 0; return; }
        if (end - cur < 512 && !eof_) complete_token();
        char* p = cur;
        if (*p == '+') ++p;
        if (fast_float(p, x)) return;
        char* e = p;
        while (e < end && (unsigned char)*e > ' ') ++e;
        char c = *e;
        *e = 0;
        char* q;
        T v = strto(p, &q, T());
        *e = c;
        if (q == p) { good_ = false, x = 0; return; }
        x = v, cur = q;
    }
    template <class T> void read_chr(T& x) {
        if (skip_ws()) x = T(*cur++);
        else good_ = false, x = 0;
    }
    char* token_end(char* p) const {
#if FASTIO_SWAR
        while (end - p >= 8) {
            unsigned long long c;
            std::memcpy(&c, p, 8);
            if (unsigned long long m = (c - 0x2121212121212121ull) & ~c & 0x8080808080808080ull)
                return p + (ctz64(m) >> 3);
            p += 8;
        }
#endif
        while (p < end && (unsigned char)*p > ' ') ++p;
        return p;
    }
    void read_str(std::string& s) {
        s.clear();
        if (!skip_ws()) { good_ = false; return; }
        for (;;) {
            char* st = cur;
            cur = token_end(cur);
            s.append(st, cur);
            if (cur < end || eof_) return;
            refill();
            if (cur == end) return;
        }
    }
    void read_cstr(char* s) {
        if (!skip_ws()) { good_ = false, *s = 0; return; }
        int c;
        while ((c = peek()) != EOF && c > ' ') *s++ = char(c), ++cur;
        *s = 0;
    }

public:
    explicit Reader(FILE* f = stdin) : buf(), in(f) { buf[0] = SENT; }
    Reader(const Reader&) = delete;

    template <class T> typename std::enable_if<is_num<T>::value, Reader&>::type operator>>(T& x) {
        read_int(x);
        return *this;
    }
    Reader& operator>>(float& x) { read_float(x); return *this; }
    Reader& operator>>(double& x) { read_float(x); return *this; }
    Reader& operator>>(long double& x) { read_float(x); return *this; }
    Reader& operator>>(char& x) { read_chr(x); return *this; }
    Reader& operator>>(signed char& x) { read_chr(x); return *this; }
    Reader& operator>>(unsigned char& x) { read_chr(x); return *this; }
    Reader& operator>>(bool& x) { int v = 0; read_int(v); x = v != 0; return *this; }
    Reader& operator>>(std::string& x) { read_str(x); return *this; }
    Reader& operator>>(char* s) { read_cstr(s); return *this; }
    Reader& operator>>(std::istream& (*)(std::istream&)) { skip_ws(); return *this; }

    explicit operator bool() const { return good_; }
    bool operator!() const { return !good_; }
    bool fail() const { return !good_; }
    bool good() const { return good_; }
    bool eof() { return peek() == EOF; }
    void clear() { good_ = true; }

    int peek() {
        if (cur == end) { if (eof_) return EOF; refill(); if (cur == end) return EOF; }
        return (unsigned char)*cur;
    }
    int get() { int c = peek(); if (c == EOF) good_ = false; else ++cur; return c; }
    Reader& get(char& c) { int r = get(); if (r != EOF) c = char(r); return *this; }
    Reader& ignore(long long n = 1, int delim = EOF) {
        while (n-- > 0) { int c = get(); if (c == EOF || c == delim) break; }
        return *this;
    }
    friend Reader& getline(Reader& r, std::string& s, char delim = '\n') {
        s.clear();
        if (r.peek() == EOF) { r.good_ = false; return r; }
        for (;;) {
            char* st = r.cur;
            char* q = (char*)std::memchr(st, delim, size_t(r.end - st));
            r.cur = q ? q : r.end;
            s.append(st, r.cur);
            if (q) { ++r.cur; break; }
            if (r.eof_) break;
            r.refill();
            if (r.cur == r.end) break;
        }
        if (delim == '\n' && !s.empty() && s.back() == '\r') s.pop_back();
        return r;
    }
    Reader* tie(const void*) { return this; }
    static bool sync_with_stdio(bool = true) { return false; }
};

class Writer {
    static constexpr int SZ = 1 << 16;
    char buf[SZ];
    char* cur = buf;
    FILE* out;
    std::ostringstream fmt;
    bool fancy = false;

    FASTIO_COLD void flush_buf() { if (cur != buf) std::fwrite(buf, 1, size_t(cur - buf), out); cur = buf; }
    size_t room() const { return size_t(buf + SZ - cur); }
    void refresh() {
        using I = std::ios_base;
        fancy = fmt.width() != 0 || (fmt.flags() & (I::oct | I::hex | I::showpos | I::showbase | I::boolalpha));
    }
    template <class T> FASTIO_COLD void slow(const T& x) {
        fmt << x;
        if (fmt.tellp() > 0) {
            std::string s = fmt.str();
            write(s.data(), s.size());
            fmt.str(std::string());
        }
        refresh();
    }
#if FASTIO_SWAR
    static unsigned long long itoa8(unsigned x) {
        unsigned long long m = (x / 10000) | ((unsigned long long)(x % 10000) << 32);
        unsigned long long t = ((m * 10486) >> 20) & 0x0000007F0000007Full;
        unsigned long long h = ((m - t * 100) << 16) | t;
        unsigned long long d = ((h * 103) >> 10) & 0x000F000F000F000Full;
        return (d | ((h - d * 10) << 8)) + 0x3030303030303030ull;
    }
    static char* put8(char* p, unsigned x) {
        unsigned long long u = itoa8(x);
        std::memcpy(p, &u, 8);
        return p + 8;
    }
    static char* putw(char* p, unsigned long long v, int w) {
        if (w > 8) p = putw(p, v / 100000000, w - 8), v %= 100000000, w = 8;
        unsigned long long u = itoa8(unsigned(v)) >> (64 - 8 * w);
        std::memcpy(p, &u, 8);
        return p + w;
    }
    static char* u32(char* p, unsigned x) {
        if (x < 100000000) {
            int n = ndigits(x);
            unsigned long long u = itoa8(x) >> (64 - 8 * n);
            std::memcpy(p, &u, 8);
            return p + n;
        }
        unsigned h = x / 100000000;
        std::memcpy(p, D2.d + 2 * h + (h < 10), 2);
        return put8(p + 1 + (h >= 10), x % 100000000);
    }
#else
    static char* put8(char* p, unsigned x) {
        for (int i = 6; i >= 0; i -= 2, x /= 100) std::memcpy(p + i, D2.d + 2 * (x % 100), 2);
        return p + 8;
    }
    static char* putw(char* p, unsigned long long v, int w) {
        for (int i = w - 1; i >= 0; --i) p[i] = char('0' + v % 10), v /= 10;
        return p + w;
    }
    static char* u32(char* p, unsigned x) {
        int n = ndigits(x);
        char* q = p + n;
        for (; x >= 100; x /= 100) q -= 2, std::memcpy(q, D2.d + 2 * (x % 100), 2);
        if (x >= 10) std::memcpy(q - 2, D2.d + 2 * x, 2);
        else q[-1] = char('0' + x);
        return p + n;
    }
#endif
    static char* u64(char* p, unsigned long long x) {
        if (x < 4294967296ull) return u32(p, unsigned(x));
        if (x < 10000000000000000ull) return put8(u32(p, unsigned(x / 100000000)), unsigned(x % 100000000));
        unsigned long long lo = x % 10000000000000000ull;
        p = u32(p, unsigned(x / 10000000000000000ull));
        return put8(put8(p, unsigned(lo / 100000000)), unsigned(lo % 100000000));
    }
#ifdef __SIZEOF_INT128__
    static char* u128(char* p, u128_t x) {
        if ((x >> 64) == 0) return u64(p, (unsigned long long)x);
        unsigned long long lo = (unsigned long long)(x % 10000000000000000ull);
        p = u128(p, x / 10000000000000000ull);
        return put8(put8(p, unsigned(lo / 100000000)), unsigned(lo % 100000000));
    }
#endif
    template <class T> void write_int(T x) {
        if (room() < 48) flush_buf();
        using U = typename to_uns<T>::type;
        U v = U(x);
        if (T(-1) < T(0)) {
            bool neg = is_neg(x, std::integral_constant<bool, (T(-1) < T(0))>());
            *cur = '-';
            cur += neg;
            v = neg ? U(U(0) - v) : v;
        }
        cur = uw(cur, v);
    }
    template <class T> static bool is_neg(T x, std::true_type) { return x < 0; }
    template <class T> static bool is_neg(T, std::false_type) { return false; }
    static char* uw(char* p, unsigned short x) { return u32(p, x); }
    static char* uw(char* p, unsigned x) { return u32(p, x); }
    static char* uw(char* p, unsigned long x) { return u64(p, x); }
    static char* uw(char* p, unsigned long long x) { return u64(p, x); }
#ifdef __SIZEOF_INT128__
    static char* uw(char* p, u128_t x) { return u128(p, x); }
#endif
    static int cfmt(char* p, size_t n, std::ios_base::fmtflags ff, int prec, double x) {
        using I = std::ios_base;
        const char* f = ff == I::fixed ? "%.*f" : ff == I::scientific ? "%.*e" : "%.*g";
        return std::snprintf(p, n, f, prec, x);
    }
    static int cfmt(char* p, size_t n, std::ios_base::fmtflags ff, int prec, long double x) {
        using I = std::ios_base;
        const char* f = ff == I::fixed ? "%.*Lf" : ff == I::scientific ? "%.*Le" : "%.*Lg";
        return std::snprintf(p, n, f, prec, x);
    }
    template <class T> void write_float(T x) {
        using I = std::ios_base;
        auto fl = fmt.flags(), ff = fl & I::floatfield;
        if (fancy || (fl & (I::showpoint | I::uppercase)) || ff == (I::fixed | I::scientific)) { slow(x); return; }
        int prec = fmt.precision() < 0 ? 6 : int(fmt.precision());
#if FLT_EVAL_METHOD == 0
        if (!std::is_same<T, long double>::value) {
            if (ff == I::fixed && prec <= 16) {
                double ax = double(x);
                bool neg = std::signbit(ax);
                double y = (neg ? -ax : ax) * P10D[prec];
                if (y < 4503599627370496.0) {
                    double fl = double((unsigned long long)y), fr = y - fl;
                    if (std::fabs(fr - 0.5) > y * 4e-16) {
                        if (room() < 64) flush_buf();
                        unsigned long long n = (unsigned long long)fl + (fr > 0.5);
                        *cur = '-';
                        cur += neg;
                        if (!prec) { cur = u64(cur, n); return; }
                        cur = u64(cur, n / P10[prec]);
                        *cur++ = '.';
                        cur = putw(cur, n % P10[prec], prec);
                        return;
                    }
                }
            } else if (ff == I::fmtflags(0) && prec <= 15) {
                int P = prec ? prec : 1;
                double ax = double(x);
                bool neg = std::signbit(ax);
                ax = neg ? -ax : ax;
                if (ax == 0) {
                    if (room() < 8) flush_buf();
                    *cur = '-';
                    cur += neg;
                    *cur++ = '0';
                    return;
                }
                if (ax > 1e-280 && ax < 1e280) {
                    unsigned long long bits;
                    std::memcpy(&bits, &ax, 8);
                    int e = ((int(bits >> 52) - 1023) * 1233) >> 12;
                    for (int it = 0; it < 3; ++it) {
                        int k = P - 1 - e;
                        if (k > 22 || k < -22) break;
                        double y = k >= 0 ? ax * P10D[k] : ax / P10D[-k];
                        if (y >= P10D[P]) { ++e; continue; }
                        if (y < P10D[P - 1] - 1) { --e; continue; }
                        double fl = double((unsigned long long)y), fr = y - fl;
                        if (!(std::fabs(fr - 0.5) > y * 4e-16)) break;
                        unsigned long long n = (unsigned long long)fl + (fr > 0.5);
                        if (n >= P10[P]) { ++e; continue; }
                        if (n < P10[P - 1]) { --e; continue; }
                        if (room() < 64) flush_buf();
                        *cur = '-';
                        cur += neg;
                        if (e >= -4 && e < P) {
                            int q = P - 1 - e;
                            unsigned long long ip = n / P10[q], fp = n % P10[q];
                            while (q && fp % 10 == 0) fp /= 10, --q;
                            cur = u64(cur, ip);
                            if (q) *cur++ = '.', cur = putw(cur, fp, q);
                        } else {
                            int q = P - 1;
                            unsigned long long r = n % P10[q];
                            *cur++ = char('0' + n / P10[q]);
                            while (q && r % 10 == 0) r /= 10, --q;
                            if (q) *cur++ = '.', cur = putw(cur, r, q);
                            *cur++ = 'e';
                            *cur++ = e < 0 ? '-' : '+';
                            unsigned ae = unsigned(e < 0 ? -e : e);
                            if (ae < 10) *cur++ = '0';
                            cur = u32(cur, ae);
                        }
                        return;
                    }
                }
            }
        }
#endif
        if (prec <= 60 && (ff != I::fixed || (x < T(1e100) && x > T(-1e100)))) {
            if (room() < 256) flush_buf();
            cur += cfmt(cur, room(), ff, prec, x);
            return;
        }
        slow(x);
    }

public:
    explicit Writer(FILE* f = stdout) : out(f) {}
    Writer(const Writer&) = delete;
    ~Writer() { flush(); }

    template <class T> void int_fancy(const T& x, std::true_type) { slow(x); }
    template <class T> void int_fancy(const T& x, std::false_type) { write_int(x); }
    template <class T> typename std::enable_if<is_num<T>::value, Writer&>::type operator<<(const T& x) {
        if (fancy) int_fancy(x, std::integral_constant<bool, (sizeof(T) <= 8)>());
        else write_int(x);
        return *this;
    }
    Writer& operator<<(bool x) { if (fancy) slow(x); else put(char('0' + x)); return *this; }
    Writer& operator<<(char x) { if (fancy) slow(x); else put(x); return *this; }
    Writer& operator<<(signed char x) { if (fancy) slow(x); else put(char(x)); return *this; }
    Writer& operator<<(unsigned char x) { if (fancy) slow(x); else put(char(x)); return *this; }
    Writer& operator<<(float x) { write_float(x); return *this; }
    Writer& operator<<(double x) { write_float(x); return *this; }
    Writer& operator<<(long double x) { write_float(x); return *this; }
    Writer& operator<<(const char* s) { if (fancy) slow(s); else write(s, std::strlen(s)); return *this; }
    Writer& operator<<(const std::string& s) { if (fancy) slow(s); else write(s.data(), s.size()); return *this; }
    template <class T> typename std::enable_if<is_other<T>::value, Writer&>::type operator<<(const T& x) {
        slow(x);
        return *this;
    }
    Writer& operator<<(std::ostream& (*m)(std::ostream&)) {
        using M = std::ostream& (*)(std::ostream&);
        if (m == static_cast<M>(std::endl)) put('\n'), flush();
        else if (m == static_cast<M>(std::flush)) flush();
        else slow(m);
        return *this;
    }
    Writer& operator<<(std::ios_base& (*m)(std::ios_base&)) { m(fmt); refresh(); return *this; }

    void put(char c) { if (cur == buf + SZ) flush_buf(); *cur++ = c; }
    Writer& write(const char* s, size_t n) {
        if (n > room()) {
            flush_buf();
            if (n >= size_t(SZ)) { std::fwrite(s, 1, n, out); return *this; }
        }
        if (n <= 16) for (size_t i = 0; i < n; ++i) cur[i] = s[i];
        else std::memcpy(cur, s, n);
        cur += n;
        return *this;
    }
    void flush() { flush_buf(); std::fflush(out); }
    std::streamsize precision() const { return fmt.precision(); }
    std::streamsize precision(std::streamsize p) { return fmt.precision(p); }
    std::ios_base::fmtflags setf(std::ios_base::fmtflags f) { auto o = fmt.setf(f); refresh(); return o; }
    std::ios_base::fmtflags setf(std::ios_base::fmtflags f, std::ios_base::fmtflags m) { auto o = fmt.setf(f, m); refresh(); return o; }
    Writer* tie(const void*) { return this; }
    static bool sync_with_stdio(bool = true) { return false; }
};

static Reader fin;
static Writer fout;
static void flush_tied() { fout.flush(); }
}
namespace std { namespace fastio = ::fastio; }

#define cin fastio::fin
#define cout fastio::fout
#endif
/* ============================== END FAST IO ============================== */
