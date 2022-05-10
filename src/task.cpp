#include <OE/task.h>
#include <OE/types/libs_oe.h>

using namespace std;

std::atomic<std::size_t> oe::task_t::current_id(0);


oe::task_info_t::task_info_t(int priority, int delay, int ticksa) {
    this->counter_  = 0;
    this->priority_ = priority;
    this->delay_    = delay;

    this->delta_ticks_ = ticksa;
    this->init_ticks_  = ticksa;
    this->ticks_       = 0;
}

void oe::task_info_t::set_type_task(task_type arg) {
    this->type_ = arg;
}

oe::task_t::task_t() : id(++oe::task_t::current_id) {
}

void oe::task_t::set_func(oe::method_type func) {
    func_ = func;
}

oe::task_action oe::task_t::execute(std::size_t ida) {
    thread_id_ = ida;
    return func_(this->get_info());
}

oe::task_t::task_t(int priority, int delay, int ticksa)
    : oe::task_info_t(priority, delay, ticksa), id(++oe::task_t::current_id) {
}

oe::task_t::task_t(int countera, int priority, int init_ticks, int ticks) : id(++oe::task_t::current_id) {
    this->counter_  = countera;
    this->priority_ = priority;
    this->delay_    = 0;

    this->ticks_      = ticks;
    this->init_ticks_ = init_ticks;
}

bool oe::task_t::operator>(const oe::task_t& other) const {
    return std::tie(this->priority_, this->id) > std::tie(other.priority_, other.id);
}

oe::task_info_t oe::task_t::get_info() {
    oe::task_info_t output;
    output.type_        = type_;
    output.counter_     = counter_;
    output.priority_    = priority_;
    output.delay_       = delay_;
    output.delta_ticks_ = delta_ticks_;
    output.init_ticks_  = init_ticks_;
    output.ticks_       = ticks_;
    if (is_event_task_set_)
        output.event_or_task_id_ = event_or_task_id_;
    else
        output.event_or_task_id_ = this->id;
    output.thread_id_ = thread_id_;
    return output;
}

void oe::task_info_t::set_event_or_task_id(std::size_t inputa) {
    event_or_task_id_  = inputa;
    is_event_task_set_ = true;
}

void oe::task_info_t::set_thread_id(std::size_t thread_id) {
    thread_id_ = thread_id;
}

void oe::task_info_t::update() {
    this->counter_ += 1;
    auto buffer        = SDL_GetTicks();
    this->ticks_       = buffer - this->delta_ticks_;
    this->delta_ticks_ = buffer;
}

int oe::task_info_t::get_counter() const {
    return this->counter_;
}
int oe::task_info_t::get_priority() const {
    return this->priority_;
}

int oe::task_info_t::get_time() const {
    return SDL_GetTicks() - this->init_ticks_;
}

std::size_t oe::task_info_t::get_thread_id() const {
    return thread_id_;
}

int oe::task_info_t::get_elapsed_time() const {
    return this->ticks_;
}

oe::task_type oe::task_info_t::get_type_task() const {
    return this->type_;
}
std::size_t oe::task_info_t::get_id() const {
    return event_or_task_id_;
}
