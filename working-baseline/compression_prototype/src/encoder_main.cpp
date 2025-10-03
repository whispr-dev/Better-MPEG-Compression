#include <iostream>
#include <string>
#include "wav.hpp"
#include "analysis.hpp"
#include "parametric.hpp"
#include "residual.hpp"
#include "bitstream.hpp"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: encoder <input.wav> <output.bin> [--nfft=N] [--hop=H] [--K=K]" << std::endl;
        return 1;
    }

    std::string inpath = argv[1];
    std::string outpath = argv[2];
    size_t nfft = 2048;
    size_t hop = 512;
    int K = 128;

    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind("--nfft=",0)==0) nfft = std::stoul(arg.substr(7));
        if (arg.rfind("--hop=",0)==0) hop = std::stoul(arg.substr(6));
        if (arg.rfind("--K=",0)==0) K = std::stoi(arg.substr(4));
    }

    wofl::WavData w = wofl::read_wav(inpath);
    auto pcm = w.samples;
    int sr = w.sample_rate;
    int ch = w.channels;

    std::cout << "Frame config: frame_size=" << nfft
              << " hop_size=" << hop
              << " -> total_frames=" << (pcm.size()/hop) << std::endl;
    std::cout << "Top-K=" << K << std::endl;

    // Encode
    wofl::BitWriter bw;
    bw.write32(nfft);
    bw.write32(hop);
    bw.write32(sr);
    bw.write32(ch);

    // analysis + parametric/residual encoding
    wofl::encode_stream(pcm, sr, ch, nfft, hop, K, bw);

   bw.save(outpath);

    std::cout << "Bytes written: " << bw.bytes_written() << std::endl;
    return 0;
}
