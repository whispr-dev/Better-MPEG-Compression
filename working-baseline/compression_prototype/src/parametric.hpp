#include "constants.hpp"
#include <cmath>

#pragma once
#include "bitio.hpp"
#include "rice.hpp"
#include "analysis.hpp"
#include "codebook.hpp"
#include "constants.hpp"
#include <vector>
#include <cmath>

namespace wofl {

struct TrackQ {
    int bin;
    int qmag;
    int qdphi; // quantized wrapped delta phase
};

struct ParametricState {
    std::vector<int> last_bin;
    std::vector<float> last_phase;
};

inline int quant_phase(float dphi) {
    const int Q = 64;
    float x = (dphi + float(wofl::PI)) / (2.0f*float(wofl::PI));
    int q = (int)std::floor(x * Q);
    if (q >= Q) q = Q-1;
    if (q < 0) q = 0;
    return q;
}

inline float dequant_phase(int q) {
    const int Q = 64;
    float x = (q + 0.5f)/Q;
    float dphi = x*2.0f*float(wofl::PI) - float(wofl::PI);
    return dphi;
}

inline void encode_tracks(BitWriter& bw, const std::vector<Peak>& peaks, ParametricState& st, MagQ& mq){
    // write count
    Rice::write_uint(bw, (uint32_t)peaks.size(), 0);
    for(size_t i=0;i<peaks.size();++i){
        int bin = peaks[i].bin;
        int qmag = mq.encode(peaks[i].mag);
        // Predict phase: last phase (if same bin), else 0
        float prev = 0.0f;
        if((int)st.last_bin.size()>i && st.last_bin[i]==bin) prev = st.last_phase[i];
        float dphi = std::arg(std::polar(1.0f, peaks[i].phase) * std::conj(std::polar(1.0f, prev)));
        int qdphi = quant_phase(dphi);
        // delta bin vs last
        int dbin = bin - ( (int)st.last_bin.size()>i ? st.last_bin[i] : 0 );
        Rice::write_sint(bw, dbin, 2);
        Rice::write_sint(bw, qmag, 3);
        Rice::write_uint(bw, (uint32_t)qdphi, 6);
    }
    // update state
    st.last_bin.resize(peaks.size());
    st.last_phase.resize(peaks.size());
    for(size_t i=0;i<peaks.size();++i){ st.last_bin[i]=peaks[i].bin; st.last_phase[i]=peaks[i].phase; }
}

inline std::vector<Peak> decode_tracks(BitReader& br, ParametricState& st, MagQ& mq){
    uint32_t count = Rice::read_uint(br, 0);
    std::vector<Peak> peaks(count);
    for(size_t i=0;i<count;++i){
        int dbin = Rice::read_sint(br, 2);
        int qmag = Rice::read_sint(br, 3);
        int qdphi = (int)br.get_bits(6);
        int lastb = (int)st.last_bin.size()>i ? st.last_bin[i] : 0;
        int bin = lastb + dbin;
        float mag = mq.decode(qmag);
        float prev = (int)st.last_phase.size()>i ? st.last_phase[i] : 0.0f;
        float phase = prev + dequant_phase(qdphi);
        peaks[i] = Peak{bin, mag, phase};
    }
    st.last_bin.resize(count);
    st.last_phase.resize(count);
    for(size_t i=0;i<count;++i){ st.last_bin[i]=peaks[i].bin; st.last_phase[i]=peaks[i].phase; }
    return peaks;
}

inline void synthesize_tracks(const std::vector<Peak>& peaks, std::vector<cpx>& X){
    for(const auto& p: peaks){
        if(p.bin>=0 && p.bin<(int)X.size()) X[p.bin] += std::polar(p.mag, p.phase);
    }
}

} // namespace wofl
