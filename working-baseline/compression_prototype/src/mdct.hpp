#pragma once
#include <vector>
#include <complex>
#include <cmath>
#include <cassert>
#include "constants.hpp"
#include "fft.hpp"

namespace wofl {

// Fast MDCT/IMDCT using FFT radix-2
struct MDCT {
    size_t N;
    std::vector<float> win;

    MDCT(size_t N_) : N(N_), win(2*N_) {
        assert(N && (N & (N-1)) == 0);
        // Sine window (same as AAC-type IV window)
        for (size_t n=0; n<2*N; ++n) {
            win[n] = std::sin(float(wofl::PI)/(2.0f*N) * (n + 0.5f));
        }
    }

    // Forward MDCT (slow reference, could be optimized similarly if needed)
    void forward(const std::vector<float>& x, size_t pos, std::vector<float>& X) const {
        X.assign(N, 0.0f);
        std::vector<std::complex<float>> temp(2*N);

        for (size_t n=0; n<2*N; ++n) {
            float sample = (pos+n < x.size()) ? x[pos+n] : 0.0f;
            temp[n] = std::complex<float>(sample * win[n], 0.0f);
        }

        fft_radix2(temp, false);

        // Take real part with pre-twiddle
        for (size_t k=0; k<N; ++k) {
            float re = temp[k].real();
            float im = temp[k].imag();
            float angle = float(wofl::PI)/N * (k + 0.5f + N/2.0f);
            X[k] = re*std::cos(angle) + im*std::sin(angle);
        }
    }

    // Inverse MDCT (fast)
    void inverse(const std::vector<float>& X, std::vector<float>& y, size_t pos) const {
        size_t n2 = 2*N;
        std::vector<std::complex<float>> temp(n2);

        // Pre-twiddle & build complex spectrum
        for (size_t k=0; k<N; ++k) {
            float angle = float(wofl::PI)/N * (k + 0.5f);
            float xr = X[k] * std::cos(angle);
            float xi = -X[k] * std::sin(angle);
            temp[k]   = std::complex<float>(xr, xi);
            temp[k+N] = std::complex<float>(-xr, xi); // symmetry
        }

        // IFFT
        fft_radix2(temp, true);

        // Overlap-add with window
        if (y.size() < pos+n2) y.resize(pos+n2, 0.0f);
        for (size_t n=0; n<n2; ++n) {
            float v = 2.0f/N * win[n] * temp[n].real();
            y[pos+n] += v;
        }
    }
};

} // namespace wofl
