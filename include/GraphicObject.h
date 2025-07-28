#pragma once

#include <string>
#include <array>
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include "Animation.h"
#include "AnimationInfo.h"
#include "AnimationPoint.h"
#include "ScenesPanel.h"

// Supported primitive and custom object types
enum class ObjectType : int {
    Background = -1,
    Line,
    Rectangle,
    Ellipse,
    Triangle,
    Star,
    COUNT
    //etc.
};

static constexpr const char* objectTypeNames[] = {
    "Line",
    "Rectangle",
    "Ellipse",
    "Triangle",
    "Star"
};

enum class GraphicParameter : std::size_t {
    Position=0, // y-mapped
    ZPosition=1,
    Rotation=2,
    XY_Rotation=3, // y-mapped
    Size=4, // y-mapped
    Hue_Sat=5, // y-mapped
    Brightness=6,
    Alpha=7,
	Stroke=8,
    COUNT
};

// Transform component: position, rotation, scale
struct Transform {
    glm::vec3 position{ 0.0f };
    glm::vec3 rotation{ 0.0f };  // Euler angles in degrees
    glm::vec3 scale{ 1.0f };
};

// Material component: color, optional texture, opacity
struct Material {
    glm::vec4 color{ 1.0f };      // RGBA
    std::string texturePath;    // empty if none
    float opacity{ 1.0f };        // 0.0 = transparent, 1.0 = opaque
};

// Base class for all graphic objects (Particle, Population, etc.)
class GraphicObject {
public:
    GraphicObject(ObjectType type, const std::string& id);
    GraphicObject(const std::shared_ptr<GraphicObject>& other, int count);
    virtual ~GraphicObject() = default;

    // Identification
    const std::string& getId() const;
    const char* getType() const;
    const ObjectType& getObjectType() const { return type_; }

    void setId(const char* newId);

    // Transform accessors
    const Transform& getTransform() const;

    void setPosition(const glm::vec3& p);
    void setZPosition(float z);
    float getZPosition() const;
    void setRotation(const glm::vec3& rot);
    void setScale(const glm::vec3& scl);
    // Uniform scaling helper
    void setScale(float uniformScale) { setScale(glm::vec3(uniformScale)); }

    // Material accessors
    const Material& getMaterial() const;
    void setColor(const glm::vec4& col);
    void setTexture(const std::string& path);
    void setOpacity(float op);

    // Animations functions
    int animations_size(int i=-1);
    void add_animation(std::shared_ptr<Animation> newAnimation, std::size_t animations_index);
    void remove_animation(std::size_t animations_index, std::size_t selected_animation);
    std::vector<std::shared_ptr<Animation>>& getAnimations(std::size_t index);
    void resetAnimations(bool restart=true);

    void setMapped(int paramIndex, bool isY);
    void setUnmapped(int paramIndex, bool isY);
    bool isMapped(int paramIndex);
	int isMappedY(int paramIndex) const;

    void setNewMapBools(int, bool);

	glm::vec2 getParameterValue(int) const;
    void setParameter(int, glm::vec2);

    // Lifecycle
    void update();
    void GraphicObject::updateYMappedParameter(int xyIndex, glm::vec2 value, bool isY);
    virtual void draw() = 0;    // render via OpenGL/ImGui

    virtual glm::vec3 getSize() const = 0;
    virtual void setSize(const glm::vec3& size) = 0;

    void setLoopType(LoopType);
    const LoopType const getLoopType();

    void updateAnimationIndex(int);
    const std::array<std::size_t, static_cast<std::size_t>(GraphicParameter::COUNT)>& getAnimationIndices();
    const unsigned int getNoMoreAnimations();
    const unsigned int getMapBools();

    virtual void setFilled(bool filled) = 0;
    bool isFilled() const { return filled_; }
    virtual void setStroke(float stroke) = 0;
    float getStroke() const { return stroke_; }

protected:
    std::string    id_;
    ObjectType     type_;
    Transform      transform_;
    Material       material_;

    bool filled_ = true;
    float stroke_ = 0.1f;

    LoopType loopType_ = LoopType::Off;
    std::array<std::size_t, static_cast<std::size_t>(GraphicParameter::COUNT)> animationIndices_{};
    unsigned int noMoreAnimations_ = 0;

    std::array<std::vector<std::shared_ptr<Animation>>, static_cast<std::size_t>(GraphicParameter::COUNT)> animations_;

    unsigned int mapBools_ = 0;  // mapBools_ is a bitmask, where each bit represents a parameter
    std::array<unsigned int, 4> isMapY_ = { 0 };
    // isMapY_ is an array of bitmasks, where each bit represents if 
    // parameters 0, 3, 4, or 5 are only X mapped, only Y mapped, or both
    unsigned int newMapBools_ = 0;
    // newMapValues_ is a bitmask that signals if values in mapValues have been updated, 
    // and if they are, Animations are bypassed in favor of the new Map values
    std::array<unsigned int, 4> isNewMapY_ = { 0 };
    // isNewMapY_ is a bitmask, where each bit signals if the Y values of parameters 0, 3, 4, or 5 are updated
};