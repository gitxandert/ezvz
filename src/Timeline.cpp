#include "Timeline.h"
#include "GlobalTransport.h"
#include "TrackFeatures.h"
#include <algorithm>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include <iostream>

namespace Timeline {

    std::vector<std::unique_ptr<TimelineTrack>> timelineTracks;
    std::vector<std::shared_ptr<Scene>> scenes;
    bool openDialog = false;
    bool isScrubberDragging = false;
    float zoom = 1.0f;
    float basePixelsPerMs = 0.0f;
    float pixelsPerMs = 0.0f;
    float userScreenWidth = 0.0f;
    const float trackHeight = 20.0f;
    float timelineFixedHeight = 250.0f;

    bool isDraggingScene = false;
    float tempSceneStart = 0.0f;
    float tempSceneEnd = 0.0f;
    ImU32 tempColor = randomColor();
    bool addSceneMode = true;

    static bool editScenesMode = false;
    static int editingSceneIndex = -1;

    std::shared_ptr<Scene> currentScene;

    static constexpr float rulerHeight = 20.0f;
    static constexpr float rulerMarginTop = 5.0f;

    // Ruler ticks:
    static constexpr float tickSpacingSeconds = 1.0f;

    static ImU32 randomColor() {
        auto rnd = []() { return 50 + (std::rand() % 156); };
        return IM_COL32(rnd(), rnd(), rnd(), 150);
    }

    void init(float screenWidth) {
        userScreenWidth = screenWidth;
        const float timelineTotalSeconds = 600.0f; // 10 minutes
        basePixelsPerMs = userScreenWidth / (timelineTotalSeconds * 1000.0f);
        pixelsPerMs = basePixelsPerMs * zoom;
    }

