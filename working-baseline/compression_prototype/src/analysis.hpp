#include <numeric> // for std::iota

#pragma once
#include "fft.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

namespace wofl {

struct Peak {
    int bin;
    float mag;
    float phase;
};

inline void mag_phase(const std::vector<cpx>& X, std::vector<float>& mag, std::vector<float>& ph){
    size_t N = X.size();
    mag.resize(N/2+1);
    ph.resize(N/2+1);
    for(size_t k=0;k<=N/2;++k){
        mag[k] = std::abs(X[k]);
        ph[k]  = std::arg(X[k]);
    }
}

inline std::vector<Peak> topk_peaks(const std::vector<float>& mag, const std::vector<float>& ph, int K){
    std::vector<int> idx(mag.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::partial_sort(idx.begin(), idx.begin()+std::min<int>(K,idx.size()), idx.end(),
        [&](int a,int b){ return mag[a] > mag[b]; });
    std::vector<Peak> out;
    int kk = std::min<int>(K, idx.size());
    out.reserve(kk);
    for(int i=0;i<kk;++i){
        int k = idx[i];
        out.push_back(Peak{ k, mag[k], ph[k] });
    }
    std::sort(out.begin(), out.end(), [](const Peak& a, const Peak& b){ return a.bin < b.bin; });
    return out;
}

} // namespace wofl
