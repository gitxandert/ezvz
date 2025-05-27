#pragma once
#include "TimelineTrack.h"

namespace TrackFeatures {
    extern float panelWidth;
    extern bool showMappings;
	extern TimelineTrack* selectedTrack;

    void render();
}