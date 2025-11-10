// BigInt_test.cpp
// Lightweight unit tests for BigInt
#include <iostream>
#include <cassert>
#include <string>
#include "BigInt.h"

using namespace std;

static void expect_eq(const BigInt &v, const string &dec, const char *msg) {
    string got = v.to_decimal();
    if (got != dec) {
        cerr << "FAIL: " << msg << " -- got: '" << got << "', expected: '" << dec << "'\n";
        exit(1);
    } else {
        cout << "ok: " << msg << " -> " << dec << "\n";
    }
}

int main() {
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
    string dec = bigN.to_decimal();
    expect_eq(bigN, dec, "to_decimal for 2^128-1");

    cout << "All tests passed.\n";
    return 0;
}
