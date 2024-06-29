#pragma once

#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define CHUNK_SIZE (int)32

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

namespace Geometry {
    std::vector<Vertex> createChunk();
    std::vector<Vertex> generateCubeVertices(float x, float y, float z, float size, const glm::vec3& color);
    std::vector<GLuint> createCubeIndices();
};