#include <glad/glad.h>
#include "Rectangle.h"
#include <iostream>

RectangleObject::RectangleObject(ObjectType rect, std::string& id) : GraphicObject(rect, id)
{}

RectangleObject::~RectangleObject() {
    cleanup();
}

glm::vec3 RectangleObject::getSize() const {
    return { size_.x, size_.y, 0.0f };
}

void RectangleObject::setSize(const glm::vec3& newSize) {
    if (newSize.x != size_.x || newSize.y != size_.y) {
        size_.x = newSize.x;
        size_.y = newSize.y;
        cleanup();
    }
}

void RectangleObject::setSize(float width, float height) {
    setSize({ width, height, 0.0f });
}

void RectangleObject::setStroke(float stroke) {
    stroke_ = stroke;
    cleanup();
}

void RectangleObject::setFilled(bool filled) {
    filled_ = filled;
    cleanup();
}

void RectangleObject::cleanup() {
    if (meshInitialized_) {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        glDeleteBuffers(1, &EBO_);
        meshInitialized_ = false;
    }
}

void RectangleObject::initMesh() {
    if (meshInitialized_) return;

    float halfW = size_.x * 0.5f;
    float halfH = size_.y * 0.5f;
    float outerHalfW = halfW + stroke_ * 0.5f;
    float outerHalfH = halfH + stroke_ * 0.5f;

    float vertices[] = {
        // Inner quad (indices 0–3)
        -halfW, -halfH,  // 0
         halfW, -halfH,  // 1
         halfW,  halfH,  // 2
        -halfW,  halfH,  // 3

        // Outer quad (indices 4–7)
        -outerHalfW, -outerHalfH,  // 4
         outerHalfW, -outerHalfH,  // 5
         outerHalfW,  outerHalfH,  // 6
        -outerHalfW,  outerHalfH   // 7
    };

    // Indices for inner fill quad (2 triangles)
    unsigned int fillIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Indices for border ring (8 triangles, 4 sides)
    unsigned int strokeIndices[] = {
        // bottom
        4, 0, 1,
        1, 5, 4,

        // right
        5, 1, 2,
        2, 6, 5,

        // top
        6, 2, 3,
        3, 7, 6,

        // left
        7, 3, 0,
        0, 4, 7
    };

    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);

    glBindVertexArray(VAO_);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Store both fill + stroke indices in one EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fillIndices) + sizeof(strokeIndices), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(fillIndices), fillIndices);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fillIndices), sizeof(strokeIndices), strokeIndices);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    meshInitialized_ = true;
}


void RectangleObject::draw() {
    if (!meshInitialized_) initMesh();

    // Assumes shader is already bound and uniforms (uModel, uColor) are set externally
    glBindVertexArray(VAO_);

    if (filled_) {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int)));
    }
    glBindVertexArray(0);
}
