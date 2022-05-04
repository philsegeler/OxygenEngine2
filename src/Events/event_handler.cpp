#include <OE/Events/event_handler.h>
#include <iostream>

using namespace std;

int oe::template_event_func(OE_Task task, string event_name) {
    // cout << event_name << endl;
    return 0;
}

oe::event_handler_t::event_handler_t() {
    done_ = false;
}
void oe::event_handler_t::init() {
    auto internal_events = this->input_handler_.create_events();
    for (auto x : internal_events) {
        this->events_list_.append(x.first, x.second);
        this->happened_events_counter_[events_list_[x.first].id_] = 0;
    }
    this->done_ = false;
}

oe::event_handler_t::~event_handler_t() {
    this->happened_events_counter_.clear();
}

// THIS IS VERY USEFUL
std::shared_ptr<oe::event_t> oe::event_handler_t::get_ievent(const string& event_name) {
    /// Updated this function to 2022
    lockMutex();

    auto event_elem = events_list_[event_name];
    if (!event_elem.is_valid()) {
        unlockMutex();
        throw oe::invalid_event(event_name);
    }
    unlockMutex();
    return event_elem.p_;
}

std::size_t oe::event_handler_t::get_event_id(const string& event_name) {
    lockMutex();

    auto event_elem = events_list_[event_name];
    if (!event_elem.is_valid()) {
        unlockMutex();
        throw oe::invalid_event(event_name);
    }
    unlockMutex();
    return event_elem.id_;
}


std::size_t oe::event_handler_t::create_user_event(const string& event_name) {

    std::shared_ptr<oe::custom_event_t> event = std::make_shared<oe::custom_event_t>();
    event->name_                              = event_name;
    event->set_func(&template_event_func);
    std::size_t event_id = event->id;

    lockMutex();
    if (events_list_.count(event_name) == 0) {
        events_list_.append(event_name, event);
        happened_events_counter_[events_list_[event_name].id_] = 0;
    }
    unlockMutex();
    return event_id;
}



void oe::event_handler_t::destroy_ievent(size_t event_id) {
    lockMutex();

    if (this->events_list_.count(event_id) != 0) {
        events_list_.remove(events_list_[event_id].id_);
    }
    else {
        // TODO: Warning
    }

    unlockMutex();
    // return output;
}
void oe::event_handler_t::set_ievent_func(size_t event_id, const oe::event_func_type func) {
    lockMutex();
    auto event = events_list_[event_id];
    if (event.is_valid()) {
        event.p_->set_func(func);
    }
    else {
        // TODO: Warning
    }

    unlockMutex();
}
void oe::event_handler_t::set_ievent_func(const string& event_name, const oe::event_func_type func) {
    lockMutex();
    auto event = events_list_[event_name];
    if (event.is_valid()) {
        event.p_->set_func(func);
    }
    else {
        unlockMutex();
        throw oe::invalid_event(event_name);
    }

    unlockMutex();
}

void oe::event_handler_t::map_ievent(size_t upper, size_t target) {
    lockMutex();
    if (this->events_list_.count(target) != 0) {
        this->events_list_[target].p_->sub_events_.insert(upper);
    }
    else {
        // TODO: Warning
    }

    unlockMutex();
}

void oe::event_handler_t::unmap_ievent(size_t upper, size_t target) {
    lockMutex();
    auto event = events_list_[target];
    if (event.is_valid()) {
        event.p_->sub_events_.erase(upper);
    }
    else {
        // TODO: Warning
    }

    unlockMutex();
}
/// so simple
void oe::event_handler_t::broadcast_ievent(size_t event_id) {

    unordered_set<size_t> tobecalled_events;
    lockMutex();

    auto event = events_list_[event_id];
    if (event.is_valid()) {
        event.flag_as_registered();
        tobecalled_events = event.p_->sub_events_;
    }
    else {
        // TODO: Warning
    }

    unlockMutex();

    /// broadcast any sub events (which in turn broadcast their sub events etc.)
    for (auto& x : tobecalled_events)
        this->broadcast_ievent(x);
}

void oe::event_handler_t::broadcast_ievent(const string& event_name) {

    unordered_set<size_t> tobecalled_events;
    lockMutex();

    auto event = events_list_[event_name];
    if (event.is_valid()) {
        event.flag_as_registered();
        tobecalled_events = event.p_->sub_events_;
    }
    else {
        unlockMutex();
        throw oe::invalid_event(event_name);
    }

    unlockMutex();

    /// broadcast any sub events (which in turn broadcast their sub events etc.)
    for (auto& x : tobecalled_events)
        this->broadcast_ievent(x);
}

// TODO
void oe::event_handler_t::broadcast_ievent_wait(size_t event_id, int milliseconds) {
}

/// so simple
int oe::event_handler_t::call_ievent(size_t event_id) {

    /// generic event management
    auto event = events_list_[event_id];
    if (event.is_valid()) {

        event.p_->lockMutex();
        event.p_->call();
        event.p_->unlockMutex();
    }
    else {
        // TODO: Warning
    }

    return 0;
}

std::size_t oe::event_handler_t::get_event_activations(std::size_t event_id) {
    size_t output = 0;
    lockMutex();
    if (this->events_list_.count(event_id) == 1) {
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
    auto   event  = events_list_[event_id];
    if (event.is_valid()) {
        event.p_->lockMutex();
        output = event.p_->task_.GetCounter();
        event.p_->unlockMutex();
    }
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


        this->happened_events_.clear();
        for (auto event_elem : this->events_list_.registered()) {
            this->happened_events_counter_[event_elem.id_]++;
            happened_events_.push_back(event_elem.id_);
        }
        this->events_list_.reset_registered();
        unlockMutex();

        for (auto a_event : happened_events_)
            call_ievent(a_event);
    }

    this->cleanup();
    return 0;
}

void oe::event_handler_t::cleanup() {

    lockMutex();

    for (auto event_elem : this->events_list_.deleted()) {
        happened_events_counter_.erase(event_elem.id_);
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
