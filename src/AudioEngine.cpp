#include "AudioEngine.h"
#include "TimelineTrack.h"
#include "GlobalTransport.h"
#include <cassert>
#include <iostream>


ma_engine audioEngine;

bool initAudio() {
    if (ma_engine_init(NULL, &audioEngine) != MA_SUCCESS) {
        std::cerr << "Failed to initialize miniaudio engine" << std::endl;
        return false;
    }
    return true;
}

void shutdownAudio() {
    ma_engine_uninit(&audioEngine);
}

void updateTrackPlayback(TimelineTrack& track, float currentTime) {
    float localTime = currentTime - track.startTime;

    // If we're looping globally and localTime has wrapped backwards:
    if (GlobalTransport::isLooping
        && track.initialized
        && track.hasPlayed
        && localTime < track.lastLocalTime) {

        track.hasPlayed = false;
    }
    track.lastLocalTime = localTime;

    if (track.muted) {
        if (track.initialized) {
            ma_sound_stop(&track.sound);
            ma_sound_uninit(&track.sound);
            track.initialized = false;
            track.hasPlayed = false;
        }
        return;
    }

    float duration = track.duration;

    if (currentTime >= track.startTime && currentTime < track.startTime + duration) {
        if (!track.initialized) {
            if (ma_sound_init_from_file(&audioEngine, track.filePath.c_str(), 0, nullptr, nullptr, &track.sound) == MA_SUCCESS) {
                track.initialized = true;
            }
            else {
                std::cerr << "Failed to init sound: " << track.displayName << std::endl;
                return;
            }
        }

        if (!track.hasPlayed) {
            ma_uint64 sampleRate = ma_engine_get_sample_rate(&audioEngine);
            ma_uint64 framePosition = static_cast<ma_uint64>(localTime * sampleRate);
            ma_sound_seek_to_pcm_frame(&track.sound, framePosition);
            ma_sound_start(&track.sound);
            track.hasPlayed = true;
        }
    }
    else {
        if (track.hasPlayed) {
            ma_sound_stop(&track.sound);
            ma_sound_uninit(&track.sound);
            track.hasPlayed = false;
            track.initialized = false;
        }
    }
}
