#include "common.h"
#include "md5.h"

#include <vector>
#include <bit>
#include <functional>
#include <iostream>

std::vector<u_int8_t> md5_trace(const std::vector<u_int8_t>& input, const std::function<void(const md5_step&)>& stream) {
    u_int64_t msg_len = input.size();

    // 1. Padding Logic
    // This adds the number of 8-bit messages to an additional 8 8-bit messages
    // This would mean an additional 64 bits to store the length of the original message in bits
    // Then it's divided by 64 (>>6) to get the number of byte blocks, and 1 is added to account for the padding block
    // The byte blocks are 64 byte in length or 512-bit in length.
    u_int32_t num_of_blocks = ((msg_len + 8) >> 6) + 1;
    // The total length is the number of blocks * 64, which would get the total
    // number of byte-blocks.
    u_int32_t total_length = num_of_blocks << 6;

    // Padded bytes is just the total length(including length and padding) - actual message length
    // This would give a number in bytes for the padded bytes + message length
    std::vector<u_int8_t> padded_bytes(total_length - msg_len, 0);
    // The first value of the padded bytes is 0111 0000
    padded_bytes[0] = 0x80;

    // This is the message length converted to bits, msg_len * 8
    u_int64_t msg_len_bits = msg_len << 3;
    // Firstly we fill in the last 8 bytes of the padded bytes with the message length
    // size - 8 + i, starts from the first point and moves up till the end
    // msg_len_bits >> i * 8, just shifts the number by i * 8 to get the next number
    // so it fills from the lower endian to the highest
    for (u_int32_t i = 0; i < 8; ++i) {
        padded_bytes[padded_bytes.size() - 8 + i] = static_cast<u_int8_t>(msg_len_bits >> (i * 8));
    }

    // 2. Initialize Registers
    u_int32_t a = INIT_A, b = INIT_B, c = INIT_C, d = INIT_D;
    std::vector<u_int32_t> buffer(16);

    // 3. Main Block Loop
    // For each 64-byte block or 512-bit block
    for (u_int32_t i = 0; i < num_of_blocks; ++i) {
        // block offset is the current block * 64 to get where the block-byte starts
        // so 0 -> 0 and 1 -> 64, 2 -> 128, etc...
        u_int32_t block_offset = i << 6;

        // Pack bytes into 32-bit words (Little Endian)
        // For each byte in the 64-byte block we get the block_offset we computed + the current index
        // if that value is less than the msg_len (in bytes) we just get that byte value
        // if it's not less than that, it means we are going to get the padded values instead
        // which is the block_offset + j - msg_len which would give an index of the byte in the padded values
        for (u_int32_t j = 0; j < 64; ++j) {
            u_int32_t byte_val = block_offset + j < msg_len ? input[block_offset + j] : padded_bytes[block_offset + j - msg_len];

            // For each j, we divide by 4(>>2) to get the current 32-bit word we are working with
            // So for 0 -> 0, 1 -> 0, 2 -> 0, 3 -> 0, 4 -> 0 and so no.
            // this would bundle the 64-byte blocks into 32-bit words.
            buffer[j >> 2] = (byte_val << 24) | (buffer[j >> 2] >> 8);
        }

        // Initialize the variables used later to compute the result
        u_int32_t o_a = a, o_b = b, o_c = c, o_d = d;

        // For each of those words we run 64 rounds
        // 4. The 64 Rounds
        for (u_int8_t j = 0; j < 64; ++j) {
            // For each round in the 64-byte block, we divide the value or j by 16(>>4)
            // meaning the possible values for div_16 are (0, 1, 2, 3)
            u_int8_t div_16 = j >> 4;
            u_int32_t f = 0;
            u_int32_t buf_index = j;

            // The possible values for buf_index:
            // case 0 -> buf_index = 1 -> 15;
            // case 1 -> The initial range is buf_index = 16 -> 31
            // but 16 * 5 + 1 = 81 and 31 * 5 + 1 = 156
            // so that the possible values [81 -> 156(not consecutive)] would always be ANDed with 0x0F
            // so that buf_index = [1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12]
            // case 2 -> the values of j -> [32 -> 47], thus the possible values based on
            // buf_index * 3 + 5 = [109 -> 146], thus the values are
            // buf_index = [5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2]
            // case 3 -> the values of j -> [48 -> 63], thus the possible values based on
            // buf_index * 7 = [336 -> 441], thus the values for
            // buf_index = [0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9]
            switch (div_16) {
                case 0: f = (b & c) | (~b & d); break;
                // buf_index here is a value dependent on j and these constants
                case 1: f = (b & d) | (c & ~d); buf_index = (buf_index * 5 + 1) & 0x0F; break;
                case 2: f = b ^ c ^ d;          buf_index = (buf_index * 3 + 5) & 0x0F; break;
                case 3: f = c ^ (b | ~d);       buf_index = (buf_index * 7) & 0x0F; break;
            }

            u_int32_t inner_sum = a + f + buffer[buf_index] + K[j];
            u_int32_t rotate_amt = shifts[(div_16 << 2) | (j & 3)];
            u_int32_t temp = b + std::rotl(inner_sum, rotate_amt);

            // Capture state BEFORE the shift for the stream
            if (stream) {
                auto data = md5_step{i, buffer, j, a, b, c, d, f, inner_sum, temp};
                stream(data);
            }

            // Shift registers
            a = d; d = c; c = b; b = temp;
        }

        a += o_a; b += o_b; c += o_c; d += o_d;
    }

    // 5. Final Output Packing
    std::vector<u_int8_t> output(16);
    u_int32_t regs[4] = {a, b, c, d};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            output[(i * 4) + j] = static_cast<u_int8_t>(regs[i] >> (j * 8));
        }
    }
    return output;
}

std::vector<u_int8_t> md5(const std::vector<u_int8_t>& input) {
    return md5_trace(input);
}
