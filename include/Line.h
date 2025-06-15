#pragma once

#include "GraphicObject.h"
#include <string>

// A 2D line primitive
class LineObject : public GraphicObject {
public:
    LineObject(ObjectType type, std::string& id);
    ~LineObject();

    glm::vec3 getSize() const override;
    void setSize(const glm::vec3& size);
    void setStroke(float stroke) override;
    void setFilled(bool filled) override;

    void draw() override;
    void cleanup();

private:
    glm::vec2 size_{ 200.0f, 0.0f };  // x = length, y = unused

    unsigned int VAO_{ 0 }, VBO_{ 0 };
    void initMesh();
    bool meshInitialized_{ false };
};
