#ifndef OE_TASKMANAGER_H
#define OE_TASKMANAGER_H

#include <OE/Carbon/interpreter.h>
#include <OE/Events/event_handler.h>
#include <OE/Events/mutex_condition.h>
#include <OE/dummy_classes.h>
#include <OE/task.h>
#include <OE/types/world.h>
#include <queue>


#ifdef __EMSCRIPTEN__
// these variables controls when the other threads start in a web environment
// because webassembly/javascript do not allow threads to be created on the fly
extern std::atomic<bool> oe_threads_ready_to_start;
#endif

class OE_TaskManager;

struct OE_ThreadData {
    /* This struct is passed to the update_thread function
     * It passes the task manager pointer and the thread name it runs on
     * so it can run methods from classes inherited from OE_TaskManager class
     * without any fuss in another thread
     */
    static OE_TaskManager* taskMgr;
    std::string            name;
};

struct OE_UnsyncThreadData {
    OE_METHOD       func;
    OE_TaskManager* taskMgr;
    std::string     name;
};

struct OE_ThreadStruct {
    /* this struct is used to store a thread. More specifically it stores:
     * - the tasks to execute which are represented as an oe::task_t
     * - the pointer methods to execute which represent an OE_METHOD
     * - a boolean to make the thread asynchronous (relic from 2016, not usable)
     */

    OE_ThreadStruct();
    virtual ~OE_ThreadStruct();


    std::set<std::string>            task_names;
    std::vector<oe::task_t>          tasks;
    std::map<std::string, OE_METHOD> functions;
    // std::vector<unsigned int>   task_queue;

    // for new tasks to be run after the next frame
    std::vector<oe::task_t>          pending_tasks;
    std::map<std::string, OE_METHOD> pending_functions;

    // for tasks to be removed
    std::queue<std::string> to_be_removed;

    void updateTaskList();

    std::string name;
    bool        synchronize, changed;
    bool        flushed          = false;
    bool        rendering_thread = false;

    int countar = 0;

    oe::task_t physics_task;
};



extern "C" int oxygen_engine_update_thread(void* data);
extern "C" int oxygen_engine_update_unsync_thread(void* data);

extern "C" {
struct SDL_Thread;
}

class OE_TaskManager : public OE_MutexCondition {

    friend int oxygen_engine_update_unsync_thread(void*);
    friend int oxygen_engine_update_thread(void*);

public:
    OE_TaskManager();
    ~OE_TaskManager();

    OE_THREAD_SAFETY_OBJECT  renderer_mutex;
    oe::renderer_base_t*     renderer{nullptr};
    oe::renderer_init_info   renderer_init_info;
    oe::renderer_update_info renderer_info;

    OE_THREAD_SAFETY_OBJECT physics_mutex;
    oe::physics_base_t*     physics{nullptr};
    oe::physics_update_info physics_info;
    oe::physics_init_info   physics_init_info;

    OE_THREAD_SAFETY_OBJECT window_mutex;
    oe::winsys_base_t*      window{nullptr};
    oe::winsys_update_info  window_info;
    oe::winsys_init_info    window_init_info;
    oe::winsys_output       window_output;

    oe::networking_base_t*   network{nullptr};
    oe::networking_init_info network_init_info;

    unsigned int GetFrameRate();
    void         SetFrameRate(unsigned int);
    /**
     * adds a function to the to-do list of the task manager
     * the first 2 arguments are mandatory and they are:
     * - the name given to the task to be accessed with
     * - the function to be added which should return an integer and take an oe::task_t as an argument
     * the rest 2 are optional and they are
     * - priority (int) higher number => lower priority
     * - the name of the thread to be run, if omitted uses the default thread
     */
    void AddTask(std::string, const OE_METHOD);
    void AddTask(std::string, const OE_METHOD, int);
    void AddTask(std::string, const OE_METHOD, int, std::string);
    void AddTask(std::string, const OE_METHOD, std::string);
    // similar to do-task, but only executes the function once after certain time has passed
    void       DoOnce(std::string, const OE_METHOD, int);
    oe::task_t GetTaskInfo(std::string, std::string);


    // Main functions
    int  Init(std::string, int, int, bool, oe::renderer_init_info, oe::winsys_init_info, oe::physics_init_info,
              oe::networking_init_info);
    void CreateNewThread(std::string);
    void CreateUnsyncThread(std::string, const OE_METHOD);
    void Step();
    void Start();
    void Destroy();


    // removes a task from the task list of a thread
    void RemoveTask(std::string);
    void RemoveTask(std::string, std::string);

    void updateThread(const std::string);

    bool is_done();

    void                      set_pending_world(std::shared_ptr<OE_World>);
    std::shared_ptr<OE_World> get_world();

    void force_restart_renderer();

private:
    bool              renderer_init_errors{false};
    std::atomic<bool> restart_renderer{false};
    bool              physics_init_errors{false};
    bool              winsys_init_errors{false};
    bool              network_init_errors{false};

    std::shared_ptr<OE_World> world{nullptr};

    std::shared_ptr<OE_World> pending_world{nullptr}; // for loading a world

    // stores threads by their name
    std::map<std::string, struct OE_ThreadStruct> threads   = {};
    std::map<std::string, SDL_Thread*>            threadIDs = {};

    // stores threads which should not be synchronized
    // std::map<std::string, struct OE_ThreadStruct>  unsync_threads = {};
    std::map<std::string, SDL_Thread*> unsync_threadIDs          = {};
    std::set<std::string>              finished_unsync_threadIDs = {};

    std::map<std::string, std::string> active_tasks = {};

    // very important variable
    std::atomic<bool> done_;
    bool              errors_on_init{false};

    void removeFinishedUnsyncThreads();

    int          getReadyThreads();
    unsigned int framerate;
    unsigned int current_framerate;

    int completed_threads;
    int started_threads{0};
    int physics_threads{0};

    int countar{0};

    void syncBeginFrame();
    void syncEndFrame();

    void updateWorld();
    void runThreadTasks(const std::string&);
    void sortThreadTasks(const std::string&);

    // error handling functions
    // those are implemented in OE_Error.cpp
    // in order to have all non-core-engine error handling at one place
    int tryRun_unsync_thread(OE_UnsyncThreadData*);
    int tryRun_task(const std::string&, oe::task_t&);

    // ALL these return true if error is found so it terminates the engine
    bool tryRun_physics_updateMultiThread(const std::string&, const int&);
    bool tryRun_renderer_updateSingleThread();
    bool tryRun_renderer_updateData();
    bool tryRun_winsys_update();

    bool tryRun_winsys_init(int, int, std::string, bool, oe::winsys_init_info);
    bool tryRun_physics_init(oe::physics_init_info);
    bool tryRun_renderer_init(oe::renderer_init_info);
    bool tryRun_network_init(oe::networking_init_info);
};

#endif
