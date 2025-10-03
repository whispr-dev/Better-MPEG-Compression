#pragma once
#include <cstdint>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace wofl {

struct WavData {
    int sample_rate = 44100;
    int channels = 1;
    std::vector<float> samples; // normalized to [-1,1]
};

// ---- WAV writing helper ----
inline void write_wav(const std::string& path,
                      const std::vector<int16_t>& data,
                      int sample_rate,
                      int channels) {
    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("cannot open wav for writing");

    uint32_t data_bytes = static_cast<uint32_t>(data.size() * sizeof(int16_t));
    uint32_t fmt_size = 16;
    uint16_t audio_fmt = 1; // PCM
    uint16_t num_channels = static_cast<uint16_t>(channels);
    uint32_t byte_rate = sample_rate * num_channels * sizeof(int16_t);
    uint16_t block_align = num_channels * sizeof(int16_t);
    uint16_t bits_per_sample = 16;

    // RIFF header
    out.write("RIFF", 4);
    uint32_t chunk_size = 36 + data_bytes;
    out.write(reinterpret_cast<const char*>(&chunk_size), 4);
    out.write("WAVE", 4);

    // fmt chunk
    out.write("fmt ", 4);
    out.write(reinterpret_cast<const char*>(&fmt_size), 4);
    out.write(reinterpret_cast<const char*>(&audio_fmt), 2);
    out.write(reinterpret_cast<const char*>(&num_channels), 2);
    out.write(reinterpret_cast<const char*>(&sample_rate), 4);
    out.write(reinterpret_cast<const char*>(&byte_rate), 4);
    out.write(reinterpret_cast<const char*>(&block_align), 2);
    out.write(reinterpret_cast<const char*>(&bits_per_sample), 2);

    // data chunk
    out.write("data", 4);
    out.write(reinterpret_cast<const char*>(&data_bytes), 4);
    out.write(reinterpret_cast<const char*>(data.data()), data_bytes);
}

// ---- WAV reading helper ----
inline WavData read_wav(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open wav for reading");

    char riff[4]; in.read(riff, 4);
    if (std::string(riff,4)!="RIFF") throw std::runtime_error("bad wav (no RIFF)");
    in.ignore(4);
    char wave[4]; in.read(wave,4);
    if (std::string(wave,4)!="WAVE") throw std::runtime_error("bad wav (no WAVE)");

    int sample_rate=0, channels=0, bits=0;
    uint32_t data_bytes=0;
    std::streampos data_pos=0;

    while (in) {
        char id[4]; in.read(id,4);
        uint32_t sz=0; in.read(reinterpret_cast<char*>(&sz),4);
        if (!in) break;
        std::string chunk(id,4);
        if (chunk=="fmt ") {
            uint16_t fmt=0; in.read(reinterpret_cast<char*>(&fmt),2);
            uint16_t chn=0; in.read(reinterpret_cast<char*>(&chn),2); channels=chn;
            uint32_t sr=0; in.read(reinterpret_cast<char*>(&sr),4); sample_rate=sr;
            uint32_t byte_rate; in.read(reinterpret_cast<char*>(&byte_rate),4);
            uint16_t block_align; in.read(reinterpret_cast<char*>(&block_align),2);
            uint16_t bps=0; in.read(reinterpret_cast<char*>(&bps),2); bits=bps;
            in.ignore(sz-16);
        } else if (chunk=="data") {
            data_bytes=sz;
            data_pos=in.tellg();
            in.seekg(sz,std::ios::cur);
        } else {
            in.seekg(sz,std::ios::cur);
        }
    }

    if (bits!=16) throw std::runtime_error("only 16-bit PCM supported");

    in.clear();
    in.seekg(data_pos);

    std::vector<int16_t> raw(data_bytes/2);
    in.read(reinterpret_cast<char*>(raw.data()),data_bytes);

    WavData out;
    out.sample_rate=sample_rate;
    out.channels=channels;
    out.samples.resize(raw.size());
    for (size_t i=0;i<raw.size();++i) {
        out.samples[i]=raw[i]/32768.0f;
    }
    return out;
}

// ---- Normalize + write float samples safely ----
inline void normalize_and_write(const std::vector<float>& pcm,
                                const std::string& path,
                                int sample_rate,
                                int channels) {
    float peak=0.0f;
    for (auto f:pcm) peak=std::max(peak,std::fabs(f));
    if (peak<1e-12f) peak=1.0f;
    float scale=0.999f/peak;

    std::vector<int16_t> data(pcm.size());
    for (size_t i=0;i<pcm.size();++i) {
        float v=pcm[i]*scale*32767.0f;
        if (v>32767.0f) v=32767.0f;
        if (v<-32768.0f) v=-32768.0f;
        data[i]=static_cast<int16_t>(std::lrint(v));
    }
    write_wav(path,data,sample_rate,channels);
}

} // namespace wofl
