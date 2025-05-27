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
    float vertices[] = {
        // positions    // UVs
        -halfW, -halfH,  0.0f, 0.0f,
            halfW, -halfH,  1.0f, 0.0f,
            halfW,  halfH,  1.0f, 1.0f,
        -halfW,  halfH,  0.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);

    glBindVertexArray(VAO_);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    // UV attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    meshInitialized_ = true;
}

void RectangleObject::draw() {
    if (!meshInitialized_) initMesh();

    // Assumes shader is already bound and uniforms (uModel, uColor) are set externally
    glBindVertexArray(VAO_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
