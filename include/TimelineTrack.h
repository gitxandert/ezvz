#pragma once

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
    ma_sound sound{};
    bool initialized = false;

    ma_decoder analyzerDecoder;
    bool decoderInitialized = false;

    AudioFeatureAnalyzer analyzer;

    float smoothedEnvelope = 0.0f;
    float smoothingAlpha = 0.1f;
    float smoothNorm = 0.0f;

    float lastLocalTime = 0.0f;

	std::array<std::vector<std::unique_ptr<Mapping>>, static_cast<int>(AudioParameter::COUNT)> mappings;

    void computeComplementaryColor();
	float getParamValue(AudioParameter param) const;

    void updateMappings();
};