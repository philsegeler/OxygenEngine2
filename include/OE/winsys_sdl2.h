#ifndef OE_SDL_WINDOWSYSTEM_H
#define OE_SDL_WINDOWSYSTEM_H

#include <OE/dummy_classes.h>
#include <OE/types/libs_oe.h>

namespace oe {

    class sdl_window_system_t : public oe::winsys_base_t {
        friend class OE_TaskManager;

    public:
        sdl_window_system_t();
        ~sdl_window_system_t();

        oe::winsys_output init(oe::winsys_init_info, oe::winsys_update_info);
        oe::winsys_output update(oe::winsys_update_info);

        bool is_mouse_locked();
        void lock_mouse();
        void unlock_mouse();

        void destroy();

    private:
        bool update_events();

        SDL_Window*   window_{nullptr};
        SDL_GLContext context_;

        SDL_Event event_;

        bool mouse_moved_{false};

        void              update_window_events();
        void              create_window(int, int);
        oe::winsys_output finish_init();

        int  resolution_x_{0};
        int  resolution_y_{0};
        int  dpi_{96};
        bool vsync_{true};

        // For different OpenGL versions
        std::string title_;
        bool        fullscreen_{false};
        int         major_{0};
        int         minor_{0};
        bool        isES_{false};
        bool        mouse_locked_{false};

        std::atomic<bool> restart_renderer_{false};
    };
}; // namespace oe

#endif
