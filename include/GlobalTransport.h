#pragma once

#include "imgui.h"

namespace GlobalTransport {
    extern float transportHeight;
    extern bool isPlaying;
    extern bool isLooping;
    extern float currentTime;
    extern float totalTime;
    extern float playStartTime;

    void setLoop();

    float render();
}