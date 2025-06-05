#pragma once

#include <iostream>
#include "Canvas.h"
#include "Animation.h"
#include "AnimationInfo.h"
#include "AnimationPoint.h"
#include "GraphicObject.h"
#include "MappingsWindow.h"
#include "Scene.h"
#include "ScenesPanel.h"
#include "Timeline.h"
#include "TrackFeatures.h"

namespace AnimationInfo {

	static bool showPoints = false;
	static bool showAddPath = false;
	static bool addPath = false;
	static bool addingPoints = false;
	std::size_t animation_index = -1;

	static int selectedAnimation = -1;
	static int selectedPoint = -1;
	static int clickedPoint = -1;

	static bool pointHovered = false;
	static int hoveredPoint = -1;

	static ImVec2 aw_cm, aw_sz;

	bool settingTrigger = false;

	glm::vec2 normalizeClick(glm::vec2 clickWorld, int p_i) {
		switch (p_i) {
		case 1: { //Rotation
			clickWorld.x *= 720.0f;
			clickWorld.x -= 360.0f;
			break;
		}
		case 0: [[fallthrough]];
		case 2: { // Position, Size
			clickWorld.x *= Canvas::screenW;
			clickWorld.y *= Canvas::screenH;
			break;
		}
		default: break;
		}

		return clickWorld;
	}

	glm::vec2 getParameter(int p_i) {
		switch (p_i) {
		case 0:
			return { Canvas::selectedObject->getTransform().position.x, Canvas::selectedObject->getTransform().position.y };
		case 1: {
			int index;
			float y;
			if (selectedAnimation > -1) {
				index = Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->getPoints().size();
				if (index > 0)
					y = Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->getPoints()[index - 1].getValue().y;
				else
					y = aw_sz.y / 2.0f + aw_cm.y;
			}
			else {
				y = aw_sz.y / 2.0f + aw_cm.y;
			}
			return { Canvas::selectedObject->getTransform().rotation.z, y };
		}
		case 2:
			return { Canvas::selectedObject->getSize().x, Canvas::selectedObject->getSize().y };
		case 3:
			return { ScenesPanel::rgb2hsv(Canvas::selectedObject->getMaterial().color).x, ScenesPanel::rgb2hsv(Canvas::selectedObject->getMaterial().color).y };
		case 4: {
			int index;
			float y;
			if (selectedAnimation > -1) {
				index = Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->getPoints().size();
				if (index > 0)
					y = Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->getPoints()[index - 1].getValue().y;
				else
					y = 0.5;
			}
			else {
				y = 0.5;
			}
			return { ScenesPanel::rgb2hsv(Canvas::selectedObject->getMaterial().color).z, y };
		}
		case 5: {
			int index;
			float y;
			if (selectedAnimation > -1) {
				index = Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->getPoints().size();
				if (index > 0)
					y = Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->getPoints()[index - 1].getValue().y;
				else
					y = 0.5;
			}
			else {
				y = 0.5;
			}
			return { Canvas::selectedObject->getMaterial().color.w, y };
		}
		default:
			return { 0.0,0.0 };
		}
	}

	void setParameter(int p_i, glm::vec2 value) {
		switch (p_i) {
		case 0: {
			Transform transform = Canvas::selectedObject->getTransform();
			Canvas::selectedObject->setPosition({ value, transform.position.z });
			break;
		}
		case 1: {
			Transform transform = Canvas::selectedObject->getTransform();
			Canvas::selectedObject->setRotation({ transform.rotation.x, transform.rotation.y, value.x });
			break;
		}
		case 2: {
			glm::vec3 size = Canvas::selectedObject->getSize();
			Canvas::selectedObject->setSize({ value, size.z });
			break;
		}
		case 3: {
			glm::vec4 color = ScenesPanel::rgb2hsv(Canvas::selectedObject->getMaterial().color);
			Canvas::selectedObject->setColor(ScenesPanel::hsv2rgb({ value, color.z, color.w }));
			break;
		}
		case 4: {
			glm::vec4 color = ScenesPanel::rgb2hsv(Canvas::selectedObject->getMaterial().color);
			Canvas::selectedObject->setColor(ScenesPanel::hsv2rgb({ color.x, color.y, value.x, color.w }));
			break;
		}
		case 5: {
			glm::vec4 color = ScenesPanel::rgb2hsv(Canvas::selectedObject->getMaterial().color);
			Canvas::selectedObject->setColor(ScenesPanel::hsv2rgb({ color.x, color.y, color.z, value.x }));
			break;
		}
		default: break;
		}
	}

