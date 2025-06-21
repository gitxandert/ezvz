#pragma once

#include "GraphicObject.h"
#include <string>

// A 5-pointed star primitive (2D)
class StarObject : public GraphicObject {
public:
    StarObject(ObjectType type, std::string& id);
    ~StarObject();

    glm::vec3 getSize() const override;
    void setSize(const glm::vec3& size);
    void setStroke(float stroke) override;
    void setFilled(bool filled) override;

    void draw() override;
    void cleanup();

private:
    glm::vec2 radii_ = { 0.5f, 0.5f };

    unsigned int VAO_{ 0 }, VBO_{ 0 }, EBO_{ 0 };
    void initMesh();
    bool meshInitialized_{ false };
};
