#ifndef OE_TASK_H
#define OE_TASK_H

#include <OE/error_oe.h>
#include <atomic>
#include <functional>

class OE_TaskManager;

namespace oe {

    enum TASK_TYPE { EVENT_TASK, NORMAL_TASK, UNSYNC_TASK, PHYSICS_TASK, RENDERER_TASK, NETWORK_TASK };

    struct task_info {};

    class task_t {

        friend class ::OE_TaskManager;
        friend struct OE_ThreadStruct;
        friend class event_t;

    public:
        std::size_t id{0};

        task_t();
        task_t(std::string, int, int, int);
        task_t(std::string, int, int, int, int);
        virtual ~task_t();

        bool operator>(const task_t&) const;
        bool operator==(const task_t&) const;

        int         get_counter() const;
        int         get_priority() const;
        int         get_time() const;
        int         get_elapsed_time() const;
        std::string get_name() const;

    protected:
        void reset_timer();
        void update();

        TASK_TYPE   type_{NORMAL_TASK};
        int         counter_{0};
        int         priority_{0};
        int         delay_{0};
        long long   delta_ticks_{0};
        std::string name_;
        int         init_ticks_{0};
        int         ticks_{0};
        std::size_t id_{0};
    };

    class task_element_t {
    public:
        static std::atomic<std::size_t> current_id;
        const std::size_t               id;
    };

}; // namespace oe

/* this is a shortcut type definition
 * to make code clearer.
 * It stores an std::function that is executed as tasks in different threads
 */

typedef std::function<int(oe::task_t)> OE_METHOD;


#endif
