#pragma once
#include <vector>
#include <complex>
#include "kiss_fft.h"

class FFTProcessor {
public:
    FFTProcessor(int n);
    ~FFTProcessor();

    void forward(const std::vector<float>& input, std::vector<std::complex<float>>& output);
    void inverse(const std::vector<std::complex<float>>& input, std::vector<float>& output);

private:
    int nfft;
    kiss_fft_cfg fwd_cfg;
    kiss_fft_cfg inv_cfg;
};
