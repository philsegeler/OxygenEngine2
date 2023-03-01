#ifndef OE_INPUTEVENTHANDLER_H
#define OE_INPUTEVENTHANDLER_H

#include <OE/Events/event.h>


/** This class is intended to store any user inputs
 *
 */

class OE_TaskManager;
class OE_SDL_WindowSystem;
class OE_WindowSystemBase;

namespace oe {
    class input_event_handler_t {
        friend class task_manager_t;
        friend class event_handler_t;
        friend class sdl_window_system_t;

    public:
        input_event_handler_t();
        ~input_event_handler_t();

        std::map<std::string, std::shared_ptr<oe::event_t>> create_events();

    private:
        std::unordered_map<uint8_t, std::string> mouseList_;
        std::unordered_map<uint8_t, std::string> keyList_;
    };
};     // namespace oe
#endif // OE_INPUTEVENTHANDLER_H
