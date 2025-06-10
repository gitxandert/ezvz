#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Animation.h"
#include "AnimationPath.h"
#include "GlobalTransport.h"
#include <iostream>

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

private:
    glm::vec2 value_;
    float duration_;

    std::vector<std::shared_ptr<AnimationPath>> paths_;
    std::size_t path_index_ = 0;
    std::vector<std::size_t> defined_sequence_;

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
