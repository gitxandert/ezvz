#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "ImGuiFileDialogConfig.h"
#include "ImGuiFileDialog.h"

#include "AudioEngine.h"
#include "TimelineTrack.h"
#include "GlobalTransport.h"
#include "Timeline.h"
#include "FileDialogHelper.h"
#include "TrackFeatures.h"
#include "ScenesPanel.h"
#include "Canvas.h"
#include "Shader.h"
#include "MappingsWindow.h"

#include <iostream>
#include <algorithm>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    Canvas::recreate(width, height);
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

    // Initialize audio engine
    if (!initAudio()) {
        return -1;
    }

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGuiContext* ctx = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize timeline system    
    Timeline::init(screenW);

    Canvas::init(screenW, screenH);
    Canvas::shader = std::make_unique<Shader>("vertex.glsl", "fragment.glsl");

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
                    if (Timeline::currentScene)
                        GlobalTransport::currentTime = Timeline::currentScene->startTime / 1000.0f;
                    GlobalTransport::playStartTime = glfwGetTime() - GlobalTransport::currentTime;
                    for (auto& scene : Timeline::scenes)
                        scene->resetObjectAnimations();
                }
            }
            // Mute selected tracks
            if (ImGui::IsKeyPressed(ImGuiKey_M)) {
                for (auto& track : Timeline::timelineTracks) {
                    if (track.selected) {
                        track.muted = !track.muted;
                        updateTrackPlayback(track, GlobalTransport::currentTime);
                    }
                }
            }
            // Delete selected tracks
            if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                if (!TrackFeatures::showMappings && !Canvas::selectedObject && !ScenesPanel::showAnimateWindow) {
                    Timeline::timelineTracks.erase(
                        std::remove_if(
                            Timeline::timelineTracks.begin(),
                            Timeline::timelineTracks.end(),
                            [](TimelineTrack& t) {
                                if (t.selected) {
                                    if (t.decoderInitialized) {
                                        ma_decoder_uninit(&t.analyzerDecoder);
                                        t.decoderInitialized = false;
                                    }
                                    if (t.initialized) {
                                        ma_sound_uninit(&t.sound);
                                        t.initialized = false;
                                    }
                                    return true;
                                }
                                return false;
                            }
                        ),
                        Timeline::timelineTracks.end()
                    );
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
            for (auto& track : Timeline::timelineTracks) {
                std::cout << "Current time = " << GlobalTransport::currentTime << '\n';
                updateTrackPlayback(track, GlobalTransport::currentTime);
                // Audio feature extraction (envelope smoothing, etc.)
                if (track.decoderInitialized && track.initialized && !track.muted) {
                    ma_uint64 totalFrames = 0;
                    ma_decoder_get_length_in_pcm_frames(&track.analyzerDecoder, &totalFrames);
                    ma_uint64 currentFrame = 0;
                    ma_sound_get_cursor_in_pcm_frames(&track.sound, &currentFrame);
                    ma_uint64 frameToRead = (std::min)(currentFrame, totalFrames > 512 ? totalFrames - 512 : 0);
                    ma_decoder_seek_to_pcm_frame(&track.analyzerDecoder, frameToRead);
                    std::vector<float> samples(512);
                    ma_uint64 framesRead = 0;
                    if (ma_decoder_read_pcm_frames(&track.analyzerDecoder, samples.data(), 512, &framesRead) == MA_SUCCESS && framesRead > 0) {
                        track.analyzer.analyze(samples.data(), static_cast<size_t>(framesRead), 1);
                        float rawEnv = track.analyzer.getEnvelope();
                        track.smoothedEnvelope = track.smoothedEnvelope * (1.0f - track.smoothingAlpha)
                            + rawEnv * track.smoothingAlpha;
                    }
                }

                track.updateMappings();
            }
        }
        else if (!Timeline::isScrubberDragging) {
            for (auto& track : Timeline::timelineTracks) {
                if (track.hasPlayed) {
                    ma_sound_stop(&track.sound);
                    ma_sound_uninit(&track.sound);
                    track.hasPlayed = false;
                    track.initialized = false;
                }
            }
        }

        int fb_w, fb_h;
        glfwGetFramebufferSize(window, &fb_w, &fb_h);
        glViewport(0, 0, fb_w, fb_h);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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
        if (track.initialized) {
            ma_sound_uninit(&track.sound);
        }
        if (track.decoderInitialized) {
            ma_decoder_uninit(&track.analyzerDecoder);
        }
    }

    Canvas::shutdown();

    shutdownAudio();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
