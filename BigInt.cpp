// BigInt.cpp (updated per requirements)
#include "BigInt.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>

using namespace std;

int BigInt::BIT_SIZE = 256; // mặc định, có thể thay đổi trước khi tạo số

// ===== Helper =====
static inline size_t word_count() { return (BigInt::BIT_SIZE + 31) / 32; }
static const uint64_t BASE = (1ULL << 32);
static const uint64_t MASK = BASE - 1;

// ===== Constructors =====
BigInt::BigInt()
{
    data.assign(word_count(), 0);
}

BigInt::BigInt(uint32_t val)
{
    data.assign(word_count(), 0);
    if (!data.empty())
        data[0] = val;
}

BigInt::BigInt(const std::string &decimal)
{
    data.assign(word_count(), 0);
    // parse thập phân: this = this*10 + digit
    for (char c : decimal)
    {
        if (c < '0' || c > '9')
            continue;
        // nhân 10
        uint64_t carry = 0;
        for (size_t i = 0; i < data.size(); ++i)
        {
            uint64_t cur = uint64_t(data[i]) * 10 + carry;
            data[i] = uint32_t(cur & MASK);
            carry = cur >> 32;
        }
        // cộng digit
        carry = uint64_t(c - '0');
        for (size_t i = 0; i < data.size(); ++i)
        {
            uint64_t cur = uint64_t(data[i]) + carry;
            data[i] = uint32_t(cur & MASK);
            carry = cur >> 32;
            if (!carry)
                break;
        }
    }
    normalize();
}

// ===== Utility =====
BigInt &BigInt::normalize()
{
    // cắt các word cao bằng 0 giữ ít nhất 1 word
    while (data.size() > 1 && data.back() == 0)
        data.pop_back();
    // mở rộng nếu cần (giữ nguyên thiết kế fixed-size: ở đây ta cho phép co lại)
    return *this;
}

bool BigInt::operator==(const BigInt &other) const
{
    size_t n = max(data.size(), other.data.size());
    for (size_t i = 0; i < n; ++i)
    {
        uint32_t a = (i < data.size() ? data[i] : 0u);
        uint32_t b = (i < other.data.size() ? other.data[i] : 0u);
        if (a != b)
            return false;
    }
    return true;
}

bool BigInt::operator<(const BigInt &other) const
{
    size_t n = max(data.size(), other.data.size());
    for (int i = int(n) - 1; i >= 0; --i)
    {
        uint32_t a = (i < (int)data.size() ? data[i] : 0u);
        uint32_t b = (i < (int)other.data.size() ? other.data[i] : 0u);
        if (a != b)
            return a < b;
    }
    return false;
}

// ===== Arithmetic =====
BigInt BigInt::operator+(const BigInt &other) const
{
    BigInt r;
    r.data.assign(max(data.size(), other.data.size()), 0);
    uint64_t carry = 0;
    size_t n = r.data.size();
    for (size_t i = 0; i < n; ++i)
    {
        uint64_t a = (i < data.size() ? data[i] : 0);
        uint64_t b = (i < other.data.size() ? other.data[i] : 0);
        uint64_t s = a + b + carry;
        r.data[i] = uint32_t(s & MASK);
        carry = s >> 32;
    }
    if (carry)
        r.data.push_back(uint32_t(carry));
    return r.normalize();
}

BigInt BigInt::operator-(const BigInt &other) const
{
    // Giả sử *this >= other
    BigInt r;
    r.data.assign(data.size(), 0);
    int64_t borrow = 0;
    for (size_t i = 0; i < data.size(); ++i)
    {
        int64_t a = data[i];
        int64_t b = (i < other.data.size() ? other.data[i] : 0);
        int64_t d = a - b - borrow;
        if (d < 0)
        {
            d += int64_t(BASE);
            borrow = 1;
        }
        else
            borrow = 0;
        r.data[i] = uint32_t(d & MASK);
    }
    return r.normalize();
}

BigInt BigInt::operator*(const BigInt &other) const
{
    size_t na = data.size(), nb = other.data.size();
    BigInt r;
    r.data.assign(na + nb, 0);
    for (size_t i = 0; i < na; ++i)
    {
        uint64_t carry = 0;
        for (size_t j = 0; j < nb; ++j)
        {
            uint64_t cur = uint64_t(data[i]) * uint64_t(other.data[j]) + uint64_t(r.data[i + j]) + carry;
            r.data[i + j] = uint32_t(cur & MASK);
            carry = cur >> 32;
        }
        if (carry)
            r.data[i + nb] += uint32_t(carry);
    }
    return r.normalize();
}

