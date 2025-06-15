#include <glad/glad.h>
#include "Triangle.h"
#include <iostream>

TriangleObject::TriangleObject(ObjectType type, std::string& id)
    : GraphicObject(type, id) {
}

TriangleObject::~TriangleObject() {
    cleanup();
}

glm::vec3 TriangleObject::getSize() const {
    return { size_.x, size_.y, 0.0f };
}

void TriangleObject::setSize(const glm::vec3& newSize) {
    if (newSize.x != size_.x || newSize.y != size_.y) {
        size_.x = newSize.x;
        size_.y = newSize.y;
        cleanup();
    }
}

void TriangleObject::setSize(float base, float height) {
    setSize({ base, height, 0.0f });
}

void TriangleObject::setStroke(float stroke) {
    stroke_ = stroke;
    cleanup();
}

void TriangleObject::setFilled(bool filled) {
    filled_ = filled;
    cleanup();
}

void TriangleObject::cleanup() {
    if (meshInitialized_) {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        glDeleteBuffers(1, &EBO_);
        meshInitialized_ = false;
    }
}

void TriangleObject::initMesh() {
    if (meshInitialized_) return;

    float halfBase = size_.x * 0.5f;
    float height = size_.y;

    // Original (inner) triangle positions
    glm::vec2 v0 = { 0.0f,        height * 0.5f };   // top
    glm::vec2 v1 = { -halfBase,  -height * 0.5f };   // bottom left
    glm::vec2 v2 = { halfBase,  -height * 0.5f };   // bottom right

    // Compute edge normals
    glm::vec2 e0 = glm::normalize(v1 - v0);
    glm::vec2 e1 = glm::normalize(v2 - v1);
    glm::vec2 e2 = glm::normalize(v0 - v2);

    glm::vec2 n0 = glm::vec2(-e0.y, e0.x); // normal to edge v0→v1
    glm::vec2 n1 = glm::vec2(-e1.y, e1.x); // normal to edge v1→v2
    glm::vec2 n2 = glm::vec2(-e2.y, e2.x); // normal to edge v2→v0

    // Offset each vertex outward using average of surrounding edge normals
    glm::vec2 o0 = v0 + stroke_ * glm::normalize(n0 + n2);
    glm::vec2 o1 = v1 + stroke_ * glm::normalize(n0 + n1);
    glm::vec2 o2 = v2 + stroke_ * glm::normalize(n1 + n2);

    float vertices[] = {
        // Inner triangle (0–2)
        v0.x, v0.y,  // 0
        v1.x, v1.y,  // 1
        v2.x, v2.y,  // 2

        // Outer triangle (3–5)
        o0.x, o0.y,  // 3
        o1.x, o1.y,  // 4
        o2.x, o2.y   // 5
    };

    unsigned int fillIndices[] = {
        0, 1, 2
    };

    unsigned int strokeIndices[] = {
        // Each edge forms a quad between inner and outer triangle
        0, 1, 4,
        4, 3, 0,

        1, 2, 5,
        5, 4, 1,

        2, 0, 3,
        3, 5, 2
    };

    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);

    glBindVertexArray(VAO_);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fillIndices) + sizeof(strokeIndices), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(fillIndices), fillIndices);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fillIndices), sizeof(strokeIndices), strokeIndices);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    meshInitialized_ = true;
}



void TriangleObject::draw() {
    if (!meshInitialized_) initMesh();

    glBindVertexArray(VAO_);

    if (filled_) {
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, (void*)(3 * sizeof(unsigned int)));
    }

    glBindVertexArray(0);
}


