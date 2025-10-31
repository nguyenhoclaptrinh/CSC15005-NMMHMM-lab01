// BigInt.cpp
#include "BigInt.h"
#include <random>
#include <sstream>
using namespace std;

int BigInt::BIT_SIZE = 512;

BigInt::BigInt() {
    data = std::vector<uint32_t>(BIT_SIZE / 32, 0);
}

BigInt::BigInt(uint64_t val) {
    data = std::vector<uint32_t>(BIT_SIZE / 32, 0);
    data[0] = (uint32_t)(val & 0xFFFFFFFF);
    if (BIT_SIZE > 32) data[1] = (uint32_t)((val >> 32) & 0xFFFFFFFF);
}

BigInt::BigInt(const std::string& str) {
    // Chuyển từ chuỗi thập phân sang BigInt đơn giản
    *this = BigInt(0);
    for (char c : str) {
        *this = *this * BigInt(10) + BigInt(c - '0');
    }
}

BigInt::BigInt(const BigInt& other) {
    data = other.data;
}

BigInt& BigInt::operator=(const BigInt& other) {
    if (this != &other) {
        data = other.data;
    }
    return *this;
}

BigInt BigInt::operator+(const BigInt& other) const {
    BigInt res;
    uint64_t carry = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        uint64_t sum = (uint64_t)data[i] + other.data[i] + carry;
        res.data[i] = (uint32_t)(sum & 0xFFFFFFFF);
        carry = sum >> 32;
    }
    return res;
}

BigInt BigInt::operator-(const BigInt& other) const {
    BigInt res;
    int64_t borrow = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        int64_t diff = (int64_t)data[i] - other.data[i] - borrow;
        if (diff < 0) {
            diff += (1LL << 32);
            borrow = 1;
        } else {
            borrow = 0;
        }
        res.data[i] = (uint32_t)(diff & 0xFFFFFFFF);
    }
    return res;
}

BigInt BigInt::operator*(const BigInt& other) const {
    BigInt res;
    for (size_t i = 0; i < data.size(); ++i) {
        uint64_t carry = 0;
        for (size_t j = 0; j + i < data.size(); ++j) {
            uint64_t mul = (uint64_t)data[i] * other.data[j] + res.data[i + j] + carry;
            res.data[i + j] = (uint32_t)(mul & 0xFFFFFFFF);
            carry = mul >> 32;
        }
    }
    return res;
}

BigInt BigInt::operator/(const BigInt& other) const {
    // Chia đơn giản, chỉ dùng cho số nhỏ
    BigInt res;
    BigInt rem = *this;
    BigInt divisor = other;
    for (int i = (int)data.size() * 32 - 1; i >= 0; --i) {
        rem = rem - divisor;
        if (!(rem < BigInt(0))) {
            res = res + BigInt(1);
        } else {
            rem = rem + divisor;
        }
    }
    return res;
}

BigInt BigInt::operator%(const BigInt& mod) const {
    BigInt div = *this / mod;
    BigInt mul = div * mod;
    BigInt rem = *this - mul;
    return rem;
}

bool BigInt::operator==(const BigInt& other) const {
    return data == other.data;
}

bool BigInt::operator>(const BigInt& other) const {
    for (int i = (int)data.size() - 1; i >= 0; --i) {
        if (data[i] > other.data[i]) return true;
        if (data[i] < other.data[i]) return false;
    }
    return false;
}

bool BigInt::operator<(const BigInt& other) const {
    for (int i = (int)data.size() - 1; i >= 0; --i) {
        if (data[i] < other.data[i]) return true;
        if (data[i] > other.data[i]) return false;
    }
    return false;
}


// Hàm nhập xuất cho BigInt
std::istream& operator>>(std::istream& in, BigInt& val) {
    std::string str;
    in >> str;
    val = BigInt(str);
    return in;
}

std::ostream& operator<<(std::ostream& out, const BigInt& val) {
    bool started = false;
    for (int i = (int)val.data.size() - 1; i >= 0; --i) {
        if (started || val.data[i] != 0) {
            if (!started) started = true;
            out << std::hex << val.data[i];
        }
    }
    if (!started) out << "0";
    return out;
}
