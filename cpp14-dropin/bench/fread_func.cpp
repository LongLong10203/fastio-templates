// 6) the common contest snippet: fread/fwrite 64KB buffers + readInt()/writeInt() functions
#include <bits/stdc++.h>
static char ibuf[1 << 16], obuf[1 << 16];
static size_t ipos = 0, ilen = 0, opos = 0;
inline int gc() {
    if (ipos == ilen) { ilen = fread(ibuf, 1, sizeof ibuf, stdin); ipos = 0; if (!ilen) return -1; }
    return ibuf[ipos++];
}
inline void pc(char c) {
    if (opos == sizeof obuf) fwrite(obuf, 1, opos, stdout), opos = 0;
    obuf[opos++] = c;
}
#define GC() gc()
#define PC(c) pc(c)
#define FLUSH() (fwrite(obuf, 1, opos, stdout), opos = 0, fflush(stdout))
#include "body_custom.inc"
int main(int, char** argv) { return run(argv[1]); }
