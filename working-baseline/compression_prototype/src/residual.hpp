
#pragma once
#include "mdct.hpp"
#include "bitio.hpp"
#include "rice.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

namespace wofl {

struct ResidualQ {
    float step;
    ResidualQ(float s=0.01f): step(s) {}
    int q(float v) const { return (int)std::round(v/step); }
    float dq(int qv) const { return qv*step; }
};

inline void encode_residual(BitWriter& bw, const std::vector<float>& coeffs, float thresh, const ResidualQ& rq){
    // RLE zero + Rice of nonzeros
    int run=0;
    for(size_t i=0;i<coeffs.size();++i){
        float c = coeffs[i];
        if(std::fabs(c) < thresh){
            run++;
        } else {
            // flush run
            Rice::write_uint(bw, (uint32_t)run, 0);
            run=0;
            int qv = rq.q(c);
            Rice::write_sint(bw, qv, 3);
        }
    }
    // flush tail run
    Rice::write_uint(bw, (uint32_t)run, 0);
    // end marker: run=0 then q=0
    Rice::write_uint(bw, 0, 0);
    Rice::write_sint(bw, 0, 3);
}

inline void decode_residual(BitReader& br, std::vector<float>& coeffs, const ResidualQ& rq){
    size_t pos=0; coeffs.assign(coeffs.size(), 0.0f);
    while(pos < coeffs.size()){
        uint32_t run = Rice::read_uint(br, 0);
        if(run==0){
            int qv = Rice::read_sint(br, 3);
            if(qv==0){
                // end marker
                break;
            } else {
                if(pos<coeffs.size()) coeffs[pos++] = rq.dq(qv);
            }
        } else {
            pos += run;
        }
    }
}

} // namespace wofl
