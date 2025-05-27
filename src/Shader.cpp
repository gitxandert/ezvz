// Shader.cpp
#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    // 1. Load source
    std::string vertSrc = loadFile(vertexPath);
    std::string fragSrc = loadFile(fragmentPath);

    // 2. Compile
    GLuint vert = compileShader(vertSrc.c_str(), GL_VERTEX_SHADER);
    GLuint frag = compileShader(fragSrc.c_str(), GL_FRAGMENT_SHADER);

    // 3. Link program
    id = glCreateProgram();
    glAttachShader(id, vert);
    glAttachShader(id, frag);
    glLinkProgram(id);

    // 4. Check linking errors
    GLint success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
            << infoLog << std::endl;
    }

    // 5. Clean up shaders (no longer needed once linked)
    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader() {
    glDeleteProgram(id);
}

std::string Shader::loadFile(const char* path) const {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR::SHADER::FILE_NOT_FOUND: " << path << std::endl;
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint Shader::compileShader(const char* src, GLenum type) const {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // error check
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::"
            << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")
            << "::COMPILATION_FAILED\n"
            << infoLog << std::endl;
    }
    return shader;
}

GLint Shader::getUniformLocation(const char* name) const {
    GLint loc = glGetUniformLocation(id, name);
    if (loc == -1) {
        std::cerr << "WARNING::SHADER::UNIFORM_NOT_FOUND: " << name << std::endl;
    }
    return loc;
}

void Shader::setUniformMat4(const char* name, const glm::mat4& mat) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setUniformVec4(const char* name, const glm::vec4& vec) const {
    glUniform4fv(getUniformLocation(name), 1, &vec[0]);
}

void Shader::setUniformInt(const char* name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setUniformFloat(const char* name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}
