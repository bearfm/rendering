#include "sdl.hpp"

Window::Window() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_INIT ERR: " << SDL_GetError() << std::endl;
        this->~Window();
        THROW();
    }

    ctx.window = SDL_CreateWindow(NAME,
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                800, 600,
                                SDL_WINDOW_VULKAN);

    if (!ctx.window) {
        std::cerr << "SDL_CREATEWINDOW ERR: " << SDL_GetError() << std::endl;
        this->~Window();
        THROW();
    }
}

Window::~Window() {
    std::cout << "Deconstructing Window at line " << __LINE__ << " in " << __FILE__ << std::endl;
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();
}

WindowContext Window::getContext() const {
    return ctx;
}