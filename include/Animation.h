#pragma once
#include "AnimationPoint.h"
#include "AnimationPath.h"
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

class Animation {
public:
	// start is initialized as (parameter's current value, length of Scene)
	Animation(AnimationPoint start)
	{
		points_.push_back(start);
	}

	void addPoint(AnimationPoint newPoint) {
		points_.push_back(newPoint);
	}

	glm::vec2 getValue(float t) {
		setElapsedTime(t);
		float duration = points_[pointsIndex_].getDuration();
		if (duration > elapsedTime_) {
			float interp = elapsedTime_ / points_[pointsIndex_].getDuration();
			return points_[pointsIndex_].getValue(interp);
		}
		else {
			pointsIndex_ += 1;
			int sizePts = points_.size();
			if (pointsIndex_ < sizePts) {
				startPoint_ = GlobalTransport::currentTime * 1000.0f;
				elapsedTime_ = 0.0f;
				return points_[pointsIndex_].getValue();
			}
			else {
				is_finished_ = true;
				return points_[pointsIndex_ - 1].getValue(1.0f);
			}
		}
	}

	glm::vec2 getValue() {
		return points_[pointsIndex_].getValue();
	}

	void setElapsedTime(float t) {
		elapsedTime_ = t - startPoint_;
	}

	std::vector<AnimationPoint>& getPoints() { return points_; }

	void resetAnimation() {
		elapsedTime_ = 0;
		startPoint_ = GlobalTransport::currentTime * 1000.0f;
		is_finished_ = false;
		pointsIndex_ = 0;
	}

	bool is_finished() { return is_finished_; }

private:
	std::vector<AnimationPoint> points_;
	std::size_t pointsIndex_ = 0;

	float elapsedTime_ = 0;
	float startPoint_ = 0;
	// pointStart_needs to be initialized with currentTime at the beginning of playback and reset if the animation loops

	bool is_looping_ = false;
	bool is_finished_ = false;
};