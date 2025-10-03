#pragma once
#include <vector>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>

namespace wofl {

class BitWriter {
    std::vector<uint8_t> buffer;
    size_t bytepos = 0;
    int bitpos = 0; // 0–7
public:
    BitWriter() : buffer(1, 0), bytepos(0), bitpos(0) {}

    // write a single bit
    void put_bit(int bit) {
        if (bit) buffer[bytepos] |= (1u << bitpos);
        bitpos++;
        if (bitpos == 8) {
            buffer.push_back(0);
            bytepos++;
            bitpos = 0;
        }
    }

    // write multiple bits from LSB of value
    void put_bits(uint32_t value, int nbits) {
        for (int i = 0; i < nbits; i++) {
            put_bit((value >> i) & 1);
        }
    }

    // convenience for full words
    void write32(uint32_t v) {
        put_bits(v, 32);
    }

    size_t bytes_written() const { return buffer.size(); }

    void save(const std::string &filename) {
        std::ofstream f(filename, std::ios::binary);
        if (!f) throw std::runtime_error("Cannot open file for writing");
        f.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    }
};

class BitReader {
    std::vector<uint8_t> buffer;
    size_t bytepos = 0;
    int bitpos = 0;
public:
    BitReader(const std::string &filename) {
        std::ifstream f(filename, std::ios::binary);
        if (!f) throw std::runtime_error("Cannot open file for reading");
        buffer.assign(std::istreambuf_iterator<char>(f), {});
        if (buffer.empty()) throw std::runtime_error("Empty bitstream file");
    }

    int get_bit() {
        if (bytepos >= buffer.size()) throw std::runtime_error("Read past end of bitstream");
        int bit = (buffer[bytepos] >> bitpos) & 1;
        bitpos++;
        if (bitpos == 8) { bitpos = 0; bytepos++; }
        return bit;
    }

    uint32_t get_bits(int nbits) {
        uint32_t v = 0;
        for (int i = 0; i < nbits; i++) {
            v |= (get_bit() << i);
        }
        return v;
    }

    uint32_t read32() { return get_bits(32); }
};

} // namespace wofl
