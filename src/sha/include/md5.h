#ifndef MD5_H
#define MD5_H
#include <functional>
#include <vector>
#include <sys/types.h>

struct md5_step {
    u_int32_t blk_index;
    std::vector<u_int32_t> block;
    u_int8_t round;
    u_int32_t a, b, c, d;
    u_int32_t f;
    u_int32_t inner_sum;
    u_int32_t temp;
};

std::vector<u_int8_t> md5(const std::vector<u_int8_t>& input);
void md5(u_int8_t *input, u_int16_t len, u_int8_t *output);
std::vector<u_int8_t> md5_trace(
    const std::vector<u_int8_t>& input,
    const std::function<void(const md5_step&)>& stream = nullptr
);

#endif
