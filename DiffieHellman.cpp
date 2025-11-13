#include <iostream>
#include <random>
#include "BigInt.h"
using namespace std;

// parity helper (was previously a method on BigInt; moved here per request)
static inline bool is_even(const BigInt &n)
{
    return (n.data.empty() ? true : ((n.data[0] & 1u) == 0));
}

// A: Triển khai hàm lũy thừa mô-đun
// Hàm thực hiện: (base^exponent) % mod
BigInt modular_exponentiation(const BigInt &base, const BigInt &exponent, const BigInt &mod)
{
    BigInt result(1);
    BigInt base_mod = base % mod;

    BigInt exp = exponent;
    // Iterate bits of exponent from least-significant to most; use shr_bits(1)
    while (!(exp == BigInt(0)))
    {
        if ((exp.data.size() > 0) && ((exp.data[0] & 1u) != 0))
        {
            result = (result * base_mod) % mod;
        }
        base_mod = (base_mod * base_mod) % mod;
        exp = exp.shr_bits(1);
    }
    return result;
}

bool millerRabinTest(const BigInt &n, const BigInt &a)
{
    if (a >= BigInt(n - BigInt(1))) return true;
    BigInt d = n - BigInt(1);
    BigInt s(0);
    // factor n-1 = d * 2^s by shifting out low bits
    while ((d.data.size() > 0) && ((d.data[0] & 1u) == 0))
    {
        d = d.shr_bits(1);
        s = s + BigInt(1);
    }
    BigInt x = modular_exponentiation(a, d, n);
    if (x == BigInt(1) || x == n - BigInt(1))
    {
        return true;
    }
    for (BigInt r = BigInt(1); r < s; r = r + BigInt(1))
    {
        x = (x * x) % n;
        if (x == n - BigInt(1))
            return true;
        if (x == BigInt(1))
            return false;
    }
    return false;
}
bool isPrime(const BigInt &n)
{
     if (n < BigInt(2))
        return false;
    if (n == BigInt(2) || n == BigInt(3))
        return true;
    if (is_even(n) || n % BigInt(3) == BigInt(0))
        return false;
    // Expanded list of small prime bases for Miller-Rabin (many bases to increase confidence for large sizes)
    std::vector<BigInt> primes = {
        BigInt(2), BigInt(3), BigInt(5), BigInt(7), BigInt(11), BigInt(13), BigInt(17), BigInt(19),
        BigInt(23), BigInt(29), BigInt(31), BigInt(37), BigInt(41), BigInt(43), BigInt(47), BigInt(53)
        // BigInt(59), BigInt(61), BigInt(67), BigInt(71), BigInt(73), BigInt(79), BigInt(83), BigInt(89),
        // BigInt(97), BigInt(101), BigInt(103), BigInt(107), BigInt(109), BigInt(113), BigInt(127), BigInt(131),
        // BigInt(137), BigInt(139), BigInt(149), BigInt(151), BigInt(157), BigInt(163), BigInt(167), BigInt(173),
        // BigInt(179), BigInt(181), BigInt(191), BigInt(193), BigInt(197), BigInt(199), BigInt(211), BigInt(223),
        // BigInt(227), BigInt(229), BigInt(233), BigInt(239), BigInt(241), BigInt(251), BigInt(257), BigInt(263),
        // BigInt(269), BigInt(271), BigInt(277), BigInt(281), BigInt(283), BigInt(293), BigInt(307), BigInt(311),
        // BigInt(313), BigInt(317), BigInt(331), BigInt(337), BigInt(347), BigInt(349), BigInt(353), BigInt(359),
        // BigInt(367), BigInt(373), BigInt(379), BigInt(383), BigInt(389), BigInt(397), BigInt(401), BigInt(409),
        // BigInt(419), BigInt(421), BigInt(431), BigInt(433), BigInt(439), BigInt(443), BigInt(449), BigInt(457),
        // BigInt(461), BigInt(463), BigInt(467), BigInt(479), BigInt(487), BigInt(491), BigInt(499), BigInt(503),
        // BigInt(509), BigInt(521), BigInt(523), BigInt(541)
    };

    for (const BigInt &a : primes)
    {
        if (!millerRabinTest(n, a))
        {
            return false;
        }
    }
    return true;
}
BigInt get_min_value_with_bit_size(int bit_size)
{
    if (bit_size <= 0)
        return BigInt(0);
    size_t words = (bit_size + 31) / 32;
    BigInt result(0);
    result.data.assign(words, 0u);
    size_t wi = size_t((bit_size - 1) / 32);
    int bi = (bit_size - 1) % 32;
    result.data[wi] = (1u << bi);
    result.normalize();
    printf("Min value with bit size %d: %s\n", bit_size, result.to_decimal().c_str());
    return result;
}
// B: Triển khai hàm sinh số nguyên tố ngẫu nhiên
BigInt generate_safe_prime(int bit_size)
{
    // 1. Cài đặt logic để sinh một số nguyên tố an toàn
    // 2. Viết hàm kiểm tra nguyên tố (ví dụ: Miller-Rabin)
    BigInt q = get_min_value_with_bit_size(bit_size - 1);
    BigInt p;
    
    if (is_even(q))
        q = q + BigInt(1);
    int tries = 0;
    while(true) {
        if(tries > 40000) {
            cout << "Please input another bit_size " ;
            p = BigInt(0);
            break;
        }
        if(q % BigInt(3) == BigInt(1)) {    // q = 1 (mod 3) -> p = 2q + 1 = 0 (mod 3) not prime
            q = q + BigInt(4);
            continue;
        }
        if(q % BigInt(5) == BigInt(2)) {   // q = 2 (mod 5) -> p = 2q + 1 = 0 (mod 5) not prime
            q = q + BigInt(2);
            continue;
        }
        if(bit_size > 3 && q % BigInt(7) == BigInt(3)) {  // q = 3 (mod 7) -> p = 2q + 1 = 0 (mod 7) not prime
            q = q + BigInt(2);
            continue;
        }
        if (isPrime(q)) {
            p = q * BigInt(2) + BigInt(1);
            if (isPrime(p))
                break;
        }
        q = q + BigInt(2);
        tries++;
    }
    return p;
}

