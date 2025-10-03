
#pragma once
#include "bitio.hpp"
#include <cstdint>
#include <cmath>

namespace wofl {

inline uint32_t zigzag(int32_t x){ return (x<<1) ^ (x>>31); }
inline int32_t unzigzag(uint32_t u){ return (u>>1) ^ -int32_t(u&1); }

struct Rice {
    static void write_uint(BitWriter& bw, uint32_t v, uint32_t k){
        uint32_t q = v >> k;
        for(uint32_t i=0;i<q;++i) bw.put_bit(1);
        bw.put_bit(0);
        bw.put_bits(v & ((1u<<k)-1u), k);
    }
    static uint32_t read_uint(BitReader& br, uint32_t k){
        uint32_t q=0; while(br.get_bit()) ++q;
        uint32_t r = k? br.get_bits(k) : 0u;
        return (q<<k) | r;
    }
    static void write_sint(BitWriter& bw, int32_t x, uint32_t k){
        write_uint(bw, zigzag(x), k);
    }
    static int32_t read_sint(BitReader& br, uint32_t k){
        return unzigzag(read_uint(br, k));
    }
};

} // namespace wofl
