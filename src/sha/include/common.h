#ifndef COMMON_H
#define COMMON_H
#include <algorithm>
#include <iomanip>
#include <vector>

constexpr u_int32_t INIT_A = 0x67452301;
constexpr u_int32_t INIT_B = 0xEFCDAB89;
constexpr u_int32_t INIT_C = 0x98BADCFE;
constexpr u_int32_t INIT_D = 0x10325476;

constexpr u_int8_t shifts[] = {
    7, 12, 17, 22,
    5,  9, 14, 20,
    4, 11, 16, 23,
    6, 10, 15, 21
};

constexpr u_int32_t K[] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

inline std::string to_hex(const u_int8_t *str, const u_int32_t len) {
    static const char* digits = "0123456789abcdef";

    std::string hex;
    hex.reserve(len * 2);
    for (u_int32_t i = 0; i < len; ++i) {
        auto ch = static_cast<u_int8_t>(str[i]);
        hex.push_back(digits[ch >> 4]);
        hex.push_back(digits[ch & 0x0f]);
    }
    return hex;
}

inline std::string to_hex(std::vector<u_int8_t> data) {
    return to_hex(data.data(), data.size());
}

inline u_int8_t hex_value(const char& c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + (c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return 10 + (c - 'A');
    }
    // invalid value
    return 0;
}

inline std::vector<u_int8_t> to_bytes(const std::string& str) {
    std::vector<u_int8_t> bytes;
    bytes.reserve(str.size() / 2);
    u_int32_t i = 0;
    while (i < str.size()) {
        const auto ch_1 = hex_value(str[i++]);
        const auto ch_2 = hex_value(str[i++]);
        bytes.push_back(((ch_1 << 4) & 0xf0) | (ch_2 & 0x0f));
    }
    return bytes;
}

inline std::vector<u_int8_t> to_byte_vector(const std::string& str) {
    std::vector<u_int8_t> bytes;
    bytes.reserve(str.size());
    std::ranges::transform(str, std::back_inserter(bytes), [](const char c) { return static_cast<u_int8_t>(c); });
    return bytes;
}

#endif
