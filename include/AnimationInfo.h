#pragma once

namespace AnimationInfo {
	extern glm::vec2 getParameter(int);
	extern void setParameter(int, glm::vec2);

	void showAnimationWindow();
	void showAnimationInfo(const std::string&, std::size_t);
}