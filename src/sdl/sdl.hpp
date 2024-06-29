#pragma once
#include <iostream>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "../error.hpp"
#include "../constants.hpp"

struct WindowContext {
    SDL_Window* window;
    SDL_GLContext glContext;
};

class Window {
    private:
        WindowContext ctx;
    public:
        Window();

        ~Window();

        WindowContext getContext() const;
};