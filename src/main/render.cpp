#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <iostream>
#include "render.hpp"
#include "../engine/object/object.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../include/tiny_gltf.hpp"

bool loadModel(const std::string& filePath, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;

    std::string err;
    std::string warn;

    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filePath);

    if (!warn.empty()) {
        std::cout << "Warning: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "Error: " << err << std::endl;
        return false;
    }

    if (!ret) {
        std::cerr << "Failed to load GLTF/GLB file" << std::endl;
        return false;
    }

    if (!model.meshes.empty()) {
        const auto& mesh = model.meshes[0];
        const auto& primitive = mesh.primitives[0];

        if (primitive.mode == TINYGLTF_MODE_TRIANGLES) {
            const auto& accessor = model.accessors[primitive.indices];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];

            const unsigned char* data_ptr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
            const int* indices_ptr = reinterpret_cast<const int*>(data_ptr);
            int num_indices = accessor.count;

            indices.reserve(num_indices);
            for (int i = 0; i < num_indices; ++i) {
                indices.push_back(static_cast<unsigned int>(indices_ptr[i]));
            }

            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                const auto& positionAccessor = model.accessors[primitive.attributes.at("POSITION")];
                const auto& positionBufferView = model.bufferViews[positionAccessor.bufferView];
                const auto& positionBuffer = model.buffers[positionBufferView.buffer];

                const float* positions_ptr = reinterpret_cast<const float*>(positionBuffer.data.data() + positionBufferView.byteOffset + positionAccessor.byteOffset);
                int num_positions = positionAccessor.count;

                vertices.reserve(num_positions * 3);

                for (int i = 0; i < num_positions * 3; ++i) {
                    vertices.push_back(positions_ptr[i]);
                }
            } else {
                std::cerr << "No POSITION attribute found in the mesh" << std::endl;
                return false;
            }
        } else {
            std::cerr << "Unsupported primitive mode: " << primitive.mode << std::endl;
            return false;
        }
    } else {
        std::cerr << "No meshes found in the GLTF/GLB file" << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    Window window;

    if (glewInit() != GLEW_OK) {
        std::cerr << "Glew init failed." << std::endl;
        window.~Window();
        std::runtime_error("Glew init failed.");
    }

    Shader shader("shaders/vert.glsl", "shaders/frag.glsl");

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    if (!loadModel("untitled.glb", vertices, indices)) {
        std::cerr << "Load model fail" << std::endl;
        std::runtime_error("Load model fail");
        std::exit(-1);
    }

    Object object(vertices, indices, shader);

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        object.draw();

        SDL_GL_SwapWindow(window.getContext().window);
    }

    return EXIT_SUCCESS;
}