#include "AnimationPoint.h"
#include "Animation.h"

Animation::Animation(std::shared_ptr<AnimationPoint>& start)
{
    points_.push_back(start);
}

void Animation::addPoint(std::shared_ptr<AnimationPoint>& newPoint) {
    points_.push_back(newPoint);
}

glm::vec2 Animation::getValue(float t) {
    // 1) If we already completed the entire animation, return the final value.
    if (is_finished_) {
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
        is_finished_ = true;
        return points_.back()->getValue(1.0f);
    }

    // 4) Now we know pointsIndex_ is valid.
    float duration = points_[pointsIndex_]->getDuration();

    // ────────────────────────────────────────────────────────────────────────
    // 5) CASE A: Still “within” this point’s duration window
    if (elapsedTime_ < duration) {
        if (hasTrigger_) {
            if (isTriggered_) {
                // — A trigger just fired “for” this point.
                //   Snap to this point’s raw value, reset timer, move to the next index.
                curValue_ = points_[pointsIndex_]->getValue(/* raw = no interp */);
                startPoint_ = t;             // zero‐out elapsedTime_ on the next call
                isTriggered_ = false;

                // Advance to the next index. If that steps off the end, mark finished.
                pointsIndex_++;
                if (pointsIndex_ >= points_.size()) {
                    // We just moved past the final keypoint → clamp and finish.
                    pointsIndex_ = points_.size() - 1;
                    is_finished_ = true;
                }
                return curValue_;
            }
            else {
                // Not triggered yet, but elapsedTime_ < duration. We haven’t fired
                // this point yet, so we “hold” the previous value. If we’re at index 0,
                // hold points_[0] at t=0. Otherwise interpolate the prior segment’s final.
                float interp = elapsedTime_ / duration;
                if (pointsIndex_ == 0) {
                    // Still waiting for the very first trigger
                    return points_[0]->getValue(0.0f);
                }
                else {
                    // “Hold” the final value of the previous point until trigger arrives
                    return points_[pointsIndex_ - 1]->getValue(interp);
                }
            }
        }
        else {
            // No trigger required: we interpolate continuously within this point.
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
        pointsIndex_++;
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
            is_finished_ = true;
            isTriggered_ = false;
            curValue_ = points_.back()->getValue(1.0f);
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
        is_finished_ = true;
        return points_.back()->getValue();
    }
    else
        return points_[pointsIndex_]->getValue();
}