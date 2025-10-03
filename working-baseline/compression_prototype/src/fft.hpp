#pragma once
#include <vector>
#include <complex>
#include <cmath>
#include <cassert>
#include "constants.hpp"

namespace wofl {

using cpx = std::complex<float>;

// iterative radix-2 FFT
inline void fft_radix2(std::vector<cpx>& a, bool inverse) {
    const size_t n = a.size();
    assert(n && (n & (n - 1)) == 0);

    // bit-reversal
    size_t j = 0;
    for (size_t i = 1; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }

    for (size_t len = 2; len <= n; len <<= 1) {
        float ang = 2.0f * float(wofl::PI) / float(len) * (inverse ? 1.0f : -1.0f);
        cpx wlen = cpx(std::cos(ang), std::sin(ang));
        for (size_t i = 0; i < n; i += len) {
            cpx w = 1.0f;
            for (size_t j = 0; j < len/2; ++j) {
                cpx u = a[i+j];
                cpx v = a[i+j+len/2] * w;
                a[i+j] = u + v;
                a[i+j+len/2] = u - v;
                w *= wlen;
            }
        }
    }
    if (inverse) {
        for (auto& x : a) x /= float(n);
    }
}

inline void stft_frame(const std::vector<float>& x, size_t pos, size_t N, std::vector<cpx>& out) {
    out.assign(N, cpx(0,0));
    for (size_t n=0; n<N; ++n) {
        float w = 0.5f*(1.0f - std::cos(2.0f * float(wofl::PI) * float(n)/float(N)));
        out[n] = (n+pos < x.size()) ? cpx(x[pos+n]*w, 0.0f) : cpx(0.0f, 0.0f);
    }
    fft_radix2(out, false);
}

inline void istft_frame(std::vector<float>& y, size_t pos, size_t N, const std::vector<cpx>& X) {
    std::vector<cpx> time = X;
    fft_radix2(time, true);
    for (size_t n=0; n<N; ++n) {
        float w = 0.5f*(1.0f - std::cos(2.0f * float(wofl::PI) * float(n)/float(N)));
        float v = time[n].real() * w;
        if (pos+n < y.size()) y[pos+n] += v;
    }
}

} // namespace wofl
