#pragma once
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include "../error.hpp"
#include "../constants.hpp"

struct WindowContext {
    SDL_Window* window;
};

class Window {
    private:
        WindowContext ctx;
    public:
        Window();

        ~Window();

        WindowContext getContext() const;
};