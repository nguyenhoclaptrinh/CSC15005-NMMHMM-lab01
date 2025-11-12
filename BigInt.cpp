// BigInt.cpp (updated per requirements)
#include "BigInt.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>

using namespace std;

static const uint64_t BASE = (1ULL << 32);
static const uint64_t MASK = BASE - 1;

// ===== Constructors =====
BigInt::BigInt()
{
    data.clear();
    data.push_back(0u);
}

BigInt::BigInt(uint32_t val)
{
    data.clear();
    data.push_back(val);
}

BigInt::BigInt(const std::string &decimal)
{
    data.clear();
    data.push_back(0u);
    // parse thập phân: this = this*10 + digit
    for (char c : decimal)
    {
        if (c < '0' || c > '9')
            continue;
        // multiply by 10
        uint64_t carry = 0;
        for (size_t i = 0; i < data.size(); ++i)
        {
            uint64_t cur = uint64_t(data[i]) * 10 + carry;
            data[i] = uint32_t(cur & MASK);
            carry = cur >> 32;
        }
        while (carry)
        {
            data.push_back(uint32_t(carry & MASK));
            carry >>= 32;
        }
        // add digit
        uint64_t add = uint64_t(c - '0');
        size_t i = 0;
        while (add)
        {
            if (i >= data.size())
                data.push_back(0u);
            uint64_t cur = uint64_t(data[i]) + add;
            data[i] = uint32_t(cur & MASK);
            add = cur >> 32;
            ++i;
        }
    }
    normalize();
}

// ===== Utility =====
BigInt &BigInt::normalize()
{
    // trim high zero words but keep at least one word
    while (data.size() > 1 && data.back() == 0)
        data.pop_back();
    return *this;
}

// shift-left by an arbitrary number of bits, return new BigInt
BigInt BigInt::shl_bits(int bits) const
{
    if (bits == 0)
        return *this;
    int word_shift = bits / 32;
    int bit_shift = bits % 32;
    BigInt r;
    r.data.clear();
    // insert word_shift zeros at low end
    r.data.insert(r.data.end(), word_shift, 0u);
    uint64_t carry = 0;
    for (size_t i = 0; i < data.size(); ++i)
    {
        uint64_t cur = (uint64_t)data[i] << bit_shift;
        uint64_t sum = cur | carry;
        r.data.push_back(uint32_t(sum & MASK));
        carry = sum >> 32;
    }
    if (carry)
        r.data.push_back(uint32_t(carry));
    return r;
}

// shift-right by arbitrary bits, return BigInt
BigInt BigInt::shr_bits(int bits) const
{
    if (bits == 0)
        return *this;
    int word_shift = bits / 32;
    int bit_shift = bits % 32;
    if ((int)data.size() <= word_shift)
        return BigInt(0);
    BigInt r;
    r.data.clear();
    // start from word_shift
    uint64_t carry = 0;
    for (int i = int(data.size()) - 1; i >= word_shift; --i)
    {
        uint64_t cur = data[i];
        if (bit_shift == 0)
        {
            uint32_t val = uint32_t(cur);
            r.data.insert(r.data.begin(), val);
        }
        else
        {
            uint64_t low = (cur >> bit_shift) | (carry << (32 - bit_shift));
            r.data.insert(r.data.begin(), uint32_t(low & MASK));
            carry = cur & ((1ULL << bit_shift) - 1ULL);
        }
    }
    r.normalize();
    return r;
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
    {
        r.data.push_back(uint32_t(carry));
    }
    return r;
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
        {
            borrow = 0;
        }
        r.data[i] = uint32_t(d & MASK);
    }
    return r;
}

BigInt BigInt::operator*(const BigInt &other) const
{
    size_t na = data.size();
    size_t nb = other.data.size();
    BigInt r;
    r.data.assign(na + nb, 0);
    for (size_t i = 0; i < na; ++i)
    {
        uint64_t carry = 0;
        for (size_t j = 0; j < nb; ++j)
        {
            uint64_t cur = uint64_t(data[i]) * uint64_t(other.data[j]);
            uint64_t sum = uint64_t(r.data[i + j]) + cur + carry;
            r.data[i + j] = uint32_t(sum & MASK);
            carry = sum >> 32;
        }
        size_t k = i + nb;
        while (carry)
        {
            if (k >= r.data.size())
                r.data.push_back(0);
            uint64_t sum = uint64_t(r.data[k]) + carry;
            r.data[k] = uint32_t(sum & MASK);
            carry = sum >> 32;
            ++k;
        }
    }
    r.normalize();
    return r;
}

BigInt BigInt::operator/(const BigInt &other) const
{
    BigInt q, r;
    divmod(other, q, r);
    return q;
}

BigInt BigInt::operator%(const BigInt &mod) const
{
    // Use divmod via Knuth D: compute quotient and remainder, return remainder
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

    BigInt q, r;
    divmod(mod, q, r);
    return r;
}

