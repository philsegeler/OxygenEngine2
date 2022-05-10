#ifndef OE_EVENT_HANDLER_H
#define OE_EVENT_HANDLER_H

#include <OE/Events/event_container.h>
#include <OE/Events/input_event_handler.h>

class OE_TaskManager;
class OE_SDL_WindowSystem;
class OE_WindowSystemBase;

namespace oe {
    class event_handler_t : public OE_THREAD_SAFETY_OBJECT {
        friend class ::OE_TaskManager;
        friend class ::OE_SDL_WindowSystem;
        friend class ::OE_WindowSystemBase;

    public:
        event_handler_t();
        ~event_handler_t();

        // internal event functions
        void                         init();
        std::shared_ptr<oe::event_t> get_event(const std::string&);

        std::size_t create_user_event(const std::string&);
        std::size_t create_physics_event(const std::string&);
        std::size_t create_network_event(const std::string&);
        std::size_t create_renderer_event(const std::string&);

        void set_event_func(std::size_t, const oe::method_type);
        void set_event_func(const std::string&, const oe::method_type);
        void broadcast_event(std::size_t);
        void broadcast_event(const std::string&);

        void        broadcast_event_wait(std::size_t, int); // TODO
        void        map_event(std::size_t, std::size_t);
        void        unmap_event(std::size_t, std::size_t);
        task_action call_event(std::size_t);
        void        destroy_event(std::size_t);

        std::size_t get_event_id(const std::string&);
        std::string get_event_name(std::size_t);
        std::size_t get_event_activations(std::size_t);
        std::size_t get_event_counter(std::size_t);



        bool done();
        void set_done(bool);

        int  get_mouse_x();
        int  get_mouse_y();
        int  get_mouse_delta_y();
        int  get_mouse_delta_x();
        bool has_mouse_moved();

    private:
        // The methods starting with internal* are only supposed to be used
        // in subclasses of OE_WindowSystemBase
        // the key strings must exist
        void internal_update_mouse_status(int x, int y, int delta_x, int delta_y);
        void internal_register_keydown_event(const std::string&);
        void internal_register_keyup_event(const std::string&);

        void update_input();
        void cleanup();
        int  handle_all_events();

        bool has_pending_events();

        std::atomic<bool> done_;

        input_event_handler_t              input_handler_;
        oe::event_container_t<oe::event_t> events_list_;

        std::unordered_map<std::size_t, std::size_t> happened_events_counter_;

        int  mouse_x_{0};
        int  mouse_y_{0};
        int  mouse_delta_y_{0};
        int  mouse_delta_x_{0};
        bool mouse_moved_{false};
    };
};     // namespace oe
#endif // OE_EVENT_HANDLER_H
