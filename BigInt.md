# BigInt — Tài liệu chi tiết

Tệp: `BigInt.cpp`, `BigInt.h`

Mục tiêu
- Mô tả chi tiết kiểu dữ liệu `BigInt` trong repo, cách lưu trữ, các toán tử, thuật toán sử dụng, độ phức tạp và các lưu ý khi sử dụng.

1) Tóm tắt nhanh
- `BigInt` lưu số nguyên không dấu dưới dạng `std::vector<uint32_t> data` little-endian.
- `BigInt::BIT_SIZE` là kích thước bit tối đa (cấu hình tĩnh). Mỗi BigInt có kích thước biến đổi (dynamic) nhưng bị giới hạn bởi BIT_SIZE.
- Các toán tử: +, -, *, /, % được triển khai bằng các thuật toán cơ bản (schoolbook, long division shift-subtract).

2) Cấu trúc lưu trữ
- `data[0]` chứa 32-bit thấp nhất.
- `BASE = 2^32`, `MASK = BASE - 1`.
- `word_count() = (BIT_SIZE + 31) / 32` là số từ tối đa cho mỗi BigInt.
- `normalize()` loại bỏ các từ cao bằng 0 (giữ canonical representation).
- `enforce_max_bits(BigInt &x)` đảm bảo BigInt không vượt quá `BIT_SIZE`:
  - resize xuống `word_count()` nếu vector quá dài
  - mask các bit cao vượt quá `BIT_SIZE` trong từ cao nhất
  - gọi `normalize()` để rút gọn nếu cần

3) Các hàm khởi tạo (constructors)
- `BigInt()` — khởi tạo 0 với `word_count()` từ `BIT_SIZE`.
- `BigInt(uint32_t val)` — khởi tạo từ 32-bit.
- `BigInt(const std::string &decimal)` — parse chuỗi thập phân bằng cách lặp: this = this * 10 + digit.
  - Lưu ý: parse bỏ các ký tự không phải chữ số.

4) Toán tử so sánh
- `operator==`, `operator<` so sánh theo từ từ cao xuống thấp (so sánh canonical bằng cách giả sử các phần không tồn tại là 0).

5) Toán tử số học
- `operator+` (cộng word-by-word với carry): O(n) với n = số word.
  - Cuối cùng gọi `enforce_max_bits` để cắt overflow.
- `operator-` (trừ với borrow): O(n), giả thiết *this >= other.
  - Kết quả được enforce_max_bits.
- `operator*` (schoolbook multiplication): O(na * nb)
  - Sử dụng uint64_t để chứa product của hai word 32-bit.
  - Sau inner loop propagate carry ra các từ cao hơn; kết quả sau đó bị enforce_max_bits.
- `operator/` và `operator%` (division): shift-subtract division
  - Tính msb_index(dividend) và msb_index(divisor), dịch divisor lên gần msb rồi dịch phải từng bit,
    subtract nếu dividend >= shifted_divisor.
  - Độ phức tạp: O(n * bits), chậm so với thuật toán Knuth-D cho very large numbers.
  - Kết quả bị enforce_max_bits.

6) Chuyển đổi ra chuỗi thập phân
- `to_decimal()` hiện dùng cơ chế divmod với base 1e9 (1000000000) để giảm số lần chia so với base 10.
  - Thuật toán: lặp divmod(tmp, 1e9) thu thập phần dư (các block 9 chữ số), sau đó ghép các block bằng padding 9 chữ số.
  - Hiệu quả tốt hơn đáng kể so với chia cho 10 mỗi lần.
  - Vì `operator/`/`operator%` đang dùng shift-subtract, `to_decimal()` vẫn phụ thuộc vào hiệu năng của division.

7) Hàm trợ giúp chính
- `msb_index(const BigInt &x)`: tìm bit 1 cao nhất, trả về -1 nếu x == 0.
- `shl_bits(const BigInt &x, int bits)`: dịch trái theo số bit (trả BigInt mới), gọi `enforce_max_bits` sau cùng.
- `shr1(BigInt &x)`: dịch phải 1 bit in-place, gọi `normalize()`.
- `set_bit(BigInt &x, int bit)`: set bit nếu bit < BIT_SIZE; không resize vượt quá limit.

8) Độ phức tạp (tóm tắt)
- Addition/Subtraction: O(n)
- Multiplication: O(n^2) (schoolbook)
- Division/Modulo: O(n * bits) — có thể cải thiện bằng Knuth-D
- Decimal conversion (to_decimal): số lần lặp ≈ ceil(bits / log2(1e9)); mỗi bước gọi division base 1e9

9) Ví dụ nhỏ
- Lưu 0x11223344_55667788:
  - data[0] = 0x55667788, data[1] = 0x11223344
- (2^32 - 1)^2 = 18446744065119617025: kiểm tra multi-word multiplication

10) Những lưu ý thực tế
- Nếu bạn cần số âm, hiện code không hỗ trợ.
- Nếu bạn cần arithmetic modulo 2^BIT_SIZE thì hiện đã thực hiện truncate; nếu muốn phát hiện overflow hãy đổi hành vi.
- For crypto: hiện chưa constant-time, chưa Montgomery; RNG dùng mt19937_64 — không phải CSPRNG.

11) Gợi ý cải tiến tiếp theo
- Implement `divmod_by_uint32(uint32_t d)` để tăng tốc `to_decimal()` (divmod_by_uint32 trả trực tiếp quotient và remainder bằng thuật toán word-level, nhanh hơn shift-subtract khi chia cho 32-bit divisor).
- Implement Knuth-D division cho hiệu năng tốt hơn khi chia hai BigInt lớn.
- Nếu cần multiplication nhanh: Karatsuba / Toom / FFT.
- Document rõ invariant: dynamic nhưng giới hạn bởi `BIT_SIZE`.

---

Nếu bạn muốn, tôi sẽ tiếp tục và:
- (A) thêm ví dụ chạy từng bước vào tài liệu,
- (B) implement `divmod_by_uint32` và thay `to_decimal()` để nhanh hơn,
- (C) thay `operator/` bằng Knuth-D.

Chọn A, B hoặc C hoặc yêu cầu khác nhé.
