#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "GlobalTransport.h"
#include <iostream>

// vector traversion
// only Animations use Connected
enum class AnimationTraversion : int {
	Sequential,
	Drunken,
	Random,
	Defined,
	Connected
};

class AnimationPoint;

class Animation {
public:
	// start is initialized as (parameter's current value, length of Scene)
	Animation(std::shared_ptr<AnimationPoint>& start);

	void addPoint(std::shared_ptr<AnimationPoint>& newPoint);

	glm::vec2 getValue(float t);
	glm::vec2 getValue();

	void setElapsedTime(float t) {
		elapsedTime_ = t - startPoint_;
	}

	std::vector<std::shared_ptr<AnimationPoint>>& getPoints() { return points_; }

	void resetAnimation() {
		std::cout << "resetting animation\n";
		elapsedTime_ = 0;
		startPoint_ = GlobalTransport::currentTime * 1000.0f;
		is_finished_ = false;
		pointsIndex_ = 0;
		curValue_ = { 0.0f, 0.0f };
	}

	void setTrigger(bool t) { hasTrigger_ = t; }
	void trigger() { isTriggered_ = true; }
	bool hasTrigger() { return hasTrigger_; }
	bool is_finished() { return is_finished_; }

private:
	std::vector<std::shared_ptr<AnimationPoint>> points_;
	std::size_t pointsIndex_ = 0;

	glm::vec2 curValue_{};

	bool hasTrigger_ = false;
	bool isTriggered_ = false;

	float elapsedTime_ = 0;
	float startPoint_ = 0;
	// pointStart_needs to be initialized with currentTime at the beginning of playback and reset if the animation loops

	bool is_looping_ = false;
	bool is_finished_ = false;
};