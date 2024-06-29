#pragma once
#include "../engine.hpp"
#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Object {
    public:
        Object(std::vector<float> vertices, std::vector<unsigned int> indices, Shader& shader);
        ~Object();
        void draw();

    private:
        GLuint VAO, VBO, EBO;
        Shader& shader;
        glm::mat4 modelMatrix;

        void setupMesh(std::vector<float> vertices, std::vector<unsigned int> indices);
};