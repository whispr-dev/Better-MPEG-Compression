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
    if (argc < 4) {
        std::cerr << "Usage: encoder <in.wav> <out.bin> <mode> [--ktop=N --qbits=M:P --threshold=T]\n";
        return 1;
    }

    std::string inFile = argv[1];
    std::string outFile = argv[2];
    std::string mode = argv[3];

    int ktop = 128;
    int qmag_bits = 12;
    int qphase_bits = 8;
    int threshold = 60;

    for (int i = 4; i < argc; i++) {
        std::string a = argv[i];
        if (a.rfind("--ktop=",0)==0)       ktop = std::stoi(a.substr(7));
        else if (a.rfind("--qbits=",0)==0) {
            auto v = a.substr(8);
            auto p = v.find(':');
            qmag_bits = std::stoi(v.substr(0,p));
            qphase_bits = std::stoi(v.substr(p+1));
        } else if (a.rfind("--threshold=",0)==0) {
            threshold = std::stoi(a.substr(12));
        }
    }

    WavData wav;
    if (!read_wav(inFile, wav)) {
        std::cerr << "Failed to read " << inFile << "\n";
        return 1;
    }

    const int frame_size = 2048;
    const int hop_size = 512;
    const int total_frames = (int)((wav.samples.size() >= (size_t)frame_size)
        ? ((wav.samples.size() - frame_size) / hop_size + 1) : 0);

    // global stats
    float peak = 0.f;
    double sum2 = 0.0;
    for (float s : wav.samples) {
        peak = std::max<float>(peak, std::fabs(s));
        sum2 += (double)s * (double)s;
    }
    float rms = (wav.samples.empty() ? 0.f : (float)std::sqrt(sum2 / wav.samples.size()));

    std::cout << "Frame config: frame_size=" << frame_size
              << " hop_size=" << hop_size
              << " -> total_frames=" << total_frames << "\n";
    std::cout << "Top-K=" << ktop << " | qmag=" << qmag_bits
              << " qphase=" << qphase_bits
              << " | threshold=" << threshold << " dB\n";
    std::cout << "Input stats: peak=" << peak << " rms=" << rms << "\n";

    BitWriter bw;
    // metadata
    bw.writeRaw<uint16_t>((uint16_t)frame_size);
    bw.writeRaw<uint16_t>((uint16_t)hop_size);
    bw.writeRaw<uint32_t>((uint32_t)wav.sample_rate);
    bw.writeFloat(peak);
    bw.writeFloat(rms);
    bw.writeRaw<uint16_t>((uint16_t)ktop);
    bw.writeRaw<uint8_t>((uint8_t)qmag_bits);
    bw.writeRaw<uint8_t>((uint8_t)qphase_bits);
    bw.writeRaw<uint16_t>((uint16_t)threshold);
    bw.writeRaw<uint32_t>((uint32_t)total_frames);

    kiss_fft_cfg cfg = kiss_fft_alloc(frame_size, 0, nullptr, nullptr);
    std::vector<kiss_fft_cpx> fin(frame_size), fout(frame_size);

    for (int f = 0; f < total_frames; ++f) {
        const size_t offset = (size_t)f * hop_size;
        for (int i = 0; i < frame_size; ++i) {
            float x = (offset + i < wav.samples.size()) ? wav.samples[offset + i] : 0.f;
            fin[i].r = x;
            fin[i].i = 0.f;
        }
        kiss_fft(cfg, fin.data(), fout.data());

        // magnitudes up to Nyquist (frame_size/2)
        std::vector<std::pair<float,int>> mags;
        mags.reserve(frame_size/2);
        for (int k = 0; k < frame_size/2; ++k) {
            float re = fout[k].r, im = fout[k].i;
            float m = std::sqrt(re*re + im*im);
            mags.push_back({m, k});
        }
        int useK = std::min(ktop, (int)mags.size());
        std::partial_sort(mags.begin(), mags.begin() + useK, mags.end(),
                          [](const auto& a, const auto& b){ return a.first > b.first; });

        bw.writeBits((uint32_t)useK, 16);

        const float magScale = (peak > 1e-12f) ? ((float)((1u<<qmag_bits)-1) / peak) : 0.f;
        const float phaseScale = (float)((1u<<qphase_bits)-1) / (2.0f*(float)M_PI);

        for (int j = 0; j < useK; ++j) {
            int idx = mags[j].second;
            float re = fout[idx].r, im = fout[idx].i;
            float mag = std::sqrt(re*re + im*im);
            float ph  = std::atan2(im, re);

            uint32_t qmag = (uint32_t)std::min<float>( (mag * magScale), (float)((1u<<qmag_bits)-1) );
            float phShift = ph + (float)M_PI; if (phShift < 0) phShift = 0; if (phShift > 2.0f*(float)M_PI) phShift = 2.0f*(float)M_PI;
            uint32_t qphase = (uint32_t)std::min<float>( phShift * phaseScale, (float)((1u<<qphase_bits)-1) );

            bw.writeBits((uint32_t)idx, 12);
            bw.writeBits(qmag,   qmag_bits);
            bw.writeBits(qphase, qphase_bits);
        }
    }

    free(cfg);

    if (!bw.writeToFile(outFile)) {
        std::cerr << "Failed to write " << outFile << "\n";
        return 1;
    }
    std::cout << "Bytes written: " << bw.sizeBytes() << "\n";
    return 0;
}
