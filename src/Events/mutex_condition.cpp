#include <OE/Events/mutex_condition.h>
#include <OE/types/libs_oe.h>

#ifdef __EMSCRIPTEN__
void oe::bool_condition_wrapper_emscripten::operator=(bool a_val) {
    lockMutex();
    value_ = a_val;
    unlockMutex();
}

bool oe::bool_condition_wrapper_emscripten::operator!() {
    lockMutex();
    auto result = value_;
    unlockMutex();
    return !result;
}
#endif

oe::mutex_condition_t::mutex_condition_t() {
}

oe::mutex_condition_t::~mutex_condition_t() {
    lockMutex();
#ifndef __EMSCRIPTEN__
    for (auto x : this->conditions_)
        SDL_DestroyCond(x);
#else
    this->conditions_.clear();
#endif
    unlockMutex();
}

void oe::mutex_condition_t::create_condition() {
    lockMutex();
#ifdef __EMSCRIPTEN__

    this->conditions_.push_back(bool_condition_wrapper_emscripten());
#else
    SDL_cond* condition = SDL_CreateCond();
    this->conditions_.push_back(condition);
#endif

    unlockMutex();
}

void oe::mutex_condition_t::cond_wait(size_t id) {
    if (this->conditions_.size() > id) {
#ifdef __EMSCRIPTEN__
        this->conditions_[id] = false;
        unlockMutex();

        while (!this->conditions_[id])
            this->pause(1);

        lockMutex();
#else
        SDL_CondWait(this->conditions_[id], this->wmutex);
#endif
    }
}

void oe::mutex_condition_t::cond_broadcast(size_t id) {
    if (this->conditions_.size() > id) {
#ifdef __EMSCRIPTEN__
        this->conditions_[id] = true;
#else
        SDL_CondBroadcast(this->conditions_[id]);
#endif
    }
}

void oe::mutex_condition_t::pause(int delay) {
    SDL_Delay(delay);
}

unsigned int oe::mutex_condition_t::getTicks() {
    return SDL_GetTicks();
}
