// Correctness tests: fastio Reader/Writer vs std::istringstream/ostringstream.
#include "../fastio.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <cmath>
#include <random>
#include <vector>
using namespace std;

static int fails = 0;
#define CHECK(cond, msg) do { if (!(cond)) { ++fails; if (fails < 20) cerr << "FAIL: " << msg << "\n"; } } while (0)

// run fn(Writer&) into a tmpfile and return what was written
template <class F> string capture(F fn) {
    FILE* f = tmpfile();
    { fastio::Writer w(f); fn(w); }
    fflush(f); rewind(f);
    string s; char b[4096]; size_t n;
    while ((n = fread(b, 1, sizeof b, f)) > 0) s.append(b, n);
    fclose(f);
    return s;
}
template <class F> string capture_std(F fn) { ostringstream o; fn(o); return o.str(); }
FILE* from_string(const string& s) {
    FILE* f = tmpfile(); fwrite(s.data(), 1, s.size(), f); rewind(f); return f;
}

#ifdef __SIZEOF_INT128__
string i128_str(__int128 v) {
    if (v == 0) return "0";
    bool neg = v < 0; unsigned __int128 u = neg ? (unsigned __int128)0 - (unsigned __int128)v : v;
    string s; while (u) s += char('0' + u % 10), u /= 10;
    if (neg) s += '-';
    reverse(s.begin(), s.end()); return s;
}
#endif

struct Pt { int a, b; };
ostream& operator<<(ostream& o, const Pt& p) { return o << '(' << p.a << ',' << p.b << ')'; }

