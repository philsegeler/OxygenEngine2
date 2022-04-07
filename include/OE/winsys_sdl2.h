#ifndef OE_SDL_WINDOWSYSTEM_H
#define OE_SDL_WINDOWSYSTEM_H

#include <OE/dummy_classes.h>
#include <OE/types/libs_oe.h>

class OE_SDL_WindowSystem : public oe::winsys_base_t {
public:
    OE_SDL_WindowSystem();
    ~OE_SDL_WindowSystem();

    bool init(int, int, std::string, bool, bool, void*);
    bool update();

    void finishInit();

    bool getMouseLockedState();
    void lockMouse();
    void unlockMouse();

    bool updateEvents();
    void destroy();

    SDL_Window*   window{nullptr};
    SDL_GLContext context;

    SDL_Event event;

    int counter{0};

    bool mouse_moved{false};

protected:
    void updateWindowEvents();
    void createWindow(int, int);
};


#endif
