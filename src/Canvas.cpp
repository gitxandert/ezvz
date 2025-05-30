﻿#include "Canvas.h"
#include "GlobalTransport.h"
#include "ScenesPanel.h"
#include "TrackFeatures.h"
#include "Timeline.h"
#include "GraphicObject.h"
#include "Rectangle.h"
#include "Scene.h"
#include <iostream>
#include <memory>

namespace Canvas {

    GLuint quadVAO = 0;
    
    static GLuint msFbo = 0, msColorRbo = 0, msDepthRbo = 0;

    static GLuint fbo = 0, colorTex = 0, depthRbo = 0;

    float padding = 10.0f;

    extern float x{}, y{}, w{}, h{};

	std::shared_ptr<GraphicObject> selectedObject = nullptr;
	bool isDraggingObject = false;
    bool clicked = false;

    float Canvas::screenW, Canvas::screenH;

    glm::mat4 Canvas::projFullScreen;

    ImVec2 cm, sz;
    ImVec2 fboDrawPos;
    float fboDrawW, fboDrawH;

    int currentW = 0;
    int currentH = 0;

    std::shared_ptr<Scene> currScene;

    // helper to turn ImGui::MousePos → world‐space click
    glm::vec2 getClickWorld(const ImGuiIO& io) {
        // 1) raw mouse relative to Canvas top‐left (origin = top‐left, +y down)
        glm::vec2 rawLocal{
          io.MousePos.x - cm.x,
          io.MousePos.y - cm.y
        };

        // 2) flip y so origin = bottom‐left (now +y up)
        rawLocal.y = sz.y - rawLocal.y;

        // 3) un‐scale back into full‐screen world-coords
        return {
          rawLocal.x / float(sz.x),
          rawLocal.y / float(sz.y)
        };
    }

    bool hitTestOBB(
        const Transform& T,             // position/rotation/scale
        const glm::vec3& fullSize,      // getSize()
        const glm::vec2& clickPos      // local = MousePos – canvasOrigin
    ) {

        // 1) half‐extents in object units
        glm::vec3 half = fullSize * 0.5f;

        // 2) build the model matrix (Z‐axis rotation)
        glm::mat4 M = glm::translate(glm::mat4(1.0f),
            glm::vec3(T.position.x,
                T.position.y,
                0.0f))
            * glm::rotate(glm::mat4(1.0f),
                glm::radians(-T.rotation.z),
                glm::vec3(0, 0, 1))
            * glm::scale(glm::mat4(1.0f),
                glm::vec3(fullSize.x,
                    fullSize.y,
                    1.0f));

        // 3) invert it
        glm::mat4 invM = glm::inverse(M);

        // 4) transform click into object‐local
        glm::vec4 localPt = invM * glm::vec4(clickPos.x, clickPos.y, 0.0f, 1.0f);

        // 5) axis‐aligned bounding‐box check
        bool hit2D =
            std::abs(localPt.x) <= half.x &&
            std::abs(localPt.y) <= half.y;
        // if you cared about Z (e.g. 3D picking), do:
        // bool hit3D = std::abs(localPt.z) <= half.z;
        // return hit2D && hit3D;

        return hit2D;
    }

