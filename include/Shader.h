#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>


class Shader {
public:
    // program ID
    GLuint id;

    // ctor: load, compile, link
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    // bind/unbind
    void bind()   const { glUseProgram(id); }
    void unbind() const { glUseProgram(0); }

    // uniform setters
    void setUniformMat4(const char* name, const glm::mat4& mat) const;
    void setUniformVec4(const char* name, const glm::vec4& vec) const;
    void setUniformInt(const char* name, int value)         const;
    void setUniformFloat(const char* name, float value)      const;

private:
    // helpers
    std::string  loadFile(const char* path)           const;
    GLuint       compileShader(const char* src, GLenum type) const;
    GLint        getUniformLocation(const char* name) const;
};