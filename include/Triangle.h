#pragma once

#include "GraphicObject.h"
#include <string>

// A 2D triangle primitive
class TriangleObject : public GraphicObject {
public:
    TriangleObject(ObjectType type, std::string& id);
    ~TriangleObject();

    // Size accessors
    glm::vec3 getSize() const override;
    void setSize(const glm::vec3& size);
    void setSize(float base, float height);
    void setStroke(float stroke) override;
    void setFilled(bool filled) override;

    // Render the triangle
    void draw() override;
    void cleanup();

private:
    glm::vec2 size_{ 200.0f, 200.0f };  // base x height

    unsigned int VAO_{ 0 }, VBO_{ 0 }, EBO_{ 0 };
    void initMesh();
    bool meshInitialized_{ false };
};
