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

void EllipseObject::setStroke(float stroke) {
    stroke_ = stroke;
    cleanup();
}

void EllipseObject::setFilled(bool filled) {
    filled_ = filled;
    cleanup();
}

// ── GL resource management ───────────────────────────────────────

void EllipseObject::cleanup() {
    if (meshInitialized_) {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        VAO_ = VBO_ = 0;
        if (strokeEBO_ != 0) {
            glDeleteBuffers(1, &strokeEBO_);
            strokeEBO_ = 0;
        }
        meshInitialized_ = false;
    }
}

void EllipseObject::initMesh() {
    if (meshInitialized_) return;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    if (filled_) {
        // --- Filled ellipse: center + rim
        const int vertCount = segments_ + 2;
        vertices.reserve(vertCount * 4);

        vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.5f, 0.5f }); // center

        for (int i = 0; i <= segments_; ++i) {
            float theta = (float(i) / segments_) * 2.0f * PI;
            float x = std::cos(theta) * radii_.x;
            float y = std::sin(theta) * radii_.y;
            float u = 0.5f + 0.5f * std::cos(theta);
            float v = 0.5f + 0.5f * std::sin(theta);
            vertices.insert(vertices.end(), { x, y, u, v });
        }

    }
    else {
        // --- Outlined ellipse: inner + outer ring vertices
        for (int i = 0; i <= segments_; ++i) {
            float theta = (float(i) / segments_) * 2.0f * PI;

            float cosT = std::cos(theta);
            float sinT = std::sin(theta);

            float xInner = cosT * radii_.x;
            float yInner = sinT * radii_.y;

            float xOuter = cosT * (radii_.x + stroke_ * 0.5f);
            float yOuter = sinT * (radii_.y + stroke_ * 0.5f);

            float u = 0.5f + 0.5f * cosT;
            float v = 0.5f + 0.5f * sinT;

            // outer first, then inner — makes strip winding consistent
            vertices.insert(vertices.end(), { xOuter, yOuter, u, v });
            vertices.insert(vertices.end(), { xInner, yInner, u, v });
        }

        // Create triangle strip indices: 2 indices per segment (2 triangles per quad)
        for (int i = 0; i < segments_; ++i) {
            unsigned int idx = i * 2;
            indices.insert(indices.end(), {
                idx, idx + 1,
                idx + 2, idx + 3
                });
        }
    }

    // Upload to GPU
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glBindVertexArray(VAO_);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    if (!filled_) {
        glGenBuffers(1, &strokeEBO_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, strokeEBO_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        strokeIndexCount_ = static_cast<GLsizei>(indices.size());
    }

    glBindVertexArray(0);
    meshInitialized_ = true;
}


// ── Draw ─────────────────────────────────────────────────────────

void EllipseObject::draw() {
    if (!meshInitialized_) initMesh();

    glBindVertexArray(VAO_);

    if (filled_) {
        glDrawArrays(GL_TRIANGLE_FAN, 0, segments_ + 2);
    }
    else {
        glDrawElements(GL_TRIANGLE_STRIP, strokeIndexCount_, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}

