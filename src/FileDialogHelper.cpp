#include "FileDialogHelper.h"
#include "GlobalTransport.h"
#include "Timeline.h"
#include "TimelineTrack.h"
#include "ImGuiFileDialog.h"
#include "ImGuiFileDialogConfig.h"
#include <filesystem>
#include <random>
#include <iostream>
#include <ctime>

namespace FileDialogHelper {
    std::string lastDirectory = ".";

    static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    static std::uniform_int_distribution<int> colorDist(80, 200);

    void process() {
        if (Timeline::openDialog) {
            ImGuiFileDialog::Instance()->OpenDialog(
                "ChooseFileDlgKey",
                "Choose Audio File",
                ".wav,.mp3,.aif",
                IGFD::FileDialogConfig{ lastDirectory }
            );
            Timeline::openDialog = false;
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", 0, ImVec2(500, 300), ImVec2(900, 600))) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                lastDirectory = std::filesystem::path(filePath).parent_path().string();

                auto newTrack = std::make_unique<TimelineTrack>();
                if (newTrack->loadTrack(filePath)) {
                    newTrack->displayName = std::filesystem::path(filePath).filename().string();
                    newTrack->startTime = GlobalTransport::currentTime;
                    newTrack->color = IM_COL32(
                        colorDist(rng),
                        colorDist(rng),
                        colorDist(rng),
                        255
                    );
                    newTrack->computeComplementaryColor();

                    Timeline::timelineTracks.push_back(std::move(newTrack));
                }
                else {
                    std::cerr << "Failed to load audio track: " << filePath << std::endl;
                }

            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
}
