#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include "sdl/sdl.hpp"
#include <fstream>
#include <sstream>
#include "engine/shader.hpp"
#include "engine/geometry.hpp"

#define offsetof(s,m) (uint64_t)(&((s*)0)->m)

void handleResize(int w, int h) {
    glViewport(0, 0, w, h);
}

GLuint createVAO(const std::vector<Vertex>& vertices) {
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, color)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return VAO;
}

int main(int argc, char** argv) {
    Window window;
    Shader geometryShader("shaders/vert.glsl", "shaders/frag.glsl");

    std::vector<Vertex> chunk = Geometry::createChunk();
    GLuint VAO = createVAO(chunk);

    glm::vec3 cameraPos(50.0f, 25.0f, 100.0f);
    glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

    float cameraSpeed = 20.0f;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float lastX = INITW / 2.0f;
    float lastY = INITH / 2.0f;
    bool firstMouse = true;

    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)INITW / (float)INITH, 0.1f, 1000.0f);

    bool running = true;
    SDL_Event event;

    Uint64 currentTime = SDL_GetPerformanceCounter();
    Uint64 lastTime = 0;
    double deltaTime = 0.0;

    int wW = INITW;
    int wH = INITH;

    while (running) {
        lastTime = currentTime;
        currentTime = SDL_GetPerformanceCounter();
        deltaTime = (double)((currentTime - lastTime) * 1000 / (double)SDL_GetPerformanceFrequency());

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    running = false;
                    break;
                }
                case SDL_WINDOWEVENT: {
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED:
                            wW = event.window.data1;
                            wH = event.window.data2;
                            handleResize(wW, wH);
                            projection = glm::perspective(glm::radians(45.0f), (float)wW / (float)wH, 0.1f, 1000.0f);
                            break;
                    }
                    break;
                }
                case SDL_MOUSEMOTION:
                    if (event.motion.x != wW / 2 || event.motion.y != wH / 2) {
                        float xOffset = event.motion.x - wW / 2;
                        float yOffset = wH / 2 - event.motion.y; // reversed since y-coordinates range from bottom to top

                        float sensitivity = 0.1f;
                        xOffset *= sensitivity;
                        yOffset *= sensitivity;

                        yaw += xOffset;
                        pitch += yOffset;

                        if (pitch > 89.0f) pitch = 89.0f;
                        if (pitch < -89.0f) pitch = -89.0f;

                        glm::vec3 front;
                        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
                        front.y = sin(glm::radians(pitch));
                        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
                        cameraFront = glm::normalize(front);

                        SDL_WarpMouseInWindow(window.getContext().window, wW / 2, wH / 2); // Warp mouse back to center
                    }
                    break;
            }
        }

        float newCamSpeed = cameraSpeed * deltaTime / 1000.0;
        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
        glm::vec3 cameraDirection = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));

        const Uint8 *keyboardState = SDL_GetKeyboardState(nullptr);

        if (keyboardState[SDL_SCANCODE_W]) {
            cameraPos += newCamSpeed * cameraDirection;
        }
        if (keyboardState[SDL_SCANCODE_S]) {
            cameraPos -= newCamSpeed * cameraDirection;
        }
        if (keyboardState[SDL_SCANCODE_A]) {
            cameraPos -= newCamSpeed * cameraRight;
        }
        if (keyboardState[SDL_SCANCODE_D]) {
            cameraPos += newCamSpeed * cameraRight;
        }
        if (keyboardState[SDL_SCANCODE_ESCAPE]) {
            running = false;
        }

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        geometryShader.use();

        GLuint viewLoc = geometryShader.getUniformLocation("view");
        GLuint projLoc = geometryShader.getUniformLocation("projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, chunk.size());
        glBindVertexArray(0);

        SDL_GL_SwapWindow(window.getContext().window);
    }

    return 0;
}
