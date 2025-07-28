#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "ImGuiFileDialogConfig.h"
#include "ImGuiFileDialog.h"

#include "TimelineTrack.h"
#include "GlobalTransport.h"
#include "Timeline.h"
#include "FileDialogHelper.h"
#include "TrackFeatures.h"
#include "ScenesPanel.h"
#include "Canvas.h"
#include "Shader.h"
#include "MappingsWindow.h"
#include "Style.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <filesystem>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    Canvas::recreate(width, height);
}

void my_audio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    float* out = (float*)pOutput;
    memset(out, 0, frameCount * 2 * sizeof(float)); // Stereo, silence

    for (auto& track : Timeline::timelineTracks) {
        if (!track->decoderInitialized || !track->playing || track->muted)
            continue;

        // Seek decoder if needed
        ma_decoder_seek_to_pcm_frame(&track->decoder, track->nextFrame);

        std::vector<float> tempBuf(frameCount * track->channelCount, 0.0f);
        ma_uint64 framesRead = 0;
        ma_decoder_read_pcm_frames(&track->decoder, tempBuf.data(), frameCount, &framesRead);

        // Mix into output buffer (ALWAYS outputting stereo)
        for (ma_uint64 f = 0; f < framesRead; ++f) {
            float L = tempBuf[f * track->channelCount + 0];
            float R = (track->channelCount > 1) ? tempBuf[f * track->channelCount + 1] : L;
            out[f * 2 + 0] += L;
            out[f * 2 + 1] += R;
        }

        // ─── Audio-feature extraction (envelope + ZCR) ──────────────────────
        track->analyzer.analyze(       /* analyzes this block */
            tempBuf.data(),        // pointer to interleaved samples
            framesRead,            // number of *frames* we actually decoded
            track->channelCount);  // 1 for mono, 2 for stereo

        track->currentEnvelope.store(track->analyzer.getSmoothedEnvelope(),
            std::memory_order_relaxed);


        // Advance playback pointer
        track->nextFrame += framesRead;

        // End/loop handling (optionally)
        ma_uint64 totalFrames = 0;
        ma_decoder_get_length_in_pcm_frames(&track->decoder, &totalFrames);
        if (track->nextFrame >= totalFrames) {
            track->playing = false; // or set nextFrame=0 to loop
        }
    }
}

std::filesystem::path getProjectRelativePath(const std::string& relativePathFromRoot) {
    std::filesystem::path base = std::filesystem::current_path();
    for (int i = 0; i < 3; ++i)
        base = base.parent_path();

    return base / relativePathFromRoot;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Audio Visualizer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL functions via GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_MULTISAMPLE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    float screenW = float(mode->width);
    float screenH = float(mode->height);

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGuiContext* ctx = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    setStyle();

    std::filesystem::path fontPath = getProjectRelativePath("assets/fonts/RobotoMono.ttf");
    if (!std::filesystem::exists(fontPath)) {
        std::cerr << "Font not found at: " << fontPath << "\n";
    }
    else {
        io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 16.0f);
    }

    io.FontDefault = io.Fonts->Fonts.back();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize timeline system    
    Timeline::init(screenW);

    Canvas::init(screenW, screenH);
    Canvas::shader = std::make_unique<Shader>("vertex.glsl", "fragment.glsl");

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;
    config.playback.channels = 2; // stereo
    config.sampleRate = 48000;
    config.dataCallback = my_audio_callback; // <-- your function
    config.pUserData = nullptr;

    ma_device device;
    ma_result result = ma_device_init(NULL, &config, &device);
    if (result != MA_SUCCESS) {
        std::cerr << "Device was unable to be initialized. Closing program.";
		return -1;
    }
    ma_device_start(&device);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (Timeline::currentScene && GlobalTransport::isPlaying) {
            for (auto& obj : Timeline::currentScene->objects) {
                obj->update();
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Keyboard controls & audio updates
        if (!io.WantCaptureKeyboard) {
            // Play/Pause toggle
            if (ImGui::IsKeyPressed(ImGuiKey_Space)) {

                GlobalTransport::isPlaying = !GlobalTransport::isPlaying;
                if (GlobalTransport::isPlaying) {
                    if (!Timeline::currentScene)
						Timeline::currentScene = Timeline::scenes.empty() ? nullptr : Timeline::scenes.back();

                    if(Timeline::currentScene)
                        GlobalTransport::currentTime = Timeline::currentScene->startTime / 1000.0f;

                    GlobalTransport::playStartTime = glfwGetTime() - GlobalTransport::currentTime;
					std::cout << "current time = " << GlobalTransport::currentTime << "\n";
                }

                for (auto& scene : Timeline::scenes) {
                    std::cout << "I am resetting all of the scenes yo\n";
                    scene->resetObjectAnimations();
                }
            }
        }

        if (GlobalTransport::isPlaying) {
            float now = glfwGetTime();
            GlobalTransport::currentTime = now - GlobalTransport::playStartTime;
            if (GlobalTransport::currentTime >= GlobalTransport::totalTime) {
                if (GlobalTransport::isLooping) {
                    GlobalTransport::playStartTime = now;
                    GlobalTransport::currentTime = 0.0f;
                }
                else {
                    GlobalTransport::currentTime = GlobalTransport::totalTime;
                    GlobalTransport::isPlaying = false;
                }
            }

			float currentTime = GlobalTransport::currentTime;
            for (auto& track : Timeline::timelineTracks) {
                bool inRegion = (currentTime >= track->startTime) && (currentTime < track->startTime + track->duration);

                if (inRegion && !track->playing) {
                    track->playTrack(currentTime);
                }
                else if (!inRegion && track->playing) {
                    track->stopTrack();
                }

                track->updateMappings();
            }
        }
        else {
            for (auto& track : Timeline::timelineTracks) {
                if (track->playing) {
                    track->stopTrack();
                }
			}
        }

        int fb_w, fb_h;
        glfwGetFramebufferSize(window, &fb_w, &fb_h);
        glViewport(0, 0, fb_w, fb_h);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(CLEAR_COL.x, CLEAR_COL.y, CLEAR_COL.z, CLEAR_COL.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render UI windows
        float currTime = GlobalTransport::render();
        Timeline::render(currTime);
        Canvas::render();
        ScenesPanel::render();
        FileDialogHelper::process();
        TrackFeatures::render();

        // Finalize frame
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        std::this_thread::yield();
    }

    // Cleanup tracks and resources
    for (auto& track : Timeline::timelineTracks) {
        track->unloadTrack();
    }

    ma_device_uninit(&device);

    Canvas::shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
