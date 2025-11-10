// DiffieHellman_test.cpp
// Clean tests for Diffie-Hellman related functions using BigInt
#include <iostream>
#include <string>
#include <vector>
#include "BigInt.h"

// Forward declarations from DiffieHellman.cpp
BigInt modular_exponentiation(const BigInt &base, const BigInt &exponent, const BigInt &mod);
bool isPrime(const BigInt &n);
BigInt generate_safe_prime(int bit_size);

using namespace std;

static void expect_eq(const BigInt &v, const string &dec, const char *msg)
{
    string got = v.to_decimal();
    if (got != dec)
    {
        cerr << "FAIL: " << msg << " -- got: '" << got << "', expected: '" << dec << "'\n";
        exit(1);
    }
    else
        cout << "ok: " << msg << " -> " << dec << "\n";
}

static void expect_true(bool cond, const char *msg)
{
    if (!cond)
    {
        cerr << "FAIL: " << msg << "\n";
        exit(1);
    }
    else
        cout << "ok: " << msg << "\n";
}

// portable mulmod for 64-bit (avoid compiler-specific 128-bit types)
static unsigned long long mulmod64(unsigned long long a, unsigned long long b, unsigned long long m)
{
    unsigned long long res = 0;
    if (m == 0)
        return 0;
    a %= m;
    while (b)
    {
        if (b & 1)
        {
            res += a;
            if (res >= m)
                res -= m;
        }
        a <<= 1;
        if (a >= m)
            a -= m;
        b >>= 1;
    }
    return res % m;
}

static unsigned long long powmod64(unsigned long long a, unsigned long long e, unsigned long long m)
{
    if (m == 0)
        return 0;
    unsigned long long r = 1 % m;
    a %= m;
    while (e)
    {
        if (e & 1)
            r = mulmod64(r, a, m);
        a = mulmod64(a, a, m);
        e >>= 1;
    }
    return r;
}

int main()
{
    cout << "Running Diffie-Hellman tests...\n";

    // 1) modular_exponentiation small test
    BigInt base("5"), exp("117"), mod("19");
    BigInt r = modular_exponentiation(base, exp, mod);
    unsigned long long expect = powmod64(5, 117, 19);
    expect_eq(r, to_string(expect), "modexp 5^117 % 19");

    // 2) modexp with exponent 0 -> 1
    BigInt r2 = modular_exponentiation(base, BigInt(0), mod);
    expect_eq(r2, "1", "modexp exponent 0 -> 1");

    // 3) isPrime small primes and composites
    vector<string> primes = {"2", "3", "5", "7", "11", "13", "17", "19", "23"};
    for (auto &s : primes)
        expect_true(isPrime(BigInt(s)), (string("isPrime(") + s + ") should be true").c_str());
    vector<string> comps = {"4", "6", "8", "9", "12", "15", "21"};
    for (auto &s : comps)
        expect_true(!isPrime(BigInt(s)), (string("isPrime(") + s + ") should be false").c_str());

    // 4) generate_safe_prime small bit size (fast)
    int bit_size = 16; // small for tests
    BigInt::BIT_SIZE = bit_size;
    BigInt p = generate_safe_prime(bit_size);
    cout << "Generated p (bit_size=" << bit_size << "): " << p.to_decimal() << "\n";
    expect_true(!p.is_even(), "safe prime p should be odd");
    expect_true(isPrime(p), "generated p should be prime");
    BigInt q = (p - BigInt(1)) / BigInt(2);
    expect_true(isPrime(q), "(p-1)/2 should be prime (q)");
    // verify p == 2*q + 1
    BigInt check = q + q + BigInt(1);
    expect_true(check == p, "p == 2*q + 1");

    // 5) small Diffie-Hellman exchange
    // Using safe small prime 23, generator 5 (common classroom example)
    BigInt p23("23"), g5("5");
    BigInt a(6), b(15);
    BigInt A = modular_exponentiation(g5, a, p23);
    BigInt B = modular_exponentiation(g5, b, p23);
    BigInt sA = modular_exponentiation(B, a, p23);
    BigInt sB = modular_exponentiation(A, b, p23);
    expect_eq(sA, sB.to_decimal(), "DH shared secret equal (23,5,a=6,b=15)");

    cout << "All Diffie-Hellman tests passed.\n";
    return 0;
}
