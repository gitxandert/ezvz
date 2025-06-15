#pragma once

#include "GraphicObject.h"
#include <string>

// A 2D rectangle primitive
class RectangleObject : public GraphicObject {
public:
    // id: unique identifier, size: width and height in world units
    RectangleObject(ObjectType rect, std::string& id);
    ~RectangleObject();

    // Size accessors
    glm::vec3 getSize() const override;
    void setSize(const glm::vec3& size);
    void setSize(float width, float height);

    // Render the rectangle
    void draw() override;
    void cleanup();

    void setStroke(float) override;
    void setFilled(bool filled) override;

private:
    glm::vec2 size_{ 200.0f, 200.0f };

    // OpenGL resources (vertex array, buffers)
    unsigned int VAO_{ 0 }, VBO_{ 0 }, EBO_{ 0 };

    // Helper: initializes the mesh data (called on first draw)
    void initMesh();
    bool meshInitialized_{ false };
};
