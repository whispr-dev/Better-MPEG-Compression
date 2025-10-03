# Curated Audio Compression Pack

This pack contains files auto-selected from your uploaded project archive that are most relevant to:
- File compression
- Audio compression using novel math
- Delta/DPCM-style prediction and residual coding
- Quantization/entropy coding, transforms (FFT/DCT/MDCT), and psychoacoustic hints

## What I did
1. Extracted the uploaded zip.
2. Scanned all text-like files for keywords (compression, codec, delta, DPCM, quantization, entropy, Huffman/arithmetic, FFT/DCT/MDCT, LPC, psychoacoustics, audio/PCM, bitrate, encoder/decoder).
3. Weighted matches by filename (+2), path (+1), and content (+1 per hit).
4. Selected files with score >= 2 and preserved original directory structure.
5. Excluded large binaries, build artifacts, and media payloads (e.g., .exe/.dll/.obj/.wav/.bin) to keep the pack lean.
6. Wrote a manifest with reasons for inclusion.

## Contents
See `CURATION_MANIFEST.json` for a full listing with scores and reasons.

## Repro/Adjust
If you want to widen or narrow the filter, tell me which keywords/paths to add/remove and I can regenerate instantly.
