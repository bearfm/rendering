#include "chunk.hpp"

Chunk::Chunk() {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                voxels[x][y][z].color = glm::vec3(1.0f, 0.0f, 0.0f);
            }
        }
    }
}