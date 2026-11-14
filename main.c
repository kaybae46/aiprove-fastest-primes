/*
 * fastest-primes — count of primes <= N (1 <= N <= 5,000,000).
 *
 * Odd-only Sieve of Eratosthenes over a byte array.
 *   index i  <->  odd number 2i+1   (i = 0 is the number 1, never prime)
 *   is_comp[i] = 1  ->  2i+1 is composite
 *
 * For each odd prime p <= sqrt(N): mark p*p, p*p+2p, ... — in index space
 * that is start = (p*p)>>1, step = p.
 *
 * Why this is fast for N <= 5e6:
 *   - 2.5 MB working set: fits in L3, so the marking loop is cache-resident.
 *   - One byte per odd, no bit fiddling; the inner loop is a single strided
 *     byte store, ~4M stores total for N = 5e6.
 *   - No sieve-of-Atkin / wheel overhead: at this size the plain odd-only
 *     sieve is memory-bound and the simple loop wins.
 *   - I/O via raw read(2)/write(2): no stdio, no locale, no float parsing.
 *   - Static single file, no dependencies beyond libc.
 *
 * Build:  cc -O3 -o primes main.c
 * Run:    echo 5000000 | ./primes        -> 348513
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    /* --- fast input: one read, manual parse --- */
    char in[32];
    ssize_t n = read(0, in, sizeof(in) - 1);
    if (n <= 0)
        return 0;
    in[n] = '\0';

    long N = 0;
    ssize_t i = 0;
    while (i < n && (in[i] < '0' || in[i] > '9'))
        i++; /* skip any leading whitespace */
    for (; i < n && in[i] >= '0' && in[i] <= '9'; i++)
        N = N * 10 + (in[i] - '0');

    if (N < 2) {
        if (write(1, "0\n", 2) < 0)
            return 1;
        return 0;
    }

    /* --- odd-only sieve --- */
    long half = (N + 1) / 2; /* odds 1,3,5,...,N */
    unsigned char *is_comp = (unsigned char *)malloc((size_t)half);
    if (!is_comp)
        return 1;
    memset(is_comp, 0, (size_t)half);

    /* integer sqrt(N), odd: smallest odd r with (r+2)^2 > N */
    long r = 1;
    while ((r + 2) * (r + 2) <= N)
        r += 2;

    for (long p = 3; p <= r; p += 2) {
        if (!is_comp[p >> 1]) {
            long step = p;
            for (long j = (p * p) >> 1; j < half; j += step)
                is_comp[j] = 1;
        }
    }

    /* --- count: 2 plus the unmarked odds >= 3 --- */
    long cnt = 1; /* prime 2 */
    for (long i = 1; i < half; i++)
        if (!is_comp[i])
            cnt++;

    free(is_comp);

    /* --- fast output: integer -> decimal, one write --- */
    char out[24];
    int len = 0;
    long v = cnt;
    char rev[24];
    int rlen = 0;
    if (v == 0) {
        rev[rlen++] = '0';
    } else {
        while (v > 0) {
            rev[rlen++] = (char)('0' + v % 10);
            v /= 10;
        }
    }
    while (rlen > 0)
        out[len++] = rev[--rlen];
    out[len++] = '\n';
    if (write(1, out, (size_t)len) < 0)
        return 1;
    return 0;
}
