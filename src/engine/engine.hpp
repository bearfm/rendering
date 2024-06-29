#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Engine {
    std::string readF(const char* path);
};

class Shader {
    public:
        GLuint ID;
        Shader(const char* vertexPath, const char* fragmentPath);
        void use();
        void setMat4(const std::string& name, const glm::mat4& mat);

    private:
        GLuint compileShader(GLenum type, const char* source);
        GLuint createShaderProgram(const char* vertPath, const char* fragPath);
        void checkCompileErrors(GLuint shader, std::string type);
};