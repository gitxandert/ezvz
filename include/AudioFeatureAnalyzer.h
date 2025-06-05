// AudioFeatureAnalyzer.h
#pragma once
#include <vector>
#include <atomic>
#include <cmath>
#include <algorithm>

class AudioFeatureAnalyzer
{
public:
    // ─────────────── constructor ───────────────
    AudioFeatureAnalyzer(std::size_t bufferize = 512,
		float smoothingAlpha = 0.10f)
        : bufferSize(bufferSize),
		smoothingAlpha(std::clamp(smoothingAlpha, 0.0f, 1.0f))
    {
        rawEnvelope.store(0.0f, std::memory_order_relaxed);
        smoothedEnvelope.store(0.0f, std::memory_order_relaxed);
        zeroCrossings.store(0, std::memory_order_relaxed);
    }

    // ─────────────── analysis (call from audio thread) ───────────────
    void analyze(const float* samples,
        std::size_t  numSamples,
        int          numChannels = 1)
    {
        float env = smoothedEnvelope.load(std::memory_order_relaxed);
        int   zcr = 0;
        float raw = 0.0f;
        float prevS = samples[0 * numChannels];

        for (std::size_t i = 0; i < numSamples; ++i)
        {
            float s = samples[i * numChannels];          // mono (L channel)

            /* ── envelope ── */
            float absS = std::fabs(s);
            raw = absS;

            env += smoothingAlpha * (absS - env);

            /* ── ZCR ── */
            if ((prevS >= 0.0f && s < 0.0f) ||
                (prevS < 0.0f && s >= 0.0f))
                ++zcr;

            prevS = s;
        }

        /* publish results atomically */
        rawEnvelope.store(raw, std::memory_order_relaxed);
        smoothedEnvelope.store(env, std::memory_order_relaxed);
        zeroCrossings.store(zcr, std::memory_order_relaxed);
    }

    /* ─────────────── accessors for the GUI thread ─────────────── */
    float getRawEnvelope()      const { return rawEnvelope.load(); }
    float getSmoothedEnvelope() const { return smoothedEnvelope.load(); }
    int   getZeroCrossingRate() const { return zeroCrossings.load(); }

    void setSmoothingAlpha(float guiVal01, float sampleRate)
    {
        guiVal01 = std::clamp(guiVal01, 0.0f, 1.0f);

        /* Map 0…1  →  τ =  2 ms … 500 ms   (log scale feels natural) */
        constexpr float tauMin = 0.002f;   // 2 ms
        constexpr float tauMax = 0.500f;   // 500 ms
        float tau = tauMin * std::pow(tauMax / tauMin, guiVal01);

        /* One-pole coefficient:  α = 1 − e^(−1 / (τ·Fs))  */
        smoothingAlpha = std::clamp(
            1.0f - std::exp(-1.0f / (tau * sampleRate)),
            1e-9f,     // never let it hit exactly 0
            1.0f - 1e-9f);
    }

    float getSmoothingAlpha(float sampleRate) const
    {
        /* convert α  →  τ  →  GUI */
        float tau = -1.0f / (sampleRate * std::log(1.0f - smoothingAlpha));

        constexpr float tauMin = 0.002f, tauMax = 0.500f;
        float gui = std::log(tau / tauMin) / std::log(tauMax / tauMin);

        return std::clamp(gui, 0.0f, 1.0f);   // guarantees slider range
    }

private:
	std::size_t bufferSize; // size of the audio buffer to process
    /* thread-safe state */
    std::atomic<float> rawEnvelope{};
    std::atomic<float> smoothedEnvelope{};

    std::atomic<int>   zeroCrossings{};
    float smoothingAlpha = 0.10f;
};
