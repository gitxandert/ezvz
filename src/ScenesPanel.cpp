#include <memory>
#include <vector>
#include <string>
#include <array>
#include <iostream>
#include "Canvas.h"
#include "Timeline.h"
#include "imgui.h"
#include "Scene.h"
#include "GraphicObject.h"
#include "Line.h"
#include "Rectangle.h"
#include "Ellipse.h"
#include "Triangle.h"
#include "Star.h"
#include "GlobalTransport.h"
#include "AnimationInfo.h"
#include "Mapping.h"
#include "MappingsWindow.h"
#include "TrackFeatures.h"

namespace ScenesPanel {

    extern float panelWidth = 350.0;
    static const float minPanelWidth = 100.0;

    static int activeScene = 0;
    static std::array<int, static_cast<size_t>(ObjectType::COUNT)> objectCount{};

    static bool hasSelectedObject = false;
    static int selectedIndex = -1;
    static int editingIndex = -1;
    static char renameBuf[64] = "";

    bool showAnimateWindow = false;
    int animPropIndex = 0;
    int mappingIndex = -1;

    const std::vector<std::string> parameters{
        "Position",
        "Rotation",
        "Size",
        "Hue/Sat.",
        "Brightness",
        "Alpha"
    };

    std::array<ImVec2, 6> minPos = { ImVec2{} };

    // color conversion functions
    glm::vec4 rgb2hsv(const glm::vec4& c) {
        float mx = glm::max(glm::max(c.r, c.g), c.b);
        float mn = glm::min(glm::min(c.r, c.g), c.b);
        float d = mx - mn;
        float h = 0.f;

        if (d > 1e-6f) {
            if (mx == c.r) h = fmod((c.g - c.b) / d, 6.f);
            else if (mx == c.g) h = (c.b - c.r) / d + 2.f;
            else                h = (c.r - c.g) / d + 4.f;
            h /= 6.f;
            if (h < 0.f) h += 1.f;
        }
        float s = (mx < 1e-6f) ? 0.f : d / mx;
        float v = mx;
        return { h, s, v, c.w };
    }

    glm::vec4 hsv2rgb(const glm::vec4& c) {
        float h = c.x * 6.f;
        float s = c.y;
        float v = c.z;
        int   i = int(floor(h)) % 6;
        float f = h - floor(h);
        float p = v * (1.f - s);
        float q = v * (1.f - s * f);
        float t = v * (1.f - s * (1.f - f));

        switch (i) {
        case 0: return { v, t, p, c.w };
        case 1: return { q, v, p, c.w };
        case 2: return { p, v, t, c.w };
        case 3: return { p, q, v, c.w };
        case 4: return { t, p, v, c.w };
        default: return { v, p, q, c.w };
        }
    }

