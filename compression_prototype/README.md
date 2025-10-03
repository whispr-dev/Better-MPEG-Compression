
# Wofl Audio Codec Prototype (Parametric + Residual)

A minimal, ready-to-build C++ prototype that demonstrates the hybrid strategy we discussed:
- **Parametric, sparse sinusoidal front-end** (top‑K partials with phase‑delta prediction)
- **Residual** via short **MDCT** with simple psycho thresholding
- **Bitstream** using bit IO + **Golomb‑Rice** coding (no external libs)
- **STFT** with a small header‑only radix‑2 FFT
- **WAV I/O** for 16‑bit PCM and 32‑bit float

It compiles cleanly on Windows (MSVC/MinGW/Clang) and Linux (gcc/clang) with CMake.

## Build

```bash
# Linux
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j

# Windows (PowerShell, MSVC)
mkdir build; cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

This produces `encoder(.exe)` and `decoder(.exe)`.

## Usage

```bash
# Encode
./encoder input.wav out.bin --nfft=2048 --hop=512 --K=128

# Decode
./decoder out.bin recon.wav
```

## Notes

- This is a **working** end‑to‑end prototype. It encodes a mid (sum) channel parametric layer and a residual MDCT; stereo side channel is currently omitted for simplicity.
- The code is fully self‑contained and uses no external dependencies.
- It’s designed to be easy to extend with:
  - Mid/Side parametric tracks and side residuals
  - Phase second‑order prediction
  - Rate‑controlled K and per‑frame bit budgeting
  - rANS/FSE entropy coding (drop‑in replacement for `rice.hpp`)
  - Transient short‑block switch and improved iSTFT overlap

## License

Prototype code © You. Purely illustrative, provided WITHOUT WARRANTY.