	void displayParameter(int parameterIndex, std::vector<AnimationPoint>& points, int pointsIndex) {
		// Pull current value
		glm::vec2 p_val = points[pointsIndex].getValue();

		// Unique IDs per point
		ImGui::PushID(pointsIndex);

		switch (parameterIndex) {
		case 0: { // Position X/Y
			ImGui::Text("x:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##PositionX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex].setValue(p_val);
				setParameter(parameterIndex, p_val);
			}

			ImGui::Text("y:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##PositionY", &p_val.y, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex].setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case 1: { // Rotation Z
			ImGui::Text("rot:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(66.0f);
			if (ImGui::InputFloat("##RotZ", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex].setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case 2: { // Size W/H
			ImGui::Text("w:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##SizeW", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex].setValue(p_val);
				setParameter(parameterIndex, p_val);
			}

			ImGui::Text("h:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##SizeH", &p_val.y, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex].setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case 3: { // Hue / Saturation
			ImGui::Text("hue:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(66.0f);
			if (ImGui::InputFloat("##HueX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex].setValue(p_val);
				setParameter(parameterIndex, p_val);
			}

			ImGui::Text("sat.:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(59.0f);
			if (ImGui::InputFloat("##SaturationY", &p_val.y, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex].setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case 4: { // Brightness
			ImGui::Text("br.:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(66.0f);
			if (ImGui::InputFloat("##BrightnessX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex].setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case 5: { // Alpha
			ImGui::Text("alpha:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(52.0f);
			if (ImGui::InputFloat("##AlphaX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex].setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		default:
			break;
		}


		ImGui::PopID();
	}

	void calibrateValue(glm::vec2& point, int param) {

		switch (param) {
		case 0: { // Position
			point.x = aw_cm.x + (point.x / Canvas::screenW) * aw_sz.x;
			point.y = aw_sz.y - (point.y / Canvas::screenH) * aw_sz.y + aw_cm.y;
			break;
		}
		case 1: { // Rotation
			point.x = aw_cm.x + ((point.x + 360.0f) / 720.0f) * aw_sz.x;
			point.y = aw_sz.y - (point.y / Canvas::screenH) * aw_sz.y + aw_cm.y;
			break;
		}
		case 2: { // Size
			point.x = (point.x / Canvas::screenW) * aw_sz.x + aw_cm.x;
			point.y = aw_sz.y - (point.y / Canvas::screenH) * aw_sz.y + aw_cm.y;
			break;
		}
		case 3: [[fallthrough]];
		case 4: [[fallthrough]];
		case 5: { // Color
			point.x = aw_cm.x + aw_sz.x * point.x;
			point.y = aw_sz.y - aw_sz.y * point.y + aw_cm.y;
			break;
		}
		default: break;
		}
	}

	void mapPoint(AnimationPoint& a_p, int parameter, int pointIndex, ImDrawList* dl) {

		glm::vec2 point = a_p.getValue();

		calibrateValue(point, parameter);

		float corner = 30.0f * (aw_sz.x / Canvas::screenW);

		float tl_x = point.x - corner;
		float tl_y = point.y - corner;
		float br_x = point.x + corner;
		float br_y = point.y + corner;

		ImVec2 tl{ tl_x, tl_y };  // top-left corner, in screen coords
		ImVec2 br{ br_x, br_y };  // bottom-right corner
		ImVec2 size{ br.x - tl.x, br.y - tl.y };

		ImGui::PushID(pointIndex);

		ImGui::SetCursorScreenPos(tl);

		ImGui::InvisibleButton("##pt", size);

		bool hovered = ImGui::IsItemHovered();
		if (hovered && !pointHovered)
			pointHovered = hovered;

		// Compute center of rect
		float cx = (tl.x + br.x) * 0.5f;
		float cy = (tl.y + br.y) * 0.5f;

		float scale = corner / 10.0f;

		// Get your text (drawn later)
		std::string label = std::to_string(pointIndex + 1);

		// Measure text size using ImGui
		ImVec2 textSz = ImGui::CalcTextSize(label.c_str());

		textSz.x *= scale;
		textSz.y *= scale;

		// Calculate true centered position
		ImVec2 textPos(cx - textSz.x * 0.5f, cy - textSz.y * 0.5f);

		dl->AddRectFilled(tl, br,
			hovered || selectedPoint == pointIndex ? IM_COL32(255, 200, 0, 255)
			: IM_COL32(255, 255, 0, 255));

		dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * scale, textPos,
			IM_COL32(0, 0, 0, 255), label.c_str());

		ImGui::PopID();
	}

	void tapPoint(AnimationPoint& a_p, int parameter, int pointIndex, ImDrawList* dl) {

		glm::vec2 point = a_p.getValue();

		calibrateValue(point, parameter);

		float corner = 30.0f * (aw_sz.x / Canvas::screenW);

		float tl_x = point.x - corner;
		float tl_y = point.y - corner;
		float br_x = point.x + corner;
		float br_y = point.y + corner;

		ImVec2 tl{ tl_x, tl_y };  // top-left corner, in screen coords
		ImVec2 br{ br_x, br_y };  // bottom-right corner
		ImVec2 size{ br.x - tl.x, br.y - tl.y };

		ImGui::PushID(pointIndex);

		ImGui::SetCursorScreenPos(tl);

		ImGui::InvisibleButton("##clpt", size);

		bool clicked = ImGui::IsItemClicked();
		if (clicked) {
			if (!addPath)
				clickedPoint = pointIndex;
			else {
				std::vector<AnimationPoint>& curPoints = Canvas::selectedObject->getAnimations(parameter)[selectedAnimation]->getPoints();
				AnimationPath newPath = { curPoints[selectedPoint].getValue(), curPoints[pointIndex].getValue() };
				curPoints[selectedPoint].addPath(newPath);
				curPoints[pointIndex].addAssociatedPath(&curPoints[selectedPoint].getPaths().back());
			}
		}

		ImGui::PopID();

	}

	void showAnimationWindow() {

		aw_cm = Canvas::fboDrawPos;
		aw_sz = { Canvas::fboDrawW, Canvas::fboDrawH };

		ImGui::SetNextWindowPos(aw_cm, ImGuiCond_Always);
		ImGui::SetNextWindowSize(aw_sz, ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.3f);
		if (ImGui::Begin("Animations", &ScenesPanel::showAnimateWindow,
			ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoScrollbar
		)) {
			if(!ScenesPanel::showAnimateWindow) {
				selectedAnimation = -1;
				showPoints = false;
				showAddPath = false;
				addPath = false;
				selectedPoint = -1;
				clickedPoint = -1;
				pointHovered = false;
				return;
			}
		}// &showWindow makes the window closable

		if (selectedAnimation > -1) {
			ImDrawList* dl = ImGui::GetWindowDrawList();
			ImGuiIO& io = ImGui::GetIO();

			std::vector<AnimationPoint>& curPoints = Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->getPoints();
			ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
			pointHovered = false;

			for (int j = curPoints.size() - 1; j > -1; --j) {
				tapPoint(curPoints[j], animation_index, j, dl);
			}
			for (int i = 0; i < curPoints.size(); ++i) {
				mapPoint(curPoints[i], animation_index, i, dl);

				std::vector<AnimationPath>& paths = curPoints[i].getPaths();
				for (int k = 0; k < paths.size(); ++k) {
					glm::vec2 start = paths[k].getStart();
					glm::vec2 end = paths[k].getEnd();
					calibrateValue(start, animation_index);
					calibrateValue(end, animation_index);
					dl->AddLine({ start.x, start.y }, { end.x, end.y }, IM_COL32(255, 255, 0, 255));
				}
			}

			if (pointHovered) {
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			}

			bool released = ImGui::IsMouseReleased(0);
			if (released) {
				clickedPoint = -1;
			}

			bool isDragging = ImGui::IsMouseDragging(0);
			if (clickedPoint > -1 && isDragging && !addPath) {
				glm::vec2 clickWorld = Canvas::getClickWorld(io);
				clickWorld.x *= Canvas::screenW;
				clickWorld.y *= Canvas::screenH;
				glm::vec2 norm = normalizeClick(clickWorld, animation_index);
				curPoints[clickedPoint].setValue(norm);
				setParameter(animation_index, norm);
			}

			if (addPath) {
				ImGui::SetCursorScreenPos(aw_cm);

				ImGui::InvisibleButton("##animwindow", aw_sz);

				if (ImGui::IsItemHovered()) {
					glm::vec2 curP = curPoints[selectedPoint].getValue();
					calibrateValue(curP, animation_index);

					dl->AddLine({ curP.x, curP.y }, io.MousePos, IM_COL32(255, 255, 0, 255));
					if (ImGui::IsItemClicked()) {
						glm::vec2 click = Canvas::getClickWorld(io);
						std::cout << "Click: " << click.x << ", " << click.y << '\n';
						glm::vec2 clickWorld = normalizeClick(click, animation_index);
						std::cout << "Click World: " << clickWorld.x << ", " << clickWorld.y << '\n';
						AnimationPath newPath = { curPoints[selectedPoint].getValue(), clickWorld };
						curPoints[selectedPoint].addPath(newPath);
					}
				}
			}
		}

		ImGui::End();
	}

	void showAnimationInfo(const std::string& parameter, std::size_t new_index) {
		if (new_index != animation_index) {
			animation_index = new_index;
			selectedAnimation = -1;
			showPoints = false;
			showAddPath = false;
			addPath = false;
			selectedPoint = -1;
			clickedPoint = -1;
			pointHovered = false;
		}

		if (!settingTrigger)
			showAnimationWindow();

		ImGuiIO& io = ImGui::GetIO();

		ImVec2 windowPos = { 0, io.DisplaySize.y - Timeline::timelineFixedHeight };
		ImVec2 windowSize = { io.DisplaySize.x, Timeline::timelineFixedHeight };

		if (settingTrigger) {
			windowPos.x = io.DisplaySize.x / 2.0f;
			windowSize.x = io.DisplaySize.x / 2.0f;
		}

		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(1.0f);
		ImGui::Begin((parameter + " Animations").c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

		if (!settingTrigger) {
			if (ImGui::Button("Add Animation")) {
				glm::vec2 param = getParameter(animation_index);
				AnimationPoint newPoint{ param, 1000.0f };
				std::shared_ptr<Animation> newAnimation = std::make_shared<Animation>(newPoint);
				Canvas::selectedObject->add_animation(newAnimation, animation_index);
			}
		}

		if (showPoints) {
			ImGui::SameLine();
			if (ImGui::Button("Add Point")) {
				glm::vec2 param = getParameter(animation_index);
				AnimationPoint newPoint{ param, 1000.0f };
				Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->addPoint(newPoint);
				int size = Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->getPoints().size();
				std::cout << "AnimationPoints size = " << size << '\n';
			}
		}

		if (showAddPath) {
			ImGui::SameLine();
			if (ImGui::Button(addPath ? "Done Adding" : "Add Path"))
				addPath = !addPath;
		}

		ImGui::BeginChild("##AnimationsList", ImVec2(300, 0), true);
		if (Canvas::selectedObject->animations_size(animation_index) < 1) {
			ImGui::Text(("No animations for " + parameter).c_str());
		}
		else {
			std::vector<std::shared_ptr<Animation>>& curAnimations = Canvas::selectedObject->getAnimations(animation_index);
			for (int i = 0; i < curAnimations.size(); ++i) {
				std::string selectableName = "Animation " + std::to_string(i + 1);
				bool is_selected = (i == selectedAnimation);

				if (ImGui::Selectable(selectableName.c_str(), is_selected)) {
					if (!settingTrigger) {
						showPoints = true;
						if (selectedAnimation != i) {
							selectedAnimation = i;
							selectedPoint = -1;
						}
						else {
							selectedAnimation = -1;
							selectedPoint = -1;
							showPoints = false;
						}
					}
					else {
						AudioParameter ap = AudioParameter(MappingsWindow::audioIndex);
						GraphicParameter gp = GraphicParameter(ScenesPanel::animPropIndex);
						auto newMapping = std::make_shared<TriggerMapping>(Canvas::selectedObject, ap, gp, selectedAnimation, MapType::Trigger);
						TrackFeatures::selectedTrack->mappings[MappingsWindow::audioIndex].push_back(newMapping);

						Canvas::selectedObject->getAnimations(animation_index)[i]->setTrigger(true);

						ScenesPanel::showAnimateWindow = false;
						MappingsWindow::addingMapping = false;
						selectedAnimation = -1;
						settingTrigger = false;
						showPoints = false;
						MappingsWindow::selectedMapping = TrackFeatures::selectedTrack->mappings[MappingsWindow::audioIndex].back();
					}
				}

				if (curAnimations[i]->hasTrigger()) {
					ImVec2 mins = ImGui::GetItemRectMin();
					ImVec2 maxs = ImGui::GetItemRectMax();
					ImDrawList* dl = ImGui::GetWindowDrawList();
					dl->AddRectFilled(mins, { maxs.x - mins.x, mins.y + (maxs.y - mins.y) }, IM_COL32(255, 127, 0, 100));
				};
			}
		}

		ImGui::EndChild();

		if (showPoints) {
			ImGui::SameLine();
			ImGui::BeginChild("##AnimationPoints", ImVec2(io.DisplaySize.x - 325, 0), true);
			std::vector<AnimationPoint>& curPoints = Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->getPoints();
			for (int j = 0; j < curPoints.size(); ++j) {
				std::string selectableName = "Point " + std::to_string(j + 1);
				bool is_selected = (j == selectedPoint);
				ImGui::SameLine();
				if (ImGui::Selectable(selectableName.c_str(), is_selected, 0, { 100, 0 })) {
					if (j != selectedPoint) {
						selectedPoint = j;
						showAddPath = true;
						setParameter(animation_index, curPoints[j].getValue());
					}
				}
			}

			ImGui::Separator();

			if (selectedPoint > -1 && selectedPoint < curPoints.size()) {
				ImGui::Indent(10.0f);
				displayParameter(animation_index, curPoints, selectedPoint);
				float dur = curPoints[selectedPoint].getDuration();
				ImGui::Text("t:");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(80.0f);
				if (ImGui::InputFloat("##DurationInput", &dur, 0.0f, 0.0f, "%.3f")) {
					curPoints[selectedPoint].setDuration(dur);
				}
				ImGui::Unindent(10.0f);
			}
			ImGui::EndChild();
		}

		ImGui::End();
	}

}