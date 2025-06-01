#include "Ellipse.h"
#include <glad/glad.h>
#include <vector>
#include <cmath>

constexpr float PI = 3.14159265359f;

// ──────────────────────────────────────────────────────────────────

EllipseObject::EllipseObject(ObjectType ellipse,
    const std::string& id,
    int segments)
    : GraphicObject(ellipse, id),
    segments_(std::max(segments, 3))   // keep it sensible
{
}

EllipseObject::~EllipseObject() {
    cleanup();
}

// ── Size helpers ─────────────────────────────────────────────────

glm::vec3 EllipseObject::getSize() const {
    return { radii_.x * 2.0f, radii_.y * 2.0f, 0.0f };
}

void EllipseObject::setSize(const glm::vec3& size) {
    glm::vec2 newRadii{ size.x * 0.5f, size.y * 0.5f };
    if (newRadii != radii_) {
        radii_ = newRadii;
        cleanup();               // will rebuild mesh on next draw
    }
}

void EllipseObject::setRadius(float rx, float ry) {
    setSize({ rx * 2.0f, ry * 2.0f, 0.0f });
}

void EllipseObject::setSegments(int segments) {
    int clamped = std::max(segments, 3);
    if (clamped != segments_) {
        segments_ = clamped;
        cleanup();
    }
}

// ── GL resource management ───────────────────────────────────────

void EllipseObject::cleanup() {
    if (meshInitialized_) {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        VAO_ = VBO_ = 0;
        meshInitialized_ = false;
    }
}

void EllipseObject::initMesh() {
    if (meshInitialized_) return;

    // Centre + (segments+1) rim vertices (duplicate first rim vtx to close fan)
    const int vertCount = segments_ + 2;
    std::vector<float> vertices;
    vertices.reserve(vertCount * 4);     // 2 pos + 2 UV each

    // --- centre vertex ---
    vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.5f, 0.5f });

    // --- rim vertices ---
    for (int i = 0; i <= segments_; ++i) {
        float theta = (static_cast<float>(i) / segments_) * 2.0f * PI;
        float x = std::cos(theta) * radii_.x;
        float y = std::sin(theta) * radii_.y;

        // UVs: simple polar mapping into [0,1]
        float u = 0.5f + 0.5f * std::cos(theta);
        float v = 0.5f + 0.5f * std::sin(theta);

        vertices.insert(vertices.end(), { x, y, u, v });
    }

    // --- upload ---
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);

    glBindVertexArray(VAO_);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW);

    // position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (void*)0);
    // UV (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    meshInitialized_ = true;
}

// ── Draw ─────────────────────────────────────────────────────────

void EllipseObject::draw() {
    if (!meshInitialized_) initMesh();

    // Assumes shader already bound, uniforms set externally
    glBindVertexArray(VAO_);
    glDrawArrays(GL_TRIANGLE_FAN, 0, segments_ + 2);
    glBindVertexArray(0);
}
