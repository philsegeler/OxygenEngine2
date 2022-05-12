#ifndef OE_TASK_H
#define OE_TASK_H

#include <OE/error_oe.h>
#include <atomic>
#include <functional>

class OE_TaskManager;

namespace oe {

    enum class task_type { EVENT, REGULAR, UNSYNC, PHYSICS };

    enum class task_action { keep, discard };


    /// This class is used as a const argument to all functions passed to the engine
    class task_info_t {
        friend class ::OE_TaskManager;
        friend struct OE_ThreadStruct;
        friend class event_t;
        friend class task_t;

    public:
        task_info_t() = default;
        task_info_t(int, int, int);

        std::size_t get_counter() const;
        std::size_t get_priority() const;
        std::size_t get_time() const;
        std::size_t get_elapsed_time() const;
        std::size_t get_id() const;
        std::size_t get_thread_id() const;
        task_type   get_type_task() const;
        bool        is_event() const;
        bool        is_regular() const;
        bool        is_unsync() const;

    protected:
        void set_thread_id(std::size_t);
        void set_type_task(task_type);
        void reset_timer();
        void update();
        void set_event_or_task_id(std::size_t);

        task_type   type_{task_type::REGULAR};
        std::size_t counter_{0};
        std::size_t priority_{0};
        std::size_t delay_{0};
        std::size_t delta_ticks_{0};
        std::size_t init_ticks_{0};
        std::size_t ticks_{0};
        std::size_t event_or_task_id_{0};
        std::size_t thread_id_{0};

        bool is_event_task_set_{false};
    };

    /// This is the main function type that is accepted all over the engine for tasks/events etc.
    typedef std::function<task_action(const oe::task_info_t)> method_type;


    /// This class is used for storing regular tasks running on synchronized threads on a shared_index_map
    class task_t : public task_info_t {

        friend class ::OE_TaskManager;
        friend struct OE_ThreadStruct;
        friend class event_t;

    public:
        static std::atomic<std::size_t> current_id;
        const std::size_t               id;

        task_t();
        task_t(int, int, int);
        task_t(int, int, int, int);

        task_info_t get_info();

        void        set_func(method_type);
        task_action execute(std::size_t);

        bool operator>(const task_t&) const;

    private:
        method_type func_;
    };


}; // namespace oe


#endif
