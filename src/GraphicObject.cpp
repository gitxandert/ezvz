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
    transform_.position.z = z;
};

float GraphicObject::getZPosition() const {
    return transform_.position.z;
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

    for (std::size_t i = 0; i < animations_.size(); ++i) {
        for (auto& animation : animations_[i]) {
            animation->resetAnimation();
        }
    }
}

void GraphicObject::setMapped(int paramIndex, bool isY) {
    mapBools_ |= (1u << paramIndex);
    if (paramIndex == 0 || (paramIndex > 2 && paramIndex < 6)) {
        std::size_t mapIndex = 0;

        switch (paramIndex) {
        case 3: mapIndex = 1; break;
        case 4: mapIndex = 2; break;
		case 5: mapIndex = 3; break;
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
    if (paramIndex == 0 || (paramIndex > 2 && paramIndex < 6)) {
        std::size_t mapIndex = 0;

        switch (paramIndex) {
        case 3: mapIndex = 1; break;
        case 4: mapIndex = 2; break;
        case 5: mapIndex = 3; break;
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
	GraphicParameter param = static_cast<GraphicParameter>(p_i);
    switch (param) {
    case GraphicParameter::Position: {
        return { transform_.position.x, transform_.position.y };
    }
    case GraphicParameter::ZPosition: {
        return { transform_.position.z, 0.0f };
    }
    case GraphicParameter::Rotation: {
        return { transform_.rotation.z, 0.0f };
    }
    case GraphicParameter::XY_Rotation: {
        return { transform_.rotation.x, transform_.rotation.y };
    }
    case GraphicParameter::Size:{
        return { getSize().x, getSize().y};
    }
    case GraphicParameter::Hue_Sat: {
        glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
        return { color.x, color.y };
    }
    case GraphicParameter::Brightness: {
        glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
        return { color.z, 0.0f };
    }
    case GraphicParameter::Alpha: {
        glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
        return { color.w, 0.0f };
    }
    case GraphicParameter::Stroke: { // Stroke
        return { stroke_, 0.0f };
    }
    default:
        return { 0.0f, 0.0f };
    }
}

void GraphicObject::setParameter(int p_i, glm::vec2 value) {
	GraphicParameter param = static_cast<GraphicParameter>(p_i);
    switch (param) {
    case GraphicParameter::Position: {
        setPosition({ value, transform_.position.z });
        break;
    }
    case GraphicParameter::ZPosition: {
        setZPosition(value.x);
		break;
    }
    case GraphicParameter::Rotation: {
        setRotation({ transform_.rotation.x, transform_.rotation.y, value.x });
        break;
    }
    case GraphicParameter::XY_Rotation: {
		setRotation({ value.x, value.y, transform_.rotation.z });
        break;
    }
    case GraphicParameter::Size: {
        setSize({ value, getSize().z});
        break;
    }
    case GraphicParameter::Hue_Sat: {
        glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
        setColor(ScenesPanel::hsv2rgb({ value, color.z, color.w }));
        break;
    }
    case GraphicParameter::Brightness: {
        glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
        setColor(ScenesPanel::hsv2rgb({ color.x, color.y, value.x, color.w }));
        break;
    }
    case GraphicParameter::Alpha: {
        glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
        setColor(ScenesPanel::hsv2rgb({ color.x, color.y, color.z, value.x }));
        break;
    }
    case GraphicParameter::Stroke: {
		setStroke(value.x);
		break;
    }
    default: break;
    }
}

void GraphicObject::setNewMapBools(int paramIndex, bool isY) {

	int mapIndex = -1;

    switch (paramIndex) {
    case 0: { mapIndex = 0; break; }
    case 3: { mapIndex = 1; break; }
    case 4: { mapIndex = 2; break; }
    case 5: { mapIndex = 3; break; }
    default: break;
    }

    if (mapIndex > -1) {
        unsigned int& currentState = isNewMapY_[mapIndex];
        if (isY) {
            if (currentState == 0)
                currentState = 2; // Y-mapped
            else if (currentState == 1)
                currentState = 3; // Y-mapped and X-mapped
        }
        else {
            if (currentState == 0)
                currentState = 1; // X-mapped
            else if (currentState == 2)
                currentState = 3; // Y-mapped and X-mapped
        }
    }
    newMapBools_ |= (1u << paramIndex);
}

void GraphicObject::updateYMappedParameter(int xyIndex, glm::vec2 value, bool isY) {
    switch (xyIndex) {
    case 0: { // Position
        if (!isY) {
            transform_.position.y = value.y;
        }
        else {
            transform_.position.x = value.x;
        }
        break;
    }
    case 1: { // XY-Rotation
        if (!isY) {
            transform_.rotation.y = value.y;
        }
        else {
            transform_.rotation.x = value.x;
        }
        break;
    }
    case 2: { // Size
        if (!isY) {
            setSize({ getSize().x, value.y, getSize().z });
        }
        else {
            setSize({ value.x, getSize().y, getSize().z });
        }
        break;
    }
    case 3: { // Hue/Saturation
        if (!isY) {
            glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
            setColor(ScenesPanel::hsv2rgb({ color.x, value.y, color.z, color.w }));
        }
        else {
            glm::vec4 color = ScenesPanel::rgb2hsv(material_.color);
            setColor(ScenesPanel::hsv2rgb({ value.x, color.y, color.z, color.w }));
        }
        break;
    }
    default: break;
    }

};

void GraphicObject::update() {
    for (int parameter = 0; parameter < animations_.size(); ++parameter) {
        std::size_t currentAnimation = animationIndices_[parameter];
        if ((newMapBools_ & (1u << parameter)) != 0) {
            int mapIndex = -1;
            switch (parameter) {
            case 0: { mapIndex = 0; break; }
            case 3: { mapIndex = 1; break; }
            case 4: { mapIndex = 2; break; }
            case 5: { mapIndex = 3; break; }
            default: break;
            }

            if (mapIndex > -1) {
                if (animations_[parameter].size() > 0) {
                    bool isY = (isNewMapY_[mapIndex] >= 2);
                    if (!animations_[parameter][currentAnimation]->is_finished() && isNewMapY_[mapIndex] < 3) {
                        glm::vec2 updateValue = animations_[parameter][currentAnimation]->getValue(GlobalTransport::currentTime * 1000.0f);
                        updateYMappedParameter(mapIndex, updateValue, isY);
                    }
                }
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
    isNewMapY_ = { 0 };
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