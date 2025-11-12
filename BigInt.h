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
    // Số bit tối đa (thiết lập trước khi tạo nhiều BigInt) - phải chia hết cho 32
    static int BIT_SIZE;
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

    // Bit utilities (member versions so they can be reused elsewhere)
    // shift-left by given number of bits, returning a new BigInt
    BigInt shl_bits(int bits) const;
    // shift-right by given number of bits, returning a new BigInt
    BigInt shr_bits(int bits) const;
    // index (0-based) of most-significant set bit, or -1 if zero
    int msb_index() const;
    // set a bit (mutates this); ignored if bit >= BIT_SIZE
    void set_bit(int bit);
    // Compute quotient and remainder: *this / divisor = quotient, remainder
    void divmod(const BigInt &divisor, BigInt &quotient, BigInt &remainder) const;

    // I/O
    friend istream &operator>>(istream &in, BigInt &val);
    friend ostream &operator<<(ostream &out, const BigInt &val);
};
