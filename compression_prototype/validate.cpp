#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <string>
#include "wav_io.h"
#include "bitpack.h"

// Compute error metrics
struct Metrics {
    double mse;
    double snr;
    double psnr;
    float peak_err;
};

static Metrics compute_metrics(const std::vector<float> &orig, const std::vector<float> &dec) {
    size_t n = std::min(orig.size(), dec.size());
    double err_sum = 0.0, sig_sum = 0.0;
    float peak_err = 0.0f;
    double max_orig = 0.0;

    for (size_t i = 0; i < n; i++) {
        double e = orig[i] - dec[i];
        err_sum += e * e;
        sig_sum += orig[i] * orig[i];
        peak_err = std::max(peak_err, static_cast<float>(std::fabs(e)));
        max_orig = std::max(max_orig, std::fabs(static_cast<double>(orig[i])));
    }

    double mse = err_sum / n;
    double snr = 10.0 * log10((sig_sum + 1e-12) / (err_sum + 1e-12));
    double psnr = 10.0 * log10((max_orig * max_orig + 1e-12) / (mse + 1e-12));
    return {mse, snr, psnr, peak_err};
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: validate <original.wav> <decoded.wav>\n";
        return 1;
    }

    std::string origFile = argv[1];
    std::string decFile  = argv[2];

    // Prepare WavData
    WavData orig, dec;

    if (!read_wav(origFile, orig)) {
        std::cerr << "Failed to load original wav: " << origFile << "\n";
        return 1;
    }
    if (!read_wav(decFile, dec)) {
        std::cerr << "Failed to load decoded wav: " << decFile << "\n";
        return 1;
    }

    if (orig.samples.empty() || dec.samples.empty()) {
        std::cerr << "Empty audio data.\n";
        return 1;
    }

    // Compute metrics
    Metrics m = compute_metrics(orig.samples, dec.samples);

    // Print validation report
    std::cout << "Validation Report\n=================\n";
    std::cout << "Samples compared: " << std::min(orig.samples.size(), dec.samples.size()) << "\n";
    std::cout << "Durations: orig=" 
              << (orig.samples.size() / static_cast<double>(orig.sample_rate)) << " s, "
              << "dec=" << (dec.samples.size() / static_cast<double>(dec.sample_rate)) << " s\n\n";

    auto compute_peak_rms = [](const std::vector<float>& data) {
        float peak = 0.0f, rms = 0.0f;
        for (float s : data) {
            peak = std::max(peak, std::fabs(s));
            rms += s * s;
        }
        rms = std::sqrt(rms / data.size());
        return std::make_pair(peak, rms);
    };

    auto [orig_peak, orig_rms] = compute_peak_rms(orig.samples);
    auto [dec_peak, dec_rms]   = compute_peak_rms(dec.samples);

    std::cout << "Original: Peak=" << orig_peak << " RMS=" << orig_rms << "\n";
    std::cout << "Decoded:  Peak=" << dec_peak   << " RMS=" << dec_rms   << "\n\n";

    std::cout << "Error Metrics:\n";
    std::cout << "  MSE:   " << m.mse << "\n";
    std::cout << "  SNR:   " << m.snr << " dB\n";
    std::cout << "  PSNR:  " << m.psnr << " dB\n";
    std::cout << "  Peak Error: " << m.peak_err << "\n";

    return 0;
}
