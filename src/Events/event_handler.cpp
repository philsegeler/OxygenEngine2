#include <OE/Events/event_handler.h>
#include <OE/api_oe.h>
#include <iostream>

using namespace std;

oe::event_handler_t::event_handler_t() {
    done_ = false;
}
void oe::event_handler_t::init() {
    auto internal_events = this->input_handler_.create_events();
    for (auto x : internal_events) {
        this->events_list_.append(x.first, x.second);
        this->happened_events_counter_[events_list_[x.first].get_id()] = 0;
    }
    this->done_ = false;
}

oe::event_handler_t::~event_handler_t() {
    this->happened_events_counter_.clear();
}

// THIS IS VERY USEFUL
std::shared_ptr<oe::event_t> oe::event_handler_t::get_event(const string& event_name) {
    /// Updated this function to 2022
    lockMutex();

    auto event_elem = events_list_[event_name];
    if (!event_elem.is_valid()) {
        unlockMutex();
        throw oe::invalid_event(event_name);
    }
    unlockMutex();
    return event_elem.get_pointer();
}

std::string oe::event_handler_t::get_event_name(std::size_t id) {
    std::string output;
    lockMutex();
    output = events_list_.get_name(id);
    unlockMutex();
    return output;
};

std::size_t oe::event_handler_t::get_event_id(const string& event_name) {
    lockMutex();

    std::size_t event_id = events_list_.get_id(event_name);
    if (event_id == 0) {
        unlockMutex();
        throw oe::invalid_event(event_name);
    }
    unlockMutex();
    return event_id;
}


std::size_t oe::event_handler_t::create_user_event(const string& event_name) {

    std::shared_ptr<oe::custom_event_t> event    = std::make_shared<oe::custom_event_t>();
    std::size_t                         event_id = event->id;


    std::unordered_set<std::string> check_prefixes = {"physics-", "network-", "keyboard-", "mouse-", "gamepad-", "renderer-"};

    for (auto prefix : check_prefixes) {
        if (event_name.starts_with(prefix)) throw oe::invalid_event_name(event_name, prefix);
    }
    lockMutex();
    if (not events_list_.contains(event_name)) {
        events_list_.append(event_name, event);
        happened_events_counter_[event->id] = 0;
    }
    unlockMutex();
    return event_id;
}

std::size_t oe::event_handler_t::create_physics_event(const string& event_name) {

    std::shared_ptr<oe::physics_event_t> event    = std::make_shared<oe::physics_event_t>();
    std::size_t                          event_id = event->id;

    if (not event_name.starts_with("physics-")) {
        throw oe::invalid_physics_event_name(event_name, event_name);
    }

    lockMutex();
    if (not events_list_.contains(event_name)) {
        events_list_.append(event_name, event);
        happened_events_counter_[event->id] = 0;
    }
    unlockMutex();
    return event_id;
}

std::size_t oe::event_handler_t::create_network_event(const string& event_name) {

    std::shared_ptr<oe::network_event_t> event    = std::make_shared<oe::network_event_t>();
    std::size_t                          event_id = event->id;

    if (not event_name.starts_with("network-")) {
        throw oe::invalid_network_event_name(event_name, event_name);
    }

    lockMutex();
    if (not events_list_.contains(event_name)) {
        events_list_.append(event_name, event);
        happened_events_counter_[event->id] = 0;
    }
    unlockMutex();
    return event_id;
}

std::size_t oe::event_handler_t::create_renderer_event(const string& event_name) {

    std::shared_ptr<oe::renderer_event_t> event    = std::make_shared<oe::renderer_event_t>();
    std::size_t                           event_id = event->id;

    if (not event_name.starts_with("renderer-")) {
        throw oe::invalid_renderer_event_name(event_name, event_name);
    }

    lockMutex();
    if (not events_list_.contains(event_name)) {
        events_list_.append(event_name, event);
        happened_events_counter_[event->id] = 0;
    }
    unlockMutex();
    return event_id;
}

void oe::event_handler_t::destroy_event(size_t event_id) {
    lockMutex();

    if (this->events_list_.contains(event_id)) {
        events_list_.remove(events_list_[event_id].get_id());
    }
    else {
        // TODO: Warning
    }

    unlockMutex();
    // return output;
}
void oe::event_handler_t::set_event_func(size_t event_id, const oe::method_type func) {
    lockMutex();
    auto event = events_list_[event_id];
    if (event.is_valid()) {
        event->set_func(func);
    }
    else {
        // TODO: Warning
    }

    unlockMutex();
}
void oe::event_handler_t::set_event_func(const string& event_name, const oe::method_type func) {
    lockMutex();
    auto event = events_list_[event_name];
    if (event.is_valid()) {
        event->set_func(func);
    }
    else {
        unlockMutex();
        throw oe::invalid_event(event_name);
    }

    unlockMutex();
}

