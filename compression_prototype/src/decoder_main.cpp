#include <iostream>
#include <string>
#include "wav.hpp"
#include "analysis.hpp"
#include "parametric.hpp"
#include "residual.hpp"
#include "bitstream.hpp"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: decoder <input.bin> <output.wav>" << std::endl;
        return 1;
    }

    std::string inpath = argv[1];
    std::string outpath = argv[2];

    wofl::BitReader br(inpath);
    size_t nfft = br.read32();
    size_t hop = br.read32();
    int sr = br.read32();
    int ch = br.read32();

    std::cout << "Metadata loaded: frame_size=" << nfft
              << " hop_size=" << hop
              << " sample_rate=" << sr
              << " channels=" << ch << std::endl;

    std::vector<float> pcm;
    wofl::decode_stream(pcm, sr, ch, nfft, hop, br);

    std::cout << "Decoded " << pcm.size() << " samples @ " << sr << " Hz" << std::endl;

    wofl::normalize_and_write(pcm, outpath, sr, ch);

    return 0;
}
