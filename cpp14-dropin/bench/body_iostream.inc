// Shared benchmark body for cin_sync / cin_nosync / fastio.
// It is the SAME source text in all three: only the stream objects behind cin/cout differ.
static unsigned long long lcg_state = 88172645463325252ull;
inline unsigned long long nxt() { return lcg_state = lcg_state * 6364136223846793005ull + 1442695040888963407ull; }
inline int gen_int() { return int((nxt() >> 33) % 2000000001ull) - 1000000000; }
inline long long gen_ll() { return (long long)(nxt() % 2000000000000000001ull) - 1000000000000000000ll; }
inline double gen_dbl() { return (double(nxt() >> 11) * (1.0 / 9007199254740992.0) - 0.5) * 2e6; }

int run(const std::string& test) {
    if (test == "read_int") {
        int n; cin >> n;
        long long s = 0;
        for (int i = 0; i < n; ++i) { int x; cin >> x; s += x; }
        cout << s << '\n';
    } else if (test == "read_ll") {
        int n; cin >> n;
        unsigned long long s = 0;
        for (int i = 0; i < n; ++i) { long long x; cin >> x; s += x; }
        cout << s << '\n';
    } else if (test == "write_ll") {
        int n; cin >> n;
        for (int i = 0; i < n; ++i) cout << gen_ll() << '\n';
    } else if (test == "write_int") {
        int n; cin >> n;
        for (int i = 0; i < n; ++i) cout << gen_int() << '\n';
    } else if (test == "write_int_endl") {
        int n; cin >> n;
        for (int i = 0; i < n; ++i) cout << gen_int() << endl;
    } else if (test == "echo_int") {
        int n; cin >> n;
        for (int i = 0; i < n; ++i) { int x; cin >> x; cout << x << '\n'; }
    } else if (test == "read_str") {
        int n; cin >> n;
        string s;
        unsigned long long tot = 0, h = 0;
        for (int i = 0; i < n; ++i) { cin >> s; tot += s.size(); h = h * 131 + s[0] * 7 + s.back(); }
        cout << tot << ' ' << h << '\n';
    } else if (test == "read_dbl") {
        int n; cin >> n;
        double s = 0;
        for (int i = 0; i < n; ++i) { double x; cin >> x; s += x; }
        cout << fixed << setprecision(6) << s << '\n';
    } else if (test == "write_dbl") {
        int n; cin >> n;
        cout << fixed << setprecision(6);
        for (int i = 0; i < n; ++i) cout << gen_dbl() << '\n';
    } else if (test == "write_dblg") {
        int n; cin >> n;
        cout << setprecision(10);
        for (int i = 0; i < n; ++i) cout << gen_dbl() << '\n';
    } else return 2;
    return 0;
}
