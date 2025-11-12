// BigInt.h
// Định nghĩa class BigInt cho số lớn với T bits (có thể điều chỉnh)
#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <cstdint>

using namespace std;

class BigInt
{
public:
    // BigInt cơ chế dynamic-size: vector chứa các word 32-bit ít quan trọng nhất ở index 0.
    // (Trước đây có BIT_SIZE giới hạn; giờ bỏ giới hạn để cho phép mở rộng động.)
    vector<uint32_t> data; // little-endian: data[0] là 32 bit thấp nhất

    // Constructors
    BigInt();                      // =0
    BigInt(uint32_t val);          // khởi tạo từ 32-bit không dấu
    BigInt(const string &decimal); // parse chuỗi thập phân
    BigInt(const BigInt &other) = default;
    BigInt &operator=(const BigInt &other) = default;

    // So sánh
    bool operator==(const BigInt &other) const;
    bool operator<(const BigInt &other) const;
    bool operator>(const BigInt &other) const { return other < *this; }
    bool operator<=(const BigInt &other) const { return !(other < *this); }
    bool operator>=(const BigInt &other) const { return !(*this < other); }

    // Toán tử số học (giả sử không overflow vượt BIT_SIZE, nhân cắt phần cao)
    BigInt operator+(const BigInt &other) const;
    BigInt operator-(const BigInt &other) const; // giả sử *this >= other
    BigInt operator*(const BigInt &other) const; // nhân
    BigInt operator/(const BigInt &other) const; // chia lấy phần nguyên
    BigInt operator%(const BigInt &mod) const;   // phần dư

    // Utility
    BigInt &normalize();
    std::string to_decimal() const;

    // Quick checks and small shifts
    // (use existing `normalize()`/`== BigInt(0)` and `shr_bits(1)`)

    // Bit utilities (member versions so they can be reused elsewhere)
    // shift-left by given number of bits, returning a new BigInt
    BigInt shl_bits(int bits) const;
    // shift-right by given number of bits, returning a new BigInt
    BigInt shr_bits(int bits) const;
    // Compute quotient and remainder: *this / divisor = quotient, remainder
    void divmod(const BigInt &divisor, BigInt &quotient, BigInt &remainder) const;

    // I/O
    friend istream &operator>>(istream &in, BigInt &val);
    friend ostream &operator<<(ostream &out, const BigInt &val);
};
