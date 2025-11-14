## Mô tả và hướng dẫn sử dụng `BigInt`

Tài liệu ngắn gọn bằng tiếng Việt, mô tả biểu diễn, các toán tử chính, ví dụ, độ phức tạp và vài gợi ý cải tiến.

1) Biểu diễn nội bộ
- `data: std::vector<uint32_t>` — các từ 32‑bit theo little‑endian (`data[0]` là LSW).
- `BASE = 2^32`, `MASK = BASE-1`.
- Gọi `normalize()` để xóa word cao bằng 0; `data` luôn có ít nhất một phần tử (0 cho số 0).

2) Khởi tạo
- `BigInt()` — 0. Ví dụ: `BigInt a;` (O(1)).
- `BigInt(uint32_t v)` — tạo từ word. Ví dụ: `BigInt b(12345);` (O(1)).
- `BigInt(const std::string &dec)` — chuyen tu string so thập phân bằng nhân dồn *10 + digit (O(L·n)). 

3) Chuẩn hoá
- `BigInt& normalize()` — xóa các từ cao bằng 0; đảm bảo `data` không rỗng (O(n)).

4) Dịch bit
- `shl_bits(int k)` — dịch trái k bit (word_shift + bit_shift). Ví dụ: `a.shl_bits(20)` (O(n + k/32)).
- `shr_bits(int k)` — dịch phải k bit; đã tối ưu tránh chèn ở đầu vector (O(n + k/32)).

5) So sánh
- `operator==` và `operator<` — so sánh từ MSW → LSW hoặc word‑theo‑word (O(n)).

6) Cộng / Trừ
- `operator+` — cộng word‑theo‑word với carry (O(n)).
- `operator-` — trừ theo borrow; API chỉ hỗ trợ `*this >= rhs` (có assert); kết quả được `normalize()` (O(n)).

7) Nhân
- `operator*` — schoolbook multiply dùng `uint64_t` intermediate. Ví dụ: `c = a * b` (O(n^2)).

8) Chia / Modulo
- `divmod(divisor, quotient, remainder)` — Knuth D cho divisor nhiều word; chia nhanh cho divisor 1‑word. Ví dụ: `a.divmod(b, q, r)` (O(n·m)).

9) Chuyển đổi thập phân
- `to_decimal()` — chia lặp theo base 1e9 và ghép chuỗi. Ví dụ: `std::cout << a.to_decimal();`.

10) I/O
- `operator>>` đọc chuỗi thập phân; `operator<<` in kết quả `to_decimal()`.

11) Ghi chú kỹ thuật ngắn
- `normalize()` đảm bảo `data` không rỗng.
- `operator-` có assert để phát hiện underflow.
- `shr_bits` đã được tối ưu để tránh O(n^2) khi dịch lớn.
- `divmod` dùng helper `leading_zeros` và đảm bảo `quotient` không rỗng.

12) Gợi ý cải tiến
- Montgomery multiplication cho modexp/Miller‑Rabin (tăng tốc trình nhân modulo).
- Windowed exponentiation (giảm số nhân trong modexp).
- Karatsuba/Toom cho sizes lớn.
- Đổi sang word 64‑bit trên nền 64‑bit để giảm số từ.

13) Bảng độ phức tạp tóm tắt
- Addition/Subtraction: O(n)
- Multiplication: O(n^2)
- Division/Modulo: O(n·m)
- Shifts: O(n)
- to_decimal: O(n · digits/9)