void oe::event_handler_t::map_event(size_t upper, size_t target) {
    lockMutex();
    if (this->events_list_.contains(target)) {
        this->events_list_[target]->sub_events_.insert(upper);
    }
    else {
        // TODO: Warning
    }

    unlockMutex();
}

void oe::event_handler_t::unmap_event(size_t upper, size_t target) {
    lockMutex();
    auto event = events_list_[target];
    if (event.is_valid()) {
        event->sub_events_.erase(upper);
    }
    else {
        // TODO: Warning
    }

    unlockMutex();
}
/// so simple
void oe::event_handler_t::broadcast_event(size_t event_id) {

    unordered_set<size_t> tobecalled_events;
    lockMutex();

    auto event = events_list_[event_id];
    if (event.is_valid()) {
        event.flag_as_registered();
        tobecalled_events = event->sub_events_;
    }
    else {
        // TODO: Warning
    }

    unlockMutex();

    /// broadcast any sub events (which in turn broadcast their sub events etc.)
    for (auto& x : tobecalled_events)
        this->broadcast_event(x);
}

void oe::event_handler_t::broadcast_event(const string& event_name) {

    unordered_set<size_t> tobecalled_events;
    lockMutex();

    auto event = events_list_[event_name];
    if (event.is_valid()) {
        event.flag_as_registered();
        tobecalled_events = event->sub_events_;
    }
    else {
        unlockMutex();
        throw oe::invalid_event(event_name);
    }

    unlockMutex();

    /// broadcast any sub events (which in turn broadcast their sub events etc.)
    for (auto& x : tobecalled_events)
        this->broadcast_event(x);
}

// TODO
void oe::event_handler_t::broadcast_event_wait(size_t event_id, int milliseconds) {
}


std::size_t oe::event_handler_t::get_event_activations(std::size_t event_id) {
    size_t output = 0;
    lockMutex();
    if (this->events_list_.contains(event_id)) {
        output = this->happened_events_counter_[event_id];
    }
    else {
        // TODO: Warning
    }
    unlockMutex();
    return output;
}

std::size_t oe::event_handler_t::get_event_counter(std::size_t event_id) {
    size_t output = 0;
    lockMutex();
    auto event = events_list_[event_id];
    if (event.is_valid()) {
        event->lockMutex();
        output = event->task_.get_counter();
        event->unlockMutex();
    }
    unlockMutex();
    return output;
}


bool oe::event_handler_t::has_pending_events() {
    lockMutex();
    bool output = this->events_list_.has_registered_events();
    unlockMutex();
    return output;
}

/*IMPORTANT
 * this function is run in the main thread only in the 2020 version
 */
int oe::event_handler_t::handle_all_events() {

    lockMutex();
    for (auto x : this->happened_events_counter_) {
        this->happened_events_counter_[x.first] = 0;
    }
    unlockMutex();

    while (this->has_pending_events()) {

        /// fetch an event and delete it from the queue
        ///  so that other threads target next events
        lockMutex();

        const std::vector<std::size_t> to_be_called_events = events_list_.registered();
        this->events_list_.reset_registered();

        for (std::size_t event_id : to_be_called_events) {
            this->happened_events_counter_[event_id]++;
        }

        for (std::size_t event_id : to_be_called_events) {
            call_event(event_id);
        }


        unlockMutex();
    }

    this->cleanup();
    return 0;
}
bool oe::event_handler_t::done() {
    return done_;
}
void oe::event_handler_t::set_done(bool param) {
    done_ = param;
}

void oe::event_handler_t::cleanup() {

    lockMutex();

    for (std::size_t event_id : this->events_list_.deleted()) {
        happened_events_counter_.erase(event_id);
    }
    this->events_list_.synchronize(false);

    unlockMutex();
}

int oe::event_handler_t::get_mouse_x() {
    return mouse_x_;
}
int oe::event_handler_t::get_mouse_y() {
    return mouse_y_;
}
int oe::event_handler_t::get_mouse_delta_y() {
    return mouse_delta_y_;
}
int oe::event_handler_t::get_mouse_delta_x() {
    return mouse_delta_x_;
}
bool oe::event_handler_t::has_mouse_moved() {
    return mouse_moved_;
}
