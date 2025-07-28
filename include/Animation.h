#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "GlobalTransport.h"
#include <iostream>

enum class EasingType;
enum class LoopType;
class AnimationPoint;

class Animation {
public:
	// start is initialized as (parameter's current value, length of Scene)
	Animation(std::shared_ptr<AnimationPoint>& start);
	Animation(const std::shared_ptr<Animation>& other);

	void addPoint(std::shared_ptr<AnimationPoint>& newPoint);

	glm::vec2 getValue(float t);
	glm::vec2 getValue();

	void setElapsedTime(float t);

	std::vector<std::shared_ptr<AnimationPoint>>& getPoints() { return points_; }
	void updatePointsIndex();

	void resetAnimation();

	void setTrigger(bool t) { hasTrigger_ = t; }
	void trigger() { isTriggered_ = true; }
	bool hasTrigger() { return hasTrigger_; }
	bool is_finished() { return is_finished_; }

	const LoopType const getLoopType();
	void setLoopType(LoopType);
	const EasingType const getEasingType();
	void setEasingType(EasingType);

	float easingFunction(float);
	void setTotalDuration();
	float getEasedTime(float);

private:
	std::vector<std::shared_ptr<AnimationPoint>> points_;
	std::size_t pointsIndex_ = 0;

	glm::vec2 curValue_{};

	bool hasTrigger_ = false;
	bool isTriggered_ = false;
	bool justStarted_ = true;

	int loopCount = 0;

	float elapsedTime_ = 0.000f;
	float startPoint_ = 0.000f;
	// startPoint_needs to be initialized with currentTime at the beginning of playback and reset if the animation loops

	LoopType animLoopType_;
	EasingType animEaseType_;

	float originalStartPoint_ = 0.000f;
	float totalElapsedTime_ = 0.000f;
	float totalDuration_ = 0.000f;
	float totalWarpTime_ = 0.000f;
	float easedElapsedTime_ = 0.000f;

	bool is_finished_ = false;
};