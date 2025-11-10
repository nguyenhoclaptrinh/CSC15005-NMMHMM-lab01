// BigInt.h
// Định nghĩa class BigInt cho số lớn với T bits (có thể điều chỉnh)
#pragma once
#include <vector>
#include <string>
#include <iostream>

using namespace std;

class BigInt {
public:
    // Số bit tối đa (thiết lập trước khi tạo nhiều BigInt) - phải chia hết cho 32
    static int BIT_SIZE;
    vector<uint32_t> data; // little-endian: data[0] là 32 bit thấp nhất

    // Constructors
    BigInt();                  // =0
    BigInt(uint32_t val); // khởi tạo từ 32-bit không dấu
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
    BigInt operator*(const BigInt &other) const; // schoolbook, cắt kết quả
    BigInt operator/(const BigInt &other) const; // chia lấy phần nguyên
    BigInt operator%(const BigInt &mod) const;   // phần dư

    // Tiện ích
    bool is_even() const { return (data.empty() ? true : (data[0] & 1u) == 0); }
    BigInt &normalize(); // loại bỏ word cao bằng 0 (giữ ít nhất 1)
    string to_decimal() const; // xuất thập phân

    // I/O
    friend istream &operator>>(istream &in, BigInt &val);
    friend ostream &operator<<(ostream &out, const BigInt &val);
};
