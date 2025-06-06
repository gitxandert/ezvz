#pragma once

namespace ScenesPanel {
	extern float panelWidth;
	extern bool showAnimateWindow;
	extern int animPropIndex;
	extern int mappingIndex;

	extern const std::vector<std::string> parameters;
	void render();

	glm::vec4 rgb2hsv(const glm::vec4& c);
	glm::vec4 hsv2rgb(const glm::vec4& c);
}