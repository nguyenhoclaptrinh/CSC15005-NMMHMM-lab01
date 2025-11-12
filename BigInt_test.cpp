// BigInt_test.cpp
// Lightweight unit tests for BigInt
#include <iostream>
#include <cassert>
#include <string>
#include "BigInt.h"
#include <random>
#include <cstdlib>

using namespace std;

static void expect_eq(const BigInt &v, const string &dec, const char *msg)
{
    BigInt expected(dec);
    if (!(v == expected))
    {
        cerr << "FAIL: " << msg << " -- not equal to expected decimal: '" << dec << "'\n";
        exit(1);
    }
    else
    {
        cout << "ok: " << msg << " -> " << dec << "\n";
    }
}

int main()
{
    cout << "Running BigInt tests...\n";

    // 1) Zero and decimal parsing
    BigInt a(0);
    BigInt b(string("0"));
    expect_eq(a, "0", "constructor 0");
    expect_eq(b, "0", "parse '0'");
    assert(a == b);

    // 2) Small addition
    BigInt one(1), two(2);
    BigInt three = one + two;
    expect_eq(three, "3", "1+2=3");

    // 3) Addition with carry across 32-bit boundary
    // 4294967295 + 1 = 4294967296
    BigInt max32(string("4294967295"));
    BigInt plus1 = max32 + BigInt(1);
    expect_eq(plus1, "4294967296", "32-bit carry");

    // 4) Multiplication small
    BigInt m1(string("12345"));
    BigInt m2(string("6789"));
    BigInt mres = m1 * m2; // 12345*6789 = 83810205
    expect_eq(mres, "83810205", "12345 * 6789");

    // 5) Division and modulo (small enough to fit in 64-bit for expected)
    unsigned long long A = 123456789012345ULL;
    unsigned long long B = 12345ULL;
    unsigned long long q = A / B;
    unsigned long long r = A % B;
    BigInt bigA(to_string(A));
    BigInt bigB(to_string(B));
    BigInt bigQ = bigA / bigB;
    BigInt bigR = bigA % bigB;
    expect_eq(bigQ, to_string(q), "division quotient");
    expect_eq(bigR, to_string(r), "division remainder");

    // 6) Subtraction
    BigInt s1(string("1000000000000"));
    BigInt s2(string("1"));
    BigInt s3 = s1 - s2;
    expect_eq(s3, "999999999999", "subtraction 1e12 - 1");

    // 7) to_decimal for a larger value
    BigInt bigN(string("340282366920938463463374607431768211455")); // 2^128-1
    expect_eq(bigN, string("340282366920938463463374607431768211455"), "to_decimal for 2^128-1");

    // 8) multiplication carry across multiple words
    BigInt max32b(string("4294967295")); // 2^32 -1
    BigInt sq = max32b * max32b;         // (2^32-1)^2 = 18446744065119617025
    expect_eq(sq, string("18446744065119617025"), "(2^32-1)^2 carry propagation");

    BigInt two32b(string("4294967296")); // 2^32
    BigInt sq2 = two32b * two32b;        // 2^64 = 18446744073709551616
    expect_eq(sq2, string("18446744073709551616"), "(2^32)^2 = 2^64");

    // 9) parsing with non-digit characters (constructor ignores non-digits)
    BigInt parsed(string("  00123abc"));
    expect_eq(parsed, string("123"), "parse ignores non-digit prefixes/suffixes");

    // 10) division/modulo edge cases
    BigInt x(string("123456789"));
    expect_eq(x / x, string("1"), "x / x == 1");
    expect_eq(x % x, string("0"), "x % x == 0");
    expect_eq(x / BigInt(1), string("123456789"), "x / 1 == x");

    // 11) (removed) bit helpers: set_bit and msb_index - not present in this build

    // 12) shift helpers
    BigInt sh(string("123456789"));
    BigInt shl = sh.shl_bits(10); // multiply by 2^10 = 1024
    BigInt expectMul = sh * BigInt(1024);
    if (!(shl == expectMul)) { cerr << "FAIL: shl_bits(10) != multiply by 1024\n"; std::_Exit(1); }
    BigInt shr = shl.shr_bits(10);
    if (!(shr == sh)) { cerr << "FAIL: shr_bits(10) did not restore original\n"; std::_Exit(1); }

    // 13) divmod exception on divide by zero
    try {
        BigInt q,r;
        bigA.divmod(BigInt(0), q, r);
        cerr << "FAIL: expected divide by zero to throw\n";
        std::_Exit(1);
    } catch (const std::runtime_error &e) {
        // expected
    }

    // 14) randomized small-precision division checks (compare with 64-bit reference)
    std::mt19937_64 rng(123456);
    for (int i = 0; i < 200; ++i)
    {
        unsigned long long A_r = (rng() % 1000000000000ULL) + 1; // up to 1e12
        unsigned long long B_r = (rng() % 1000000ULL) + 1;      // up to 1e6
        unsigned long long qq = A_r / B_r;
        unsigned long long rr = A_r % B_r;
        BigInt BA(std::to_string(A_r));
        BigInt BB(std::to_string(B_r));
        BigInt BQ = BA / BB;
        BigInt BR = BA % BB;
        expect_eq(BQ, std::to_string(qq), "random div quotient");
        expect_eq(BR, std::to_string(rr), "random div remainder");
    }

    // 15) (skipped) set_bit >= BIT_SIZE ignored - set_bit/membership not available

    cout << "All tests passed.\n";
    std::_Exit(0);
}
