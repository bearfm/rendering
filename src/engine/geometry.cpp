#include "geometry.hpp"

std::vector<Vertex> Geometry::createChunk() {
    std::vector<Vertex> vertices;

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                glm::vec3 pos(x, y, z);
                glm::vec3 color = glm::vec3(float(x) / CHUNK_SIZE, float(y) / CHUNK_SIZE, float(z) / CHUNK_SIZE);
                vertices.push_back({ pos, color });
            }
        }
    }

    return vertices;
}

std::vector<Vertex> Geometry::generateCubeVertices(float x, float y, float z, float size, const glm::vec3& color) {
    float halfSize = size / 2.0f;

    std::vector<Vertex> vertices = {
        // Frontface
        { { x - halfSize, y - halfSize, z + halfSize }, color },
        { { x + halfSize, y - halfSize, z + halfSize }, color },
        { { x + halfSize, y + halfSize, z + halfSize }, color },
        { { x - halfSize, y + halfSize, z + halfSize }, color },

        // Backface
        { { x - halfSize, y - halfSize, z - halfSize }, color },
        { { x + halfSize, y - halfSize, z - halfSize }, color },
        { { x + halfSize, y + halfSize, z - halfSize }, color },
        { { x - halfSize, y + halfSize, z - halfSize }, color },
    };

    return vertices;
}

std::vector<GLuint> Geometry::createCubeIndices() {
    return {
        // Frontface
        0, 1, 2,
        2, 3, 0,

        // Backface
        4, 5, 6,
        6, 7, 4,

        // Leftface
        4, 0, 3,
        3, 7, 4,

        // Rightface
        1, 5, 6,
        6, 2, 1,

        // Topface
        3, 2, 6,
        6, 7, 3,

        // Bottomface
        4, 5, 1,
        1, 0, 4
    };
}