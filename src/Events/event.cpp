#include <OE/Events/event.h>
#include <OE/math_oe.h>
#include <OE/types/libs_oe.h>

using namespace std;

oe::task_action oe::template_event_func(const task_info_t task) {
    // cout << oe::get_event_name(event_id) << endl;
    return oe::task_action::keep;
}

std::atomic<size_t> oe::event_t::current_id(0);

oe::event_t::event_t() : id(++oe::event_t::current_id) {
}

oe::event_t::~event_t() {
}

void oe::event_t::set_func(const oe::method_type a_func) {
    lockMutex();
    func_   = a_func;
    active_ = true;
    unlockMutex();
}
oe::task_action oe::event_t::call() {

    return internal_call();
}

oe::task_action oe::event_t::internal_call() {
    /***************************/
    /// generic handling

    if (!this->has_init_) {

        this->task_ = task_info_t(0, 0, SDL_GetTicks());
        this->task_.set_type_task(task_type::EVENT);
        this->task_.set_event_or_task_id(this->id);
        this->has_init_ = true;
    }

    task_.update();
    // only call the function if the event has been given a function other than the default
    if (active_) return func_(task_);
    return oe::task_action::keep;
    /**************************/
}


// keyboard
oe::keyboard_event_t::keyboard_event_t() : oe::event_t() {
    type_     = oe::KEYBOARD_EVENT;
    keystate_ = oe::BUTTON_RELEASE;
}
oe::keyboard_event_t::~keyboard_event_t() {
}

oe::task_action oe::keyboard_event_t::call() {

    return internal_call();
}

// mouse
oe::mouse_event_t::mouse_event_t() : oe::event_t() {
    type_     = oe::MOUSE_EVENT;
    keystate_ = oe::BUTTON_RELEASE;
}
oe::mouse_event_t::~mouse_event_t() {
}

oe::task_action oe::mouse_event_t::call() {

    return internal_call();
}

// mouse move
oe::mouse_move_event_t::mouse_move_event_t() : oe::event_t() {
    type_ = oe::MOUSE_MOVE_EVENT;
}
oe::mouse_move_event_t::~mouse_move_event_t() {
}

oe::task_action oe::mouse_move_event_t::call() {

    return internal_call();
}

// gamepad
oe::gamepad_event_t::gamepad_event_t() : oe::event_t() {
    type_      = oe::GAMEPAD_EVENT;
    axis_      = 0;
    axismoved_ = false;
}
oe::gamepad_event_t::~gamepad_event_t() {
}

oe::task_action oe::gamepad_event_t::call() {

    return internal_call();
}

// custom
oe::custom_event_t::custom_event_t() : oe::event_t() {
    type_ = oe::CUSTOM_EVENT;
}
oe::custom_event_t::~custom_event_t() {
}

// physics
oe::physics_event_t::physics_event_t() : oe::event_t() {
    type_ = oe::PHYSICS_EVENT;
}
oe::physics_event_t::~physics_event_t() {
}

// network
oe::network_event_t::network_event_t() : oe::event_t() {
    type_ = oe::NETWORK_EVENT;
}
oe::network_event_t::~network_event_t() {
}
