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
        return currentEnvelope;
    case AudioParameter::ZCR:
        return static_cast<float>(analyzer.getZeroCrossingRate());
	default:
        return 0.0f;
    }
}

void TimelineTrack::updateMappings() {
    for (std::size_t ap = 0; ap < mappings.size(); ++ap) {
        auto& v = mappings[ap];
        v.erase(
            std::remove_if(
                v.begin(),
                v.end(),
                [](const std::shared_ptr<Mapping>& m) {
                    return !m->isMapped();   // expired target?
                }
            ),
            v.end()
        );
        for (auto& m : v) {
            std::cout << "i'm mapping\n";
            m->mapParameter(getParamValue(static_cast<AudioParameter>(ap)));
        }
    }
}

bool TimelineTrack::loadTrack(const std::string& path) {

    ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_f32, 2, 48000);
    if (ma_decoder_init_file(path.c_str(), &decoderConfig, &decoder) != MA_SUCCESS) {
        return false;
    }
    filePath = path;
    decoderInitialized = true;
    playing = false;
    updateDecoderParams();

    ma_uint64 totalFrames = 0;
    ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrames);
    duration = float(totalFrames) / sampleRate;

    return true;
}

void TimelineTrack::playTrack(float timelineTime) {
    playing = true;
    nextFrame = uint64_t((timelineTime - startTime) * sampleRate);
    if (nextFrame < 0) nextFrame = 0;
    ma_decoder_seek_to_pcm_frame(&decoder, nextFrame);
}

void TimelineTrack::stopTrack() {
    playing = false;
}

void TimelineTrack::unloadTrack() {
    if (decoderInitialized) {
        ma_decoder_uninit(&decoder);
        decoderInitialized = false;
    }
}
