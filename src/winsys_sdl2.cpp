#include <OE/Renderer/api_gpu.h>
#include <OE/types/libs_oe.h>
#include <OE/winsys_sdl2.h>
#include <iostream>

using namespace std;

OE_SDL_WindowSystem::OE_SDL_WindowSystem() {
    this->winsys_ = oe::WINSYS_SDL;
#ifdef OE_PLATFORM_LINUX
    this->os_ = oe::OS_LINUX;
#endif
#ifdef OE_PLATFORM_WINDOWS
    this->os_ = oe::OS_WINDOWS;
#endif
#ifdef OE_PLATFORM_ANDROID
    this->os_ = oe::OS_ANDROID;
#endif
#ifdef OE_PLATFORM_WEB
    this->os_ = oe::OS_WEB;
#endif
}

OE_SDL_WindowSystem::~OE_SDL_WindowSystem() {

    SDL_Quit();
}

void OE_SDL_WindowSystem::createWindow(int x, int y) {
    if (!this->fullscreen)
#ifndef OE_PLATFORM_WEB
        this->window = SDL_CreateWindow(this->title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, x, y,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
#else
        this->window = SDL_CreateWindow(this->title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, x, y,
                                        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
#endif
    else
        this->window = SDL_CreateWindow(this->title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0,
                                        SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL);
}

oe::winsys_output OE_SDL_WindowSystem::init(oe::winsys_init_info init_info, oe::winsys_update_info update_info) {

    int x = update_info.res_x;
    int y = update_info.res_y;

    bool use_legacy_renderer = init_info.requested_backend == nre::gpu::GLES2;

    this->title      = update_info.title;
    this->fullscreen = update_info.use_fullscreen;

#ifndef OE_PLATFORM_WEB
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::stringstream ss;
        ss << "Could not initialize SDL2, " << SDL_GetError();
        throw oe::winsys_init_failed(ss.str());
    }

    SDL_GL_LoadLibrary(NULL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    // Request an OpenGL 3.3 core context
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    this->major = 3;
    this->minor = 3;
    this->isES  = false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);

    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#else
    SDL_Init(SDL_INIT_VIDEO);
#endif



    if (not use_legacy_renderer) {

#ifndef OE_PLATFORM_WEB
        if (init_info.requested_backend == nre::gpu::GL) {
            this->createWindow(x, y);
            this->context = SDL_GL_CreateContext(this->window);
            if (context == NULL) {
                cout << "OE WARNING: Could not initialize OpenGL 3.3 Core Context, " << SDL_GetError() << endl;
                SDL_DestroyWindow(window);
                this->createWindow(x, y);
            }
            else {
                return this->finishInit();
            }
        }
#endif
        // Request an OpenGL ES 3.0 context


#ifdef OE_PLATFORM_WINDOWS
        SDL_SetHint(SDL_HINT_VIDEO_WIN_D3DCOMPILER, "d3dcompiler_43.dll");
        SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
#endif
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

        this->major = 3;
        this->minor = 0;
        this->isES  = true;

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);

        // Also request a depth buffer
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        this->createWindow(x, y);
        this->context = SDL_GL_CreateContext(this->window);
        if (context == NULL) {
            cout << "OE WARNING: Could not initialize OpenGL ES 3.0 Context, " << SDL_GetError() << endl;
            SDL_DestroyWindow(window);
            this->createWindow(x, y);
        }
        else {
            return this->finishInit();
        }
    }

    // Request an OpenGL ES 2.0 context if everything else fails
    // If this does not work either then consider not trying to run the engine on prehistoric stuff that
    // does not even support basic shaders. Or use a software OpenGL renderer.

#ifdef OE_PLATFORM_WINDOWS
    SDL_SetHint(SDL_HINT_VIDEO_WIN_D3DCOMPILER, "d3dcompiler_43.dll");
    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");
#endif


    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    this->major = 2;
    this->minor = 0;
    this->isES  = true;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#ifndef OE_PLATFORM_WEB
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);
#endif
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#ifndef OE_PLATFORM_WINDOWS
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
#else
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
#endif

    this->createWindow(x, y);
    this->context = SDL_GL_CreateContext(this->window);
    if (context == NULL) {
        cout << "OE WARNING: Could not initialize OpenGL ES 2.0 Context, " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        this->createWindow(x, y);
    }
    else {
        return this->finishInit();
    }

    return oe::winsys_output();
}

