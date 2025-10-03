#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "kiss_fft.h"

struct kiss_fft_state {
    int nfft;
    int inverse;
    float* twiddles;
};

static void make_twiddles(struct kiss_fft_state* st) {
    st->twiddles = (float*)malloc(2 * st->nfft * sizeof(float));
    for (int k = 0; k < st->nfft; k++) {
        double phase = -2.0 * M_PI * k / st->nfft;
        if (st->inverse) phase = -phase;
        st->twiddles[2*k]   = (float)cos(phase);
        st->twiddles[2*k+1] = (float)sin(phase);
    }
}

kiss_fft_cfg kiss_fft_alloc(int nfft, int inverse_fft, void* mem, size_t* lenmem) {
    struct kiss_fft_state* st = (struct kiss_fft_state*)malloc(sizeof(struct kiss_fft_state));
    st->nfft = nfft;
    st->inverse = inverse_fft;
    make_twiddles(st);
    return st;
}

void kiss_fft(kiss_fft_cfg cfg, const kiss_fft_cpx* fin, kiss_fft_cpx* fout) {
    int nfft = cfg->nfft;
    for (int k = 0; k < nfft; k++) {
        float sumr = 0.0f, sumi = 0.0f;
        for (int n = 0; n < nfft; n++) {
            int tw = (k * n) % nfft;
            float wr = cfg->twiddles[2*tw];
            float wi = cfg->twiddles[2*tw+1];
            sumr += fin[n].r * wr - fin[n].i * wi;
            sumi += fin[n].r * wi + fin[n].i * wr;
        }
        fout[k].r = sumr;
        fout[k].i = sumi;
    }

    if (cfg->inverse) {
        for (int k = 0; k < nfft; k++) {
            fout[k].r /= nfft;
            fout[k].i /= nfft;
        }
    }
}

void kiss_fft_free(kiss_fft_cfg cfg) {
    if (cfg) {
        if (cfg->twiddles) free(cfg->twiddles);
        free(cfg);
    }
}
