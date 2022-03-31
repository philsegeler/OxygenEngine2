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
        std::shared_ptr<oe::event_t> getIEvent(std::string);
        std::shared_ptr<oe::event_t> getIEventUNSAFE(std::string);

        void createUserEvent(std::string);
        void setIEventFunc(std::string, const oe::event_func_type);
        void broadcastIEvent(std::string);

        void broadcastIEventWait(std::string, int); // TODO
        void mapIEvent(std::string, std::string);
        void unmapIEvent(std::string, std::string);
        int  callIEvent(std::string);
        void destroyIEvent(std::string);

        std::size_t getEventActivations(std::string);
        std::size_t getEventCounter(std::string);

        void updateInput();
        void cleanup();
        int  handleAllEvents();


        void updatePostInputLoop();

        // The methods starting with internal* are only supposed to be usede
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

        std::map<std::string, std::shared_ptr<oe::event_t>> internal_events;
        std::vector<std::string>                            obsolete_events;
        std::vector<std::string>                            pending_events;

        std::vector<std::string>                     happened_events;
        std::unordered_map<std::string, std::size_t> happened_events_counter;

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
