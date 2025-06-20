#include "GraphicObject.h"

GraphicObject::GraphicObject(ObjectType type, const std::string& id) : type_(type), id_(id) {}

const char* GraphicObject::getType() const {
    auto idx = static_cast<std::size_t>(type_);
    return objectTypeNames[idx];
}

const std::string& GraphicObject::getId() const {
    return id_;
}

void GraphicObject::setId(const char* newId) {
    id_ = newId;  // implicit conversion to std::string
}

const Transform& GraphicObject::getTransform() const {
    return transform_;
}

void GraphicObject::setPosition(const glm::vec3& p) { 
    transform_.position = p; 
}

void GraphicObject::setZPosition(float z) {
    const float zNear = 9.0f;
    const float zFar = -90.0f;
    const float bias = 3.0f;

    if (z >= 0.0f) {
        transform_.position.z = z * zNear;
    }
    else {
        float t = -z;
        t = std::pow(t, bias);
        transform_.position.z = t * zFar;
    }
};

float GraphicObject::getSliderFromZ() const {
    const float zNear = 9.0f;
    const float zFar = -90.0f;
    const float bias = 3.0f;

    float z = transform_.position.z;
    if (z >= 0.0f)
        return z / zNear;
    else
        return -std::pow(z / zFar, 1.0f / bias);
}

void GraphicObject::setRotation(const glm::vec3& rot) {
    transform_.rotation = rot;
}

void GraphicObject::setScale(const glm::vec3& scl) {
    transform_.scale = scl;
}

const Material& GraphicObject::getMaterial() const {
    return material_;
}

void GraphicObject::setColor(const glm::vec4& col) {
    material_.color = col;
}

void GraphicObject::setTexture(const std::string& path) {
    material_.texturePath = path;
}

void GraphicObject::setOpacity(float op) {
    material_.opacity = op;
}

int GraphicObject::animations_size(int i) {
    if (i > -1) {
        return animations_[i].size();
    }
    else {
        int size = 0;
        for (auto& parameter : animations_)
            size += parameter.size();

        return size;
    }
}

void GraphicObject::add_animation(std::shared_ptr<Animation> newAnimation, std::size_t animations_index) {
    animations_[animations_index].push_back(std::move(newAnimation));
}

std::vector<std::shared_ptr<Animation>>& GraphicObject::getAnimations(std::size_t index) {
    return animations_[index];
}

void GraphicObject::resetAnimations(bool restart) {
    if (restart)
        noMoreAnimations_ = 0;

    for(auto& animationIndex : animationIndices_)
        animationIndex = 0;

    for (std::size_t i = 0; i < animations_size(); ++i) {
        for (auto& animation : animations_[i]) {
            animation->resetAnimation();
        }
    }
}

void GraphicObject::setMapped(int paramIndex, bool isY) {
    mapBools_ |= (1u << paramIndex);
    if (paramIndex == 0 || paramIndex == 2 || paramIndex == 3) {
        std::size_t mapIndex = 0;

        switch (paramIndex) {
        case 2: mapIndex = 1; break;
        case 3: mapIndex = 2; break;
        default: break;
        }

        if (isY) {
            if (isMapY_[mapIndex] == 1)
                isMapY_[mapIndex] = 3;
            else
                isMapY_[mapIndex] = 2;
        }
        else {
            if (isMapY_[mapIndex] == 2)
                isMapY_[mapIndex] = 3;
            else
				isMapY_[mapIndex] = 1;
        }
    }
}

void GraphicObject::setUnmapped(int paramIndex, bool isY) {
    mapBools_ &= ~(1u << paramIndex);
    if (paramIndex == 0 || paramIndex == 2 || paramIndex == 3) {
        std::size_t mapIndex = 0;

        switch (paramIndex) {
        case 2: mapIndex = 1; break;
        case 3: mapIndex = 2; break;
        default: break;
        }

        if (isY)
            if(isMapY_[mapIndex] == 3)
                isMapY_[mapIndex] = 1;
            else
                isMapY_[mapIndex] = 0;
        else {
            if (isMapY_[mapIndex] == 3)
                isMapY_[mapIndex] = 2;
            else
				isMapY_[mapIndex] = 0;
        }
    }
}

bool GraphicObject::isMapped(int paramIndex) {
    return (mapBools_ & (1u << paramIndex)) != 0;
}

int GraphicObject::isMappedY(int paramIndex) const {
	return isMapY_[paramIndex];
}

