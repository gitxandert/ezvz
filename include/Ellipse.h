#pragma once

#include "GraphicObject.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

// A 2-D ellipse / circle primitive
class EllipseObject : public GraphicObject {
public:
    // 'segments' = number of rim vertices (≥ 3).  Higher = smoother.
    EllipseObject(ObjectType ellipse, const std::string& id, int segments = 64);
    ~EllipseObject();

    // ── Size / radius ──────────────────────────────────────────────
    // Returns full width/height (diameter) in world units
    glm::vec3 getSize() const override;

    // Sets full width & height
    void setSize(const glm::vec3& size);
    void setSize(float width, float height) { setSize({ width, height, 0.0f }); }
    void setStroke(float stroke) override;

    // Convenience: set radii directly
    void setRadius(float rx, float ry);
    void setFilled(bool filled) override;

    // Mesh smoothness
    void setSegments(int segments);

    // ── Rendering ─────────────────────────────────────────────────
    void draw() override;
    void cleanup();

private:
    void initMesh();

    glm::vec2 radii_{ 100.0f, 100.0f };   // (rx, ry)
    int        segments_{ 64 };           // rim vertex count
    GLuint strokeEBO_ = 0;
    GLsizei strokeIndexCount_ = 0;

    unsigned int VAO_{ 0 }, VBO_{ 0 };
    bool meshInitialized_{ false };
};
