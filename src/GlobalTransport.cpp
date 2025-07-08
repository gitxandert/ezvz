#include "GlobalTransport.h"
#include "Timeline.h"
#include "Scene.h"
#include "imgui.h"
#include "Menu.h"
#include <iostream>

namespace GlobalTransport {
    float transportHeight = 80.0f;

    bool isPlaying = false;
    bool isLooping = false;
    float currentTime = 0.0f;
    float totalTime = 300.0f;
    float playStartTime = 0.0f;
    float loopStart = 0.0f;
    float loopEnd = 0.0f;
    static std::shared_ptr<Scene> loopScene = nullptr;

    void setLoop() {
        loopScene = Timeline::currentScene;
        loopStart = loopScene->startTime / 1000.0f;
        loopEnd = loopScene->endTime / 1000.0f;
    }

    void resetLoop() {
        isLooping = false;
        loopScene = nullptr;
    }

    float render() {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, transportHeight), ImGuiCond_Always);
        ImGui::Begin("Global Transport", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);

        menuBar();

        float centerX = io.DisplaySize.x * 0.3f;
        float contentWidth = 500.0f;
        ImGui::SetCursorPosX(centerX - contentWidth * 0.5f);

        if (ImGui::Button("|<<")) currentTime = 0.0f;
        ImGui::SameLine();
        
        if (isLooping && loopScene) {
            if (currentTime >= loopEnd) {
                Timeline::currentScene = loopScene;
                currentTime = loopStart;
                playStartTime = glfwGetTime() - currentTime;
                for (auto& obj : loopScene->objects)
                    obj->resetAnimations();
            }
        }

        if (ImGui::Button(isPlaying ? "||" : ">")) {
            isPlaying = !isPlaying;
            if (isPlaying) {
                if (Timeline::currentScene) {
                    currentTime = Timeline::currentScene->startTime / 1000.0f;
                }
                playStartTime = glfwGetTime() - currentTime;
                for (auto& scene : Timeline::scenes)
                    scene->resetObjectAnimations();
            }
        }


        ImGui::SameLine();
        if (ImGui::Button(isLooping ? "Loop On" : "Loop Off")) {
            isLooping = !isLooping;
            if (isLooping) {
                if (Timeline::currentScene) {
                    setLoop();
                }
            }
        }

        ImGui::SameLine();

        ImGui::SetNextItemWidth(io.DisplaySize.x * 0.4f);
        ImGui::SliderFloat("##TransportScrubber", &currentTime, 0.0f, totalTime, "%.2f s", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine();

        char timeBuf[64];
        snprintf(timeBuf, sizeof(timeBuf), "%.2f / %.2f sec", currentTime, totalTime);
        ImGui::Text("%s", timeBuf);

        ImGui::End();

        return currentTime;
    }
}