glm::vec2 GraphicObject::getParameterValue(int p_i) const {
    switch (p_i) {
    case 0: {
        return { transform_.position.x, transform_.position.y };
    }
    case 1: {
        return { transform_.rotation.z, 0.0f };
    }
    case 2: {
        return { getSize().x, getSize().y};
    }
    case 3: {
        glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
        return { color.x, color.y };
    }
    case 4: {
        glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
        return { color.z, 0.0f };
    }
    case 5: {
        glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
        return { color.w, 0.0f };
    }
    case 6: { // Stroke
        return { stroke_, 0.0f };
    }
    default:
        return { 0.0f, 0.0f };
    }
}
void GraphicObject::setParameter(int p_i, glm::vec2 value) {
    switch (p_i) {
    case 0: {
        setPosition({ value, transform_.position.z });
        break;
    }
    case 1: {
        setZPosition(value.x);
		break;
    }
    case 2: {
        setRotation({ transform_.rotation.x, transform_.rotation.y, value.x });
        break;
    }
    case 3: {
        setSize({ value, getSize().z});
        break;
    }
    case 4: {
        glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
        setColor(ScenesPanel::hsv2rgb({ value, color.z, color.w }));
        break;
    }
    case 5: {
        glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
        setColor(ScenesPanel::hsv2rgb({ color.x, color.y, value.x, color.w }));
        break;
    }
    case 6: {
        glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
        setColor(ScenesPanel::hsv2rgb({ color.x, color.y, color.z, value.x }));
        break;
    }
    case 7: {
		setStroke(value.x);
		break;
    }
    default: break;
    }
}

void GraphicObject::setNewMapBools(int paramIndex, bool isY) {
	newMapBools_ |= (1u << paramIndex);
    if(isY)
	    isNewMapY_ |= (isY << paramIndex);
}

void GraphicObject::update() {
    for (int parameter = 0; parameter < animations_.size(); ++parameter) {
        std::size_t currentAnimation = animationIndices_[parameter];
        if ((newMapBools_ & (1u << parameter)) != 0) {
            switch (parameter) {
            case 0: [[fallthrough]];
            case 2: [[fallthrough]];
            case 3: {
                if ((isNewMapY_ & (1u << parameter)) != 0) {
                    if (animations_[parameter].size() > 0) {
                        if (!animations_[parameter][currentAnimation]->is_finished()) {
                            glm::vec2 updateValue = animations_[parameter][currentAnimation]->getValue(GlobalTransport::currentTime * 1000.0f);
                            transform_.position.x = updateValue.x;
                        }
                    }
                }
                else {
                    if (animations_[parameter].size() > 0) {
                        if (!animations_[parameter][currentAnimation]->is_finished()) {
                            glm::vec2 updateValue = animations_[parameter][currentAnimation]->getValue(GlobalTransport::currentTime * 1000.0f);
                            transform_.position.y = updateValue.y;
                        }
                    }
                }
                break;
            }
            default: break;
            }
        }
        else if (animations_[parameter].size() > 0 && (noMoreAnimations_ & (1u << parameter)) == 0) {
            bool animationIsFinished = animations_[parameter][currentAnimation]->is_finished();
            std::cout << "animationIsFinished = " << std::boolalpha << animationIsFinished << '\n';
            if (!animationIsFinished) {
                glm::vec2 updateValue = animations_[parameter][currentAnimation]->getValue(GlobalTransport::currentTime * 1000.0f);

                setParameter(parameter, updateValue);
            }
            else {
                updateAnimationIndex(parameter);
            }
        }
    }

    newMapBools_ = 0;
    isNewMapY_ = 0;
}


void GraphicObject::setLoopType(LoopType loop) {
    loopType_ = loop;
}

const LoopType const GraphicObject::getLoopType() {
    return loopType_;
}

void GraphicObject::updateAnimationIndex(int parameter) {
    auto& currentAnimationIndex = animationIndices_[parameter];
    std::cout << "currentAnimationIndex = " << currentAnimationIndex << '\n';

    switch (loopType_) {
    case LoopType::Off: {
        animations_[parameter][currentAnimationIndex]->resetAnimation();
        currentAnimationIndex++;
        if (currentAnimationIndex >= animations_size(parameter)) {
            currentAnimationIndex -= 1;
            noMoreAnimations_ |= (1u << parameter);
        }
        else {
            animations_[parameter][currentAnimationIndex]->resetAnimation();
        }
        break;
    }
    case LoopType::Sequence: {
        animations_[parameter][currentAnimationIndex]->resetAnimation();
        currentAnimationIndex++;
        if (currentAnimationIndex >= animations_size(parameter)) {
            currentAnimationIndex = 0;
        }
        animations_[parameter][currentAnimationIndex]->resetAnimation();
        break;
    }
    case LoopType::Random: {
        animations_[parameter][currentAnimationIndex]->resetAnimation();
        std::uniform_int_distribution<int> dist(0, animations_size(parameter) - 1);
        int random_index = dist(get_rng());
        currentAnimationIndex = (std::size_t)(random_index);
        animations_[parameter][currentAnimationIndex]->resetAnimation();
        break;
    }
    }
}