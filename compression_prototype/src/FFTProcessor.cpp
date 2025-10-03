#include "FFTProcessor.h"
#include <stdexcept>

FFTProcessor::FFTProcessor(int n) : nfft(n) {
    fwd_cfg = kiss_fft_alloc(nfft, 0, nullptr, nullptr);
    inv_cfg = kiss_fft_alloc(nfft, 1, nullptr, nullptr);
    if (!fwd_cfg || !inv_cfg) throw std::runtime_error("KissFFT alloc failed");
}

FFTProcessor::~FFTProcessor() {
    kiss_fft_free(fwd_cfg);
    kiss_fft_free(inv_cfg);
}

void FFTProcessor::forward(const std::vector<float>& input, std::vector<std::complex<float>>& output) {
    if (input.size() != static_cast<size_t>(nfft))
        throw std::runtime_error("Input size mismatch");

    std::vector<kiss_fft_cpx> out(nfft);
    std::vector<kiss_fft_cpx> in(nfft);

    for (int i = 0; i < nfft; i++) {
        in[i].r = input[i];
        in[i].i = 0.0f;
    }

    kiss_fft(fwd_cfg, in.data(), out.data());

    output.resize(nfft);
    for (int i = 0; i < nfft; i++) {
        output[i] = std::complex<float>(out[i].r, out[i].i);
    }
}

void FFTProcessor::inverse(const std::vector<std::complex<float>>& input, std::vector<float>& output) {
    if (input.size() != static_cast<size_t>(nfft))
        throw std::runtime_error("Input size mismatch");

    std::vector<kiss_fft_cpx> out(nfft);
    std::vector<kiss_fft_cpx> in(nfft);

    for (int i = 0; i < nfft; i++) {
        in[i].r = input[i].real();
        in[i].i = input[i].imag();
    }

    kiss_fft(inv_cfg, in.data(), out.data());

    output.resize(nfft);
    for (int i = 0; i < nfft; i++) {
        output[i] = out[i].r / nfft; // normalize
    }
}
