#pragma once
#include <vector>
#include <cstdint>
#include "bitio.hpp"

namespace wofl {

// stub encoder/decoder for now — just passes samples through
inline void encode_stream(const std::vector<float> &pcm, int sr, int ch,
                          size_t nfft, size_t hop, int K,
                          BitWriter &bw) {
    // For now: quantize trivially and dump
    bw.write32((uint32_t)pcm.size());
    for (float s : pcm) {
        int16_t q = (int16_t)(s * 32767.0f);
        bw.write32((uint16_t)q); // store as 16-bit padded into 32
    }
    // user must call bw.save(filename) outside
}

inline void decode_stream(std::vector<float> &pcm, int sr, int ch,
                          size_t nfft, size_t hop,
                          BitReader &br) {
    size_t nsamp = br.read32();
    pcm.resize(nsamp);
    for (size_t i = 0; i < nsamp; i++) {
        uint16_t q = (uint16_t)br.read32();
        pcm[i] = (int16_t)q / 32768.0f;
    }
}

} // namespace wofl
