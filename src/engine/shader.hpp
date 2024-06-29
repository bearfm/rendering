#pragma once

#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <fstream>

class Shader {
    public:
        Shader(const char* vertexPath, const char* fragmentPath);
        void use();
        void setBool(const std::string& name, bool value) const;
        void setInt(const std::string& name, int value) const;
        void setFloat(const std::string& name, float value) const;
        GLint getUniformLocation(const std::string& name) const;
    private:
        unsigned int ID;
        void checkCompileErrors(unsigned int shader, const std::string& type);
};