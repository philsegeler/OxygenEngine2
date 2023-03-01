#ifndef OE_MUTEXCONDITION_H
#define OE_MUTEXCONDITION_H

#include <OE/types/base_types.h>

#ifdef __EMSCRIPTEN__
namespace oe {
    struct bool_condition_wrapper_emscripten : public OE_THREAD_SAFETY_OBJECT {
        bool value_{false};

        void operator=(bool a_val);
        bool operator!();
    };
}; // namespace oe
#else
extern "C" {
struct SDL_cond;
}
#endif
/* This class abstracts away the SDL part of locking mutexes
 * and conditions
 */
namespace oe {
    class mutex_condition_t : public OE_THREAD_SAFETY_OBJECT {
    public:
        mutex_condition_t();
        ~mutex_condition_t();
        void create_condition();

        void         cond_broadcast(std::size_t);
        void         cond_wait(std::size_t);
        void         pause(int);
        unsigned int getTicks();
        // list so that it does not get invalidated by adding new conditions
#ifdef __EMSCRIPTEN__
        std::vector<bool_condition_wrapper_emscripten> conditions_;
#else
        std::vector<SDL_cond*> conditions_;
#endif
    };
}; // namespace oe
#endif
