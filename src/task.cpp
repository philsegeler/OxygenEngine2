#include <OE/task.h>
#include <OE/types/libs_oe.h>

using namespace std;

std::atomic<std::size_t> oe::task_element_t::current_id(0);

oe::task_t::task_t() {
}

oe::task_t::~task_t() {
}

oe::task_t::task_t(string name, int priority, int delay, int ticksa) {
    this->counter_  = 0;
    this->priority_ = priority;
    this->delay_    = delay;

    this->name_        = name;
    this->delta_ticks_ = ticksa;
    this->init_ticks_  = ticksa;
    this->ticks_       = 0;
}

oe::task_t::task_t(string name, int countera, int priority, int init_ticks, int ticks) {
    this->counter_  = countera;
    this->priority_ = priority;
    this->delay_    = 0;

    this->name_       = name;
    this->ticks_      = ticks;
    this->init_ticks_ = init_ticks;
}

bool oe::task_t::operator>(const oe::task_t& other) const {
    return std::tie(this->priority_, this->id) > std::tie(other.priority_, other.id);
}

bool oe::task_t::operator==(const oe::task_t& other) const {
    bool output = true;
    output      = output && this->name_ == other.name_;
    output      = output && this->counter_ == other.counter_;
    return output;
}


void oe::task_t::update() {
    this->counter_ += 1;
    auto buffer        = SDL_GetTicks();
    this->ticks_       = buffer - this->delta_ticks_;
    this->delta_ticks_ = buffer;
}

int oe::task_t::get_counter() const {
    return this->counter_;
}
int oe::task_t::get_priority() const {
    return this->priority_;
}

int oe::task_t::get_time() const {
    return SDL_GetTicks() - this->init_ticks_;
}

int oe::task_t::get_elapsed_time() const {
    return this->ticks_;
}

string oe::task_t::get_name() const {
    return this->name_;
}
