#ifndef OE_TASK_H
#define OE_TASK_H

#include <OE/error_oe.h>
#include <atomic>
#include <functional>

class OE_TaskManager;

namespace oe {

    enum task_type { EVENT, REGULAR, UNSYNC, PHYSICS, RENDERER, NETWORK };

    class task_info_t {
        friend class ::OE_TaskManager;
        friend struct OE_ThreadStruct;
        friend class event_t;
        friend class task_t;

    public:
        task_info_t() = default;
        task_info_t(int, int, int);

        int         get_counter() const;
        int         get_priority() const;
        int         get_time() const;
        int         get_elapsed_time() const;
        std::size_t get_id() const;
        task_type   get_type_task() const;

    protected:
        void set_type_task(task_type);
        void reset_timer();
        void update();
        void set_event_or_task_id(std::size_t);

        task_type   type_{task_type::REGULAR};
        int         counter_{0};
        int         priority_{0};
        int         delay_{0};
        long long   delta_ticks_{0};
        int         init_ticks_{0};
        int         ticks_{0};
        std::size_t event_or_task_id_{0};
    };

    class task_t : public task_info_t {

        friend class ::OE_TaskManager;
        friend struct OE_ThreadStruct;
        friend class event_t;

    public:
        static std::atomic<std::size_t> current_id;
        std::size_t                     id;

        task_t();
        task_t(std::string, int, int, int);
        task_t(std::string, int, int, int, int);

        task_info_t get_info();

        bool operator>(const task_t&) const;
        bool operator==(const task_t&) const;

        std::string get_name() const;

    private:
        std::string name_;
    };


}; // namespace oe

/* this is a shortcut type definition
 * to make code clearer.
 * It stores an std::function that is executed as tasks in different threads
 */

typedef std::function<int(const oe::task_info_t)> OE_METHOD;


#endif