int main() {
    mt19937_64 rng(12345);

    // ---------- integers: write + read round trip vs iostream ----------
    vector<long long> ll = {0, 1, -1, 9, 10, 99, 100, -100, LLONG_MAX, LLONG_MIN, INT_MAX, INT_MIN};
    for (int i = 0; i < 200000; ++i) {
        int bits = rng() % 64;
        long long v = (long long)(rng() >> (63 - bits));
        ll.push_back(rng() & 1 ? v : -v);
    }
    for (unsigned long long pw = 1; pw <= 1000000000000000000ull; pw *= 10)
        for (long long d = -2; d <= 2; ++d) { long long v = (long long)pw + d; ll.push_back(v); ll.push_back(-v); }
    for (long long v = -1000; v <= 100000; ++v) ll.push_back(v);
    vector<int> iv; for (auto x : ll) iv.push_back((int)x);
    vector<unsigned long long> uv = {0, ULLONG_MAX, ULLONG_MAX / 10, 10000000000000000000ull, 9999999999999999999ull, 4294967295ull, 4294967296ull, 99999999ull, 100000000ull};
    for (int i = 0; i < 100000; ++i) uv.push_back(rng() >> (rng() % 64));
    vector<short> sv; for (auto x : ll) sv.push_back((short)x);

    auto wr = [&](auto& o) {
        for (auto x : ll) o << x << ' ';
        o << '\n';
        for (auto x : iv) o << x << '\n';
        for (auto x : uv) o << x << ' ';
        for (auto x : sv) o << x << ' ';
        o << (unsigned)4000000000u << ' ' << (unsigned short)65535 << ' ' << true << false << '\n';
    };
    string a = capture([&](fastio::Writer& w) { wr(w); }), b = capture_std([&](ostream& o) { wr(o); });
    CHECK(a == b, "integer writing differs from ostream");

    {
        FILE* f = from_string(b); fastio::Reader r(f); istringstream is(b);
        bool ok = true;
        for (size_t i = 0; i < ll.size(); ++i) { long long x, y; r >> x; is >> y; ok &= x == y && x == ll[i]; }
        for (size_t i = 0; i < iv.size(); ++i) { int x, y; r >> x; is >> y; ok &= x == y; }
        for (size_t i = 0; i < uv.size(); ++i) { unsigned long long x; r >> x; ok &= x == uv[i]; }
        for (size_t i = 0; i < sv.size(); ++i) { short x; r >> x; ok &= x == sv[i]; }
        unsigned u; unsigned short us; int bt;
        r >> u >> us >> bt;
        ok &= u == 4000000000u && us == 65535 && bt == 10;
        CHECK(ok, "integer reading mismatch");
        int z; CHECK(!(r >> z), "read past EOF should fail");
        fclose(f);
    }

    {   // every digit length 1..19, leading zeros, '+', numbers glued to EOF (8-digit fast path edges)
        string s; vector<unsigned long long> want;
        for (int len = 1; len <= 19; ++len)
            for (int rep = 0; rep < 2000; ++rep) {
                string d;
                for (int k = 0; k < len; ++k) d += char('0' + rng() % 10);
                int z = rng() % 4;  // leading zeros
                string tok = string(z, '0') + d;
                if (rng() % 5 == 0) tok = "+" + tok;
                s += tok; s += " \n\t"[rng() % 3];
                want.push_back(stoull(d));
            }
        FILE* f = from_string(s); fastio::Reader r(f); bool ok = true;
        for (auto w : want) { unsigned long long x; r >> x; ok &= x == w; }
        CHECK(ok, "digit-length / leading-zero sweep"); fclose(f);
        for (int len = 1; len <= 12; ++len) {  // single number with no trailing newline
            string d(len, '7'); d[0] = '1';
            FILE* g = from_string("  -" + d); fastio::Reader rr(g); long long x = 0; rr >> x;
            CHECK(x == -stoll(d), "number at EOF len=" << len); fclose(g);
        }
        FILE* g = from_string("12345678x 123456789abc"); fastio::Reader rr(g); int a1 = 0, a2 = 0; string rest;
        rr >> a1 >> rest >> a2 >> rest;
        CHECK(a1 == 12345678 && a2 == 123456789 && rest == "abc", "digits followed by letters"); fclose(g);
    }

    {   // UTF-8 / high bytes inside tokens, tabs, CR, very long line through getline
        string longline(150000, 'a'); longline[77777] = 'b';
        string s = "h\xc3\xa9llo w\xe2\x82\xacrld\tx\r\n" + longline + "\r\nend";
        FILE* f = from_string(s); fastio::Reader r(f);
        string a1, a2, a3, l1, l2, l3;
        r >> a1 >> a2 >> a3; getline(r, l1); getline(r, l2); getline(r, l3);
        CHECK(a1 == "h\xc3\xa9llo" && a2 == "w\xe2\x82\xacrld" && a3 == "x", "high bytes in tokens");
        CHECK(l1.empty() && l2 == longline && l3 == "end", "150KB line via getline");
        fclose(f);
    }
    {   // words of every length 1..40 vs istringstream
        string s;
        for (int i = 0; i < 200000; ++i) { int L = 1 + rng() % 40; for (int k = 0; k < L; ++k) s += char('!' + rng() % 94); s += " \n\t\r"[rng() % 4]; }
        FILE* f = from_string(s); fastio::Reader r(f); istringstream is(s); string x, y; bool ok = true; int cnt = 0;
        while (is >> y) { r >> x; ok &= x == y; ++cnt; }
        CHECK(ok && cnt == 200000 && !(r >> x), "word sweep");
        fclose(f);
    }
    {   // signed ints / long longs of mixed lengths vs istringstream
        string s;
        for (int i = 0; i < 300000; ++i) {
            int L = 1 + rng() % 19; string d; for (int k = 0; k < L; ++k) d += char('0' + rng() % 10);
            if (d.size() == 19 && d[0] == '9') d[0] = '8';
            s += (rng() % 3 == 0 ? "-" : rng() % 7 == 0 ? "+" : "") + d + " ";
        }
        FILE* f = from_string(s); fastio::Reader r(f); istringstream is(s); long long x, y; bool ok = true;
        while (is >> y) { r >> x; ok &= x == y; }
        CHECK(ok, "mixed-length long long");
        fclose(f);
        f = from_string(s); fastio::Reader r2(f); istringstream is2(s); int xi; long long yi; ok = true;
        while (is2 >> yi) { r2 >> xi; ok &= xi == int((unsigned long long)yi); }
        CHECK(ok, "mixed-length int (wrapping like the ll value mod 2^32)");
        fclose(f);
    }

    {   // random interleaving of >> int / >> long long / >> string / >> char / get() / getline vs istringstream
        int bad = 0;
        for (int round = 0; round < 40 && !bad; ++round) {
            string s;
            while (s.size() < 200000) {
                int kind = rng() % 10;
                if (kind < 6) { int L = 1 + rng() % 9; string d; for (int k = 0; k < L; ++k) d += char('0' + rng() % 10); s += (rng() % 4 == 0 ? "-" : "") + d; }
                else if (kind < 8) { int L = 1 + rng() % 12; for (int k = 0; k < L; ++k) s += char('a' + rng() % 26); }
                else if (kind < 9) { s += to_string(rng() % 1000) + "ab"; }
                else s += to_string(rng() % 100000) + "." + to_string(rng() % 1000);
                int ws = 1 + rng() % 3; for (int k = 0; k < ws; ++k) s += " \n\t"[rng() % 3];
            }
            FILE* f = from_string(s); fastio::Reader r(f); istringstream is(s);
            for (int step = 0; step < 100000; ++step) {
                int op = rng() % 12;
                if (op < 5) { int x = 1, y = 2; r >> x; is >> y; if (bool(r) != bool(is) || (is && x != y)) { if (bad++ < 3) cerr << "   int mismatch step " << step << " " << x << " " << y << "\n"; break; } }
                else if (op < 7) { long long x = 1, y = 2; r >> x; is >> y; if (bool(r) != bool(is) || (is && x != y)) { if (bad++ < 3) cerr << "   ll mismatch step " << step << "\n"; break; } }
                else if (op < 9) { string x, y; r >> x; is >> y; if (bool(r) != bool(is) || x != y) { if (bad++ < 3) cerr << "   string mismatch step " << step << " [" << x << "] [" << y << "]\n"; break; } }
                else if (op < 10) { char x = 1, y = 2; r >> x; is >> y; if (bool(r) != bool(is) || (is && x != y)) { if (bad++ < 3) cerr << "   char mismatch step " << step << "\n"; break; } }
                else if (op < 11) { int x = r.get(), y = is.get(); if (x != y) { if (bad++ < 3) cerr << "   get mismatch step " << step << " " << x << " " << y << "\n"; break; } }
                else { string x, y; getline(r, x); getline(is, y); if (bool(r) != bool(is) || x != y) { if (bad++ < 3) cerr << "   getline mismatch step " << step << " [" << x << "] [" << y << "]\n"; break; } }
                if (!is) {
                    if (is.eof()) break;
                    is.clear(), r.clear();
                    string t1, t2; r >> t1; is >> t2;
                    if (t1 != t2) { if (bad++ < 3) cerr << "   resync mismatch step " << step << " [" << t1 << "] [" << t2 << "]\n"; break; }
                }
            }
            fclose(f);
        }
        CHECK(!bad, "random interleaving of read operations");
    }

#ifdef __SIZEOF_INT128__
    {   // __int128 both ways
        vector<__int128> v128;
        __int128 mx = (__int128)(((unsigned __int128)1 << 127) - 1), mn = -mx - 1;
        v128 = {0, 1, -1, mx, mn, (__int128)LLONG_MAX + 1, (__int128)ULLONG_MAX, -(__int128)ULLONG_MAX - 7};
        for (int i = 0; i < 50000; ++i) {
            __int128 x = ((__int128)rng() << 64) | rng(); x >>= rng() % 127; v128.push_back(rng() & 1 ? x : -x);
        }
        string got = capture([&](fastio::Writer& w) { for (auto x : v128) w << x << ' '; });
        string exp; for (auto x : v128) exp += i128_str(x) + ' ';
        CHECK(got == exp, "__int128 writing");
        FILE* f = from_string(exp); fastio::Reader r(f); bool ok = true;
        for (auto x : v128) { __int128 y; r >> y; ok &= x == y; }
        CHECK(ok, "__int128 reading"); fclose(f);
        unsigned __int128 umx = ~(unsigned __int128)0;
        CHECK(capture([&](fastio::Writer& w) { w << umx; }) == "340282366920938463463374607431768211455", "u128 max");
    }
#endif

    // ---------- floating point output: every format/precision vs ostream ----------
    vector<double> dv = {0.0, -0.0, 1.0, -1.5, 0.1, 1e-300, 1e300, 123456789.987654321, 2.5, 0.5, 1.0 / 3,
                         5e-324, 1.7976931348623157e308, 999999.5, 0.000123456, 100.0, 1e21, 1e22,
                         numeric_limits<double>::infinity(), -numeric_limits<double>::infinity(), NAN};
    uniform_real_distribution<double> U(-1e6, 1e6);
    for (int i = 0; i < 30000; ++i) dv.push_back(U(rng));
    for (int i = 0; i < 30000; ++i) { uint64_t bitsv = rng(); double d; memcpy(&d, &bitsv, 8); if (isfinite(d)) dv.push_back(d); }
    for (int mode = 0; mode < 3; ++mode)
        for (int prec : {0, 1, 3, 6, 9, 12, 17, 20}) {
            auto wd = [&](auto& o) {
                if (mode == 1) o << fixed; else if (mode == 2) o << scientific;
                o << setprecision(prec);
                for (auto x : dv) o << x << '\n';
                o << 1.25f << ' ' << 3.141592653589793238L << '\n';
            };
            string x = capture([&](fastio::Writer& w) { wd(w); }), y = capture_std([&](ostream& o) { wd(o); });
            CHECK(x == y, "double write mode=" << mode << " prec=" << prec);
            if (x != y) {
                size_t k = 0; while (k < x.size() && x[k] == y[k]) ++k;
                cerr << "   first diff near: fast=[" << x.substr(k > 20 ? k - 20 : 0, 40) << "] std=[" << y.substr(k > 20 ? k - 20 : 0, 40) << "]\n";
            }
        }
    // default cout formatting (no manipulators)
    {
        auto wd = [&](auto& o) { for (auto x : dv) o << x << ' '; };
        CHECK(capture([&](fastio::Writer& w) { wd(w); }) == capture_std([&](ostream& o) { wd(o); }), "default double fmt");
    }

    {   // fixed output: every precision 0..18, exact ties, values just around ties, floats
        vector<double> tv = {0.125, 0.375, 2.5, 3.5, 0.5, 1.5, -2.5, 0.0625, 1.005, 2.675, 1e-7, 4.35, 0.045,
                             999999.9999995, 0.9999995, 9.5, 99.5, 1e15 + 0.5, 4503599627370495.5, 1e16, 123456789.125,
                             -0.0, -1e-9, 5e-324, 1e-300, 0.1, 0.2, 0.3, 1.0 / 3, 2.0 / 3};
        for (int k = 0; k < 64; ++k) for (int m = 1; m < 12; ++m) tv.push_back(double(k) / double(1 << m) + (k % 3) * 1000);
        for (double v : vector<double>(tv)) { tv.push_back(nextafter(v, 1e300)); tv.push_back(nextafter(v, -1e300)); }
        uniform_real_distribution<double> mag(-12, 16);
        for (int i = 0; i < 20000; ++i) { double v = pow(10.0, mag(rng)) * (rng() & 1 ? 1 : -1); tv.push_back(v); tv.push_back(round(v * 1000) / 1000 + 0.0005); }
        for (int prec = 0; prec <= 18; ++prec) {
            auto w = [&](auto& o) { o << fixed << setprecision(prec); for (double v : tv) o << v << ' ' << float(v) << '\n'; };
            string x = capture([&](fastio::Writer& ww) { w(ww); }), y = capture_std([&](ostream& o) { w(o); });
            CHECK(x == y, "fixed precision " << prec);
            if (x != y) { size_t k = 0; while (k < x.size() && x[k] == y[k]) ++k; cerr << "   near: fast=[" << x.substr(k > 30 ? k - 30 : 0, 60) << "] std=[" << y.substr(k > 30 ? k - 30 : 0, 60) << "]\n"; }
        }
        for (int k = -12; k <= 22; ++k) {
            double v = pow(10.0, k);
            for (double m : {1.0, 0.9999995, 0.99999995, 0.999999995, 1.0000005, 9.9999995, 9.99999995, 0.5, 5.0, 4.99999995})
                tv.push_back(v * m), tv.push_back(nextafter(v * m, 0.0)), tv.push_back(nextafter(v * m, 1e300));
        }
        for (double v : {123456.0, 1234567.0, 999999.5, 9999995.0, 0.0001, 0.00009999995, 0.000099999995, 1e-5, 100000.0, 1e6})
            tv.push_back(v), tv.push_back(-v);
        for (int prec = 0; prec <= 18; ++prec) {
            auto w = [&](auto& o) { o << defaultfloat << setprecision(prec); for (double v : tv) o << v << ' ' << float(v) << '\n'; };
            string x = capture([&](fastio::Writer& ww) { w(ww); }), y = capture_std([&](ostream& o) { w(o); });
            CHECK(x == y, "general precision " << prec);
            if (x != y) { size_t k = 0; while (k < x.size() && x[k] == y[k]) ++k; cerr << "   near: fast=[" << x.substr(k > 30 ? k - 30 : 0, 60) << "] std=[" << y.substr(k > 30 ? k - 30 : 0, 60) << "]\n"; }
        }
        auto wn = [&](auto& o) { o.precision(-1); o << fixed << 3.14159265 << ' ' << setprecision(4) << 2.0 << '\n'; };
        CHECK(capture([&](fastio::Writer& ww) { wn(ww); }) == capture_std([&](ostream& o) { wn(o); }), "negative precision");
    }
    {   // double input in many textual forms, bit-exact vs strtod
        vector<string> toks = {"0", "-0", "0.0", "-0.0", "+3.25", ".5", "-.75", "5.", "1e5", "1E-3", "2.5e+10", "-1.5E2",
                               "9007199254740993", "9007199254740992", "9007199254740991.5", "123456789012345678.5",
                               "0.000000000000000000001", "1234567890123456789", "12345678901234567890", "0.1", "0.30000000000000004",
                               "179769313486231570000000000000000000000.0", "4.9e-324", "inf", "-inf", "nan",
                               "00000000000000000000000012.5", "1.00000000000000000000000000001", "3.14159265358979323846264338"};
        char b[64];
        for (int i = 0; i < 100000; ++i) {
            double v = pow(10.0, uniform_real_distribution<double>(-25, 25)(rng)) * (rng() & 1 ? 1 : -1);
            int pr = rng() % 20;
            snprintf(b, sizeof b, rng() % 4 ? "%.*f" : "%.*e", pr, v); toks.push_back(b);
        }
        string all; for (auto& t : toks) all += t + (rng() % 2 ? " " : "\n");
        FILE* f = from_string(all); fastio::Reader r(f); bool ok = true; int bad = 0;
        for (auto& t : toks) {
            double x = 0; r >> x; double y = strtod(t.c_str(), nullptr);
            bool same = (isnan(x) && isnan(y)) || memcmp(&x, &y, 8) == 0;
            if (!same && bad++ < 5) cerr << "   token [" << t << "] fast=" << setprecision(17) << x << " strtod=" << y << "\n";
            ok &= same;
        }
        CHECK(ok, "double token forms");
        fclose(f);
        FILE* g = from_string("1.5abc 2.25"); fastio::Reader rr(g); double d1 = 0, d2 = 0; string mid;
        rr >> d1 >> mid >> d2;
        CHECK(d1 == 1.5 && mid == "abc" && d2 == 2.25, "double followed by letters");
        fclose(g);
    }

    // ---------- floating point input: exact vs strtod ----------
    {
        string s; char b[64];
        vector<double> want;
        for (auto x : dv) if (isfinite(x)) { snprintf(b, 64, "%.17g", x); s += b; s += rng() % 2 ? " " : "\n"; want.push_back(strtod(b, nullptr)); }
        s += "+2.5 -.75 1e5 3. 7\n";
        FILE* f = from_string(s); fastio::Reader r(f); bool ok = true;
        for (auto w : want) { double x; r >> x; ok &= memcmp(&x, &w, 8) == 0; }
        double p, q, e, t; float fl; r >> p >> q >> e >> t >> fl;
        ok &= p == 2.5 && q == -0.75 && e == 1e5 && t == 3.0 && fl == 7.0f;
        CHECK(ok, "double reading not bit-exact"); fclose(f);
    }

    // ---------- strings, chars, getline, CRLF, long tokens across buffer edges ----------
    {
        string big(200000, 'x'); big[100000] = 'y';
        string s = "hello  world\r\n\tabc\nline two here\r\n" + big + " z\n\nlast";
        FILE* f = from_string(s); fastio::Reader r(f);
        string w1, w2, w3, line, t, t2, l2, l3, l4;
        r >> w1 >> w2 >> w3;
        getline(r, line);          // rest of "abc" line -> ""
        getline(r, l2);            // "line two here" (\r stripped)
        r >> t >> t2;
        getline(r, l3); getline(r, l3);  // empty line
        getline(r, l4);                  // "last" (no trailing newline)
        CHECK(w1 == "hello" && w2 == "world" && w3 == "abc", "string >>");
        CHECK(line.empty() && l2 == "line two here", "getline + CRLF");
        CHECK(t == big && t2 == "z", "200KB token across buffer refills");
        CHECK(l3.empty() && l4 == "last", "getline empty / last line");
        string more; CHECK(!getline(r, more), "getline at EOF fails");
        fclose(f);
    }
    {
        FILE* f = from_string(" a b\nc 42 x\n"); fastio::Reader r(f);
        char c1, c2, c3; int n; char arr[16];
        r >> c1 >> c2 >> c3 >> n >> arr;
        CHECK(c1 == 'a' && c2 == 'b' && c3 == 'c' && n == 42 && string(arr) == "x", "char / char[] reading");
        int cnt = 0, x; while (r >> x) ++cnt;
        CHECK(cnt == 0, "while(cin>>x) on empty tail");
        fclose(f);
    }
    {   // while (cin >> x) counting, numbers glued to EOF without newline
        string s; long long sum = 0;
        for (int i = 0; i < 300000; ++i) { long long v = (long long)(rng() % 2000001) - 1000000; sum += v; s += to_string(v); s += i % 7 ? ' ' : '\n'; }
        s += "123";
        FILE* f = from_string(s); fastio::Reader r(f);
        long long x, got = 0; int cnt = 0;
        while (r >> x) got += x, ++cnt;
        CHECK(cnt == 300001 && got == sum + 123, "while(cin>>x) loop, cnt=" << cnt);
        fclose(f);
    }

    // ---------- iostream-style formatting through the fallback path ----------
    {
        auto wf = [&](auto& o) {
            o << setw(6) << 42 << '|' << setfill('*') << setw(5) << "ab" << '|' << left << setw(4) << 7 << '|' << right << '\n';
            o << hex << 255 << ' ' << showbase << 255 << dec << noshowbase << ' ' << 255 << '\n';
            o << boolalpha << true << ' ' << false << noboolalpha << ' ' << true << '\n';
            o << showpos << 5 << ' ' << -5 << noshowpos << ' ' << 5 << '\n';
            o << fixed << setprecision(2) << 3.14159 << ' ' << setw(10) << 2.5 << ' ' << defaultfloat << setprecision(6) << 2.5 << '\n';
            o << string("str") << ' ' << string("sv") << ' ' << (const char*)"cs" << ' ' << 'c' << '\n';
        };
        string x = capture([&](fastio::Writer& w) { wf(w); }), y = capture_std([&](ostream& o) { wf(o); });
        CHECK(x == y, "manipulators:\n" << x << "vs\n" << y);
        // custom type with ostream operator<<
        string z = capture([&](fastio::Writer& w) { w << Pt{1, -2} << ' ' << 3; });
        CHECK(z == "(1,-2) 3", "user type via ostream operator<<: " << z);
    }
    {   // endl flushes immediately
        FILE* f = tmpfile();
        {
            fastio::Writer w(f);
            w << 12 << endl;
            rewind(f); char b[8] = {}; size_t n = fread(b, 1, 8, f);
            CHECK(n == 3 && string(b) == "12\n", "endl flushes");
        }
        fclose(f);
    }
    {   // output bigger than the 64KB buffer, incl. one huge string
        string huge(300000, 'q');
        auto wb = [&](auto& o) { for (int i = 0; i < 100000; ++i) o << i << ' '; o << huge << '\n' << "end"; };
        CHECK(capture([&](fastio::Writer& w) { wb(w); }) == capture_std([&](ostream& o) { wb(o); }), "large output");
    }

    if (fails) { cerr << fails << " FAILURES\n"; return 1; }
    cerr << "all tests passed\n";
}