// C: Triển khai hàm sinh khóa riêng ngẫu nhiên
BigInt generate_private_key(const BigInt &p)
{
    // Khởi tạo bộ sinh số ngẫu nhiên
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint64_t> dis;

    BigInt private_key(0);

    // Sinh số ngẫu nhiên cho mỗi phần tử 32-bit trong vector data
    for (size_t i = 0; i < private_key.data.size(); ++i)
    {
        private_key.data[i] = dis(gen);
    }

    // Đảm bảo private_key nằm trong khoảng [2, p-2]
    BigInt p_minus_2 = p - BigInt(2);
    private_key = private_key % p_minus_2;
    private_key = private_key + BigInt(2);

    return private_key;
}

// D: Hoàn thành logic trao đổi khóa Diffie-Hellman

#ifndef UNIT_TEST
int main()
{
    // 1. Sinh số nguyên tố lớn p và phần tử sinh g
    int bit_size = 32; // Kích thước bit ví dụ, có thể điều chỉnh
    printf("Enter bit size for prime p: ");
    cin >> bit_size;
    BigInt p = generate_safe_prime(bit_size); // Sinh một số nguyên tố
    BigInt g = BigInt(2);                     // Phần tử sinh, sinh viên cần tìm hiểu và chọn giá trị khác

    printf("Generated safe prime p: %s\n", p.to_decimal().c_str());
    printf("Using generator g: %s\n", g.to_decimal().c_str());

    // 2. Sinh khóa riêng của Alice và Bob
    BigInt a = generate_private_key(p); // Khóa riêng của Alice
    BigInt b = generate_private_key(p); // Khóa riêng của Bob

    // 3. Tính giá trị công khai của Alice và Bob
    BigInt A = modular_exponentiation(g, a, p); // Alice tính A = g^a % p
    BigInt B = modular_exponentiation(g, b, p); // Bob tính B = g^b % p

    // 4. Tính bí mật chung
    BigInt alice_shared_secret = modular_exponentiation(B, a, p); // Alice tính s = B^a % p
    BigInt bob_shared_secret = modular_exponentiation(A, b, p);   // Bob tính s = A^b % p

    // 5. Hiển thị kết quả và xác minh rằng bí mật chung trùng khớp
    std::cout << "Bi mat chung Alice nhan duoc: " << alice_shared_secret << "\n";
    std::cout << "Bi mat chung Bob nhan duoc: " << bob_shared_secret << "\n";
    std::cout << "Qua trinh tinh toan dung khong? " << (alice_shared_secret == bob_shared_secret) << "\n";

    return 0;
}
#endif