oe::winsys_output OE_SDL_WindowSystem::finishInit() {

#ifndef OE_PLATFORM_WEB
    if (!this->isES)
        gladLoadGLLoader(SDL_GL_GetProcAddress);
    else
        gladLoadGLES2Loader(SDL_GL_GetProcAddress);
#else
    SDL_GL_MakeCurrent(this->window, this->context);
#endif
    std::stringstream ss;
    ss << "Vendor:   '" << glGetString(GL_VENDOR) << "'"
       << "\n";
    ss << "Renderer: '" << glGetString(GL_RENDERER) << "'"
       << "\n";
    ss << "Version:  '" << glGetString(GL_VERSION) << "'"
       << "\n";

    cout << ss.str();
    OE_WriteToLog(ss.str());
    SDL_GL_SetSwapInterval(1);

    SDL_GetWindowSize(window, &this->resolution_x, &this->resolution_y);
    glViewport(0, 0, this->resolution_x, this->resolution_y);

    // SDL_GL_SwapWindow(this->window);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    SDL_GL_SwapWindow(this->window);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    SDL_GL_SwapWindow(this->window);

    oe::winsys_output output;

    if (!this->isES) {
        output.backend = nre::gpu::GL;
    }
    else {
        if ((this->major != 2) or (this->major == 3 and this->minor == 0)) {
            output.backend = nre::gpu::GLES;
        }
        else {
            output.backend = nre::gpu::GLES2;
        }
    }

    this->event_handler_.init();
    output.res_x = this->resolution_x;
    output.res_y = this->resolution_y;
    output.major = this->major;
    output.minor = this->minor;

    return output;
}

bool OE_SDL_WindowSystem::is_mouse_locked() {
    lockMutex();
    bool output = mouse_locked;
    unlockMutex();
    return output;
}

void OE_SDL_WindowSystem::lock_mouse() {
    SDL_SetRelativeMouseMode(SDL_TRUE);
    lockMutex();
    this->mouse_locked = true;
    unlockMutex();
}

void OE_SDL_WindowSystem::unlock_mouse() {
    SDL_SetRelativeMouseMode(SDL_FALSE);
    lockMutex();
    this->mouse_locked = false;
    unlockMutex();
}

oe::winsys_output OE_SDL_WindowSystem::update(oe::winsys_update_info update_info) {

    this->counter++;
    this->counter = this->counter % 100;

    SDL_GL_SwapWindow(this->window);

    // change title
    if (this->title != update_info.title) {
        this->title = update_info.title;
        SDL_SetWindowTitle(this->window, this->title.c_str());
    }

    // Change viewport resolution
    int x;
    int y;
    SDL_GetWindowSize(window, &x, &y);

    this->resolution_x = x;
    this->resolution_y = y;


    this->event_handler_.update_input();
    this->mouse_moved = false;

    while (SDL_PollEvent(&this->event)) {

        // exit before handling SDL events
        if (event.type == SDL_QUIT) {
            oe::winsys_output output;
            this->event_handler_.done_ = true;
            output.done                = this->event_handler_.done_;
            return output;
        }
        this->update_events();
        this->updateWindowEvents();
    }

    if (this->mouse_moved) {
        // fetch mouse position, since this IS needed
        this->event_handler_.lockMutex();

        int mouse_x, mouse_y, mouse_delta_x, mouse_delta_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        SDL_GetRelativeMouseState(&mouse_delta_x, &mouse_delta_y);
        this->event_handler_.internal_update_mouse_status(mouse_x, mouse_y, mouse_delta_x, mouse_delta_y);

        this->event_handler_.unlockMutex();

        this->event_handler_.broadcast_ievent("mouse-motion");
    }

    // This is needed to support things like OE_Finish()
    oe::winsys_output output;
    if (!this->isES) {
        output.backend = nre::gpu::GL;
    }
    else {
        if ((this->major != 2) or (this->major == 3 and this->minor == 0)) {
            output.backend = nre::gpu::GLES;
        }
        else {
            output.backend = nre::gpu::GLES2;
        }
    }
    output.done        = this->event_handler_.done_;
    output.res_x       = this->resolution_x;
    output.res_y       = this->resolution_y;
    output.major       = this->major;
    output.minor       = this->minor;
    output.mouse_moved = this->mouse_moved;
    return output;
}

