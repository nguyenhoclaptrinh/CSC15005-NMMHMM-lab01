#include <iostream>
#include "BigInt.h"
using namespace std;

// A: Triển khai hàm lũy thừa mô-đun
// Hàm thực hiện: (base^exponent) % mod
BigInt modular_exponentiation(const BigInt &base, const BigInt &exponent, const BigInt &mod)
{
    BigInt result(1);
    BigInt base_mod = base % mod;

    BigInt exp = exponent;
    while (!(exp == BigInt(0)))
    {
        if (exp % BigInt(2) == BigInt(1))
        {
            result = (result * base_mod) % mod;
        }
        base_mod = (base_mod * base_mod) % mod;
        exp = exp / BigInt(2);
    }
    return result;
}

bool millerRabinTest(const BigInt& n, const BigInt& a) {
    if (n == BigInt(2) || n == BigInt(3)) return true;
    if (n.is_even()) return false;
    
    BigInt d = n - BigInt(1);
    BigInt s(0);
    while (d.is_even()) {
        d = d/BigInt(2);
        s = s + BigInt(1);
    }
    BigInt x = modular_exponentiation(a, d, n);
    if (x == BigInt(1) || x == n - BigInt(1)) {
         return true;
    }
   BigInt s_minus_1 = s - BigInt(1);
   for (BigInt r = BigInt(1); r <= s_minus_1; r = r + BigInt(1)) {
        x = (x * x) % n;
        if (x == n - BigInt(1)) return true;
        if (x == BigInt(1)) return false;
    }
    return false;
}
bool isPrime(const BigInt& n) {
    if (n < BigInt(2)) return false;
    if (n == BigInt(2) || n == BigInt(3)) return true;
    if (n.is_even() || n % BigInt(3) == BigInt(0)) return false;
    BigInt primes[] = {
        BigInt(2), BigInt(3), BigInt(5)
    };
    
    for (const BigInt& a : primes) {
        if (a >= n) continue;  
        if (!millerRabinTest(n, a)) {
            return false;
        }
    }
    return true;
}
BigInt get_min_value_with_bit_size(int bit_size) {
    BigInt result(0);
    if (bit_size <= 0) return result;
    
    int word_index = (bit_size - 1) / 32;
    int bit_index = (bit_size - 1) % 32;
    
    if (word_index < result.data.size()) {
        result.data[word_index] = 1U << bit_index;
    }
    return result;
}
// B: Triển khai hàm sinh số nguyên tố ngẫu nhiên
BigInt generate_safe_prime(int bit_size)
{
    // 1. Cài đặt logic để sinh một số nguyên tố an toàn
    // 2. Viết hàm kiểm tra nguyên tố (ví dụ: Miller-Rabin)
    BigInt p = get_min_value_with_bit_size(bit_size);
    if (p.is_even()) p = p + BigInt(1);
    while(true) {
        if(isPrime(p)){
             BigInt q = (p - BigInt(1))/ BigInt(2);
             if(isPrime(q)) 
                break;
        }
        p = p + BigInt(2);
    }
    return p;
}

// C: Triển khai hàm sinh khóa riêng ngẫu nhiên
BigInt generate_private_key(const BigInt &p)
{
    // Sử dụng sinh số ngẫu nhiên để tạo khóa riêng
    // Khóa riêng nên nằm trong khoảng [2, p-2]
    BigInt private_key(0);
    return private_key;
}

// D: Hoàn thành logic trao đổi khóa Diffie-Hellman

#ifndef UNIT_TEST
int main()
{
    // 1. Sinh số nguyên tố lớn p và phần tử sinh g
    int bit_size = 512; // Kích thước bit ví dụ, có thể điều chỉnh
    BigInt::BIT_SIZE = bit_size;
    BigInt p = generate_safe_prime(bit_size); // Sinh một số nguyên tố
    BigInt g = BigInt(2);                     // Phần tử sinh, sinh viên cần tìm hiểu và chọn giá trị khác

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
    std::cout << "Bí mật chung Alice nhận được: " << alice_shared_secret << "\n";
    std::cout << "Bí mật chung Bob nhận được: " << bob_shared_secret << "\n";
    std::cout << "Quá trình tính toán đúng không? " << (alice_shared_secret == bob_shared_secret) << "\n";

    return 0;
}
#endif
