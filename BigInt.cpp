// BigInt.cpp (updated per requirements)
#include "BigInt.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>

using namespace std;

/*
 BigInt - chú thích tóm tắt (Tiếng Việt)

 Nguyên lý lưu trữ (representation):
 - Dùng vector<uint32_t> `data` lưu các từ 32-bit theo thứ tự little-endian.
     data[0] là 32 bit thấp nhất (least-significant word), data[1] là tiếp theo, v.v.
 - Hằng số BASE = 2^32; MASK = BASE - 1 dùng để mask 32-bit thấp.
 - `BigInt::BIT_SIZE` (static) xác định kích thước bit tối đa cho các BigInt trong chương trình.
     Hàm `word_count()` trả số words tương ứng (ceil(BIT_SIZE/32)).

 Thiết kế hành vi:
 - BigInt trong repo hoạt động theo chế độ "dynamic nhưng bị giới hạn":
         + Các BigInt có thể co lại (normalize() loại bỏ các word cao bằng 0),
         + Nhưng không được mở rộng vượt quá `BIT_SIZE` (các hàm gọi `enforce_max_bits` để cắt/mask kết quả).
 - Các phép toán chính (+, -, *, /, %) được triển khai bằng các thuật toán cổ điển
     (schoolbook multiplication, shift-subtract division) để rõ mã và dễ kiểm thử.

 Mục đích file:
 - Cung cấp các toán tử toán học cơ bản cho BigInt không dấu, các hàm trợ giúp
     (msb_index, shl_bits, shr1, set_bit) và các I/O cơ bản.

 Lưu ý quan trọng:
 - Hàm operator- giả sử *this >= other (không xử lý số âm).
 - Khi cần arithmetic modulo 2^BIT_SIZE (ví dụ trong một số ứng dụng crypto),
     hiện tại BigInt sẽ tự động cắt phần cao; nếu muốn phát hiện overflow thay
     vì cắt thì cần thay đổi enforce_max_bits để throw hoặc báo lỗi.

*/

int BigInt::BIT_SIZE = 512; // mặc định, có thể thay đổi trước khi tạo số

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
    // normalize() removed — trimming is intentionally omitted per refactor
}

// ===== Utility =====
BigInt &BigInt::normalize()
{
    // trim high zero words but keep at least one word
    while (data.size() > 1 && data.back() == 0)
        data.pop_back();
    return *this;
}

// Ensure BigInt does not exceed the maximum capacity defined by BigInt::BIT_SIZE.
// Keeps representation dynamic (may shrink) but truncates any excess high words/bits.
static void enforce_max_bits(BigInt &x)
{
    size_t wc = word_count();
    if (x.data.size() > wc)
        x.data.resize(wc);
    // mask top bits beyond BIT_SIZE
    int top_bits = BigInt::BIT_SIZE % 32;
    if (top_bits != 0 && wc > 0)
    {
        uint32_t mask = (top_bits == 32) ? 0xFFFFFFFFu : ((1u << top_bits) - 1u);
        x.data[wc - 1] &= mask;
        // if the top word cleared becomes zero we allow normalize to shrink representation
        x.normalize();
    }
    else
    {
        // also allow trimming of trailing zeros
        x.normalize();
    }
}

// shift-left by an arbitrary number of bits, return new BigInt
static BigInt shl_bits(const BigInt &x, int bits)
{
    if (bits == 0)
        return x;
    int word_shift = bits / 32;
    int bit_shift = bits % 32;
    BigInt r;
    r.data.clear();
    // insert word_shift zeros at low end
    r.data.insert(r.data.end(), word_shift, 0u);
    uint64_t carry = 0;
    for (size_t i = 0; i < x.data.size(); ++i)
    {
        uint64_t cur = (uint64_t)x.data[i] << bit_shift;
        uint64_t sum = cur | carry;
        r.data.push_back(uint32_t(sum & MASK));
        carry = sum >> 32;
    }
    if (carry)
        r.data.push_back(uint32_t(carry));
    enforce_max_bits(r);
    enforce_max_bits(r);
    return r;
}

// shift-right by 1 bit in-place
static void shr1(BigInt &x)
{
    uint32_t carry = 0;
    for (int i = int(x.data.size()) - 1; i >= 0; --i)
    {
        uint64_t cur = (uint64_t)carry << 32 | x.data[i];
        x.data[i] = uint32_t(cur >> 1);
        carry = uint32_t(cur & 1u);
    }
    x.normalize();
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
    enforce_max_bits(r);
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
            borrow = 0;
        r.data[i] = uint32_t(d & MASK);
    }
    enforce_max_bits(r);
    return r;
}

BigInt BigInt::operator*(const BigInt &other) const
{
    size_t na = data.size(),
           nb = other.data.size();
    BigInt r;
    // ensure capacity
    r.data.assign(na + nb, 0);
    for (size_t i = 0; i < na; ++i)
    {
        uint64_t carry = 0;
        for (size_t j = 0; j < nb; ++j)
        {
            // accumulate product + existing + carry
            uint64_t cur = uint64_t(data[i]) * uint64_t(other.data[j]);
            uint64_t sum = uint64_t(r.data[i + j]) + cur + carry;
            r.data[i + j] = uint32_t(sum & MASK);
            carry = sum >> 32;
        }
        // propagate carry into higher words; this may cascade
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
    return r;
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
// get_bit and shl1 removed (not used); shl_bits and shr1 are used instead
static void set_bit(BigInt &x, int bit)
{
    if (bit < 0)
        return;
    if (bit >= BigInt::BIT_SIZE)
        return; // ignore bits beyond max capacity
    size_t wi = size_t(bit / 32);
    int bi = bit % 32;
    size_t wc = word_count();
    if (wi >= wc)
    {
        // should not happen because bit < BIT_SIZE implies wi < wc, but guard anyway
        return;
    }
    if (wi >= x.data.size())
        x.data.resize(wi + 1, 0);
    x.data[wi] |= (1u << bi);
    enforce_max_bits(x);
}

BigInt BigInt::operator/(const BigInt &other) const
{
    // Improved division: shift-subtract method using bit shifts of divisor.
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

    BigInt dividend = *this;
    BigInt quotient(0);

    int msd_dividend = msb_index(dividend);
    int msd_divisor = msb_index(other);
    int shift = msd_dividend - msd_divisor;

    BigInt dshift = shl_bits(other, shift);
    for (int b = shift; b >= 0; --b)
    {
        if (!(dividend < dshift))
        {
            dividend = dividend - dshift;
            set_bit(quotient, b);
        }
        // shift dshift right by 1 for next bit
        shr1(dshift);
    }
    quotient.normalize();
    enforce_max_bits(quotient);
    return quotient;
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

    BigInt dividend = *this;
    int msd_dividend = msb_index(dividend);
    int msd_divisor = msb_index(mod);
    int shift = msd_dividend - msd_divisor;
    BigInt dshift = shl_bits(mod, shift);
    for (int b = shift; b >= 0; --b)
    {
        if (!(dividend < dshift))
        {
            dividend = dividend - dshift;
        }
        shr1(dshift);
    }
    dividend.normalize();
    enforce_max_bits(dividend);
    return dividend;
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
