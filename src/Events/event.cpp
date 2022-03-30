#include <OE/Carbon/interpreter.h>
#include <OE/Events/event.h>
#include <OE/math_oe.h>

using namespace std;

// bool oe::event_t::finished = false;

oe::event_t::event_t() {
    active_ = false;
    name_   = "";
}

oe::event_t::~event_t() {
}

void oe::event_t::set_func(const oe::event_func_type a_func) {
    lockMutex();
    func_ = a_func;
    unlockMutex();
}

// keyboard
oe::keyboard_event_t::keyboard_event_t() {
    type_     = oe::KEYBOARD_EVENT;
    keystate_ = oe::BUTTON_RELEASE;
}
oe::keyboard_event_t::~keyboard_event_t() {
}

int oe::keyboard_event_t::call() {

    return internal_call();
}

// mouse
oe::mouse_event_t::mouse_event_t() {
    type_     = oe::MOUSE_EVENT;
    keystate_ = oe::BUTTON_RELEASE;
}
oe::mouse_event_t::~mouse_event_t() {
}

int oe::mouse_event_t::call() {

    return internal_call();
}

// mouse move
oe::mouse_move_event_t::mouse_move_event_t() {
    type_ = oe::MOUSE_MOVE_EVENT;
}
oe::mouse_move_event_t::~mouse_move_event_t() {
}

int oe::mouse_move_event_t::call() {

    return internal_call();
}

// gamepad
oe::gamepad_event_t::gamepad_event_t() {
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
oe::custom_event_t::custom_event_t() {
    type_ = oe::CUSTOM_EVENT;
}
oe::custom_event_t::~custom_event_t() {
}

int oe::custom_event_t::call() {

    return internal_call();
}
