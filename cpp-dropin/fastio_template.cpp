/* ================================ FAST IO ================================
 * Drop-in replacement for cin / cout that keeps the >> and << syntax.
 * 2-10x faster than  cin.tie(0)->sync_with_stdio(0)  on big inputs.
 *
 *  - Paste this block at the top of your file. Your #includes and code can go
 *    before or after it.
 *  - The two #defines at the end make `cin` / `cout` (and `std::cin` /
 *    `std::cout`) mean the fast versions, so the rest of your code stays the same.
 *    Delete them if you prefer writing  fastio::fin >> x  /  fastio::fout << x.
 *  - Needs C++17. Tested on GCC 9-13 and Clang 18; written to also build on
 *    GCC 7-8 and MSVC 2017+ (not tested there).
 *
 * Supported:
 *   cin  >> int / long long / unsigned / __int128 / char / bool / string / char[]
 *        >> double / float / long double  >> ws ;  while (cin >> x) ;  getline(cin, s)
 *        cin.get()  cin.peek()  cin.ignore()  cin.eof()
 *   cout << all of the above, const char*, string, string_view, endl, flush,
 *        fixed / scientific / setprecision / setw / setfill / hex / boolalpha ...
 *        and any type that has an  ostream& operator<<(ostream&, const T&).
 *
 * Do NOT use for:
 *   - interactive problems (input is read in 64 KB blocks, so it waits for data
 *     the judge never sends)  -> delete this template for those, cin/cout are fine.
 *   - mixing with printf / puts / scanf (separate buffers, output order breaks).
 *   - `endl` in a loop: it flushes every line and erases the speed-up; use '\n'.
 * ========================================================================= */
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#ifdef _MSC_VER
#include <intrin.h>
#endif
#ifndef FASTIO_CHARCONV
#if defined(__has_include)
#if __has_include(<charconv>)
#include <charconv>
#endif
#endif
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
#define FASTIO_CHARCONV 1
#else
#define FASTIO_CHARCONV 0
#endif
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
template <class T> inline constexpr bool is_chr =
    std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>;
template <class T> inline constexpr bool is_num =
    is_int<T>::value && !is_chr<T> && !std::is_same_v<T, bool>;
template <class> inline constexpr bool always_false = false;

struct Digits2 {
    char d[200];
    constexpr Digits2() : d() {
        for (int i = 0; i < 100; ++i) d[2 * i] = char('0' + i / 10), d[2 * i + 1] = char('0' + i % 10);
    }
};
inline constexpr Digits2 D2{};

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
inline constexpr DigitCount DC{};
inline constexpr unsigned long long P10[20] = {1ull, 10ull, 100ull, 1000ull, 10000ull, 100000ull, 1000000ull,
    10000000ull, 100000000ull, 1000000000ull, 10000000000ull, 100000000000ull, 1000000000000ull,
    10000000000000ull, 100000000000000ull, 1000000000000000ull, 10000000000000000ull,
    100000000000000000ull, 1000000000000000000ull, 10000000000000000000ull};
