#include <glad/glad.h>
#include "Line.h"
#include <iostream>

LineObject::LineObject(ObjectType type, std::string& id)
    : GraphicObject(type, id) {
}

LineObject::LineObject(const std::shared_ptr<GraphicObject>& other, int count)
    : GraphicObject(other, count)
{
    setSize(other->getSize());
}

LineObject::~LineObject() {
    cleanup();
}

glm::vec3 LineObject::getSize() const {
    return { size_.x, size_.y, 0.0f };
}

void LineObject::setSize(const glm::vec3& newSize) {
    size_.x = newSize.x;
    size_.y = newSize.y;
    cleanup();
}

void LineObject::setStroke(float stroke) {
    stroke_ = stroke;
    cleanup();
}
void LineObject::setFilled(bool filled) {};

void LineObject::cleanup() {
    if (meshInitialized_) {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        meshInitialized_ = false;
    }
}

void LineObject::initMesh() {
    if (meshInitialized_) return;

    float halfLength = size_.x * 0.5f;
    float vertices[] = {
        -halfLength, 0.0f, 0.0f, 0.0f,
         halfLength, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);

    glBindVertexArray(VAO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    meshInitialized_ = true;
}

void LineObject::draw() {
    if (!meshInitialized_) initMesh();

    glBindVertexArray(VAO_);

    glLineWidth(size_.y);

    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}
