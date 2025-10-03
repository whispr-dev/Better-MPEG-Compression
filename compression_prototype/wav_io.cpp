// compression_prototype/wav_io.cpp
#define _CRT_SECURE_NO_WARNINGS
#include "wav_io.h"
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <vector>

static inline bool read_exact(FILE* f, void* p, size_t n)  { return std::fread(p,1,n,f) == n; }
static inline bool write_exact(FILE* f, const void* p, size_t n){ return std::fwrite(p,1,n,f) == n; }
static inline bool skip_bytes(FILE* f, uint32_t n) { return std::fseek(f, (long)n, SEEK_CUR) == 0; }

static inline int16_t fast_round_to_i16(float x) {
    float y = (x >= 0.0f) ? (x + 0.5f) : (x - 0.5f);
    int v = (int)y;
    if (v > 32767) v = 32767;
    if (v < -32768) v = -32768;
    return (int16_t)v;
}

#pragma pack(push, 1)
struct RiffHeader { char riff[4]; uint32_t size; char wave[4]; };
struct ChunkHeader { char id[4]; uint32_t size; };
struct FmtPCM {
    uint16_t audio_format;   // 1 = PCM
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample; // 16
};
#pragma pack(pop)

bool read_wav(const std::string& filename, WavData& out) {
    FILE* f = std::fopen(filename.c_str(), "rb");
    if (!f) return false;

    RiffHeader rh{};
    if (!read_exact(f,&rh,sizeof(rh))) { std::fclose(f); return false; }
    if (std::memcmp(rh.riff,"RIFF",4) || std::memcmp(rh.wave,"WAVE",4)) { std::fclose(f); return false; }

    bool have_fmt=false, have_data=false;
    FmtPCM fmt{};
    std::vector<uint8_t> data;

    while (true) {
        ChunkHeader ch{};
        if (!read_exact(f,&ch,sizeof(ch))) break;

        if (!std::memcmp(ch.id,"fmt ",4)) {
            if (ch.size < sizeof(FmtPCM)) { std::fclose(f); return false; }
            if (!read_exact(f,&fmt,sizeof(fmt))) { std::fclose(f); return false; }
            have_fmt = true;
            if (ch.size > sizeof(FmtPCM))
                if (!skip_bytes(f, ch.size - sizeof(FmtPCM))) { std::fclose(f); return false; }
        } else if (!std::memcmp(ch.id,"data",4)) {
            data.resize(ch.size);
            if (ch.size && !read_exact(f, data.data(), ch.size)) { std::fclose(f); return false; }
            have_data = true;
        } else {
            if (!skip_bytes(f, ch.size)) { std::fclose(f); return false; }
        }
    }
    std::fclose(f);

    if (!have_fmt || !have_data) return false;
    if (fmt.audio_format != 1 || fmt.bits_per_sample != 16) return false;

    out.sample_rate  = (int)fmt.sample_rate;
    out.num_channels = (int)fmt.num_channels;

    size_t total_samples = data.size() / 2; // int16 count
    const int16_t* src = reinterpret_cast<const int16_t*>(data.data());
    size_t frames = total_samples / fmt.num_channels;
    out.samples.resize(frames);

    if (fmt.num_channels == 1) {
        for (size_t i=0;i<frames;i++)
            out.samples[i] = src[i] / 32768.0f;
    } else {
        for (size_t i=0;i<frames;i++) {
            int acc = 0;
            for (int ch=0; ch<fmt.num_channels; ++ch)
                acc += src[i*fmt.num_channels + ch];
            out.samples[i] = (acc / float(fmt.num_channels)) / 32768.0f;
        }
        out.num_channels = 1;
    }
    return true;
}

bool write_wav(const std::string& filename, const WavData& in) {
    FILE* f = std::fopen(filename.c_str(), "wb");
    if (!f) return false;

    const uint16_t bits = 16;
    const uint16_t channels = 1; // write mono
    const uint32_t sr = (in.sample_rate > 0 ? (uint32_t)in.sample_rate : 44100);
    const uint32_t byte_rate = sr * channels * (bits/8);
    const uint16_t block_align = channels * (bits/8);
    const uint32_t data_bytes = (uint32_t)(in.samples.size() * channels * (bits/8));

    RiffHeader rh{{'R','I','F','F'}, (uint32_t)(36 + data_bytes), {'W','A','V','E'}};
    if (!write_exact(f,&rh,sizeof(rh))) { std::fclose(f); return false; }

    // "fmt " chunk
    const char fmtid[4] = {'f','m','t',' '};
    if (!write_exact(f, fmtid, 4)) { std::fclose(f); return false; }
    uint32_t fmt_size = 16;
    if (!write_exact(f, &fmt_size, 4)) { std::fclose(f); return false; }

    FmtPCM fmt{1, channels, sr, byte_rate, block_align, bits};
    if (!write_exact(f,&fmt,sizeof(fmt))) { std::fclose(f); return false; }

    // "data" chunk
    const char dataid[4] = {'d','a','t','a'};
    if (!write_exact(f, dataid, 4)) { std::fclose(f); return false; }
    if (!write_exact(f, &data_bytes, 4)) { std::fclose(f); return false; }

    for (float s : in.samples) {
        float cl = std::min(1.0f, std::max(-1.0f, s));
        int16_t v = fast_round_to_i16(cl * 32767.0f);
        if (!write_exact(f,&v,sizeof(v))) { std::fclose(f); return false; }
    }
    std::fclose(f);
    return true;
}
