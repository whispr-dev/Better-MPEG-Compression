
#pragma once
#include <vector>
#include <cmath>

namespace wofl {

struct MagQ {
    // Log-domain scalar quantizer with step size
    float step;
    MagQ(float s=0.125f): step(s) {}
    int encode(float mag){
        float lm = std::log1p(mag); // 0..
        return (int)std::round(lm/step);
    }
    float decode(int q){
        float lm = q*step;
        return std::expm1(lm);
    }
};

} // namespace wofl
