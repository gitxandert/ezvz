#pragma once

#include <vector>
#include <string>
#include "imgui.h"
#include "TimelineTrack.h"
#include "Scene.h"

namespace Timeline {
    extern std::vector<std::unique_ptr<TimelineTrack>> timelineTracks;
    extern std::vector<std::shared_ptr<Scene>> scenes;

    extern bool openDialog;
    extern bool isScrubberDragging;
    extern float zoom;
    extern float basePixelsPerMs;
    extern float pixelsPerMs;
    extern const float trackHeight;
    extern float timelineFixedHeight;
    extern float userScreenWidth;

    extern bool isDraggingScene;
    extern float tempSceneStart;
    extern float tempSceneEnd;
    extern ImU32 tempColor;
    extern bool addSceneMode;

    static ImU32 randomColor();
    void init(float screenWidth);

    extern std::shared_ptr<Scene> currentScene;
    void render(float currentTime);
}