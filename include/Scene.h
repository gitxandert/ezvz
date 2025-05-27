#pragma once
#include <vector>
#include <memory>
#include "imgui.h"
#include "GraphicObject.h"

struct Scene {
	float startTime = 0.0f;
	float endTime	= 0.0f;
	ImU32 color		= IM_COL32_WHITE;

	float sceneLength() { return endTime - startTime; }

	void resetObjectAnimations() {
		for (auto& obj : objects) {
			obj->resetAnimations();
		}
	}

	std::vector<std::shared_ptr<GraphicObject>> objects;
};