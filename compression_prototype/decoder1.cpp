#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include "wav_io.h"
#include "kiss_fft.h"
#include "bitpack.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: decoder <in.bin> <out.wav>\n";
        return 1;
    }
    std::string inFile = argv[1];
    std::string outFile = argv[2];

    std::vector<uint8_t> buf;
    if (!BitReader::readFile(inFile, buf)) {
        std::cerr << "Failed to read " << inFile << "\n";
        return 1;
    }
    BitReader br(buf);

    const int frame_size   = (int)br.readBits(16);
    const int hop_size     = (int)br.readBits(16);
    const int sample_rate  = (int)br.readBits(32);
    const float peak_ref   = br.readFloat();
    const float rms_ref    = br.readFloat();
    const int ktop_global  = (int)br.readBits(16);
    const int qmag_bits    = (int)br.readBits(8);
    const int qphase_bits  = (int)br.readBits(8);
    const int threshold    = (int)br.readBits(16);
    const int total_frames = (int)br.readBits(32);

    std::cout << "Metadata loaded: frame_size=" << frame_size
              << " hop_size=" << hop_size
              << " sample_rate=" << sample_rate
              << " peak_ref=" << peak_ref
              << " rms_ref=" << rms_ref
              << " total_frames=" << total_frames << "\n";

    if (frame_size <= 0 || hop_size <= 0 || total_frames < 0) {
        std::cerr << "Corrupt header.\n";
        return 1;
    }

    std::vector<float> samples((size_t)frame_size + (size_t)hop_size * (size_t)total_frames, 0.0f);

    kiss_fft_cfg cfg = kiss_fft_alloc(frame_size, 1, nullptr, nullptr);
    std::vector<kiss_fft_cpx> fin(frame_size), fout(frame_size);

    const float magScaleInv   = (peak_ref > 1e-12f) ? (peak_ref / (float)((1u<<qmag_bits)-1)) : 0.f;
    const float phaseScaleInv = (2.0f*(float)M_PI) / (float)((1u<<qphase_bits)-1);

    for (int f = 0; f < total_frames; ++f) {
        int useK = (int)br.readBits(16);

        for (int i = 0; i < frame_size; ++i) { fin[i].r = 0.f; fin[i].i = 0.f; }

        for (int j = 0; j < useK; ++j) {
            int idx       = (int)br.readBits(12);
            uint32_t qmag = br.readBits(qmag_bits);
            uint32_t qph  = br.readBits(qphase_bits);

            float mag   = (float)qmag * magScaleInv;
            float phase = (float)qph * phaseScaleInv - (float)M_PI;

            fin[idx].r = mag * std::cos(phase);
            fin[idx].i = mag * std::sin(phase);
        }

        kiss_fft(cfg, fin.data(), fout.data());

        const size_t offset = (size_t)f * (size_t)hop_size;
        for (int i = 0; i < frame_size; ++i) {
            float s = fout[i].r / (float)frame_size; // inverse scaling
            if (offset + (size_t)i < samples.size())
                samples[offset + (size_t)i] += s;
        }
    }

    free(cfg);

    // Gain match to reference peak
    float peak = 0.f;
    for (float s : samples) peak = std::max<float>(peak, std::fabs(s));
    const float gain = (peak > 1e-12f) ? (peak_ref / peak) : 1.f;
    for (auto& s : samples) s *= gain;

    WavData out;
    out.sample_rate = sample_rate;
    out.num_channels = 1;
    out.samples = std::move(samples);

    if (!write_wav(outFile, out)) {
        std::cerr << "Failed to write " << outFile << "\n";
        return 1;
    }

    std::cout << "Decoded " << total_frames
              << " frames -> " << out.samples.size()
              << " samples @ " << sample_rate << " Hz\n";
    std::cout << "Gain applied=" << gain << "\n";
    return 0;
}
