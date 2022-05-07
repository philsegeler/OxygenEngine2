#include <OE/Carbon/interpreter.h>
#include <OE/Events/event.h>
#include <OE/math_oe.h>
#include <OE/types/libs_oe.h>
using namespace std;

// bool oe::event_t::finished = false;
std::atomic<size_t> oe::event_t::current_id(0);

oe::event_t::event_t() : id(++oe::event_t::current_id) {
}

oe::event_t::~event_t() {
}

void oe::event_t::set_func(const oe::event_func_type a_func) {
    lockMutex();
    func_   = a_func;
    active_ = true;
    unlockMutex();
}

int oe::event_t::call() {

    return internal_call();
}

int oe::event_t::internal_call() {
    /***************************/
    /// generic handling

    if (!this->has_init_) {

        this->task_     = task_t("event" + to_string(this->id), 0, 0, SDL_GetTicks());
        this->has_init_ = true;
    }

    task_.update();
    // only call the function if the event has been given a function other than the default
    if (active_) func_(task_, id);
    return 0;
    /**************************/
}


// keyboard
oe::keyboard_event_t::keyboard_event_t() : oe::event_t() {
    type_     = oe::KEYBOARD_EVENT;
    keystate_ = oe::BUTTON_RELEASE;
}
oe::keyboard_event_t::~keyboard_event_t() {
}

int oe::keyboard_event_t::call() {

    return internal_call();
}

// mouse
oe::mouse_event_t::mouse_event_t() : oe::event_t() {
    type_     = oe::MOUSE_EVENT;
    keystate_ = oe::BUTTON_RELEASE;
}
oe::mouse_event_t::~mouse_event_t() {
}

int oe::mouse_event_t::call() {

    return internal_call();
}

// mouse move
oe::mouse_move_event_t::mouse_move_event_t() : oe::event_t() {
    type_ = oe::MOUSE_MOVE_EVENT;
}
oe::mouse_move_event_t::~mouse_move_event_t() {
}

int oe::mouse_move_event_t::call() {

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

int oe::gamepad_event_t::call() {

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
