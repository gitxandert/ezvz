#include "FileDialogHelper.h"
#include "AudioEngine.h"
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

                ma_sound tempSound;
                ma_result result = ma_sound_init_from_file(
                    &audioEngine,
                    filePath.c_str(),
                    MA_SOUND_FLAG_DECODE,
                    NULL,
                    NULL,
                    &tempSound
                );

                if (result == MA_SUCCESS) {
                    float duration = 0.0f;
                    ma_sound_get_length_in_seconds(&tempSound, &duration);
                    ma_sound_uninit(&tempSound);

                    TimelineTrack newTrack;
                    newTrack.filePath = filePath;
                    newTrack.displayName = std::filesystem::path(filePath).filename().string();
                    newTrack.startTime = GlobalTransport::currentTime;
                    newTrack.duration = duration;
                    newTrack.color = IM_COL32(
                        colorDist(rng),
                        colorDist(rng),
                        colorDist(rng),
                        255
                    );
                    newTrack.computeComplementaryColor();

                    ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_f32, 1, 0);
                    if (ma_decoder_init_file(newTrack.filePath.c_str(), &decoderConfig, &newTrack.analyzerDecoder) == MA_SUCCESS) {
                        newTrack.decoderInitialized = true;
                    }

                    Timeline::timelineTracks.push_back(std::move(newTrack));
                }
                else {
                    std::cerr << "Failed to load file for duration check: " << filePath << std::endl;
                }
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
}