// Bit helpers
static int msb_index(const BigInt &x)
{
    for (int wi = int(x.data.size()) - 1; wi >= 0; --wi)
    {
        uint32_t w = x.data[wi];
        if (w)
        {
            for (int b = 31; b >= 0; --b)
                if ((w >> b) & 1u)
                    return wi * 32 + b;
        }
    }
    return -1;
}
static int get_bit(const BigInt &x, int bit)
{
    if (bit < 0)
        return 0;
    int wi = bit / 32;
    int bi = bit % 32;
    if (wi >= (int)x.data.size())
        return 0;
    return (x.data[wi] >> bi) & 1u;
}
static void shl1(BigInt &x)
{
    uint32_t carry = 0;
    for (size_t i = 0; i < x.data.size(); ++i)
    {
        uint64_t cur = (uint64_t)x.data[i] * 2 + carry;
        x.data[i] = uint32_t(cur & MASK);
        carry = uint32_t(cur >> 32);
    }
    if (carry)
        x.data.push_back(carry);
}
static void set_bit(BigInt &x, int bit)
{
    int wi = bit / 32;
    int bi = bit % 32;
    if (wi >= (int)x.data.size())
        x.data.resize(wi + 1, 0);
    x.data[wi] |= (1u << bi);
}

BigInt BigInt::operator/(const BigInt &other) const
{
    // Chia nhị phân (bit-by-bit). Không tối ưu nhưng đúng.
    // Kiểm tra zero divisor
    bool zero = true;
    for (auto w : other.data)
        if (w)
        {
            zero = false;
            break;
        }
    if (zero)
        throw runtime_error("divide by zero");
    if (*this < other)
        return BigInt(0);
    BigInt q(0), r(0);
    int nbit = msb_index(*this);
    for (int b = nbit; b >= 0; --b)
    {
        shl1(r);
        if (get_bit(*this, b))
            r.data[0] |= 1u;
        if (!(r < other))
        {
            r = r - other;
            set_bit(q, b);
        }
    }
    return q.normalize();
}

BigInt BigInt::operator%(const BigInt &mod) const
{
    bool zero = true;
    for (auto w : mod.data)
        if (w)
        {
            zero = false;
            break;
        }
    if (zero)
        throw runtime_error("mod by zero");
    if (*this < mod)
        return *this;
    BigInt r(0);
    int nbit = msb_index(*this);
    for (int b = nbit; b >= 0; --b)
    {
        shl1(r);
        if (get_bit(*this, b))
            r.data[0] |= 1u;
        if (!(r < mod))
            r = r - mod;
    }
    return r.normalize();
}

// ===== Decimal conversion =====
std::string BigInt::to_decimal() const
{
    // Chia liên tiếp cho 10, thu thập remainder (slow nhưng đơn giản)
    if (data.size() == 1 && data[0] == 0)
        return "0";
    BigInt tmp = *this;
    std::string out;
    BigInt ten(10);
    while (!(tmp.data.size() == 1 && tmp.data[0] == 0))
    {
        // divmod tmp / 10
        BigInt q(0), r(0);
        int nbit = msb_index(tmp);
        for (int b = nbit; b >= 0; --b)
        {
            shl1(r);
            if (get_bit(tmp, b))
                r.data[0] |= 1u;
            if (!(r < ten))
            {
                r = r - ten;
                set_bit(q, b);
            }
        }
        // remainder r là digit
        uint32_t digit = (r.data.empty() ? 0u : r.data[0]);
        out.push_back(char('0' + digit));
        tmp = q.normalize();
    }
    reverse(out.begin(), out.end());
    return out;
}

// ===== I/O =====
std::istream &operator>>(std::istream &in, BigInt &val)
{
    std::string s;
    in >> s;
    val = BigInt(s);
    return in;
}

std::ostream &operator<<(std::ostream &out, const BigInt &val)
{
    // Xuất hex đơn giản
    BigInt v = val;
    v.normalize();
    std::ostringstream oss;
    for (int i = int(v.data.size()) - 1; i >= 0; --i)
    {
        if (i == int(v.data.size()) - 1)
            oss << std::hex << v.data[i];
        else
            oss << std::hex << std::setw(8) << std::setfill('0') << v.data[i];
    }
    out << oss.str();
    return out;
}
