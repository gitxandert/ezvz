#include "AnimationPoint.h"
#include "Animation.h"

void Animation::resetAnimation() {
    // — reset playhead
    pointsIndex_ = 0;
    loopCount = 0;

    justStarted_ = true;
    is_finished_ = false;
    isTriggered_ = false;
    elapsedTime_ = 0.f;
    totalElapsedTime_ = 0.f;
    easedElapsedTime_ = 0.f;
    totalWarpTime_ = 0.f;

    originalStartPoint_ = GlobalTransport::currentTime * 1000.f;
    startPoint_ = originalStartPoint_;

    curValue_ = points_[0]->getValue();
}

Animation::Animation(std::shared_ptr<AnimationPoint>& start){
    points_.push_back(start);
    animLoopType_ = LoopType::Off;
    animEaseType_ = EasingType::Linear;
}

void Animation::addPoint(std::shared_ptr<AnimationPoint>& newPoint) {
    points_.push_back(newPoint);
}

void Animation::updatePointsIndex() {
    switch(animLoopType_){
    case LoopType::Off: {
        pointsIndex_++;
        if (pointsIndex_ >= points_.size()) {
            pointsIndex_ = points_.size() - 1;
            is_finished_ = true;
        }
        break;
    }
    case LoopType::Sequence: {
        pointsIndex_++;
        if (pointsIndex_ >= points_.size()) {
            if (animLoopType_ == LoopType::Off) {
                pointsIndex_ = points_.size() - 1;
                if (points_[pointsIndex_]->getLoopType() == LoopType::Off) {
                    // We just moved past the final keypoint → clamp and finish.
                    is_finished_ = true;
                }
                else {
                    points_[pointsIndex_]->updatePathIndex();
                }
            }
            else {
                pointsIndex_ = 0;
            }
        }
        break;
    }
    case LoopType::Random: {
        loopCount += 1;
        if (loopCount >= points_.size()) {
            loopCount = 0;
        }
        std::uniform_int_distribution<int> dist(0, points_.size() - 1);
        int random_index = dist(get_rng());
        pointsIndex_ = (std::size_t)(random_index);
        break;
    }
    }
}

void Animation::setElapsedTime(float t) {
    elapsedTime_ = t - startPoint_;
}

glm::vec2 Animation::getValue(float t) {
    // 1) If we already completed the entire animation, return the final value.
    if (is_finished_)
        return curValue_;

    // 2) ease t and check totalElapsedTime_ against totalDuration_
    easedElapsedTime_ = getEasedTime(t);

    // 3) since we know pointsIndex_ is still valid...
    float duration = points_[pointsIndex_]->getDuration();

    // 4) Update elapsedTime_ relative to the last startPoint_.
    setElapsedTime(t);

    // ────────────────────────────────────────────────────────────────────────
    // 5) CASE A: Still “within” this point’s duration window
    if (easedElapsedTime_ < duration) {
        if (hasTrigger_) {
            std::cout << "has trigger\n";
            if (isTriggered_) {
                std::cout << "Trigger received for point " << pointsIndex_ << "\n";
                isTriggered_ = false;

                // If we just started, no need to advance index
                if (justStarted_) {
                    justStarted_ = false;
                }
                else {
                    // If this point has a looping path, advance its internal path index
                    if (points_[pointsIndex_]->getLoopType() != LoopType::Off)
                        points_[pointsIndex_]->updatePathIndex();

                    updatePointsIndex();  // advance to next AnimationPoint
                }

                // Reset time references
                startPoint_ = t;
                originalStartPoint_ = t - totalWarpTime_;  // maintain existing warp offset
                elapsedTime_ = 0.0f;

                // Reset eased time for next interpolation
                easedElapsedTime_ = 0.0f;

                curValue_ = points_[pointsIndex_]->getValue(); // or raw value
                return curValue_;
            }

            else {
                // Not triggered yet
                if (justStarted_)
                    curValue_ = points_[pointsIndex_]->getValue();
                else {
                    float interp = (std::clamp)(easedElapsedTime_ / duration, 0.00f, 1.00f);
                    curValue_ = points_[pointsIndex_]->getValue(interp);
                }
                return curValue_;
            }
        }
        else {
            // No trigger required: we interpolate continuously within this point.
            float interp = (std::clamp)(easedElapsedTime_ / duration, 0.00f, 1.00f);
            curValue_ = points_[pointsIndex_]->getValue(interp);
            return curValue_;
        }
    }

    // ────────────────────────────────────────────────────────────────────────
    // 6) CASE B: elapsedTime_ >= duration → this point has “expired”
    if (!hasTrigger_ || (hasTrigger_ && isTriggered_)) {
        // Either no trigger is needed, or we just got a trigger exactly when it expired.
        // In both cases, advance to the next index.
        if (points_[pointsIndex_]->getLoopType() != LoopType::Off)
            points_[pointsIndex_]->updatePathIndex();
        
        updatePointsIndex();
        std::cout << "is_finished = " << std::boolalpha << is_finished_ << '\n';
        totalWarpTime_ += duration;
        startPoint_ = t;
        elapsedTime_ = 0.0f;

        if(!is_finished_)
            curValue_ = points_[pointsIndex_]->getValue();

        return curValue_;
    }
    else {
        // ────────────────────────────────────────────────────────────────────
        // CASE C: hasTrigger_ == true && !isTriggered_ && elapsedTime_ >= duration
        // We expired this point but have not yet received the next trigger. 
        // Simply “hold” the last computed curValue_ (no index change).
        return curValue_;
    }
}


glm::vec2 Animation::getValue() {
    if (pointsIndex_ >= points_.size()) {
        curValue_ = points_[pointsIndex_]->getValue();
    }
    else {
        updatePointsIndex();
        curValue_ = points_[pointsIndex_]->getValue();
    }

    return curValue_;
}

// t = elapsed time / duration
float Animation::easingFunction(float t) {
    switch (animEaseType_)
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

void Animation::setTotalDuration() {
    totalDuration_ = 0.000f;
    for (auto& p : points_) 
        totalDuration_ += p->getDuration();
}

const LoopType const Animation::getLoopType() {
    return animLoopType_;
}

void Animation::setLoopType(LoopType newType) {
    animLoopType_ = newType;
    pointsIndex_ = 0;
}

const EasingType const Animation::getEasingType() {
    return animEaseType_;
}

void Animation::setEasingType(EasingType newType) {
    animEaseType_ = newType;
}

float Animation::getEasedTime(float t) {
    totalElapsedTime_ = t - originalStartPoint_;
    if (totalElapsedTime_ > totalDuration_) {
        if (animLoopType_ == LoopType::Off) {
            is_finished_ = true;
            totalElapsedTime_ = totalDuration_;
        }
        else {
            isTriggered_ = false;
            totalElapsedTime_ = 0.000f;
            originalStartPoint_ = t;
            startPoint_ = t;
            loopCount = 0;
            totalWarpTime_ = 0.000f;
            pointsIndex_ = 0;
            loopCount = 0;
        }
    }
    
    float normElapsedTime = totalElapsedTime_ / totalDuration_;

    float easedNorm = easingFunction(normElapsedTime);

    return easedNorm * totalDuration_ - totalWarpTime_;
}