// Compute quotient and remainder: *this / divisor = quotient, remainder
void BigInt::divmod(const BigInt &divisor, BigInt &quotient, BigInt &remainder) const
{
    // Prepare normalized copies so we can detect actual word sizes and avoid
    // calling __builtin_clz on zero.
    BigInt u = *this;
    BigInt v = divisor;
    u.normalize();
    v.normalize();

    // handle zero divisor
    if (v.data.size() == 0 || (v.data.size() == 1 && v.data[0] == 0))
        throw runtime_error("divide by zero");

    // if dividend < divisor
    if (u < v)
    {
        quotient = BigInt(0);
        remainder = u;
        remainder.normalize();
        return;
    }

    // Shortcut: single-word divisor
    if (v.data.size() == 1)
    {
        uint64_t d = v.data[0];
        quotient.data.assign(u.data.size(), 0);
        uint64_t rem = 0;
        for (int i = int(u.data.size()) - 1; i >= 0; --i)
        {
            uint64_t cur = (rem << 32) | u.data[i];
            uint64_t qq = cur / d;
            rem = cur % d;
            quotient.data[i] = uint32_t(qq & MASK);
        }
        quotient.normalize();
        remainder = BigInt(uint32_t(rem & MASK));
        return;
    }

    // General Knuth D algorithm
    // u and v are normalized copies with no leading zero words
    // normalize so highest bit of v is set
    uint32_t v_high = v.data.back();
    int s = __builtin_clz(v_high);
    if (s > 0)
    {
        u = u.shl_bits(s);
        v = v.shl_bits(s);
    }

    size_t m = v.data.size();
    // ensure u has one extra word
    u.data.push_back(0);
    size_t n = u.data.size();
    size_t k = n - m; // number of quotient words
    quotient.data.assign(k, 0);

    for (int j = int(k) - 1; j >= 0; --j)
    {
        // estimate qhat using top two words of u
        uint64_t numerator = ((uint64_t)u.data[j + m] << 32) | (uint64_t)u.data[j + m - 1];
        uint64_t v_m1 = v.data[m - 1];
        uint64_t qhat = numerator / v_m1;
        uint64_t rhat = numerator % v_m1;

        // correction loop
        while (qhat >= (1ULL << 32) || (uint64_t)qhat * (uint64_t)v.data[m - 2] > (((uint64_t)rhat << 32) | (uint64_t)u.data[j + m - 2]))
        {
            qhat -= 1;
            rhat += v_m1;
            if (rhat >= (1ULL << 32))
                break;
        }

        // multiply v by qhat and subtract from u at position j
        uint64_t borrow = 0;
        for (size_t i = 0; i < m; ++i)
        {
            uint64_t p = (uint64_t)qhat * (uint64_t)v.data[i];
            uint64_t p_lo = p & MASK;
            uint64_t p_hi = p >> 32;
            uint64_t cur = (uint64_t)u.data[j + i];
            uint64_t sub = cur - p_lo - borrow;
            u.data[j + i] = uint32_t(sub & MASK);
            borrow = p_hi + ((cur < p_lo + borrow) ? 1ULL : 0ULL);
        }
        uint64_t cur = (uint64_t)u.data[j + m];
        uint64_t sub = cur - borrow;
        u.data[j + m] = uint32_t(sub & MASK);

        if (cur < borrow)
        {
            // qhat was too big; add back v
            qhat -= 1;
            uint64_t carry2 = 0;
            for (size_t i = 0; i < m; ++i)
            {
                uint64_t sum = (uint64_t)u.data[j + i] + (uint64_t)v.data[i] + carry2;
                u.data[j + i] = uint32_t(sum & MASK);
                carry2 = sum >> 32;
            }
            u.data[j + m] = uint32_t(((uint64_t)u.data[j + m] + carry2) & MASK);
        }
        quotient.data[j] = uint32_t(qhat & MASK);
    }

    // remainder: shift right s bits
    remainder = u.shr_bits(s);
    remainder.normalize();
    quotient.normalize();
}

// ===== Decimal conversion =====
std::string BigInt::to_decimal() const
{
    // Use base 1e9 division to reduce number of divmod iterations
    BigInt zero(0);
    if (*this == zero)
        return string("0");
    BigInt tmp = *this;
    tmp.normalize();
    const uint32_t BASE_DEC = 1000000000u; // 1e9
    BigInt baseDec(BASE_DEC);
    std::vector<uint32_t> parts;
    while (!(tmp == zero))
    {
        BigInt q = tmp / baseDec;
        BigInt r = tmp % baseDec;
        uint32_t rem = (r.data.empty() ? 0u : r.data[0]);
        parts.push_back(rem);
        tmp = q;
    }
    // format parts into decimal string
    std::string out;
    if (!parts.empty())
    {
        // most significant part
        out += std::to_string(parts.back());
        for (int i = int(parts.size()) - 2; i >= 0; --i)
        {
            // pad with leading zeros to width 9
            char buf[16];
            snprintf(buf, sizeof(buf), "%09u", parts[i]);
            out += buf;
        }
    }
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
    // Xuất dạng thập phân (dùng to_decimal)
    BigInt v = val;
    v.normalize();
    out << v.to_decimal();
    return out;
}
