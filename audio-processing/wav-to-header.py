import wave
import struct
import sys
import os

if len(sys.argv) < 2:
    print("Usage: python script.py <input.wav>")
    sys.exit(1)

input_filename = sys.argv[1]
output_filename = os.path.splitext(os.path.basename(input_filename))[0] + "_samples.h"

wav = wave.open(input_filename, 'rb')
samples = wav.readframes(wav.getnframes())
wav.close()

samples = struct.unpack("<" + "h" * (len(samples) // 2), samples)

with open(output_filename, "w") as f:
    f.write(f"#define WAV_SAMPLE_COUNT {len(samples)}\n")
    f.write("static const int16_t wav_data[] = {\n")
    for i, sample in enumerate(samples):
        f.write(f"{sample}, ")
        if i % 10 == 9:
            f.write("\n")
    f.write("\n};\n")

