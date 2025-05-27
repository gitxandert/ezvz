#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class AudioFeatureAnalyzer {
public:
    AudioFeatureAnalyzer(size_t bufferSize = 512)
        : envelope(0.0f), zcr(0), attack(0.9f), release(0.999f), bufferSize(bufferSize) {}

    // Call this every audio frame
    void analyze(const float* samples, size_t numSamples, int numChannels = 1) {
        float sumSquares = 0.0f;
        int zeroCrossings = 0;

        float prevSample = samples[0 * numChannels]; // take left channel if stereo
        for (size_t i = 0; i < numSamples; ++i) {
            float s = samples[i * numChannels]; // mono or left channel

            // RMS
            sumSquares += s * s;

            // Envelope (peak follower)
            float absS = std::abs(s);
            if (absS > envelope)
                envelope = attack * envelope + (1.0f - attack) * absS;
            else
                envelope = release * envelope + (1.0f - release) * absS;

            // ZCR
            if ((prevSample >= 0.0f && s < 0.0f) || (prevSample < 0.0f && s >= 0.0f))
                zeroCrossings++;

            prevSample = s;
        }

        zcr = zeroCrossings;
    }

    float getEnvelope() const { return envelope; }
    int getZeroCrossingRate() const { return zcr; }

private:
    float envelope;
    int zcr;

    float attack;
    float release;
    size_t bufferSize;
};
