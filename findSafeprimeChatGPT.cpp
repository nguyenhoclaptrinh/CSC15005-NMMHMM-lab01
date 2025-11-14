// find_safeprime.cpp
// Compile: g++ -std=c++17 -O2 find_safeprime.cpp -o find_safeprime
// Requires Boost.Multiprecision (header-only)

#include <boost/multiprecision/cpp_int.hpp>
#include <random>
#include <chrono>
#include <iostream>

using boost::multiprecision::cpp_int;
using u64 = unsigned long long;

// Fast modular exponentiation: base^exp % mod
cpp_int mod_pow(cpp_int base, cpp_int exp, const cpp_int &mod)
{
    cpp_int res = 1;
    base %= mod;
    while (exp > 0)
    {
        if ((exp & 1) != 0)
            res = (res * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return res;
}

// Miller-Rabin probabilistic primality test
bool miller_rabin(const cpp_int &n, int rounds, std::mt19937_64 &rng)
{
    if (n < 2)
        return false;
    // small primes quick test
    static const int small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for (int p : small_primes)
    {
        if (n == p)
            return true;
        if (n % p == 0)
            return false;
    }

    cpp_int d = n - 1;
    unsigned int s = 0;
    while ((d & 1) == 0)
    {
        d >>= 1;
        ++s;
    } // n-1 = d * 2^s with d odd

    std::uniform_int_distribution<u64> dist64(0, ULLONG_MAX);

    auto rand_between = [&](const cpp_int &a, const cpp_int &b) -> cpp_int
    {
        // returns random integer in [a, b]
        cpp_int range = b - a + 1;
        // build random by concatenating 64-bit blocks until >= range
        cpp_int r = 0;
        size_t needed_bits = boost::multiprecision::msb(range) + 1;
        size_t chunks = (needed_bits + 63) / 64;
        do
        {
            r = 0;
            for (size_t i = 0; i < chunks; ++i)
            {
                u64 w = dist64(rng);
                cpp_int part = cpp_int(w);
                r <<= 64;
                r += part;
            }
            // reduce
            r %= range;
        } while (r < 0);
        return a + r;
    };

    for (int i = 0; i < rounds; ++i)
    {
        cpp_int a = rand_between(2, n - 2);
        cpp_int x = mod_pow(a, d, n);
        if (x == 1 || x == n - 1)
            continue;
        bool composite = true;
        for (unsigned int r = 1; r < s; ++r)
        {
            x = (x * x) % n;
            if (x == n - 1)
            {
                composite = false;
                break;
            }
        }
        if (composite)
            return false;
    }
    return true; // probable prime
}

// Generate random odd integer with exactly 'bits' bits
cpp_int random_bits_int(unsigned int bits, std::mt19937_64 &rng)
{
    if (bits == 0)
        return 0;
    std::uniform_int_distribution<u64> dist64(0, ULLONG_MAX);
    cpp_int x = 0;
    unsigned int full_chunks = bits / 64;
    unsigned int rem_bits = bits % 64;
    for (unsigned int i = 0; i < full_chunks; ++i)
    {
        x <<= 64;
        x += cpp_int(dist64(rng));
    }
    if (rem_bits)
    {
        u64 tail = dist64(rng);
        tail &= ((rem_bits == 64) ? ~0ULL : ((1ULL << rem_bits) - 1));
        x <<= rem_bits;
        x += cpp_int(tail);
    }
    // set highest bit to ensure exact bit length
    x |= (cpp_int(1) << (bits - 1));
    // make odd
    x |= 1;
    return x;
}

// Find safe prime p with p_bits bits (probabilistic)
cpp_int find_safe_prime(unsigned int p_bits, int mr_rounds = 64)
{
    if (p_bits < 3)
        throw std::runtime_error("p_bits must be >= 3");
    // q will have p_bits-1 bits (since p = 2q+1)
    unsigned int q_bits = p_bits - 1;

    // RNG seeded from high-quality source and time
    std::random_device rd;
    std::seed_seq seed{rd(), (unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count()};
    std::mt19937_64 rng(seed);

    while (true)
    {
        cpp_int q = random_bits_int(q_bits, rng);
        // optionally force q % 3 != 1 to avoid p divisible by 3:
        // if (q % 3 == 1) { q += 2; if (boost::multiprecision::msb(q) != q_bits-1) q = random_bits_int(q_bits, rng); }
        // Test q
        if (!miller_rabin(q, mr_rounds, rng))
            continue;
        cpp_int p = q * 2 + 1;
        // Ensure p has exactly p_bits
        if (boost::multiprecision::msb(p) + 1 != p_bits)
            continue;
        if (!miller_rabin(p, mr_rounds, rng))
            continue;
        return p;
    }
}

int main(int argc, char **argv)
{
    unsigned int bits = 513;
    // printf("Input number of bits for safe prime (default 1024): ");
    
    int rounds = 64;
    if (argc >= 2)
        bits = (unsigned int)std::stoul(argv[1]);
    if (argc >= 3)
        rounds = std::stoi(argv[2]);

    std::cout << "Searching safe prime with " << bits << " bits (Miller-Rabin rounds=" << rounds << ")...\n";
    auto t0 = std::chrono::high_resolution_clock::now();
    cpp_int p = find_safe_prime(bits, rounds);
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dt = t1 - t0;

    cpp_int q = (p - 1) / 2;
    std::cout << "Found p (safe prime) in " << dt.count() << " s\n";
    std::cout << "p (bits=" << (boost::multiprecision::msb(p) + 1) << "):\n"
              << p << "\n\n";
    std::cout << "q=(p-1)/2 (bits=" << (boost::multiprecision::msb(q) + 1) << "):\n"
              << q << "\n";
    return 0;
}
