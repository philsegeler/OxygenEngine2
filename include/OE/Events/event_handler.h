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
        std::shared_ptr<oe::event_t> getIEvent(const std::string&);

        void createUserEvent(const std::string&);
        void setIEventFunc(std::size_t, const oe::event_func_type);
        void setIEventFunc(const std::string&, const oe::event_func_type);
        void broadcastIEvent(std::size_t);
        void broadcastIEvent(const std::string&);

        void broadcastIEventWait(std::size_t, int); // TODO
        void mapIEvent(std::size_t, std::size_t);
        void unmapIEvent(std::size_t, std::size_t);
        int  callIEvent(std::size_t);
        void destroyIEvent(std::size_t);

        std::size_t get_event_id(const std::string&);
        std::size_t getEventActivations(std::size_t);
        std::size_t getEventCounter(std::size_t);

        void updateInput();
        void cleanup();
        int  handleAllEvents();

        // The methods starting with internal* are only supposed to be used
        // in subclasses of OE_WindowSystemBase
        // the key strings must exist

        void internal_update_mouse_status(int x, int y, int delta_x, int delta_y);
        void internalBroadcastKeyDownEvent(const std::string&);
        void internalBroadcastKeyUpEvent(const std::string&);

        std::atomic<bool> done;

        input_event_handler_t input_handler;

        int  get_mouse_x();
        int  get_mouse_y();
        int  get_mouse_delta_y();
        int  get_mouse_delta_x();
        bool has_mouse_moved();

    protected:
        bool havePendingEvents();

        oe::event_container_t<oe::event_t> events_list_;
        // std::map<std::string, std::shared_ptr<oe::event_t>> internal_events;
        // std::vector<std::string>                            obsolete_events;
        // std::vector<std::string>                            pending_events;

        std::vector<std::size_t>                     happened_events_;
        std::unordered_map<std::size_t, std::size_t> happened_events_counter_;

        uint8_t index = -1;

    private:
        int  mouse_x_{0};
        int  mouse_y_{0};
        int  mouse_delta_y_{0};
        int  mouse_delta_x_{0};
        bool mouse_moved_{false};
    };
};     // namespace oe
#endif // OE_EVENT_HANDLER_H