    void renderAddObjectPopup() {
        if (ImGui::BeginPopup("AddObjectPopup")) {
            for (int idx{ 0 }; idx < static_cast<int>(ObjectType::COUNT); ++idx) {
                if (ImGui::Selectable(objectTypeNames[idx])) {
                    auto type{ static_cast<ObjectType>(idx) };
                    std::shared_ptr<GraphicObject> obj;
                    std::string id = objectTypeNames[idx];
                    objectCount[idx] += 1;
                    id += "_";
                    id += std::to_string(objectCount[idx]);
                    switch (type)
                    {
                    case ObjectType::Line: {
                        obj = std::make_shared<LineObject>(type, id);
                        break;
                    }
                    case ObjectType::Rectangle: {
                        obj = std::make_shared<RectangleObject>(type, id);
                        break;
                    }
                    case ObjectType::Ellipse: {
                        obj = std::make_shared<EllipseObject>(type, id);
                        break;
                    }
                    case ObjectType::Triangle: {
                        obj = std::make_shared<TriangleObject>(type, id);
                        break;
                    }
                    case ObjectType::Star: {
                        obj = std::make_shared<StarObject>(type, id);
                        break;
                    }
                    default: break;
                    }
                    obj->setPosition(Canvas::c_center());
                    Timeline::currentScene->objects.push_back(obj);
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
    }

    void hoverFunc(ImDrawList* dl, ImVec2 pos, float avail, float lh, int parameterIndex) {
        if (MappingsWindow::addingMapping) {
            if (ImGui::IsItemHovered()) {

                ImGuiIO& io = ImGui::GetIO();
                ImVec2 mousePos = io.MousePos;
                ImVec2 paramPos = { pos.x + avail * 2.0f / 3.0f, pos.y };

                bool isY = false;

                if (mousePos.x > pos.x + avail * 0.65f / 2.0f)
                    isY = true;

                if (!MappingsWindow::isTrigger) {
                    if (parameterIndex == 0 || parameterIndex == 2 || parameterIndex == 3) {
                        avail *= 0.65f / 2.0f;

                        if (Canvas::selectedObject && !Canvas::selectedObject->isMapped(parameterIndex)) {
                            dl->AddRectFilled(
                                paramPos,
                                { paramPos.x + avail, paramPos.y + lh },
                                IM_COL32(255, 127, 0, 100)
                            );
                        }

                        if (isY)
                            pos.x += avail;

                        dl->AddRectFilled(
                            pos,
                            { pos.x + avail, pos.y + lh },
                            IM_COL32(255, 127, 0, 100)
                        );
                    }
                    else if (Canvas::selectedObject && !Canvas::selectedObject->isMapped(parameterIndex)) {
                        dl->AddRectFilled(
                            pos,
                            { pos.x + avail, pos.y + lh },
                            IM_COL32(255, 127, 0, 100)
                        );
                    }

                    if (ImGui::IsItemClicked()) {
                        Canvas::selectedObject->setMapped(parameterIndex, isY);

                        AudioParameter ap = AudioParameter(MappingsWindow::audioIndex);
                        GraphicParameter gp = GraphicParameter(parameterIndex);
                        auto newMapping = std::make_shared<SyncMapping>(Canvas::selectedObject, ap, gp, MapType::Sync, isY);
                        TrackFeatures::selectedTrack->mappings[MappingsWindow::audioIndex].push_back(newMapping);

                        MappingsWindow::addingMapping = false;
                        mappingIndex = parameterIndex;
                        MappingsWindow::selectedMapping = TrackFeatures::selectedTrack->mappings[MappingsWindow::audioIndex].back();
                    }
                }
                else
                {
                    if (ImGui::IsItemHovered()) {
                        dl->AddRectFilled(
                            pos,
                            { pos.x + avail, pos.y + lh },
                            IM_COL32(255, 255, 0, 100)
                        );

                        if (ImGui::IsItemClicked()) {
                            animPropIndex = parameterIndex;
                            AnimationInfo::settingTrigger = true;
                            showAnimateWindow = true;
                        }
                    }
                }
            }
        } else if (showAnimateWindow) {
            if (ImGui::IsItemHovered()) {
                dl->AddRectFilled(
                    pos,
                    { pos.x + avail, pos.y + lh },
                    IM_COL32(255, 255, 0, 100)
                );

                if (ImGui::IsItemClicked()) 
                    animPropIndex = parameterIndex;
            }
        }
    }

    void drawRects(ImDrawList* dl, float avail, float lh) {
        if (showAnimateWindow) {
            dl->AddRectFilled(
                minPos[animPropIndex],
                { minPos[animPropIndex].x + avail, minPos[animPropIndex].y + lh },
                IM_COL32(255, 255, 0, 100)
            );
        }

        if (TrackFeatures::showMappings) {
            for (std::size_t i = 0; i < 6; ++i) {
                if (Canvas::selectedObject && Canvas::selectedObject->isMapped(i)) {
                    ImVec2 pos = minPos[i];
                    ImVec2 paramPos = { pos.x + avail * 2.0f / 3.0f, pos.y };
					float portion = avail;

                    if (i == 0 || i == 2 || i == 3) {
                        portion *= 0.65f / 2.0f;

                        dl->AddRectFilled(
                            paramPos,
                            { paramPos.x + portion, paramPos.y + lh },
                            IM_COL32(255, 127, 0, 100)
                        );
                        if (mappingIndex == i) {
                            dl->AddRect(
                                paramPos,
                                { paramPos.x + portion, paramPos.y + lh },
                                IM_COL32(255, 255, 255, 255)
                            );
						}

                        std::size_t yIndex = 0;

                        switch (i) {
                        case 2: yIndex = 1; break;
						case 3: yIndex = 2; break;
                        }
                        
                        switch (Canvas::selectedObject->isMappedY(yIndex)) {
                        case 1: {
                            dl->AddRectFilled(
                                pos,
                                { pos.x + portion, pos.y + lh },
                                IM_COL32(255, 127, 0, 100)
							);
                            if(mappingIndex == i) {
                                dl->AddRect(
                                    pos,
                                    { pos.x + portion, pos.y + lh },
                                    IM_COL32(255, 255, 255, 255)
                                );
							}
                            break;
                        }
                        case 2: {
                            dl->AddRectFilled(
                                { pos.x + portion, pos.y },
                                { pos.x + portion * 2.0f, pos.y + lh },
                                IM_COL32(255, 127, 0, 100)
                            );
                            if (mappingIndex == i) {
                                dl->AddRect(
                                    { pos.x + portion, pos.y },
                                    { pos.x + portion * 2.0f, pos.y + lh },
                                    IM_COL32(255, 255, 255, 255)
                                );
                            }
							break;
                        }
                        case 3: {
                            dl->AddRectFilled(
                                pos,
                                { pos.x + portion, pos.y + lh },
                                IM_COL32(255, 127, 0, 100)
                            );
                            dl->AddRectFilled(
                                { pos.x + portion, pos.y },
                                { pos.x + portion * 2.0f, pos.y + lh },
                                IM_COL32(255, 127, 0, 100)
                            );
                            if (mappingIndex == i) {
                                if (!MappingsWindow::selectedMapping->getGParamY()) {
                                    dl->AddRect(
                                        pos,
                                        { pos.x + portion, pos.y + lh },
                                        IM_COL32(255, 255, 255, 255)
                                    );
                                }
                                else {
                                    dl->AddRect(
                                        { pos.x + portion, pos.y },
                                        { pos.x + portion * 2.0f, pos.y + lh },
                                        IM_COL32(255, 255, 255, 255)
                                    );
                                }
                            }
							break;
                        }
                        }
                    }
                    else {
                        dl->AddRectFilled(
                            pos,
                            { pos.x + portion, pos.y + lh },
                            IM_COL32(255, 127, 0, 100)
                        );
                        if(mappingIndex == i) {
                            dl->AddRect(
                                pos,
                                { pos.x + portion, pos.y + lh },
                                IM_COL32(255, 255, 255, 255)
                            );
						}
                    }
				}
            }
		}
    }

	void render() {

        ImGuiIO& io = ImGui::GetIO();

        float displayX = io.DisplaySize.x;
        float panelY = io.DisplaySize.y - GlobalTransport::transportHeight - Timeline::timelineFixedHeight - 5;

        ImGui::SetNextWindowPos(ImVec2(displayX - panelWidth, GlobalTransport::transportHeight + 5), ImGuiCond_Always);
        ImGui::SetNextWindowSize(
            ImVec2(panelWidth, panelY),
            ImGuiCond_Always
        );
        ImGui::Begin("Scenes", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        bool panelHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_ChildWindows);

        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float halfW = (panelWidth - spacing) * 0.5f;

        const float tabListWidth = 100.0f;            // width of the vertical tab list
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));  // optional padding

        // 1) Left pane: vertical “tabs”
        ImGui::BeginChild("##SceneTabList", ImVec2(tabListWidth, 0), true);
        for (int i = 0; i < (int)Timeline::scenes.size(); ++i) {
            // build label
            std::string label = "Scene " + std::to_string(i + 1);
            bool isActive = (i == activeScene);
            float availW = ImGui::GetContentRegionAvail().x;
            if (ImGui::Selectable(label.c_str(),
                isActive,
                0,
                ImVec2(availW, 0)))
            {
                activeScene = i;
                Canvas::clearSelected();
                GlobalTransport::currentTime = Timeline::scenes[i]->startTime / 1000.0f;
                Timeline::currentScene = Timeline::scenes[i];
                showAnimateWindow = false;
            }
        }
        ImGui::EndChild();

        // 2) Same line: content pane
        ImGui::SameLine();
        ImGui::BeginChild("##SceneTabContent", ImVec2(0, 0), false);
        // guard against empty

        if (!Timeline::scenes.empty()) {
            auto& scene = Timeline::scenes[activeScene];
            std::cout << "objects.size() = " << scene->objects.size() << '\n';
            // --- header ---
            ImGui::TextUnformatted("Objects");
            ImGui::Separator();

            if (ImGui::Button("Add Object")) {
                ImGui::OpenPopup("AddObjectPopup");
            }

            renderAddObjectPopup();

            if (!TrackFeatures::showMappings) {

                const char* animateBtnLabel = showAnimateWindow ? "Hide Animations" : "View Animations";
                if (Canvas::selectedObject) {
                    ImGui::SameLine();
                    if (ImGui::Button(animateBtnLabel)) {
                        showAnimateWindow = !showAnimateWindow;
                        if (showAnimateWindow) {
                            GlobalTransport::setLoop();
                            GlobalTransport::isLooping = true;
                        }
                        else
                            GlobalTransport::isLooping = false;
                    }
                }
            }

            ImGui::Columns(1);
            ImGui::Separator();

            // --- list each object in the scene ---
            for (size_t i = 0; i < scene->objects.size(); ++i) {
                auto& obj = scene->objects[i];
                ImGui::PushID((int)i);

                if (editingIndex == (int)i) {
                    // --- Inline edit mode ---
                    // give it focus so you actually see the cursor
                    ImGui::SetKeyboardFocusHere();

                    if (ImGui::InputText(
                        "##rename",
                        renameBuf,
                        sizeof(renameBuf),
                        ImGuiInputTextFlags_EnterReturnsTrue |
                        ImGuiInputTextFlags_AutoSelectAll)
                        || ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered()
                    )
                    {
                        // commit and exit edit
                        obj->setId(renameBuf);
                        editingIndex = -1;
                    }
                }
                else {
                    // --- Normal selectable mode ---
                    if (ImGui::Selectable(obj->getId().c_str(), selectedIndex == (int)i)) {
                        if (selectedIndex != (int)i) {
                            selectedIndex = (int)i;
                            Canvas::setSelected(obj);
                            showAnimateWindow = false;
                        }
                        else {
                            selectedIndex = -1;
							Canvas::clearSelected();
							showAnimateWindow = false;
                        }
                    }

                    if (selectedIndex == (int)i){
                        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R)) {
                            editingIndex = i;
                            strncpy(renameBuf,
                                obj->getId().c_str(),
                                sizeof(renameBuf) - 1);
                            renameBuf[sizeof(renameBuf) - 1] = '\0';
                        }
                        else if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                            scene->objects.erase(scene->objects.begin() + selectedIndex);
                            selectedIndex = -1;
                            Canvas::clearSelected();
							TrackFeatures::selectedTrack->updateMappings();
                            ImGui::PopID();
                            break;
                        }
                    }
                }

                if (selectedIndex == i) {

                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    float       avail = ImGui::GetContentRegionAvail().x;
                    float       lh = ImGui::GetTextLineHeightWithSpacing() + 5;

                    ImGui::Separator();
                    ImGui::Indent(10.0f);

                    ImGui::Text("Properties");

                    // Position
                    glm::vec3 worldPos{ obj->getTransform().position };

                    minPos[0] = ImGui::GetCursorScreenPos();

                    if (ImGui::DragFloat2(parameters[0].c_str(), &worldPos.x, 1.0f, -FLT_MAX, FLT_MAX, "%.1f")) {
                        worldPos.x = worldPos.x;
                        worldPos.y = worldPos.y;
                        obj->setPosition(worldPos);
                    }

					hoverFunc(dl, minPos[0], avail, lh, 0);

                    // Rotation
                    glm::vec3 rot = obj->getTransform().rotation;

                    minPos[1] = ImGui::GetCursorScreenPos();

                    if (ImGui::DragFloat(parameters[1].c_str(), &rot.z, 0.5f, -360.0f, 360.0f, "%.0f°")) {
                        obj->setRotation(rot);
                    }

					hoverFunc(dl, minPos[1], avail, lh, 1);

                    // Size
                    glm::vec3 size = obj->getSize();

                    minPos[2] = ImGui::GetCursorScreenPos();
 
                    if (ImGui::DragFloat2(parameters[2].c_str(), &size.x, 1.0f, -FLT_MAX, FLT_MAX, "%.1f"))
                        obj->setSize(size);

					hoverFunc(dl, minPos[2], avail, lh, 2);

                    // Hue/Saturation
                    glm::vec4 col = rgb2hsv(obj->getMaterial().color);

                    minPos[3] = ImGui::GetCursorScreenPos();

                    if (ImGui::DragFloat2(parameters[3].c_str(), &col.x, 0.005f, 0.0f, 1.0f, "%.3f"))
                        obj->setColor(hsv2rgb(col));

					hoverFunc(dl, minPos[3], avail, lh, 3);
                    
					// Brightness
                    minPos[4] = ImGui::GetCursorScreenPos();

                    if (ImGui::DragFloat(parameters[4].c_str(), &col.z, 0.005f, 0.0f, 1.0f, "%.3f"))
                        obj->setColor(hsv2rgb(col));

					hoverFunc(dl, minPos[4], avail, lh, 4);

					// Alpha
                    minPos[5] = ImGui::GetCursorScreenPos();

                    if (ImGui::DragFloat(parameters[5].c_str(), &col.w, 0.005f, 0.0f, 1.0f, "%.3f"))
                        obj->setColor(hsv2rgb(col));

                    if (obj->getObjectType() != ObjectType::Line) {
                        bool isFilled = obj->isFilled();
                        if (ImGui::Checkbox("Fill", &isFilled)) {
                            obj->setFilled(isFilled);
                        }
                        if (!obj->isFilled()) {
                            float stroke = obj->getStroke();
                            ImGui::SameLine();
                            if (ImGui::DragFloat("##Stroke", &stroke, 1.0f, 20.0f, 0.1f, "%.1f", true)) {
                                obj->setStroke(stroke);
                            }
                        }
                    }

					hoverFunc(dl, minPos[5], avail, lh, 5);

                    drawRects(dl, avail, lh);

                    ImGui::Unindent(10.0f);
                }

                ImGui::Separator();

                ImGui::PopID();
            }
        }
        ImGui::EndChild();

        ImGui::PopStyleVar();

        ImGui::End();

        if (showAnimateWindow) {
            AnimationInfo::showAnimationInfo(parameters[animPropIndex], animPropIndex);
        }
        else {
			AnimationInfo::resetAnimationWindow();
        }
	}
}