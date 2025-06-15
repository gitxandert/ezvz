#pragma once
#include <vector>
#include <memory>
#include "imgui.h"
#include "Canvas.h"
#include "GraphicObject.h"
#include "Rectangle.h"

struct Scene {
	Scene(float start, float end, ImU32 col)
		: color(col)
	{
		if (end - start <= 1000.0f) {
			end = start + 1000.0f;
		}
		startTime = start;
		endTime = end;
	}

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