    void render(float currentTime) {
        if (!ScenesPanel::showAnimateWindow && !TrackFeatures::showMappings) {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - timelineFixedHeight + 5), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, timelineFixedHeight - 5), ImGuiCond_Always);
            ImGui::Begin("Timeline", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

            ImGui::SetNextItemWidth(io.DisplaySize.x * 0.7f);
            if (ImGui::Button("+ Add Track")) openDialog = true;

            const char* sceneBtnLabel = addSceneMode ? "+ Add Scene" : "Set Scene";

            ImGui::SameLine();
            if (ImGui::Button(sceneBtnLabel)) {
                if (addSceneMode) {
                    tempSceneStart = scenes.empty() ? 0.0f : scenes.back()->endTime;
                    tempSceneEnd = tempSceneStart;
                    tempColor = randomColor();
                    isDraggingScene = true;
                }
                else {
                    scenes.push_back(std::make_shared<Scene>(Scene{ tempSceneStart, tempSceneEnd, tempColor }));
                    isDraggingScene = false;
                }

                addSceneMode = !addSceneMode;
            }

            ImGui::SameLine();
            if (addSceneMode && scenes.size() > 0)
            {
                if (ImGui::Button(editScenesMode ? "Done Editing" : "Edit Scenes")) {
                    editScenesMode = !editScenesMode;
                    editingSceneIndex = -1;  // clear any in-flight edits
                }
            }
            ImGui::SameLine();
            ImGui::Text("Zoom:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100.0f);
            if (ImGui::SliderFloat("##zoom", &zoom, 0.1f, 5.0f, "%.1fx", ImGuiSliderFlags_AlwaysClamp)) {
                pixelsPerMs = basePixelsPerMs * zoom;
            }

            // Compute dynamic content width
            float maxTimelineExtent = userScreenWidth;
            for (const auto& track : timelineTracks) {
                float endPx = (track->startTime + track->duration) * 1000.0f * pixelsPerMs;
                maxTimelineExtent = (std::max)(maxTimelineExtent, endPx) + 100.0;
            }

            for (auto& s : scenes) {
                // s.startTime / endTime are already in ms, so:
                float sceneEndPx = s->endTime * pixelsPerMs;
                maxTimelineExtent = (std::max)(maxTimelineExtent, sceneEndPx) + 100.0;
            }

            if (isDraggingScene) {
                float previewEndPx = tempSceneEnd * pixelsPerMs;
                maxTimelineExtent = (std::max)(maxTimelineExtent, previewEndPx) + 100.0;
            }

            ImVec2 childSize(io.DisplaySize.x, -1);
            ImGui::BeginChild("TimelineScrollRegion", childSize, false, ImGuiWindowFlags_HorizontalScrollbar);

            ImVec2 cursorStart = ImGui::GetCursorScreenPos();
            ImVec2 mousePos = ImGui::GetMousePos();
            bool anyClicked = false;
            ImGui::Dummy(ImVec2(maxTimelineExtent, rulerHeight + timelineTracks.size() * (trackHeight + 5)));
            ImGui::SetCursorScreenPos(cursorStart);
            float scrollX = ImGui::GetScrollX();

            float tickSpacingPixels = tickSpacingSeconds * 1000.0f * pixelsPerMs;
            float timelineStart = cursorStart.x;
            float timelineTop = cursorStart.y;

            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            int endTick = static_cast<int>(maxTimelineExtent / tickSpacingPixels) + 1;
            for (int i = 0; i < endTick; ++i) {
                float x = timelineStart + i * tickSpacingPixels - scrollX;
                float tickLength = (i % 5) ? (timelineTop + rulerHeight * 0.5f) : (timelineTop + rulerHeight);
                draw_list->AddLine(ImVec2(x, timelineTop), ImVec2(x, tickLength), IM_COL32(200, 200, 200, 255), 1.0f);
            }

            float       x0 = cursorStart.x;                   // left edge of timeline
            float       y0 = cursorStart.y;                   // top edge of timeline
            float       h = timelineFixedHeight;        // height of the timeline strip
            float       w = userScreenWidth;

            currentScene = nullptr;
            float curTimeInMs = currentTime * 1000.0f;

            for (size_t i{ 0 }; i < scenes.size(); ++i) {
                auto& s = scenes[i];
                float sx = x0 + s->startTime * pixelsPerMs - scrollX;
                float ex = x0 + s->endTime * pixelsPerMs - scrollX;
                // filled highlight
                draw_list->AddRectFilled(
                    ImVec2(sx, y0),
                    ImVec2(ex, y0 + h),
                    s->color
                );

                draw_list->AddLine(
                    ImVec2(ex, y0),
                    ImVec2(ex, y0 + h),
                    IM_COL32(200, 200, 200, 255),
                    2.0f
                );

                if (editScenesMode) {
                    const float handleW = 8.0f;

                    // — Start-edge handle —
                    ImVec2 startHandlePos(sx - handleW * 0.5f, y0);
                    ImGui::SetCursorScreenPos(startHandlePos);
                    ImGui::InvisibleButton(
                        (std::string("##SceneStart") + std::to_string(i)).c_str(),
                        ImVec2(handleW, timelineFixedHeight)
                    );
                    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    if (i > 0) {
                        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                            float mx = ImGui::GetIO().MousePos.x;
                            // clamp between 0 and this scene’s end
                            float newStart = (mx - x0 + scrollX) / pixelsPerMs;
                            s->startTime = std::clamp(newStart, 0.0f, s->endTime);
                            scenes[i - 1]->endTime = newStart;
                        }
                    }

                    // — End-edge handle —
                    ImVec2 endHandlePos(ex - handleW * 0.5f, y0);
                    ImGui::SetCursorScreenPos(endHandlePos);
                    ImGui::InvisibleButton(
                        (std::string("##SceneEnd") + std::to_string(i)).c_str(),
                        ImVec2(handleW, timelineFixedHeight)
                    );
                    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                        float mx = ImGui::GetIO().MousePos.x;
                        // clamp between this scene’s start and timeline max
                        float maxTime = maxTimelineExtent / pixelsPerMs;
                        float newEnd = (mx - x0 + scrollX) / pixelsPerMs;
                        s->endTime = std::clamp(newEnd, s->startTime, maxTime);
                        if (i < scenes.size() - 1)
                            scenes[i + 1]->startTime = newEnd;
                    }
                }

                if (curTimeInMs < s->endTime && curTimeInMs >= s->startTime) {
                    currentScene = scenes[i];
                }
            }

            if(!currentScene) {
                currentScene = scenes.empty() ? nullptr : scenes.back();
			}

            if (isDraggingScene) {
                float sx = x0 + tempSceneStart * pixelsPerMs - scrollX;
                float ex = x0 + tempSceneEnd * pixelsPerMs - scrollX;
                // translucent overlay
                draw_list->AddRectFilled(
                    ImVec2(sx, y0),
                    ImVec2(ex, y0 + h),
                    tempColor
                );

                draw_list->AddLine(
                    ImVec2(ex, y0),
                    ImVec2(ex, y0 + h),
                    IM_COL32(200, 200, 200, 255),
                    2.0f
                );

                // 1) compute handle bounds (centered on the vertical line)
                const float handleWidth = 8.0f;
                float  xHandlePx = x0 + tempSceneEnd * pixelsPerMs - scrollX;
                ImVec2 handlePos = ImVec2(xHandlePx - handleWidth * 0.5f, y0);

                // 2) place an invisible button over that region
                ImGui::SetCursorScreenPos(handlePos);
                ImGui::InvisibleButton("##SceneHandle", ImVec2(handleWidth, timelineFixedHeight));

                // 3) if active and dragging, update tempSceneEnd
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    float mx = ImGui::GetIO().MousePos.x;
                    // clamp between start of this scene and end of the timeline
                    float minX = x0 + tempSceneStart * pixelsPerMs - scrollX;
                    float maxX = x0 + maxTimelineExtent - scrollX;
                    float clampedX = std::clamp(mx, minX, maxX);
                    // convert back to time
                    tempSceneEnd = (clampedX - x0 + scrollX) / pixelsPerMs;
                }

                // 4) (optional) change mouse cursor when hovering
                if (ImGui::IsItemHovered())
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }

            for (size_t i = 0; i < timelineTracks.size(); ++i) {
                auto& track = timelineTracks[i];
                float rectStartX = track->startTime * 1000.0f * pixelsPerMs;
                float rectWidth = track->duration * 1000.0f * pixelsPerMs;

                ImVec2 p0(cursorStart.x + rectStartX - scrollX,
                    cursorStart.y + rulerHeight + 5 + i * (trackHeight + 5));
                ImVec2 p1(p0.x + rectWidth, p0.y + trackHeight);

                bool hovered = ImGui::IsMouseHoveringRect(p0, p1);
                bool clicked = hovered && ImGui::IsMouseClicked(0);

                if (clicked) {
                    if (ImGui::GetIO().KeyShift) {
                        track->selected = !track->selected;
                    }
                    else {
                        for (auto& t : timelineTracks) t->selected = false;
                        track->selected = true;
                    }
                    anyClicked = true;
                    for (auto& t : timelineTracks) {
                        if (t->selected) {
                            t->dragging = true;
                            t->dragStartMouseX = mousePos.x;
                            t->dragStartTrackX = t->startTime;
                        }
                    }
                }

                if (!ImGui::IsMouseDown(0)) {
                    track->dragging = false;
                }

                if (track->dragging && track->selected) {
                    float deltaX = mousePos.x - track->dragStartMouseX;
                    float deltaTime = deltaX / (1000.0f * pixelsPerMs);
                    for (auto& t : timelineTracks) {
                        if (t->selected) {
                            t->startTime = (std::max)(0.0f, t->dragStartTrackX + deltaTime);
                        }
                    }
                }

                ImU32 drawColor = track->color;
                if (track->muted) {
                    int a = (track->color >> IM_COL32_A_SHIFT) & 0xFF;
                    a = static_cast<int>(a * 0.3f);
                    drawColor = (track->color & 0x00FFFFFF) | (a << IM_COL32_A_SHIFT);
                }
                draw_list->AddRectFilled(p0, p1, drawColor);

                // Clip any subsequent drawing to the inside of [p0,p1]
                draw_list->PushClipRect(
                    ImVec2(p0.x + 4, p0.y),
                    ImVec2(p1.x - 4, p1.y), 
                    true                       
                );
                ImVec2 textPos = {
                    p0.x + 4,
                    p0.y + (trackHeight - ImGui::GetFontSize()) * 0.5f
                };
                draw_list->AddText(textPos, track->labelColor, track->displayName.c_str());
                draw_list->PopClipRect();

                if (track->selected) {
                    draw_list->AddRect(p0, p1, IM_COL32(255, 255, 255, 150), 3.0f, 0, 2.5f);
                }
            }

            if (ImGui::IsMouseClicked(0) && !anyClicked && !ImGui::GetIO().WantCaptureMouse) {
                for (auto& t : timelineTracks) t->selected = false;
            }

            // Scrubber
            float scrubberX = cursorStart.x + (GlobalTransport::currentTime * 1000.0f * pixelsPerMs) - scrollX;
            ImVec2 lineStart(scrubberX, cursorStart.y);
            ImVec2 lineEnd(scrubberX, cursorStart.y + rulerHeight + 5 + timelineTracks.size() * (trackHeight + 5));
            draw_list->AddLine(lineStart, lineEnd, IM_COL32(57, 255, 20, 255), 2.0f);

            float grabZoneHalfWidth = 4.0f;
            ImVec2 scrubberGrabMin(scrubberX - grabZoneHalfWidth, timelineTop);
            ImVec2 scrubberGrabMax(scrubberX + grabZoneHalfWidth, timelineTop + rulerHeight);
            if (ImGui::IsMouseHoveringRect(scrubberGrabMin, scrubberGrabMax)) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                if (ImGui::IsMouseClicked(0)) {
                    isScrubberDragging = true;
                }
            }

            if (!ImGui::IsMouseDown(0)) {
                isScrubberDragging = false;
            }

            if (isScrubberDragging) {
                float mouseX = ImGui::GetMousePos().x;
                float timelineX = mouseX - cursorStart.x + scrollX;
                GlobalTransport::currentTime = std::clamp(timelineX / (1000.0f * pixelsPerMs), 0.0f, GlobalTransport::totalTime);
                GlobalTransport::isPlaying = false;
            }

            ImGui::EndChild();
            ImGui::End();
        }
        else {
            currentScene = nullptr;
            float curTimeInMs = currentTime * 1000.0f;
            for (size_t i{ 0 }; i < scenes.size(); ++i) {
                if (curTimeInMs < scenes[i]->endTime && curTimeInMs >= scenes[i]->startTime) {
                    currentScene = scenes[i];
                }
            }

            if (!currentScene) {
                currentScene = scenes.empty() ? nullptr : scenes.back();
            }
        }
    }
}
