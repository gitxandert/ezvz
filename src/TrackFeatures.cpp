#include "TrackFeatures.h"
#include "Timeline.h"
#include "TimelineTrack.h"
#include "imgui.h"
#include "MappingsWindow.h"
#include "ScenesPanel.h"
#include <cmath>

namespace TrackFeatures {

    extern float panelWidth = 250.0;
    bool showMappings = false;

    std::size_t p_index = 0;

    const std::vector<std::string> parameters{
        "Envelope",
        "Zero Crossing Rate",
        "Spectral Centroid",
        "Spectral Flatness",
        "Spectral Rolloff",
        "Spectral Contrast",
        "Spectral Bandwidth",
        "Spectral Entropy",
        "Spectral Flux",
        "Spectral Skewness",
		"Spectral Kurtosis"
    };

	TimelineTrack* selectedTrack = nullptr;

    void render() {
        // Find selected track
        selectedTrack = nullptr;
        for (auto& track : Timeline::timelineTracks) {
            if (track->selected) {
                selectedTrack = track.get();
                break;
            }
        }

        if (selectedTrack) {
            ImGuiIO& io = ImGui::GetIO();
            const float timelineFixedHeight = Timeline::timelineFixedHeight;

            // Position the feature panel below the transport bar and above the timeline
            ImGui::SetNextWindowPos(ImVec2(0, 60), ImGuiCond_Always);
            ImGui::SetNextWindowSize(
                ImVec2(panelWidth, io.DisplaySize.y - 60 - timelineFixedHeight),
                ImGuiCond_Always
            );
            ImGui::Begin("Track Features", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

            const char* name = selectedTrack->displayName.c_str();
            bool trunc = selectedTrack->displayName.size() > 7;
            ImGui::Text("Track: %.*s%s",
                7,
                name,
                trunc ? "..." : ""
            );

            const char* mapBtnLabel = showMappings ? "Hide Mappings" : "View Mappings";
            if (!ScenesPanel::showAnimateWindow) {
                ImGui::SameLine();
                if (ImGui::Button(mapBtnLabel)) {
                    showMappings = !showMappings;
                    if (!showMappings)
                        MappingsWindow::addingMapping = false;
                }
            }

            ImGui::Separator();

            float env = selectedTrack->currentEnvelope.load();
            int zcr = selectedTrack->analyzer.getZeroCrossingRate();

            ImDrawList* dl = ImGui::GetWindowDrawList();
            float avail = ImGui::GetContentRegionAvail().x;
            float lh = ImGui::GetTextLineHeightWithSpacing() + 5;

            ImVec2 minPos0 = ImGui::GetCursorScreenPos();
            ImGui::Text("Envelope: %.4f", env);
            if (showMappings && ImGui::IsItemClicked()) {
                p_index = 0;
            }

            ImGui::ProgressBar(env, ImVec2(-FLT_MIN, 0));

            ImVec2 minPos1 = ImGui::GetCursorScreenPos();
            float posdiff = minPos1.y - minPos0.y;

            if (showMappings && p_index == 0) {
                dl->AddRectFilled(
                    minPos0,
                    { minPos0.x + avail, minPos1.y },
                    IM_COL32(255, 127, 0, 100)
                );
            }

            if (showMappings && ImGui::IsItemClicked()) {
                p_index = 0;
            }

            ImGui::SliderFloat(
                "Smoothness",
                &selectedTrack->smoothNorm,
                0.0f,
                1.0f,
                "%.2f"
            );

            ImGui::Separator();

			ImVec2 minPos2 = ImGui::GetCursorScreenPos();
            if (showMappings && p_index == 1) {
                dl->AddRectFilled(
                    minPos2,
                    { minPos2.x + avail, minPos2.y + lh },
                    IM_COL32(255, 127, 0, 100)
				);
            }
            ImGui::Text("ZCR: %d", zcr);
            if(showMappings && ImGui::IsItemClicked()) {
                p_index = 1;
			}

            float t = selectedTrack->smoothNorm;
            // Recalculate smoothing alpha based on norm
            selectedTrack->smoothingAlpha = 0.5f * powf(0.01f / 0.5f, t);

            ImGui::End();

            if (showMappings)
                MappingsWindow::showMappingsWindow(selectedTrack, parameters[p_index], p_index);
        }
    }
}
