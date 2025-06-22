#pragma once
#include <vector>
#include <memory>
#include <random>
#include <glm/glm.hpp>
#include "Animation.h"
#include "AnimationPath.h"
#include "GlobalTransport.h"
#include <iostream>

enum class LoopType
{
    Off,
    Sequence,
    Random,
    COUNT
};

inline std::mt19937& get_rng() {
    static std::mt19937 rng{ std::random_device{}() };
    return rng;
};

class AnimationPoint
{
public:
    AnimationPoint(glm::vec2 val, float dur)
        : value_(val)
        , duration_(dur)
    {
    }

    glm::vec2 getValue();
    glm::vec2 getValue(float t);
    void setValue(glm::vec2 val);
    void setValueThroughEnd(glm::vec2 val);
    glm::vec2 updateValue(float t);

    // Access and modify paths
    std::vector<std::shared_ptr<AnimationPath>>& getPaths() { return paths_; }
    void addPath(const std::shared_ptr<AnimationPath>& newPath) { paths_.push_back(newPath); }

    // Store weak references to associated paths
    void addAssociatedPath(const std::shared_ptr<AnimationPath>& newAssPtr) {
        associatedPaths_.push_back(newAssPtr);
    }

    float getDuration();
    void setDuration(float dur);

    void setLoopType(LoopType loop);
	LoopType getLoopType() const { return loopType_; }

    void updatePathIndex();

private:
    glm::vec2 value_;
    float duration_;

	LoopType loopType_ = LoopType::Off; // Default to no looping

    std::vector<std::shared_ptr<AnimationPath>> paths_;
    std::size_t path_index_ = 0;

    std::vector<std::weak_ptr<AnimationPath>> associatedPaths_;  // weak pointers
};

inline glm::vec2 AnimationPoint::updateValue(float t) {
    return paths_[path_index_]->updateValue(t);
}

inline glm::vec2 AnimationPoint::getValue() {
    return value_;
}

inline glm::vec2 AnimationPoint::getValue(float t) {
    if (!paths_.empty()) {
        return updateValue(t);
    }
    return value_;
}

inline void AnimationPoint::setValue(glm::vec2 val) {
    value_ = val;
    // Update start value of all paths
    for (auto& path : paths_) {
        path->setStart(val);
    }
    // Update end value of all associated paths if they still exist
    for (auto& weakPath : associatedPaths_) {
        if (auto path = weakPath.lock()) {
			std::cout << "Updating associated path end value." << std::endl;
            path->setEnd(val);
        }
    }
}

inline void AnimationPoint::setValueThroughEnd(glm::vec2 val) {
    value_ = val;
    // Update start value of all paths
    for (auto& path : paths_) {
        path->setStart(val);
    }
}

inline float AnimationPoint::getDuration() {
    return duration_;
}

inline void AnimationPoint::setDuration(float dur) {
    duration_ = dur;
}

inline void AnimationPoint::updatePathIndex() {
    std::cout << "Updating path index\n";

    switch (loopType_) {
    case LoopType::Sequence: {
        ++path_index_;
        if (path_index_ >= paths_.size())
			path_index_ = 0; // Loop back to the start
        break;
    }
    case LoopType::Random: {
        std::uniform_int_distribution<int> dist(0, paths_.size() - 1); 
        int random_index = dist(get_rng());
        path_index_ = (std::size_t)(random_index);
        break;
    }
    default: break;
    }
}

inline void AnimationPoint::setLoopType(LoopType loop) {
    loopType_ = loop;
    path_index_ = 0;
}