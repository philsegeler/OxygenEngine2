#ifndef OE_EVENT_H
#define OE_EVENT_H

#include <OE/Events/event_parser.h>
#include <OE/task.h>
#include <atomic>
#include <functional>
#include <memory>
#include <set>
#include <unordered_set>

/** Temporary documentation: (OUTDATED)
 *
 * event_t::call(OE_Task*):
 * Return values:
 *  0: successfully run
 *  1: event is executed in another thread
 *  2++ or -1--: papares gia tin ora (add your own if necessary)
 * ------------------------------------
 * Insert custom internal event type here
 * This is needed to be able to do the correct casts afterwards
 * Andreas' TODO:
 * -Collision Events
 * -Network Events
 * -Anything else his subsystems need
 * Philsegeler's TODO:
 * -Anything else his subsystems need
 */
namespace oe {

    enum event_type {
        CUSTOM_EVENT     = 0,
        KEYBOARD_EVENT   = 1,
        MOUSE_EVENT      = 2,
        GAMEPAD_EVENT    = 3,
        NETWORK_EVENT    = 4,
        COLLISION_EVENT  = 5,
        MOUSE_MOVE_EVENT = 6,
        EVENT_COMBO      = 7
    };

    int template_event_func(OE_Task, std::string);

    typedef std::function<int(OE_Task, std::string)> event_func_type;

    /* general event type */
    class event_t : public OE_THREAD_SAFETY_OBJECT {
        friend class event_handler_t;
        friend class input_event_handler_t;

    public:
        // static bool finished;
        event_t();
        virtual ~event_t();
        virtual int call() = 0;

        static std::atomic<std::size_t> current_id;
        std::size_t                     id;

    protected:
        // internal_call() is implemented in OE_Error.cpp
        int internal_call();

        void set_func(const event_func_type);

        bool        active_{false};
        std::string name_;

        event_type      type_;
        event_func_type func_;

        OE_Task task_;

        bool                            has_init_{false};
        std::unordered_set<std::size_t> sub_events_;
    };

    /*button event used in keyboard/mouse/gamepad*/


    enum button_type { BUTTON_RELEASE = 0, BUTTON_PRESS = 2, BUTTON_JUST_PRESS = 1, BUTTON_JUST_RELEASE = 3 };

    /// class intended to store keyboard events (3 for each )
    class keyboard_event_t : public event_t {
        friend class event_handler_t;
        friend class input_event_handler_t;

    public:
        keyboard_event_t();
        ~keyboard_event_t();
        int call();

    protected:
        uint8_t     keystate_;
        std::string key_;
    };

    /// class intended to store mouse events (3 for each mouse buttons 1-5)
    class mouse_event_t : public event_t {
        friend class event_handler_t;
        friend class input_event_handler_t;

    public:
        mouse_event_t();
        ~mouse_event_t();
        int call();

    protected:
        uint8_t keystate_;

        std::string key_;
    };

    /// class intended to store mouse move events
    class mouse_move_event_t : public event_t {
        friend class event_handler_t;
        friend class input_event_handler_t;

    public:
        mouse_move_event_t();
        ~mouse_move_event_t();
        int call();
    };

    /// class intended to store gamepad events (3 for each mouse buttons 1-5 and mouse position)
    class gamepad_event_t : public event_t {
        friend class event_handler_t;
        friend class input_event_handler_t;

    public:
        gamepad_event_t();
        ~gamepad_event_t();
        int call();

    protected:
        uint8_t     keystate_;
        std::string key_;

        int  axis_;
        bool axismoved_;
        int  x_, y_, delta_x_, delta_y_;
    };

    /// class intended for user events
    class custom_event_t : public event_t {
        friend class event_handler_t;
        friend class input_event_handler_t;

    public:
        custom_event_t();
        ~custom_event_t();
        int call();

    protected:
    };

};     // namespace oe
#endif // OE_EVENT_H
