#include "AnimationPoint.h"
#include "Animation.h"

void Animation::resetAnimation() {
     std::cout << "resetting animation\n";
     elapsedTime_ = 0;
     startPoint_ = GlobalTransport::currentTime * 1000.0f;
     is_finished_ = false;
     pointsIndex_ = 0;
     curValue_ = points_[pointsIndex_]->getValue();
}

Animation::Animation(std::shared_ptr<AnimationPoint>& start){
    points_.push_back(start);
}

void Animation::addPoint(std::shared_ptr<AnimationPoint>& newPoint) {
    points_.push_back(newPoint);
}

glm::vec2 Animation::getValue(float t) {
    // 1) If we already completed the entire animation, return the final value.
    std::cout << "Getting value at Point " << pointsIndex_ + 1 << "\n";

    if (is_finished_) {
        std::cout << "Animation is finished\n";
        return points_.back()->getValue(1.0f);
    }

    // 2) Update elapsedTime_ relative to the last startPoint_.
    setElapsedTime(t);  // e.g. elapsedTime_ = t - startPoint_;

    // 3) If pointsIndex_ has already run off the end, clamp it and mark finished.
    if (pointsIndex_ >= points_.size()) {
        // We stepped off the last index sometime before—or we got called
        // with getValue() again after finishing. In either case, we should
        // “seal off” the animation here.
        pointsIndex_ = points_.size() - 1;
        std::cout << "pointsIndex_ = " << pointsIndex_ << '\n';
        if (points_[pointsIndex_]->getLoopType() == LoopType::Off) {
            is_finished_ = true;
            return points_.back()->getValue(1.0f);
        }
        else {
            points_[pointsIndex_]->updatePathIndex();
        }
    }

    // 4) Now we know pointsIndex_ is valid.
    float duration = points_[pointsIndex_]->getDuration();

    // ────────────────────────────────────────────────────────────────────────
    // 5) CASE A: Still “within” this point’s duration window
    if (elapsedTime_ < duration) {
        if (hasTrigger_) {
            if (isTriggered_) {
                std::cout << "Triggering\n";
                // — A trigger just fired “for” this point.
                //   Snap to this point’s raw value, reset timer, move to the next index.
                curValue_ = points_[pointsIndex_]->getValue(/* raw = no interp */);
                startPoint_ = t;             // zero‐out elapsedTime_ on the next call
                isTriggered_ = false;

                // Advance to the next index. If that steps off the end, mark finished.
                if (points_[pointsIndex_]->getLoopType() == LoopType::Off) {
                    std::cout << "Incrementing pointsIndex at isTriggered_\n";
                    pointsIndex_++;
                }
                else
					points_[pointsIndex_]->updatePathIndex();

                if (pointsIndex_ >= points_.size()) {
                    pointsIndex_ = points_.size() - 1;
                    if (points_[pointsIndex_]->getLoopType() == LoopType::Off) {
                        // We just moved past the final keypoint → clamp and finish.
                        is_finished_ = true;
                    }
                    else {
                        points_[pointsIndex_]->updatePathIndex();
                    }
                }
                return curValue_;
            }
            else {
                // Not triggered yet, but elapsedTime_ < duration. We haven’t fired
                // this point yet, so we “hold” the previous value. If we’re at index 0,
                // hold points_[0] at t=0. Otherwise interpolate the prior segment’s final.
                float interp = elapsedTime_ / duration;
                if (pointsIndex_ == 0 && points_[pointsIndex_]->getLoopType() == LoopType::Off) {
                    // Still waiting for the very first trigger
                    return points_[0]->getValue(0.0f);
                }
                else {
                    // “Hold” the final value of the previous point until trigger arrives
                    return points_[pointsIndex_]->getValue(interp);
                }
            }
        }
        else {
            // No trigger required: we interpolate continuously within this point.
            std::cout << "no trigger\n";
            float interp = elapsedTime_ / duration;
            curValue_ = points_[pointsIndex_]->getValue(interp);
            return curValue_;
        }
    }

    // ────────────────────────────────────────────────────────────────────────
    // 6) CASE B: elapsedTime_ >= duration → this point has “expired”
    if (!hasTrigger_ || (hasTrigger_ && isTriggered_)) {
        // Either no trigger is needed, or we just got a trigger exactly when it expired.
        // In both cases, advance to the next index.
        if (points_[pointsIndex_]->getLoopType() == LoopType::Off) {
            std::cout << "Incrementing pointsIndex at isTriggered_ (expired)\n";
            pointsIndex_++;
        }
        else
            points_[pointsIndex_]->updatePathIndex();

        if (pointsIndex_ < points_.size()) {
            // There is another point to show. Snap to its raw value and reset the clock.
            curValue_ = points_[pointsIndex_]->getValue(/* raw = no interp */);
            startPoint_ = GlobalTransport::currentTime * 1000.0f;
            elapsedTime_ = 0.0f;
            isTriggered_ = false;
            return curValue_;
        }
        else {
            // We just ran off the end. Clamp index, mark finished, return final value.
            pointsIndex_ = points_.size() - 1;
            if (points_[pointsIndex_]->getLoopType() == LoopType::Off) {
                is_finished_ = true;
                isTriggered_ = false;
                curValue_ = points_.back()->getValue(1.0f);
            }
            else
            {
                points_[pointsIndex_]->updatePathIndex();
            }
            return curValue_;
        }
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
        pointsIndex_ = points_.size() - 1;
        if (points_[pointsIndex_]->getLoopType() == LoopType::Off) {
            is_finished_ = true;
            return points_.back()->getValue();
        }
        else {
            points_[pointsIndex_]->updatePathIndex();
        }
    }
    else
        return points_[pointsIndex_]->getValue();
}