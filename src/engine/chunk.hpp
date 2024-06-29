#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "geometry.hpp"
#include "voxel.hpp"

struct Chunk {
    Voxel voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    Chunk();
};