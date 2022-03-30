#include <OE/Events/event_handler.h>
#include <iostream>

using namespace std;

int oe::template_event_func(OE_Task task, string event_name) { /*cout << event_name << endl;*/
    return 0;
}

oe::event_handler_t::event_handler_t() {
    done = false;
}
void oe::event_handler_t::init() {
    this->input_handler.createEvents(&this->internal_events);
    for (auto x : this->internal_events) {
        this->happened_events_counter[x.first] = 0;
    }
    this->done = false;
}

oe::event_handler_t::~event_handler_t() {
    this->internal_events.clear();
    this->happened_events_counter.clear();
}

// THIS IS VERY USEFUL
std::shared_ptr<oe::event_t> oe::event_handler_t::getIEvent(string a_name) {
    /// wraps getIEventUNSAFE in a mutex, (now in 2020: like... seriously????)
    lockMutex();

    std::shared_ptr<oe::event_t> output = getIEventUNSAFE(a_name);

    if (output == nullptr) {
        unlockMutex();
        throw oe::invalid_event(a_name);
    }

    unlockMutex();
    return output;
}

std::shared_ptr<oe::event_t> oe::event_handler_t::getIEventUNSAFE(string a_name) {
    /// function to simplify fetching an event by name
    /// WARNING: DO NOT USE THE FUNCTION OUTSIDE OF MUTEXES

    if (internal_events.count(a_name) != 0) return internal_events[a_name];

    return nullptr;
}

void oe::event_handler_t::createUserEvent(string a_name) {

    std::shared_ptr<oe::custom_event_t> event = std::make_shared<oe::custom_event_t>();
    event->name_                              = a_name;
    event->set_func(&template_event_func);

    lockMutex();
    if (internal_events.count(a_name) == 0) {
        internal_events[a_name]         = event;
        happened_events_counter[a_name] = 0;
    }
    unlockMutex();
}



void oe::event_handler_t::destroyIEvent(string a_name) {
    lockMutex();

    this->obsolete_events.push_back(a_name);

    unlockMutex();
    // return output;
}
void oe::event_handler_t::setIEventFunc(string a_name, const oe::event_func_type func) {
    lockMutex();
    if (getIEventUNSAFE(a_name) != nullptr) {
        getIEventUNSAFE(a_name)->set_func(func);
    }
    else {
        unlockMutex();
        throw oe::invalid_event(a_name);
    }

    unlockMutex();
}

void oe::event_handler_t::mapIEvent(string upper, string target) {
    lockMutex();
    if (getIEventUNSAFE(target) != nullptr) {
        getIEventUNSAFE(target)->sub_events_.insert(upper);
    }
    else {
        unlockMutex();
        throw oe::invalid_event(upper);
    }

    unlockMutex();
}

void oe::event_handler_t::unmapIEvent(string upper, string target) {
    lockMutex();
    if (getIEventUNSAFE(target) != nullptr) {
        getIEventUNSAFE(target)->sub_events_.erase(getIEventUNSAFE(target)->sub_events_.find(upper));
    }
    else {
        unlockMutex();
        throw oe::invalid_event(upper);
    }

    unlockMutex();
}
/// so simple
void oe::event_handler_t::broadcastIEvent(string a_name) {

    set<string> tobecalled_events;
    lockMutex();

    if (getIEventUNSAFE(a_name) != nullptr) {
        this->pending_events.push_back(a_name);
        tobecalled_events = this->getIEventUNSAFE(a_name)->sub_events_;
    }
    else {
        unlockMutex();
        throw oe::invalid_event(a_name);
    }

    unlockMutex();

    /// broadcast any sub events (which in turn broadcast their sub events etc.)
    for (auto& x : tobecalled_events)
        this->broadcastIEvent(x);
}

// TODO
void oe::event_handler_t::broadcastIEventWait(string a_name, int milliseconds) {
}

/// so simple
int oe::event_handler_t::callIEvent(string a_name) {

    /// generic event management
    auto event = getIEvent(a_name);
    if (event != nullptr) {

        event->lockMutex();
        event->call();
        event->unlockMutex();
    }
    else {
        cout << "OE Event is nullptr: '" + a_name + "'" << endl;
        OE_WriteToLog("OE Event is nullptr: '" + a_name + "'" + "\n");
    }

    return 0;
}

std::size_t oe::event_handler_t::getEventActivations(std::string a_name) {
    size_t output = 0;
    lockMutex();
    if (this->happened_events_counter.count(a_name) == 1) {
        output = this->happened_events_counter[a_name];
    }
    else {
        unlockMutex();
        throw oe::invalid_event(a_name);
    }
    unlockMutex();
    return output;
}

std::size_t oe::event_handler_t::getEventCounter(std::string a_name) {
    size_t output = 0;
    auto   event  = getIEvent(a_name);
    if (event != nullptr) {
        event->lockMutex();
        output = event->task_.GetCounter();
        event->unlockMutex();
    }
    return output;
}


bool oe::event_handler_t::havePendingEvents() {
    lockMutex();
    bool output = !this->pending_events.empty();
    unlockMutex();
    return output;
}

/*IMPORTANT
 * this function is run in the main thread only in the 2020 version
 */
int oe::event_handler_t::handleAllEvents() {

    lockMutex();
    for (auto x : this->happened_events_counter) {
        this->happened_events_counter[x.first] = 0;
    }
    unlockMutex();

    while (havePendingEvents()) {

        /// fetch an event and delete it from the queue
        ///  so that other threads target next events
        lockMutex();


        this->happened_events.clear();

        if (pending_events.empty()) {
            unlockMutex();
            return 0;
        }

        this->happened_events = std::exchange(this->pending_events, {});
        for (const auto& x : this->happened_events) {
            this->happened_events_counter[x]++;
        }
        unlockMutex();

        for (auto a_event : happened_events)
            callIEvent(a_event);
    }
    return 0;
}

void oe::event_handler_t::cleanup() {

    lockMutex();

    for (auto& a_name : obsolete_events) {
        if (getIEventUNSAFE(a_name) != nullptr) {
            internal_events.erase(a_name);
            happened_events_counter.erase(a_name);
        }
    }
    obsolete_events.clear();

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
