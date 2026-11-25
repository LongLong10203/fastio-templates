# fastio-templates

Fast input/output templates for competitive programming.

| Template | Language | What it is |
|---|---|---|
| [cpp-dropin](cpp-dropin/) | C++17 | Drop-in replacement for `cin` / `cout`. Keep writing `cin >> x` and `cout << x`, and get 2–11× faster I/O than `cin.tie(0)->sync_with_stdio(0)`, with byte-identical output. Works for interactive problems too. |
| [cpp14-dropin](cpp14-dropin/) | C++14 | The same template written in strict C++14, for judges without C++17 (e.g. SPOJ). Same features and speed. |

Each template lives in its own folder with a README, tests and benchmarks.

## License

[MIT](LICENSE)
