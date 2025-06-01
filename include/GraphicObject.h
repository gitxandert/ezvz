#pragma once

#include <string>
#include <array>
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include "Animation.h"
#include "AnimationInfo.h"
#include "ScenesPanel.h"

// Supported primitive and custom object types
enum class ObjectType : int {
    Point = 0,
    Line,
    Rectangle,
    Ellipse,
    COUNT
    //etc.
};

static constexpr const char* objectTypeNames[] = {
    "Point",
    "Line",
    "Rectangle",
    "Ellipse"
};

enum class GraphicParameter : std::size_t {
    Position,
    Rotation,
    Size,
    Hue_Sat,
    Brightness,
    Alpha,
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
    virtual ~GraphicObject() = default;

    // Identification
    const std::string& getId() const;
    const char* getType() const;

    void setId(const char* newId);

    // Transform accessors
    const Transform& getTransform() const;

    void setPosition(const glm::vec3& p);
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
    void add_animation(Animation newAnimation, std::size_t animations_index);
    std::vector<Animation>& getAnimations(std::size_t index);
    void resetAnimations();

  
    void setMapped(int paramIndex, bool isY);
    void setUnmapped(int paramIndex, bool isY);
    bool isMapped(int paramIndex);
	int isMappedY(int paramIndex) const;

    void setNewMapBools(int, bool);

	glm::vec2 getParameterValue(int) const;
    void setParameter(int, glm::vec2);

    // Lifecycle
    void update();
    virtual void draw() = 0;    // render via OpenGL/ImGui

    virtual glm::vec3 getSize() const = 0;
    virtual void setSize(const glm::vec3& size) = 0;

protected:
    std::string    id_;
    ObjectType     type_;
    Transform      transform_;
    Material       material_;

    std::array<std::vector<Animation>, static_cast<std::size_t>(GraphicParameter::COUNT)> animations_;
    unsigned int mapBools_ = 0;  // mapBools_ is a bitmask, where each bit represents a parameter
    std::array<unsigned int, 3> isMapY_ = { 0 };
    // isMapY_ is an array of bitmasks, where each bit represents if 
    // parameters 0, 2, or 3 are only X mapped, only Y mapped, or both
    unsigned int newMapBools_ = 0;
    // newMapValues_ is a bitmask that signals if values in mapValues have been updated, 
    // and if they are, Animations are bypassed in favor of the new Map values
	unsigned int isNewMapY_ = 0;
    // isNewMapY_ is a bitmask, where each bit signals if the Y values of parameters 0, 2, or 3 are updated
};