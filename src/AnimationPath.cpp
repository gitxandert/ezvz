#include "AnimationPoint.h"
#include "AnimationPath.h"

void AnimationPath::updateEndPoint(glm::vec2 val) {
	if (auto pt = endPoint_.lock()) {
		pt->setValueThroughEnd(val);
	}
}