#ifndef OE_EVENT_HANDLER_H
#define OE_EVENT_HANDLER_H

#include <OE/Events/event_container.h>
#include <OE/Events/input_event_handler.h>

namespace oe {
    class event_handler_t : public OE_THREAD_SAFETY_OBJECT {
    public:
        friend class OE_TaskManager;

        event_handler_t();
        ~event_handler_t();

        // internal event functions
        void                         init();
        std::shared_ptr<oe::event_t> get_ievent(const std::string&);

        std::size_t create_user_event(const std::string&);
        void        set_ievent_func(std::size_t, const oe::event_func_type);
        void        set_ievent_func(const std::string&, const oe::event_func_type);
        void        broadcast_ievent(std::size_t);
        void        broadcast_ievent(const std::string&);

        void broadcast_ievent_wait(std::size_t, int); // TODO
        void map_ievent(std::size_t, std::size_t);
        void unmap_ievent(std::size_t, std::size_t);
        int  call_ievent(std::size_t);
        void destroy_ievent(std::size_t);

        std::size_t get_event_id(const std::string&);
        std::string get_event_name(std::size_t);
        std::size_t get_event_activations(std::size_t);
        std::size_t get_event_counter(std::size_t);

        void update_input();
        void cleanup();
        int  handle_all_events();

        // The methods starting with internal* are only supposed to be used
        // in subclasses of OE_WindowSystemBase
        // the key strings must exist

        void internal_update_mouse_status(int x, int y, int delta_x, int delta_y);
        void internal_register_keydown_event(const std::string&);
        void internal_register_keyup_event(const std::string&);

        std::atomic<bool> done_;

        input_event_handler_t input_handler_;

        int  get_mouse_x();
        int  get_mouse_y();
        int  get_mouse_delta_y();
        int  get_mouse_delta_x();
        bool has_mouse_moved();

    protected:
        bool has_pending_events();

        oe::event_container_t<oe::event_t> events_list_;

        std::vector<std::size_t>                     happened_events_;
        std::unordered_map<std::size_t, std::size_t> happened_events_counter_;

    private:
        int  mouse_x_{0};
        int  mouse_y_{0};
        int  mouse_delta_y_{0};
        int  mouse_delta_x_{0};
        bool mouse_moved_{false};
    };
};     // namespace oe
#endif // OE_EVENT_HANDLER_H
