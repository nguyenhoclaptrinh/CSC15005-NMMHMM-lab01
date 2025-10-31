#include <iostream>
#include "BigInt.h"

// A: Triển khai hàm lũy thừa mô-đun
// Hàm thực hiện: (base^exponent) % mod
BigInt modular_exponentiation(const BigInt& base, const BigInt& exponent, const BigInt& mod)
{
    BigInt result = 1;
    BigInt base_mod = base % mod;

    // Ví dụ về logic lũy thừa mô-đun
    BigInt exp = exponent;
    while (exp > 0) {
        if (exp % BigInt(2) == BigInt(1)) {
            result = (result * base_mod) % mod;
        }
        base_mod = (base_mod * base_mod) % mod;
        exp = exp / BigInt(2);
    }
    return result;
}

// B: Triển khai hàm sinh số nguyên tố ngẫu nhiên
BigInt generate_safe_prime(int bit_size)
{
    // 1. Cài đặt logic để sinh một số nguyên tố an toàn
    // 2. Viết hàm kiểm tra nguyên tố (ví dụ: Miller-Rabin)

    // TODO: Cài đặt sinh số nguyên tố an toàn thực tế
    return BigInt::random(bit_size);
}

// C: Triển khai hàm sinh khóa riêng ngẫu nhiên
BigInt generate_private_key(const BigInt& p)
{
    // Sử dụng sinh số ngẫu nhiên để tạo khóa riêng
    // Khóa riêng nên nằm trong khoảng [2, p-2]
    // TODO: Sinh số ngẫu nhiên trong khoảng [2, p-2]
    return BigInt::random(p.to_string().size() * 4); // Giả lập, cần chỉnh lại cho đúng khoảng
}

// D: Hoàn thành logic trao đổi khóa Diffie-Hellman
// #include "BigInt.h" // Sẽ tạo class này ở bước tiếp theo

int main()
{

    // 1. Sinh số nguyên tố lớn p và phần tử sinh g
    int bit_size = 512; // Kích thước bit ví dụ, có thể điều chỉnh
    BigInt p = generate_safe_prime(bit_size); // Sinh một số nguyên tố
    BigInt g = BigInt(2); // Phần tử sinh, sinh viên cần tìm hiểu và chọn giá trị khác

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
    std::cout << "Bí mật chung Alice nhận được: " << alice_shared_secret.to_string() << "\n";
    std::cout << "Bí mật chung Bob nhận được: " << bob_shared_secret.to_string() << "\n";
    std::cout << "Quá trình tính toán đúng không? " << (alice_shared_secret == bob_shared_secret) << "\n";

    return 0;
}