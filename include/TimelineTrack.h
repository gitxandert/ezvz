#pragma once

#include <atomic>
#include <string>
#include <array>
#include <vector>
#include <cmath>
#include <memory>
#include "miniaudio.h"
#include "AudioFeatureAnalyzer.h"
#include "Mapping.h"
#include "imgui.h"

struct TimelineTrack {
	TimelineTrack() = default;

    TimelineTrack(const TimelineTrack&) = delete;
    TimelineTrack& operator=(const TimelineTrack&) = delete;
    TimelineTrack(TimelineTrack&&) = delete;
    TimelineTrack& operator=(TimelineTrack&&) = delete;

    std::string filePath;
    std::string displayName;
    float startTime = 0.0f;
    float duration = 0.0f;

    ImU32 color = IM_COL32_WHITE;
    ImU32 labelColor = IM_COL32_WHITE;
    bool selected = false;
    bool dragging = false;
    bool muted = false;
    float dragStartMouseX = 0.0f;
    float dragStartTrackX = 0.0f;

    bool hasPlayed = false;
    bool initialized = false;

    ma_decoder decoder;
    bool decoderInitialized = false;
    uint32_t channelCount = 2;
    uint64_t nextFrame = 0;
	float sampleRate = 48000.0f; // Default sample rate
    bool playing = false;

    AudioFeatureAnalyzer analyzer;

    std::atomic<float> currentEnvelope{ 0.0f };   // raw, per-block value
    std::atomic<float> smoothedEnvelope{ 0.0f };  // low-pass output
    float              smoothingAlpha = 0.10f;  // 0 … 1, higher = quicker response

    float lastLocalTime = 0.0f;

	std::array<std::vector<std::shared_ptr<Mapping>>, static_cast<int>(AudioParameter::COUNT)> mappings;
    
    bool loadTrack(const std::string& path);
    void playTrack(float);
    void stopTrack();
    void unloadTrack();

    void computeComplementaryColor();
	float getParamValue(AudioParameter param) const;

    void updateMappings();

    void updateDecoderParams() {
        channelCount = decoder.outputChannels;
        sampleRate = static_cast<float>(decoder.outputSampleRate);
    }
};