    void init(int w, int h)
    {
        screenW = static_cast<float>(w);
        screenH = static_cast<float>(h);

        projFullScreen = glm::ortho(0.0f, screenW, screenH, 0.0f);

        glGenFramebuffers(1, &msFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, msFbo);

        // color as a renderbuffer (4 samples)
        glGenRenderbuffers(1, &msColorRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, msColorRbo);
        glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            4,               // samples
            GL_RGBA8,
            w, h
        );
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_RENDERBUFFER,
            msColorRbo
        );

        // depth+stencil as a renderbuffer (4 samples)
        glGenRenderbuffers(1, &msDepthRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, msDepthRbo);
        glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            4,
            GL_DEPTH24_STENCIL8,
            w, h
        );
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER,
            msDepthRbo
        );

        // check...
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "MSAA FBO incomplete\n";

        // 1) Generate the color texture
        glGenTextures(1, &colorTex);
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
            w, h,
            0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 2) Depth renderbuffer
        glGenRenderbuffers(1, &depthRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
        glRenderbufferStorage(GL_RENDERBUFFER,
            GL_DEPTH24_STENCIL8,
            w, h);

        // 3) Framebuffer
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // 4) Attachments
        glFramebufferTexture2D(GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            colorTex, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER,
            depthRbo);

        // 5) **Tell GL which color attachments to use**
        GLenum drawBufs[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBufs);

        // 6) Check completeness
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Canvas FBO not complete in init(): 0x"
                << std::hex << status << std::dec << "\n";
        }

        // 7) Unbind
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (quadVAO == 0) {
            // a unit quad from (0,0) to (1,1)
            float verts[] = {
                0,0,
                1,0,
                1,1,
                0,0,
                1,1,
                0,1
            };
            GLuint vbo;
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &vbo);

            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glBindVertexArray(0);
        }

        currentW = w;
        currentH = h;
    }

    void shutdown() {
        if (quadVAO) {
            GLuint vbo = 0;
            glBindVertexArray(quadVAO);
            glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, (GLint*)&vbo);
            glBindVertexArray(0);
            glDeleteBuffers(1, &vbo);
            glDeleteVertexArrays(1, &quadVAO);
            quadVAO = 0;
        }

        glDeleteRenderbuffers(1, &msColorRbo);
        glDeleteRenderbuffers(1, &msDepthRbo);
        glDeleteFramebuffers(1, &msFbo);

        glDeleteTextures(1, &colorTex);
        glDeleteRenderbuffers(1, &depthRbo);
        glDeleteFramebuffers(1, &fbo);
    }

    void recreate(int newW, int newH) {
        if (newW == currentW && newH == currentH) return;

        shutdown();

        glGenFramebuffers(1, &msFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, msFbo);

        // color as a renderbuffer (4 samples)
        glGenRenderbuffers(1, &msColorRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, msColorRbo);
        glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            4,               // samples
            GL_RGBA8,
            w, h
        );
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_RENDERBUFFER,
            msColorRbo
        );

        // depth+stencil as a renderbuffer (4 samples)
        glGenRenderbuffers(1, &msDepthRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, msDepthRbo);
        glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            4,
            GL_DEPTH24_STENCIL8,
            w, h
        );
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER,
            msDepthRbo
        );

        // check...
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "MSAA FBO incomplete\n";

        // 1) Generate the color texture
        glGenTextures(1, &colorTex);
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
            w, h,
            0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 2) Depth renderbuffer
        glGenRenderbuffers(1, &depthRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
        glRenderbufferStorage(GL_RENDERBUFFER,
            GL_DEPTH24_STENCIL8,
            w, h);

        // 3) Framebuffer
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // 4) Attachments
        glFramebufferTexture2D(GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            colorTex, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER,
            depthRbo);

        // 5) **Tell GL which color attachments to use**
        GLenum drawBufs[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBufs);

        // 6) Check completeness
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Canvas FBO not complete in init(): 0x"
                << std::hex << status << std::dec << "\n";
        }

        // 7) Unbind
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (quadVAO == 0) {
            // a unit quad from (0,0) to (1,1)
            float verts[] = {
                0,0,
                1,0,
                1,1,
                0,0,
                1,1,
                0,1
            };
            GLuint vbo;
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &vbo);

            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glBindVertexArray(0);
        }

        currentW = w;
        currentH = h;
    }

    glm::vec3 c_center() {
        return {
            screenW * 0.5f,
            screenH * 0.5f,
            0.0f
        };
    }

    std::unique_ptr<Shader> shader;

    void Canvas::render() {
        if (Timeline::currentScene) {
            if (Timeline::currentScene != currScene) {
                currScene = Timeline::currentScene;
                selectedObject = nullptr;
                ScenesPanel::showAnimateWindow = false;
            }
        }
        else {
            currScene = nullptr;
            selectedObject = nullptr;
            ScenesPanel::showAnimateWindow = false;
        }

        ImGuiIO& io = ImGui::GetIO();

        // 1) Compute and reserve your Canvas window
        const float topBarHeight = 60.0f;
        const float timelineHeight = Timeline::timelineFixedHeight;
        const float scenesPanelWidth = ScenesPanel::panelWidth;
        const float featuresPanelWidth = TrackFeatures::panelWidth;

        x = padding + featuresPanelWidth;
        y = topBarHeight + padding;
        w = io.DisplaySize.x
            - scenesPanelWidth
            - featuresPanelWidth
            - 2 * padding;
        h = io.DisplaySize.y
            - topBarHeight
            - timelineHeight
            - 2 * padding;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowPos({ x,y }, ImGuiCond_Always);
        ImGui::SetNextWindowSize({ w,h }, ImGuiCond_Always);
        ImGui::Begin("Canvas", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBackground
        );

        // 2) Grab the true drawable region
        cm = ImGui::GetCursorScreenPos();
        sz = ImGui::GetContentRegionAvail();
        if (sz.x <= 0 || sz.y <= 0) {
            ImGui::End();
            return;
        }

        float aspectFBO = screenW / screenH;
        float aspectCanvas = sz.x / sz.y;

        float drawW, drawH;
        if (aspectCanvas > aspectFBO) {
            drawH = Canvas::sz.y;
            drawW = Canvas::sz.y * aspectFBO;
        }
        else {
            drawW = Canvas::sz.x;
            drawH = Canvas::sz.x / aspectFBO;
        }
        fboDrawW = drawW;
        fboDrawH = drawH;
        fboDrawPos = Canvas::cm;
        fboDrawPos.x += (Canvas::sz.x - drawW) * 0.5f;
        fboDrawPos.y += (Canvas::sz.y - drawH) * 0.5f;



        int newW = int(sz.x), newH = int(sz.y);
        if (newW != currentW || newH != currentH) {
            // Recreate MSAA + single-sample attachments at exactly newW×newH
            recreate(newW, newH);
            // Update “world” dims
            projFullScreen = glm::ortho(0.0f, screenW, screenH, 0.0f);
            currentW = newW;
            currentH = newH;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, msFbo);
        glViewport(0, 0, newW, newH);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader->bind();
        shader->setUniformMat4("uProjection", projFullScreen);

        // 6) Draw shapes
        if (currScene) {
            for (auto& obj : currScene->objects) {

                // 1) grab its transform
                auto const& T = obj->getTransform();

                // 2) build the “base” model matrix (we’ll reuse for both passes)
                glm::mat4 translate = glm::translate(glm::mat4(1.0f),
                    { T.position.x,
                      T.position.y,
                      0.0f });
                glm::mat4 rotate = glm::rotate(glm::mat4(1.0f),
                    glm::radians(-T.rotation.z),
                    glm::vec3{ 0,0,1 });
                glm::mat4 baseScale = glm::scale(glm::mat4(1.0f),
                    { T.scale.x, T.scale.y, 1.0f });

                // ── HALO PASS ──
                if (obj == selectedObject) {
                    // turn off depth‐testing so the halo doesn’t write to (or get occluded by) the depth buffer
                    glDisable(GL_DEPTH_TEST);
                    // make it e.g. 10% larger:
                    float haloFactor = 1.08f;
                    glm::mat4 haloScale = glm::scale(glm::mat4(1.0f),
                        { T.scale.x * haloFactor,
                            T.scale.y * haloFactor,
                            1.0f });

                    glm::mat4 haloModel = translate * rotate * haloScale;
                    shader->setUniformMat4("uModel", haloModel);

                    // translucent yellow/orange
                    shader->setUniformVec4("uColor",
                        { 1.0f, 1.0f, 1.0f, 0.4f });
                    // draw only edges
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glLineWidth(2.0f);              // thickness of outline

                    obj->draw();

                    // restore
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glLineWidth(1.0f);
                    glEnable(GL_DEPTH_TEST);
                }

                // ── NORMAL PASS ──
                glm::mat4 model = translate * rotate * baseScale;
                shader->setUniformMat4("uModel", model);
                shader->setUniformVec4("uColor", obj->getMaterial().color);
                obj->draw();
            }
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, msFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glBlitFramebuffer(
            0, 0, newW, newH,
            0, 0, newW, newH,
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST
        );

        // 7) Unbind FBO and display in ImGui
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        ImGui::SetCursorScreenPos(fboDrawPos);
        ImGui::Image((ImTextureID)(uintptr_t)colorTex,
            ImVec2(fboDrawW, fboDrawH)
        );
        
        ImGui::SetCursorScreenPos(fboDrawPos);
        ImGui::InvisibleButton("canvas_click",
            ImVec2(fboDrawW, fboDrawH)
        );

        static glm::vec2 dragOffset{ 0.0f };

        clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
        if (clicked) {
            glm::vec2 pixel = {
                io.MousePos.x - cm.x,
                io.MousePos.y - cm.y
            };
            // flip Y:
            pixel.y = fboDrawH - pixel.y;
            glm::vec2 world = {
              pixel.x * (screenW / fboDrawW),
              pixel.y * (screenH / fboDrawH)
            };

            if (selectedObject) {
                auto& T = selectedObject->getTransform();
                bool over = hitTestOBB(
                    T,
                    selectedObject->getSize(),
                    world
                );
                if (!over) {
                    std::cout << "Missed!\n";
                    isDraggingObject = false;
                    Canvas::clearSelected();
                }
                else {
                    std::cout << "Hit object\n";
                    isDraggingObject = true;
                    dragOffset = glm::vec2(T.position.x, T.position.y) - world;
                }
            }
        }

        if (isDraggingObject) {
            glm::vec2 pixel = {
                io.MousePos.x - cm.x,
                io.MousePos.y - cm.y
            };
            // flip Y:
            pixel.y = fboDrawH - pixel.y;
            glm::vec2 world = {
              pixel.x * (screenW / fboDrawW),
              pixel.y * (screenH / fboDrawH)
            };

            if (io.MouseDown[ImGuiMouseButton_Left]) {
                selectedObject->setPosition({ world.x + dragOffset.x, world.y + dragOffset.y, 0.0f });
            }
            else {
                isDraggingObject = false;
            }
        }

        ImGui::PopStyleVar();
        ImGui::End();
    }
}