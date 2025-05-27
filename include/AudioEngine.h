#pragma once

#include "miniaudio.h"

struct TimelineTrack; // forward declaration

extern ma_engine audioEngine;

bool initAudio();
void shutdownAudio();
void updateTrackPlayback(TimelineTrack& track, float currentTime);