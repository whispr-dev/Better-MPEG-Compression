#pragma once
#include <string>
#include <vector>

struct WavData {
    std::vector<float> samples;  // mono, normalized [-1, 1]
    int sample_rate = 44100;     // Hz
    int num_channels = 1;        // stored/output as mono
};

bool read_wav(const std::string& filename, WavData& out);
bool write_wav(const std::string& filename, const WavData& in);