bool OE_SDL_WindowSystem::update_events() {

    switch (this->event.type) {
    // check for key presses
    case SDL_KEYDOWN:
        if (this->event_handler_.input_handler_.keyList_.count(this->event.key.keysym.sym) != 0) {
            auto key_string = this->event_handler_.input_handler_.keyList_[this->event.key.keysym.sym];
            this->event_handler_.internal_register_keydown_event("keyboard-" + key_string);
        }
        break;

    // check for releases
    case SDL_KEYUP:
        if (this->event_handler_.input_handler_.keyList_.count(this->event.key.keysym.sym) != 0) {
            auto key_string = this->event_handler_.input_handler_.keyList_[this->event.key.keysym.sym];
            this->event_handler_.internal_register_keyup_event("keyboard-" + key_string);
        }
        break;

        // update mouse position
    case SDL_MOUSEMOTION:
        // cout << "MOUSE_MOTION EVENt" << endl;
        this->mouse_moved = true;
        break;

        // update mouse down events
    case SDL_MOUSEBUTTONDOWN:
        if (this->event_handler_.input_handler_.mouseList_.count(this->event.button.button) != 0) {
            auto key_string = this->event_handler_.input_handler_.mouseList_[this->event.button.button];
            this->event_handler_.internal_register_keydown_event("mouse-" + key_string);
        }
        break;

    // update mouse up events
    case SDL_MOUSEBUTTONUP:
        if (this->event_handler_.input_handler_.mouseList_.count(this->event.button.button) != 0) {
            auto key_string = this->event_handler_.input_handler_.mouseList_[this->event.button.button];
            this->event_handler_.internal_register_keyup_event("mouse-" + key_string);
        }
        break;

    // update mouse wheel events
    case SDL_MOUSEWHEEL:

        this->event_handler_.lockMutex();
        // mouse_mouse_wheel = event.wheel.y;
        //  fetch mouse position, since this may be needed
        this->event_handler_.unlockMutex();

        this->event_handler_.broadcast_ievent("mouse-wheel");
        break;
    }

    return true;
}

void OE_SDL_WindowSystem::updateWindowEvents() {

    if (this->event.type == SDL_WINDOWEVENT) {
        switch (this->event.window.event) {
        case SDL_WINDOWEVENT_SHOWN:

            printf("Window %d shown", this->event.window.windowID);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            printf("Window %d hidden", this->event.window.windowID);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            printf("Window %d exposed", this->event.window.windowID);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_MOVED:
            printf("Window %d moved to %d,%d", this->event.window.windowID, this->event.window.data1, this->event.window.data2);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_RESIZED:
            printf("Window %d resized to %dx%d", this->event.window.windowID, this->event.window.data1,
                   this->event.window.data2);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            printf("Window %d size changed to %dx%d", this->event.window.windowID, this->event.window.data1,
                   this->event.window.data2);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            printf("Window %d minimized", this->event.window.windowID);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            printf("Window %d maximized", this->event.window.windowID);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_RESTORED:
            printf("Window %d restored", this->event.window.windowID);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_ENTER:
            printf("Mouse entered window %d", this->event.window.windowID);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_LEAVE:
            printf("Mouse left window %d", this->event.window.windowID);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            printf("Window %d gained keyboard focus", this->event.window.windowID);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            printf("Window %d lost keyboard focus", this->event.window.windowID);
            cout << endl;
            break;
        case SDL_WINDOWEVENT_CLOSE:
            printf("Window %d closed", this->event.window.windowID);
            cout << endl;
            break;
        }
    }
}


void OE_SDL_WindowSystem::destroy() {

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
}
