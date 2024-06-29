#include "sdl.hpp"

Window::Window() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_INIT ERR: " << SDL_GetError() << std::endl;
        std::runtime_error("SDL_INIT ERR");
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
    SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
    SDL_SetHint(SDL_HINT_XINPUT_ENABLED, "0");
    SDL_SetHint(SDL_HINT_GAMECONTROLLERCONFIG, "");

    ctx.window = SDL_CreateWindow(NAME,
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                INITW, INITH,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!ctx.window) {
        std::cerr << "SDL_CREATEWINDOW ERR: " << SDL_GetError() << std::endl;
        std::runtime_error("SDL_CREATEWINDOW ERR");
        std::exit(-1);
    }

    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_ShowCursor(SDL_DISABLE);

    ctx.glContext = SDL_GL_CreateContext(ctx.window);
    if (!ctx.glContext) {
        std::cerr << "SDL_GL_CREATECONTEXT ERR: " << SDL_GetError() << std::endl;
        std::runtime_error("SDL_GL_CREATECONTEXT ERR");
        std::exit(-1);
    }

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        std::exit(-1);
    }

    glEnable(GL_DEPTH_TEST);
}

Window::~Window() {
    std::cout << "Deconstructing Window at line " << __LINE__ << " in " << __FILE__ << std::endl;
    SDL_GL_DeleteContext(ctx.glContext);
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();
}

WindowContext Window::getContext() const {
    return ctx;
}