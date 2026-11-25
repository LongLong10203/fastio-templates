// Solution side of the interactive tests. run_tests.sh pastes the template (with its
// #defines) above this file. It never calls flush or endl: the template must flush cout
// by itself before it waits for input, like std::cin does.
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <string>
using namespace std;

int main(int argc, char** argv) {
    string mode = argc > 1 ? argv[1] : "";
    int T;
    cin >> T;
    if (mode == "guess") {
        while (T--) {
            long long lo = 1, hi = 1000000000;
            while (lo < hi) {
                long long mid = (lo + hi) / 2;
                cout << "? " << mid << '\n';
                string r;
                cin >> r;
                if (r == "=") { lo = hi = mid; break; }
                if (r == "<") lo = mid + 1;
                else hi = mid - 1;
            }
            cout << "! " << lo << '\n';
        }
    } else if (mode == "split") {
        while (T--) { long long v; cin >> v; cout << 2 * v << '\n'; }
    } else if (mode == "mixed") {
        while (T--) {
            double d;
            string w;
            cin >> d >> w;
            reverse(w.begin(), w.end());
            cout << fixed << setprecision(6) << 2 * d << ' ' << w << '\n';
        }
    }
}