inline constexpr double P10D[23] = {1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11,
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

class Reader {
    static constexpr int SZ = 1 << 16;
    static constexpr char SENT = 'X';
    char buf[SZ + 16];
    char *cur = buf, *end = buf;
    bool eof_ = false, good_ = true;
    FILE* in;

    FASTIO_COLD void refill() {
        size_t rem = size_t(end - cur);
        if (rem) std::memmove(buf, cur, rem);
        cur = buf, end = buf + rem;
        if (!eof_) {
            size_t want = SZ - rem, got = std::fread(end, 1, want, in);
            if (got < want) eof_ = true;
            end += got;
        }
        *end = SENT;
    }
    FASTIO_COLD bool prepare() {
        for (;;) {
            while (cur < end && (unsigned char)*cur <= ' ') ++cur;
            if (end - cur >= 64 || eof_) return cur < end;
            refill();
        }
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
        if constexpr (sizeof(U) >= 4 && sizeof(U) <= 8) {
            unsigned long long c;
            std::memcpy(&c, p, 8);
            c ^= 0x3030303030303030ull;
            if (unsigned long long m = nondigits(c)) {
                int len = ctz64(m) >> 3;
                if (len) v = U(swar8(c << (64 - 8 * len))), p += len;
            } else {
                v = U(swar8(c)), p += 8;
                if constexpr (sizeof(U) == 8) {
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
    template <class T> void read_float(T& x) {
        if (!skip_ws()) { good_ = false, x = 0; return; }
        if (end - cur < 512 && !eof_) refill();
        char* p = cur;
        if (*p == '+') ++p;
#if FLT_EVAL_METHOD == 0
        if constexpr (std::is_same_v<T, double>) {
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
                return;
            }
        }
#endif
#if FASTIO_CHARCONV
        auto r = std::from_chars(p, end, x);
        if (r.ec == std::errc()) { cur = const_cast<char*>(r.ptr); return; }
#endif
        char* e = p;
        while (e < end && (unsigned char)*e > ' ') ++e;
        char c = *e;
        *e = 0;
        char* q;
        T v;
        if constexpr (std::is_same_v<T, float>) v = std::strtof(p, &q);
        else if constexpr (std::is_same_v<T, double>) v = std::strtod(p, &q);
        else v = std::strtold(p, &q);
        *e = c;
        if (q == p) { good_ = false, x = 0; return; }
        x = v, cur = q;
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

    template <class T> Reader& operator>>(T& x) {
        if constexpr (is_num<T>) read_int(x);
        else if constexpr (std::is_floating_point_v<T>) read_float(x);
        else if constexpr (is_chr<T>) { if (skip_ws()) x = T(*cur++); else good_ = false, x = 0; }
        else if constexpr (std::is_same_v<T, bool>) { int v = 0; read_int(v); x = v != 0; }
        else if constexpr (std::is_same_v<T, std::string>) read_str(x);
        else if constexpr (std::is_array_v<T> && is_chr<std::remove_extent_t<T>>) read_cstr((char*)x);
        else static_assert(always_false<T>, "fastio: unsupported type for >>");
        return *this;
    }
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
        if constexpr (T(-1) < T(0)) {
            bool neg = x < 0;
            *cur = '-';
            cur += neg;
            v = neg ? U(U(0) - v) : v;
        }
        if constexpr (sizeof(U) <= 4) cur = u32(cur, unsigned(v));
        else if constexpr (sizeof(U) <= 8) cur = u64(cur, (unsigned long long)v);
#ifdef __SIZEOF_INT128__
        else cur = u128(cur, v);
#endif
    }
    template <class T> void write_float(T x) {
        using I = std::ios_base;
        auto fl = fmt.flags(), ff = fl & I::floatfield;
        if (fancy || (fl & (I::showpoint | I::uppercase)) || ff == (I::fixed | I::scientific)) { slow(x); return; }
        int prec = fmt.precision() < 0 ? 6 : int(fmt.precision());
#if FLT_EVAL_METHOD == 0
        if constexpr (!std::is_same_v<T, long double>) {
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
#if FASTIO_CHARCONV
        auto cf = ff == I::fixed ? std::chars_format::fixed
                : ff == I::scientific ? std::chars_format::scientific : std::chars_format::general;
        if (room() < 1024) flush_buf();
        auto r = std::to_chars(cur, buf + SZ, x, cf, prec);
        if (r.ec == std::errc()) { cur = r.ptr; return; }
#else
        if (prec <= 60 && (ff != I::fixed || (x < T(1e100) && x > T(-1e100)))) {
            if (room() < 256) flush_buf();
            const char* f = ff == I::fixed ? "%.*Lf" : ff == I::scientific ? "%.*Le" : "%.*Lg";
            cur += std::snprintf(cur, room(), f, prec, (long double)x);
            return;
        }
#endif
        slow(x);
    }

public:
    explicit Writer(FILE* f = stdout) : out(f) {}
    Writer(const Writer&) = delete;
    ~Writer() { flush(); }

    template <class T> Writer& operator<<(const T& x) {
        if constexpr (is_num<T>) {
            if constexpr (sizeof(T) <= 8) if (fancy) { slow(x); return *this; }
            write_int(x);
        }
        else if constexpr (std::is_same_v<T, bool>) { if (fancy) slow(x); else put(char('0' + x)); }
        else if constexpr (is_chr<T>) { if (fancy) slow(x); else put(char(x)); }
        else if constexpr (std::is_floating_point_v<T>) write_float(x);
        else if constexpr (std::is_convertible_v<const T&, std::string_view>) {
            if (fancy) slow(x);
            else { std::string_view s(x); write(s.data(), s.size()); }
        }
        else slow(x);
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

inline Reader fin;
inline Writer fout;
}
namespace std { namespace fastio = ::fastio; }

#define cin fastio::fin
#define cout fastio::fout
/* ============================== END FAST IO ============================== */

#include <bits/stdc++.h>
using namespace std;

int main() {
    int n = 0;
    cin >> n;
    long long sum = 0;
    for (int i = 0; i < n; ++i) { long long x; cin >> x; sum += x; }
    cout << sum << '\n';
}
