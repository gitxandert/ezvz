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
	static bool showDelete = false;
	static bool showDuplicate = false;

	static bool addPath = false;
	static bool addingPoints = false;
	std::size_t animation_index = -1;

	static int selectedAnimation = -1;
	static int selectedPoint = -1;
	static int selectedPath = -1;
	static int clickedPoint = -1;

	static bool pointHovered = false;
	static int hoveredPoint = -1;

	static ImVec2 aw_cm, aw_sz;

	bool settingTrigger = false;

	void resetAnimationWindow() {
		showPoints = false;
		showAddPath = false;
		showDuplicate = false;
		showDelete = false;

		addPath = false;
		selectedAnimation = -1;
		selectedPoint = -1;
		selectedPath = -1;
		clickedPoint = -1;
		pointHovered = false;
		settingTrigger = false;
		GlobalTransport::resetLoop();
	}

	glm::vec2 normalizeClick(glm::vec2 screenPoint, int param) {
		GraphicParameter gp = static_cast<GraphicParameter>(param);

		switch (gp) {
		case GraphicParameter::Position: { // Position
			glm::vec2 local = { screenPoint.x - Canvas::fboDrawPos.x, screenPoint.y - Canvas::fboDrawPos.y };  // pixel coords relative to Canvas
			glm::vec2 world = Canvas::getClickWorld(local);   // convert to world-space at z = 0
			screenPoint = world;
			break;
		}
		case GraphicParameter::ZPosition: { // Z-Position
			float xNorm = (screenPoint.x - Canvas::fboDrawPos.x) / aw_sz.x;
			xNorm *= 99.0f;
			xNorm -= 90.0f;
			screenPoint.x = xNorm;

			float yNorm = 1.0f - (screenPoint.y - Canvas::fboDrawPos.y) / aw_sz.y;
			screenPoint.y = yNorm * Canvas::screenH;
			break;
		}
		case GraphicParameter::Rotation: [[fallthrough]];
		case GraphicParameter::XY_Rotation: { // Rotation
			float xNorm = (screenPoint.x - Canvas::fboDrawPos.x) / aw_sz.x;
			screenPoint.x = xNorm * 720.0f - 360.0f;
			
			float yNorm = 1.0f - (screenPoint.y - Canvas::fboDrawPos.y) / aw_sz.y;
			screenPoint.y = yNorm * 720.0f - 360.0f;
			break;
		}
		case GraphicParameter::Size: { // Size
			glm::vec2 local = { screenPoint.x - Canvas::fboDrawPos.x, screenPoint.y - Canvas::fboDrawPos.y };  // pixel coords relative to Canvas
			glm::vec2 world = Canvas::getClickWorld(local);   // convert to world-space at z = 0
			screenPoint = world;
			break;
		}
		case GraphicParameter::Hue_Sat: [[fallthrough]];
		case GraphicParameter::Brightness: [[fallthrough]];
		case GraphicParameter::Alpha: { // Color (normalized)
			float xNorm = (screenPoint.x - Canvas::fboDrawPos.x) / aw_sz.x;
			float yNorm = 1.0f - (screenPoint.y - Canvas::fboDrawPos.y) / aw_sz.y;
			screenPoint = glm::vec2(xNorm, yNorm);
			break;
		}
		case GraphicParameter::Stroke: { // Stroke
			float xNorm = (screenPoint.x - Canvas::fboDrawPos.x) / aw_sz.x;
			xNorm *= 2.0f;
			xNorm -= 1.0f; // Normalize to [-1, 1]
			float yNorm = 1.0f - (screenPoint.y - Canvas::fboDrawPos.y) / aw_sz.y;
			screenPoint = glm::vec2(xNorm, yNorm);
			break;
		}
		default: break;
		}

		return screenPoint;
	}

	float getNewY(float defaultY) {
		// If the parameter has only one value (is not an xy pair),
		// get the last point of the selected animation if it exists,
		// so that the new y-value is the last point's y-value
		// (makes sure the new point spawns where the last point is)
		float y;
		if (selectedAnimation > -1) {
			std::vector<std::shared_ptr<AnimationPoint>>& curpoints = Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->getPoints();
			int index = curpoints.size();
			if (index > 0)
				y = curpoints[index - 1]->getValue().y;
			else
				y = defaultY;
		}
		else {
			y = defaultY;
		}
		return y;
	}

	glm::vec2 getParameter(int p_i) {

		GraphicParameter gp = static_cast<GraphicParameter>(p_i);

		switch (gp) {
		case GraphicParameter::Position: { // Position
			Transform tf = Canvas::selectedObject->getTransform();
			glm::vec3 pos = tf.position;
			return { pos.x, pos.y };
		}
		case GraphicParameter::ZPosition: { // Z-Position
			Transform tf = Canvas::selectedObject->getTransform();
			glm::vec3 pos = tf.position;
			return { pos.z,  aw_cm.y + aw_sz.y / 2 };
		}
		case GraphicParameter::Rotation: { // RotationZ
			float y = getNewY(aw_sz.y / 2.0f + aw_cm.y);
			Transform tf = Canvas::selectedObject->getTransform();
			glm::vec3 rot = tf.rotation;
			return { rot.z, y };
		}
		case GraphicParameter::XY_Rotation: { // XY-Rotation
			Transform tf = Canvas::selectedObject->getTransform();
			glm::vec3 rot = tf.rotation;
			return { rot.x, rot.y };
		}
		case GraphicParameter::Size: { // Size
			glm::vec3 size = Canvas::selectedObject->getSize();
			return { size.x, size.y };
		}
		case GraphicParameter::Hue_Sat: { // Color (Hue/Saturation)
			Material mat = Canvas::selectedObject->getMaterial();
			glm::vec4 hsv = ScenesPanel::rgb2hsv(mat.color);
			return { hsv.x, hsv.y };
		}
		case GraphicParameter::Brightness: { // Brightness
			float y = getNewY(0.5f);
			Material mat = Canvas::selectedObject->getMaterial();
			glm::vec4 hsv = ScenesPanel::rgb2hsv(mat.color);
			return { hsv.z, y };
		}
		case GraphicParameter::Alpha: { // Alpha
			float y = getNewY(0.5f);
			Material mat = Canvas::selectedObject->getMaterial();
			glm::vec4 hsv = ScenesPanel::rgb2hsv(mat.color);
			return { hsv.w, y };
		}
		case GraphicParameter::Stroke: { // Stroke
			float y = getNewY(0.5f);
			return { Canvas::selectedObject->getStroke(), y };
		}
		default: return { 0.0,0.0 };
		}
	}

	void setParameter(int p_i, glm::vec2 value) {

		GraphicParameter gp = static_cast<GraphicParameter>(p_i);

		switch (gp) {
		case GraphicParameter::Position: { //Position
			Transform transform = Canvas::selectedObject->getTransform();
			Canvas::selectedObject->setPosition({ value, transform.position.z });
			break;
		}
		case GraphicParameter::ZPosition: { //Z-Position
			Canvas::selectedObject->setZPosition(value.x);
		}
		case GraphicParameter::Rotation: { //Rotation
			Transform transform = Canvas::selectedObject->getTransform();
			Canvas::selectedObject->setRotation({ transform.rotation.x, transform.rotation.y, value.x });
			break;
		}
		case GraphicParameter::XY_Rotation: { //XY-Rotation
			Transform transform = Canvas::selectedObject->getTransform();
			Canvas::selectedObject->setRotation({ value, transform.rotation.z });
			break;
		}
		case GraphicParameter::Size: { //Size
			glm::vec3 size = Canvas::selectedObject->getSize();
			Canvas::selectedObject->setSize({ value, size.z });
			break;
		}
		case GraphicParameter::Hue_Sat: { //Hue/Saturation
			Material mat = Canvas::selectedObject->getMaterial();
			glm::vec4 hsv = ScenesPanel::rgb2hsv(mat.color);
			glm::vec4 newColor = ScenesPanel::hsv2rgb({ value, hsv.z, hsv.w });
			Canvas::selectedObject->setColor(newColor);
			break;
		}
		case GraphicParameter::Brightness: { //Brightness
			Material mat = Canvas::selectedObject->getMaterial();
			glm::vec4 hsv = ScenesPanel::rgb2hsv(mat.color);
			glm::vec4 newColor = ScenesPanel::hsv2rgb({ hsv.x, hsv.y, value.x, hsv.w });
			Canvas::selectedObject->setColor(newColor);
			break;
		}
		case GraphicParameter::Alpha: { //Alpha
			Material mat = Canvas::selectedObject->getMaterial();
			glm::vec4 hsv = ScenesPanel::rgb2hsv(mat.color);
			glm::vec4 newColor = ScenesPanel::hsv2rgb({ hsv.x, hsv.y, hsv.z, value.x });
			Canvas::selectedObject->setColor(newColor);
			break;
		}
		case GraphicParameter::Stroke: { //Stroke
			Canvas::selectedObject->setStroke(value.x);
			break;
		}
		default: break;
		}
	}

	void displayParameter(int parameterIndex, std::vector<std::shared_ptr<AnimationPoint>>& points, int pointsIndex) {
		// Pull current value
		glm::vec2 p_val = points[pointsIndex]->getValue();

		// Unique IDs per point
		ImGui::PushID(pointsIndex);

		GraphicParameter gp = static_cast<GraphicParameter>(parameterIndex);

		switch (gp) {
		case GraphicParameter::Position: { // Position X/Y
			ImGui::Text("x:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##PositionX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}

			ImGui::Text("y:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##PositionY", &p_val.y, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case GraphicParameter::ZPosition: { // Z-Position
			ImGui::Text("z:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##ZPosition", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case GraphicParameter::Rotation: { // (Z) Rotation
			ImGui::Text("rot:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(66.0f);
			if (ImGui::InputFloat("##RotZ", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case GraphicParameter::XY_Rotation: { // XY Rotation
			ImGui::Text("rotx:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(59.0f);
			if (ImGui::InputFloat("##RotX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}

			ImGui::Text("roty:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(59.0f);
			if (ImGui::InputFloat("##RotY", &p_val.y, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case GraphicParameter::Size: { // Size W/H
			ImGui::Text("w:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##SizeW", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}

			ImGui::Text("h:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##SizeH", &p_val.y, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case GraphicParameter::Hue_Sat: { // Hue / Saturation
			ImGui::Text("hue:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(66.0f);
			if (ImGui::InputFloat("##HueX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}

			ImGui::Text("sat.:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(59.0f);
			if (ImGui::InputFloat("##SaturationY", &p_val.y, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case GraphicParameter::Brightness: { // Brightness
			ImGui::Text("br.:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(66.0f);
			if (ImGui::InputFloat("##BrightnessX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case GraphicParameter::Alpha: { // Alpha
			ImGui::Text("alpha:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(52.0f);
			if (ImGui::InputFloat("##AlphaX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		case GraphicParameter::Stroke: { // Stroke
			ImGui::Text("stroke:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(49.0f);
			if (ImGui::InputFloat("##StrokeX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				points[pointsIndex]->setValue(p_val);
				setParameter(parameterIndex, p_val);
			}
			break;
		}
		default:
			break;
		}


		ImGui::PopID();
	}

	void displayPathEnd(int parameterIndex, std::vector<std::shared_ptr<AnimationPath>>& paths, int pathsIndex) {
		// Pull current value
		glm::vec2 p_val = paths[pathsIndex]->getEnd();

		// Unique IDs per point
		ImGui::PushID(pathsIndex);

		GraphicParameter gp = static_cast<GraphicParameter>(parameterIndex);

		switch (gp) {
		case GraphicParameter::Position: { // Position
			ImGui::Text("   x:  "); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##PositionX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}

			ImGui::Text("   y:  "); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##PositionY", &p_val.y, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}
			break;
		}
		case GraphicParameter::ZPosition: { // Z-Position
			ImGui::Text("   z:  "); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##ZPosition", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}
			break;
		}
		case GraphicParameter::Rotation: { // (Z) Rotation
			ImGui::Text("  rot: "); ImGui::SameLine();
			ImGui::SetNextItemWidth(66.0f);
			if (ImGui::InputFloat("##RotZ", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}
			break;
		}
		case GraphicParameter::XY_Rotation: { // XY-Rotation
			ImGui::Text(" rotx: "); ImGui::SameLine();
			ImGui::SetNextItemWidth(59.0f);
			if (ImGui::InputFloat("##Rotx", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}

			ImGui::Text(" roty: "); ImGui::SameLine();
			ImGui::SetNextItemWidth(59.0f);
			if (ImGui::InputFloat("##RotY", &p_val.y, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}
			break;
		}
		case GraphicParameter::Size: { // Size
			ImGui::Text("   w:  "); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##SizeW", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}

			ImGui::Text("   h:  "); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			if (ImGui::InputFloat("##SizeH", &p_val.y, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}
			break;
		}
		case GraphicParameter::Hue_Sat: { // Hue / Saturation
			ImGui::Text("  hue: "); ImGui::SameLine();
			ImGui::SetNextItemWidth(66.0f);
			if (ImGui::InputFloat("##HueX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}

			ImGui::Text(" sat.: "); ImGui::SameLine();
			ImGui::SetNextItemWidth(59.0f);
			if (ImGui::InputFloat("##SaturationY", &p_val.y, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}
			break;
		}
		case GraphicParameter::Brightness: { // Brightness
			ImGui::Text("  br.  :"); ImGui::SameLine();
			ImGui::SetNextItemWidth(66.0f);
			if (ImGui::InputFloat("##BrightnessX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}
			break;
		}
		case GraphicParameter::Alpha: { // Alpha
			ImGui::Text(" alpha:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(52.0f);
			if (ImGui::InputFloat("##AlphaX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}
			break;
		}
		case GraphicParameter::Stroke: { // Stroke
			ImGui::Text("stroke:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(49.0f);
			if (ImGui::InputFloat("##StrokeX", &p_val.x, 0.0f, 0.0f, "%.3f")) {
				paths[pathsIndex]->setEnd(p_val);
			}
			break;
		}
		default:
			break;
		}

		ImGui::PopID();
	}

	void calibrateValue(glm::vec2& point, int param) {

		GraphicParameter gp = static_cast<GraphicParameter>(param);

		switch (gp) {
		case GraphicParameter::Position: { // Position
			point = Canvas::worldToScreen({ point, 0.0f }, aw_cm, aw_sz);
			break;
		}
		case GraphicParameter::ZPosition: { // Z-Position
			point.x = aw_cm.x + (point.x + 90.0f) / 99.0f * aw_sz.x;
			point.y = aw_sz.y - (point.y / Canvas::screenH) * aw_sz.y + aw_cm.y;
			break;
		}
		case GraphicParameter::Rotation: [[fallthrough]];
		case GraphicParameter::XY_Rotation: { // Rotation
			point.x = aw_cm.x + ((point.x + 360.0f) / 720.0f) * aw_sz.x;
			point.y = aw_sz.y - ((point.y + 360.0f) / 720.0f) * aw_sz.y + aw_cm.y;
			break;
		}
		case GraphicParameter::Size: { // Size
			point = Canvas::worldToScreen({ point, 0.0f }, aw_cm, aw_sz);
			break;
		}
		case GraphicParameter::Hue_Sat: [[fallthrough]];
		case GraphicParameter::Brightness: [[fallthrough]];
		case GraphicParameter::Alpha: { // Color
			point.x = aw_cm.x + aw_sz.x * point.x;
			point.y = aw_sz.y - aw_sz.y * point.y + aw_cm.y;
			break;
		}
		case GraphicParameter::Stroke: { // Stroke
			point.x = aw_cm.x + (point.x + 1.0f) / 2.0f * aw_sz.x;
			point.y = aw_sz.y - aw_sz.y * point.y + aw_cm.y;
			break;
		}
		default: break;
		}
	}

	void mapPoint(std::shared_ptr<AnimationPoint>& a_p, int parameter, int pointIndex, ImDrawList* dl) {

		glm::vec2 point = a_p->getValue();

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

	void tapPoint(std::shared_ptr<AnimationPoint>& a_p, int parameter, int pointIndex, ImDrawList* dl) {

		glm::vec2 point = a_p->getValue();

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
				std::vector<std::shared_ptr<AnimationPoint>>& curPoints = Canvas::selectedObject->getAnimations(parameter)[selectedAnimation]->getPoints();

				AnimationPath newPath{ curPoints[selectedPoint]->getValue(), curPoints[pointIndex]->getValue() };
				newPath.setEndPoint(curPoints[pointIndex]);

				std::shared_ptr<AnimationPath> newPathPtr = std::make_shared<AnimationPath>(newPath);
				curPoints[selectedPoint]->addPath(newPathPtr);
				curPoints[pointIndex]->addAssociatedPath(curPoints[selectedPoint]->getPaths().back());
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
		ImGui::Begin("Animations", &ScenesPanel::showAnimateWindow,
			ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoScrollbar
		);

		if (selectedAnimation > -1) {
			ImDrawList* dl = ImGui::GetWindowDrawList();
			ImGuiIO& io = ImGui::GetIO();

			std::vector<std::shared_ptr<AnimationPoint>>& curPoints = Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->getPoints();
			ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
			pointHovered = false;

			for (int j = curPoints.size() - 1; j > -1; --j) {
				std::vector<std::shared_ptr<AnimationPath>>& paths = curPoints[j]->getPaths();
				for (int k = 0; k < paths.size(); ++k) {
					glm::vec2 start = paths[k]->getStart();
					glm::vec2 end = paths[k]->getEnd();
					calibrateValue(start, animation_index);
					calibrateValue(end, animation_index);
					dl->AddLine({ start.x, start.y }, { end.x, end.y }, IM_COL32(255, 255, 0, 255));
				}
				tapPoint(curPoints[j], animation_index, j, dl);
			}
			for (int i = 0; i < curPoints.size(); ++i) {
				mapPoint(curPoints[i], animation_index, i, dl);
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
				glm::vec2 clickWorld = normalizeClick({io.MousePos.x, io.MousePos.y}, animation_index);
				curPoints[clickedPoint]->setValue(clickWorld);
				setParameter(animation_index, clickWorld);
			}

			if (addPath) {
				ImGui::SetCursorScreenPos(aw_cm);

				ImGui::InvisibleButton("##animwindow", aw_sz);

				if (ImGui::IsItemHovered()) {
					glm::vec2 curP = curPoints[selectedPoint]->getValue();
					calibrateValue(curP, animation_index);

					dl->AddLine({ curP.x, curP.y }, io.MousePos, IM_COL32(255, 255, 0, 255));
					if (ImGui::IsItemClicked()) {
						glm::vec2 clickWorld = normalizeClick({ io.MousePos.x, io.MousePos.y }, animation_index);
						AnimationPath newPath = { curPoints[selectedPoint]->getValue(), clickWorld };
						std::shared_ptr<AnimationPath> newPathPtr = std::make_shared<AnimationPath>(newPath);
						curPoints[selectedPoint]->addPath(newPathPtr);
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
			selectedPath = -1;
			clickedPoint = -1;
			pointHovered = false;
		}

		if (!settingTrigger)
			showAnimationWindow();

		ImGuiIO& io = ImGui::GetIO();

		ImVec2 windowPos = { 0, io.DisplaySize.y - Timeline::timelineFixedHeight + 5 };
		ImVec2 windowSize = { io.DisplaySize.x, Timeline::timelineFixedHeight - 5 };

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
				std::shared_ptr<AnimationPoint> newPointPtr = std::make_shared<AnimationPoint>(newPoint);
				std::shared_ptr<Animation> newAnimation = std::make_shared<Animation>(newPointPtr);
				newAnimation->setTotalDuration();
				Canvas::selectedObject->add_animation(newAnimation, animation_index);
			}
		}

		if (showDuplicate) {
			ImGui::SameLine();
			if (ImGui::Button("Duplicate Animation")) {
				std::shared_ptr<Animation> newAnimation = std::make_shared<Animation>(Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]);
				Canvas::selectedObject->add_animation(newAnimation, animation_index);
			}
		}

		if (showDelete) {
			ImGui::SameLine();
			if (ImGui::Button("Delete Animation")) {
				Canvas::selectedObject->remove_animation(animation_index, selectedAnimation);
				resetAnimationWindow();
			}
		}

		if (showPoints) {
			ImGui::SameLine();
			if (ImGui::Button("Add Point")) {
				glm::vec2 param = getParameter(animation_index);
				AnimationPoint newPoint{ param, 1000.0f };
				std::shared_ptr<AnimationPoint> newPointPtr = std::make_shared<AnimationPoint>(newPoint);
				Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->addPoint(newPointPtr);
				Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation]->setTotalDuration();
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
						showDelete = true;
						showDuplicate = true;
						if (selectedAnimation != i) {
							selectedAnimation = i;
							selectedPoint = -1;
						}
						else {
							selectedAnimation = -1;
							selectedPoint = -1;
							showPoints = false;
							showDelete = false;
							showDuplicate = false;
						}
					}
					else {
						AudioParameter ap = AudioParameter(MappingsWindow::audioIndex);
						GraphicParameter gp = GraphicParameter(ScenesPanel::animPropIndex);
						auto newMapping = std::make_shared<TriggerMapping>(Canvas::selectedObject, ap, gp, static_cast<std::size_t>(i), MapType::Trigger);
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

		static const char* easingNames[] = {
			"Linear",
			"EaseIn",
			"EaseOut",
			"EaseInOut"
		};
		static const char* loopNames[] = {
			"Off",
			"Sequence",
			"Random"
		};

		if (showPoints) {
			std::shared_ptr<Animation> curAnimation = Canvas::selectedObject->getAnimations(animation_index)[selectedAnimation];

			ImGui::SameLine();
			float pointsX = io.DisplaySize.x - 620.0f;
			ImGui::BeginChild("##AnimationPoints", ImVec2(pointsX, 0), true);
			std::vector<std::shared_ptr<AnimationPoint>>& curPoints = curAnimation->getPoints();
			for (int j = 0; j < curPoints.size(); ++j) {
				std::string selectableName = "Point " + std::to_string(j + 1);
				bool is_selected = (j == selectedPoint);
				ImGui::SameLine();
				if (ImGui::Selectable(selectableName.c_str(), is_selected, 0, { 100, 0 })) {
					if (j != selectedPoint) {
						selectedPoint = j;
						showAddPath = true;
						setParameter(animation_index, curPoints[j]->getValue());
					}
					else {
						selectedPoint = -1;
						showAddPath = false;
					}
				}
			}

			ImGui::Separator();

			if (selectedPoint > -1) {

				float paramX = 120.0f;
				float paramY = 0.0f;
				ImGui::BeginChild("##ParamInfo", ImVec2(paramX, paramY), true);
				displayParameter(animation_index, curPoints, selectedPoint);
				float dur = curPoints[selectedPoint]->getDuration();
				ImGui::Text("t:");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(80.0f);
				if (ImGui::InputFloat("##DurationInput", &dur, 0.0f, 0.0f, "%.3f")) {
					curPoints[selectedPoint]->setDuration(dur);
					curAnimation->setTotalDuration();
				}
				ImGui::EndChild(/* ParamInfo */);

				ImGui::SameLine();
				ImGui::BeginChild("Paths", ImVec2(pointsX - paramX - 25.0f, paramY), true);
				std::size_t paths_size = curPoints[selectedPoint]->getPaths().size();
				if (paths_size > 0) {
					ImGui::BeginChild("##PathSelectables", ImVec2(paramX, 110.0f), true);
					for (std::size_t i = 0; i < paths_size; ++i) {
						ImGui::PushID(i);

						const std::string pathName = "-> Path " + std::to_string(i + 1);
						bool is_selected = (i == selectedPath);
						if (ImGui::Selectable(pathName.c_str(), is_selected, 0, { 100, 0 })) {
							if (i != selectedPath) {
								selectedPath = i;
							}
							else {
								selectedPath = -1;
							}
						}

						ImGui::PopID();
					}
					ImGui::EndChild(/* PathSelectables */);

					if (selectedPath > -1) {
						ImGui::SameLine();
						ImGui::BeginChild("##PathTo", ImVec2(paramX * 2.0f, paramY), true);
						displayPathEnd(animation_index, curPoints[selectedPoint]->getPaths(), selectedPath);

						auto& path = curPoints[selectedPoint]->getPaths()[selectedPath];
						EasingType curType = path->getEasingType();
						const char* ease_preview = easingNames[(int)(curType)];

						ImGui::Text("Easing:");
						ImGui::SameLine();
						ImGui::SetNextItemWidth(100.0f);
						if (ImGui::BeginCombo("##Easing Type", ease_preview, ImGuiComboFlags_PopupAlignLeft)) {
							for (int i = 0; i < static_cast<int>(EasingType::COUNT); ++i) {
								const char* name = easingNames[i];
								bool   is_current = (curType == static_cast<EasingType>(i));
								if (ImGui::Selectable(name, is_current)) {
									path->setEasingType(static_cast<EasingType>(i));
								}
								if (is_current)
									ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}

						ImGui::EndChild(/* PathTo */);

					}
					ImGui::SameLine();
					ImGui::BeginChild("##PathOptions", ImVec2(paramX, paramY), true);

					LoopType curLoop = curPoints[selectedPoint]->getLoopType();
					const char* loop_preview = loopNames[(int)(curLoop)];

					ImGui::Text("Path Loop: ");
					ImGui::SetNextItemWidth(100.0f);
					if (ImGui::BeginCombo("##Looping", loop_preview, ImGuiComboFlags_PopupAlignLeft)) {
						for (int i = 0; i < static_cast<int>(LoopType::COUNT); ++i) {
							const char* name = loopNames[i];
							bool   is_current = (curLoop == static_cast<LoopType>(i));
							if (ImGui::Selectable(name, is_current)) {
								curPoints[selectedPoint]->setLoopType(static_cast<LoopType>(i));
							}
							if (is_current)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					ImGui::EndChild(/* PathOptions*/);

				}
				else {
					const std::string noLines = "No Paths for Point " + std::to_string(selectedPoint + 1);
					ImGui::Text(noLines.c_str());
				}


				ImGui::EndChild(/* Paths */);

			}
			else {
				showAddPath = false;
				addPath = false;
			}
			ImGui::EndChild(/* AnimationPoints */);

			ImGui::SameLine();
			ImGui::BeginChild("##AnimationOptions", ImVec2(120.0f, 0.0f), true);

			LoopType curLoop = curAnimation->getLoopType();
			const char* loop_preview = loopNames[(int)(curLoop)];

			ImGui::Text("Points Loop: ");
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::BeginCombo("##PointsLoop", loop_preview, true)) {
				for (int i = 0; i < static_cast<int>(LoopType::COUNT); ++i) {
					const char* name = loopNames[i];
					bool   is_current = (curLoop == static_cast<LoopType>(i));
					if (ImGui::Selectable(name, is_current)) {
						curAnimation->setLoopType(static_cast<LoopType>(i));
					}
					if (is_current)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			EasingType curEasing = curAnimation->getEasingType();
			const char* ease_preview = easingNames[(int)(curEasing)];

			ImGui::Text("Points Easing: ");
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::BeginCombo("##PointsEasing", ease_preview, true)){
				for (int i = 0; i < static_cast<int>(EasingType::COUNT); ++i) {
					const char* name = easingNames[i];
					bool   is_current = (curEasing == static_cast<EasingType>(i));
					if (ImGui::Selectable(name, is_current)) {
						curAnimation->setEasingType(static_cast<EasingType>(i));
					}
					if (is_current)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::EndChild(/* AnimationOptions */);
		}

		if (Canvas::selectedObject->animations_size() > 0) {
			ImGui::SameLine();
			ImGui::BeginChild("##AnimationsLoop", ImVec2(150.0f, 0.0f), true);

			LoopType curLoop = Canvas::selectedObject->getLoopType();
			const char* loop_preview = loopNames[(int)(curLoop)];

			ImGui::Text("Animations Loop: ");
			ImGui::SetNextItemWidth(100.0f);
			if (ImGui::BeginCombo("##PointsLoop", loop_preview, true)) {
				for (int i = 0; i < static_cast<int>(LoopType::COUNT); ++i) {
					const char* name = loopNames[i];
					bool   is_current = (curLoop == static_cast<LoopType>(i));
					if (ImGui::Selectable(name, is_current)) {
						Canvas::selectedObject->setLoopType(static_cast<LoopType>(i));
					}
					if (is_current)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::EndChild(/* AnimationsLoop */);
		}

		ImGui::End();
	}

}