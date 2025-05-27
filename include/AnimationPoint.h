#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Animation.h"
#include "AnimationPath.h"
#include "GlobalTransport.h"
#include <iostream>

class AnimationPoint {
public:
	AnimationPoint(glm::vec2 val, float dur)
		: value_( val )
		, duration_(dur)
	{}

	glm::vec2 getValue();
	glm::vec2 getValue(float t);
	void setValue(glm::vec2 val);
	glm::vec2 updateValue(float t);

	std::vector<AnimationPath>& getPaths() { return paths_; };
	void addPath(AnimationPath newPath) { paths_.push_back(newPath); }
	void addAssociatedPath(AnimationPath* newAssPtr) { associatedPaths_.push_back(newAssPtr); }

	float getDuration();
	void setDuration(float dur);

private:
	glm::vec2 value_;
	float duration_;

	std::vector<AnimationPath> paths_;
	std::size_t path_index_ = 0;
	std::vector<std::size_t> defined_sequence_;

	std::vector<AnimationPath*> associatedPaths_;
};

inline glm::vec2 AnimationPoint::updateValue(float t) {
	return paths_[path_index_].updateValue(t);
}

inline glm::vec2 AnimationPoint::getValue() {
	return value_;
}

inline glm::vec2 AnimationPoint::getValue(float t) {
	if (paths_.size() > 0) {
		return updateValue(t);
	}
	else {
		return value_;
	}
}

inline void AnimationPoint::setValue(glm::vec2 val) {
	value_ = val;
	for (int i = 0; i < paths_.size(); ++i)
		paths_[i].setStart(val);
	for (int j = 0; j < associatedPaths_.size(); ++j) {
		associatedPaths_[j]->setEnd(val);
	}
}

inline float AnimationPoint::getDuration() {
	return duration_;
}

inline void AnimationPoint::setDuration(float dur) {
	duration_ = dur;
}