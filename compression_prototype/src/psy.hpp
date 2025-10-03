
#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace wofl {

inline void bark_band_edges(int nfft, int sr, std::vector<int>& bands){
    // 24 Bark-ish bands up to Nyquist
    int nb=24;
    bands.resize(nb+1);
    for(int b=0;b<=nb;++b){
        float frac = float(b)/nb;
        float f = frac * (sr*0.5f);
        int k = int(std::round(f / (sr*0.5f) * (nfft/2)));
        bands[b] = std::min(nfft/2, std::max(0, k));
    }
}

inline std::vector<float> band_energy(const std::vector<float>& mag, const std::vector<int>& bands){
    int nb = (int)bands.size()-1;
    std::vector<float> e(nb, 0.0f);
    for(int b=0;b<nb;++b){
        float s=0.0f;
        for(int k=bands[b]; k<bands[b+1]; ++k) s += mag[k]*mag[k];
        e[b]=s + 1e-12f;
    }
    return e;
}

// Salience score: peak mag^2 divided by local band energy + small constant
inline float salience_of_peak(const std::vector<float>& bandE, const std::vector<int>& bands, int bin, float mag2){
    int b=0;
    while(b+1<(int)bands.size() && bin>=bands[b+1]) ++b;
    float denom = bandE[std::min(b, (int)bandE.size()-1)] + 1e-9f;
    return mag2 / denom;
}

} // namespace wofl
