#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <type_traits>
#include <cstring>

struct BitWriter {
    std::vector<uint8_t> buffer;
    uint32_t bitpos = 0; // position in bits

    void writeBits(uint32_t value, int nbits) {
        if (nbits <= 0) return;
        // ensure capacity
        uint32_t needBits = bitpos + nbits;
        if ((needBits + 7) / 8 > buffer.size()) {
            buffer.resize((needBits + 7) / 8, 0);
        }
        // write LSB-first into stream
        for (int i = 0; i < nbits; ++i) {
            uint32_t bit = (value >> i) & 1u;
            uint32_t byteIndex = (bitpos >> 3);
            uint32_t bitIndex  = (bitpos & 7);
            buffer[byteIndex] = static_cast<uint8_t>(buffer[byteIndex] | (bit << bitIndex));
            ++bitpos;
        }
    }

    template<typename T>
    void writeRaw(const T& v) {
        static_assert(std::is_trivially_copyable<T>::value, "writeRaw needs trivially copyable");
        // align to next byte boundary
        if (bitpos & 7) bitpos += (8 - (bitpos & 7));
        size_t off = buffer.size();
        buffer.resize(off + sizeof(T));
        std::memcpy(buffer.data() + off, &v, sizeof(T));
        bitpos += static_cast<uint32_t>(sizeof(T) * 8);
    }

    void writeFloat(float f) {
        static_assert(sizeof(float)==4, "float not 32-bit?");
        uint32_t u;
        std::memcpy(&u, &f, 4);
        writeRaw(u);
    }

    bool writeToFile(const std::string& path) {
        std::ofstream ofs(path, std::ios::binary);
        if (!ofs) return false;
        // flush partial byte if any
        size_t nbytes = (bitpos + 7) / 8;
        ofs.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(nbytes));
        return ofs.good();
    }

    size_t sizeBytes() const {
        return (bitpos + 7) / 8;
    }
};

struct BitReader {
    const std::vector<uint8_t>* buf = nullptr;
    uint32_t bitpos = 0; // position in bits

    BitReader() = default;
    explicit BitReader(const std::vector<uint8_t>& b) : buf(&b), bitpos(0) {}

    static bool readFile(const std::string& path, std::vector<uint8_t>& out) {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs) return false;
        ifs.seekg(0, std::ios::end);
        std::streamsize n = ifs.tellg();
        if (n < 0) return false;
        ifs.seekg(0, std::ios::beg);
        out.resize(static_cast<size_t>(n));
        if (n > 0) ifs.read(reinterpret_cast<char*>(out.data()), n);
        return ifs.good();
    }

    uint32_t bitsRemaining() const {
        if (!buf) return 0;
        return static_cast<uint32_t>(buf->size() * 8) - bitpos;
    }

    bool eof() const {
        return !buf || bitpos >= buf->size() * 8;
    }

    uint32_t readBits(int nbits) {
        if (!buf || nbits <= 0) return 0;
        uint32_t out = 0;
        for (int i = 0; i < nbits; ++i) {
            if (bitpos >= buf->size() * 8) break;
            uint32_t byteIndex = (bitpos >> 3);
            uint32_t bitIndex  = (bitpos & 7);
            uint32_t bit = ((*buf)[byteIndex] >> bitIndex) & 1u;
            out |= (bit << i);
            ++bitpos;
        }
        return out;
    }

    template<typename T>
    T readRaw() {
        static_assert(std::is_trivially_copyable<T>::value, "readRaw needs trivially copyable");
        // align to next byte boundary
        if (bitpos & 7) bitpos += (8 - (bitpos & 7));
        T v{};
        size_t byteIndex = bitpos >> 3;
        if (!buf || byteIndex + sizeof(T) > buf->size()) return v;
        std::memcpy(&v, buf->data() + byteIndex, sizeof(T));
        bitpos += static_cast<uint32_t>(sizeof(T) * 8);
        return v;
    }

    float readFloat() {
        uint32_t u = readRaw<uint32_t>();
        float f;
        std::memcpy(&f, &u, 4);
        return f;
    }
};
