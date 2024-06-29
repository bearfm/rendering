#pragma once
#include "chunk.hpp"

using std::vector;

struct World {
    vector<vector<vector<Chunk>>> chunks;
    World();
};