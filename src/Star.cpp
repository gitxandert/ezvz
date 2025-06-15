#include <glad/glad.h>
#include "Star.h"
#include <cmath>
#include <iostream>

StarObject::StarObject(ObjectType type, std::string& id)
    : GraphicObject(type, id) {
}

StarObject::~StarObject() {
    cleanup();
}

glm::vec3 StarObject::getSize() const {
    return { radii_.x * 2.0f, radii_.y * 2.0f, 0.0f };
}

void StarObject::setSize(const glm::vec3& size) {
    radii_.x = 0.5f * size.x;
    radii_.y = 0.5f * size.y;
    meshInitialized_ = false;  // force rebuild on next draw
}

void StarObject::setStroke(float stroke) {
    stroke_ = stroke;
    cleanup();
}

void StarObject::setFilled(bool filled) {
    filled_ = filled;
    cleanup();
}

void StarObject::cleanup() {
    if (meshInitialized_) {
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        glDeleteBuffers(1, &EBO_);
        meshInitialized_ = false;
    }
}

void StarObject::initMesh() {
    if (meshInitialized_) return;

    const int numSpikes = 5;
    const int numVerts = numSpikes * 2;
    const float PI = 3.1415926f;

    std::vector<glm::vec2> baseVerts;   // Base star points
    std::vector<glm::vec2> outerVerts;  // Stroke outline points

    // Generate base star vertices
    for (int i = 0; i < numVerts; ++i) {
        float angle = (PI / numSpikes) * i - PI / 2.0f;
        float baseRadius = (i % 2 == 0) ? 1.0f : 0.5f;

        float rawX = baseRadius * cosf(angle);
        float rawY = baseRadius * sinf(angle);

        float x = rawX * radii_.x;
        float y = rawY * radii_.y;

        baseVerts.emplace_back(x, y);
    }

    // Compute outer (stroke) vertices
    for (int i = 0; i < numVerts; ++i) {
        glm::vec2 prev = baseVerts[(i - 1 + numVerts) % numVerts];
        glm::vec2 curr = baseVerts[i];
        glm::vec2 next = baseVerts[(i + 1) % numVerts];

        glm::vec2 dir1 = glm::normalize(curr - prev);
        glm::vec2 dir2 = glm::normalize(next - curr);

        glm::vec2 norm1 = glm::vec2(-dir1.y, dir1.x);
        glm::vec2 norm2 = glm::vec2(-dir2.y, dir2.x);

        glm::vec2 avgNormal = glm::normalize(norm1 + norm2);
        outerVerts.push_back(curr + avgNormal * stroke_);
    }

    std::vector<float> vertices;
    std::vector<unsigned int> fillIndices;
    std::vector<unsigned int> strokeIndices;

    // Center vertex (for triangle fan)
    vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(0.5f); vertices.push_back(0.5f);

    // Add inner star points
    for (auto& v : baseVerts) {
        float u = (v.x / (radii_.x * 2.0f)) + 0.5f;
        float v_uv = (v.y / (radii_.y * 2.0f)) + 0.5f;
        vertices.push_back(v.x); vertices.push_back(v.y);
        vertices.push_back(u); vertices.push_back(v_uv);
    }

    // Add outer stroke points
    for (auto& v : outerVerts) {
        float u = (v.x / (radii_.x * 2.0f)) + 0.5f;
        float v_uv = (v.y / (radii_.y * 2.0f)) + 0.5f;
        vertices.push_back(v.x); vertices.push_back(v.y);
        vertices.push_back(u); vertices.push_back(v_uv);
    }

    // Fill indices (triangle fan from center)
    for (int i = 1; i <= numVerts; ++i) {
        fillIndices.push_back(0); // center
        fillIndices.push_back(i);
        fillIndices.push_back(i % numVerts + 1);
    }

    // Stroke indices: connect each inner/outer vertex pair into a quad (2 triangles)
    int outerOffset = 1 + numVerts;
    for (int i = 0; i < numVerts; ++i) {
        int i0 = 1 + i;                     // inner vertex
        int i1 = 1 + (i + 1) % numVerts;
        int o0 = outerOffset + i;
        int o1 = outerOffset + (i + 1) % numVerts;

        strokeIndices.push_back(i0);
        strokeIndices.push_back(i1);
        strokeIndices.push_back(o1);

        strokeIndices.push_back(o1);
        strokeIndices.push_back(o0);
        strokeIndices.push_back(i0);
    }

    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);

    glBindVertexArray(VAO_);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (fillIndices.size() + strokeIndices.size()) * sizeof(unsigned int), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, fillIndices.size() * sizeof(unsigned int), fillIndices.data());
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, fillIndices.size() * sizeof(unsigned int), strokeIndices.size() * sizeof(unsigned int), strokeIndices.data());

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    meshInitialized_ = true;
}



void StarObject::draw() {
    if (!meshInitialized_) initMesh();

    glBindVertexArray(VAO_);

    if (filled_) {
        glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0); // 10 triangles × 3
    }
    else {
        glDrawElements(GL_TRIANGLES, 6 * 10, GL_UNSIGNED_INT, (void*)(30 * sizeof(unsigned int))); // stroke only
    }

    glBindVertexArray(0);
}
