#ifndef OE_SDL_WINDOWSYSTEM_H
#define OE_SDL_WINDOWSYSTEM_H

#include <OE/dummy_classes.h>
#include <OE/types/libs_oe.h>

class OE_SDL_WindowSystem : public oe::winsys_base_t {
    friend class OE_TaskManager;

public:
    OE_SDL_WindowSystem();
    ~OE_SDL_WindowSystem();

    oe::winsys_output init(oe::winsys_init_info, oe::winsys_update_info);
    oe::winsys_output update(oe::winsys_update_info);

    bool is_mouse_locked();
    void lock_mouse();
    void unlock_mouse();

    void destroy();

private:
    bool update_events();

    SDL_Window*   window{nullptr};
    SDL_GLContext context;

    SDL_Event event;

    int counter{0};

    bool mouse_moved{false};

    void              updateWindowEvents();
    void              createWindow(int, int);
    oe::winsys_output finishInit();

    int  resolution_x{0};
    int  resolution_y{0};
    int  dpi{96};
    bool vsync{true};

    // For different OpenGL versions
    std::string title;
    bool        fullscreen{false};
    int         major{0};
    int         minor{0};
    bool        isES{false};
    bool        mouse_locked{false};

    std::atomic<bool> restart_renderer{false};
};


#endif
