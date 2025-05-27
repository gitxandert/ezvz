#include "TimelineTrack.h"
#include "imgui.h"

void TimelineTrack::computeComplementaryColor() {
    auto srgbToLinear = [](float c) -> float {
        return powf(c / 255.0f, 2.2f);
        };

    auto linearToSrgb = [](float c) -> int {
        return static_cast<int>(powf(1.0f - c, 1.0f / 2.2f) * 255.0f);
        };

    int r = (color >> IM_COL32_R_SHIFT) & 0xFF;
    int g = (color >> IM_COL32_G_SHIFT) & 0xFF;
    int b = (color >> IM_COL32_B_SHIFT) & 0xFF;

    int compR = linearToSrgb(srgbToLinear(static_cast<float>(r)));
    int compG = linearToSrgb(srgbToLinear(static_cast<float>(g)));
    int compB = linearToSrgb(srgbToLinear(static_cast<float>(b)));

    float brightness = 0.2126f * compR / 255.0f
        + 0.7152f * compG / 255.0f
        + 0.0722f * compB / 255.0f;

    if (brightness < 0.5f) {
        compR = static_cast<int>((255 - compR) * 1.2f);
        compG = static_cast<int>((255 - compG) * 1.2f);
        compB = static_cast<int>((255 - compB) * 1.2f);
    }
    else {
        compR = static_cast<int>((255 - compR) * 0.6f);
        compG = static_cast<int>((255 - compG) * 0.6f);
        compB = static_cast<int>((255 - compB) * 0.6f);
    }

    labelColor = IM_COL32(compR, compG, compB, 255);
}

float TimelineTrack::getParamValue(AudioParameter param) const {
    switch (param) {
    case AudioParameter::Envelope:
        return smoothedEnvelope;
    case AudioParameter::ZCR:
        return static_cast<float>(analyzer.getZeroCrossingRate());
	default:
        return 0.0f;
    }
}

void TimelineTrack::updateMappings() {
    for (std::size_t ap = 0; ap < static_cast<std::size_t>(AudioParameter::COUNT); ++ap) {
        auto& v = mappings[ap];
        v.erase(
            std::remove_if(
                v.begin(),
                v.end(),
                [](const std::unique_ptr<Mapping>& m) {
                    return !m->isMapped();   // expired target?
                }
            ),
            v.end()
        );
        for (auto& m : v)
            m->mapParameter(getParamValue(static_cast<AudioParameter>(ap)));
    }
}