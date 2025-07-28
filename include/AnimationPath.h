#pragma once
#include <glm/glm.hpp>
#include <iostream>

enum class EasingType {
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut,
	COUNT // used to determine the number of easing types, not an actual type
};

class AnimationPoint;

class AnimationPath {
public:
	AnimationPath(glm::vec2 st, glm::vec2 en)
	: start_(st)
	, end_(en)
	{
		distance_ = end_ - start_;
	}

	std::shared_ptr<AnimationPath> clone() const;

	float easingFunction(float t);
	void setEasingType(EasingType type) { easing_ = type; }
	const EasingType& getEasingType() const { return easing_; }

	glm::vec2 updateValue(float t);

	const glm::vec2 getStart() const { return start_; }
	const glm::vec2 getEnd() const { return end_; }
	void setStart(glm::vec2 val) {
		start_ = val;
		distance_ = end_ - start_;
	}
	void setEnd(glm::vec2 val) { 
		end_ = val;
		distance_ = end_ - start_;
		if(endPoint_.lock()) {
			updateEndPoint(val);
		}
	}

	void setEndPoint(const std::shared_ptr<AnimationPoint>& pt) {
		endPoint_ = pt;
	}
	void updateEndPoint(glm::vec2 val);

private:
	glm::vec2 start_;
	glm::vec2 end_;
	glm::vec2 distance_;

	std::weak_ptr<AnimationPoint> endPoint_;

	float fst_handle_ = 0.0f;
	float snd_handle_ = 0.0f;
	
	EasingType easing_ = EasingType::Linear;
};

inline std::shared_ptr<AnimationPath> AnimationPath::clone() const {
	auto newPath = std::make_shared<AnimationPath>(start_, end_);
	newPath->setEasingType(easing_);
	newPath->fst_handle_ = fst_handle_;
	newPath->snd_handle_ = snd_handle_;
	newPath->endPoint_ = endPoint_;  // copies the weak pointer
	return newPath;
}

// t = elapsed time / duration
inline float AnimationPath::easingFunction(float t) {
	switch (easing_)
	{
	case EasingType::Linear:
		return t;
	case EasingType::EaseIn:
		return t * t;
	case EasingType::EaseOut:
		return t * (2 - t);
	case EasingType::EaseInOut:
		return t < 0.5 ? (2 * t * t) : (-1 + (4 - 2 * t) * t);
	default:
		return t;
	}
}

inline glm::vec2 AnimationPath::updateValue(float t) {
	// 1) force t into [0,1]
	std::cout << "t = " << t << '\n';
	t = glm::clamp(t, 0.0f, 1.0f);

	// 2) at exactly 100%, just return the end point
	if (t >= 1.0f) {
		return end_;
	}

	// 3) otherwise do the easing
	float e = easingFunction(t);
	return start_ + distance_ * e;
}
