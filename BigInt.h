// BigInt.h
// Định nghĩa class BigInt cho số lớn với T bits (có thể điều chỉnh)
#pragma once
#include <vector>
#include <string>
#include <iostream>

class BigInt {
public:
    // Số bit tối đa
    static int BIT_SIZE;
    std::vector<uint32_t> data; // Lưu trữ số lớn dưới dạng mảng các phần tử 32 bit

    BigInt();
    BigInt(uint64_t val);
    BigInt(const std::string& str);
    BigInt(const BigInt& other);

    // Các phép toán cơ bản
    BigInt operator%(const BigInt& mod) const;
    BigInt operator*(const BigInt& other) const;
    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator/(const BigInt& other) const;
    BigInt& operator=(const BigInt& other);
    bool operator==(const BigInt& other) const;
    bool operator>(const BigInt& other) const;
    bool operator<(const BigInt& other) const;

    // Hàm nhập xuất
    friend std::istream& operator>>(std::istream& in, BigInt& val);
    friend std::ostream& operator<<(std::ostream& out, const BigInt& val);
};
